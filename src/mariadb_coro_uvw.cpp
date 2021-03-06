#include "mariadb_coro_uvw.h"

#include <mysql.h>
#include <mysqld_error.h>

#include <uvw/poll.h>


namespace uvw {


namespace {


struct Awaiter
{
    uvw::Loop& _loop;
    MYSQL& _mysql;
    int _status;

    constexpr bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> coro) {
        auto nativeHandle = ::mysql_get_socket(&_mysql);
        auto poll = _loop.resource<uvw::PollHandle>(nativeHandle);

        poll->once<uvw::PollEvent>([coro](const auto&, uvw::PollHandle& poll) {
            poll.stop();
            poll.close();

            coro.resume();
        });

        assert(_status != 0);
        using Flag = uvw::Flags<uvw::PollHandle::Event>;
        auto flags =
                (_status & MYSQL_WAIT_READ   ? Flag{uvw::PollHandle::Event::READABLE}    : Flag{}) |
                (_status & MYSQL_WAIT_WRITE  ? Flag{uvw::PollHandle::Event::WRITABLE}    : Flag{}) |
                (_status & MYSQL_WAIT_EXCEPT ? Flag{uvw::PollHandle::Event::PRIORITIZED} : Flag{});

        poll->start(flags);
    }

    constexpr void await_resume() const noexcept {}
};


} // Anonymous namespace


cppcoro::task<TableResult> MariaDBCoro::query(std::string_view stmp) {
    int err, status;
    MYSQL mysql, *ret;

    ::mysql_init(&mysql);
    ::mysql_optionsv(&mysql, MYSQL_OPT_NONBLOCK, 0);

    status = ::mysql_real_connect_start(&ret, &mysql, _host.c_str(), _user.c_str(), _password.c_str(), _dbName.c_str(), 0, nullptr, 0);
    while (status) {
        co_await Awaiter{_loop, mysql, status};
        status = ::mysql_real_connect_cont(&ret, &mysql, status);
    }
    if (!ret) {
        throw std::runtime_error{::mysql_error(&mysql)};
    }

    status = ::mysql_real_query_start(&err, &mysql, stmp.data(), stmp.size());
    while (status) {
        co_await Awaiter{_loop, mysql, status};
        status = ::mysql_real_query_cont(&err, &mysql, status);
    }
    if (err) {
        throw std::runtime_error{::mysql_error(&mysql)};
    }

    // This method cannot block.
    MYSQL_RES* mysqlResult = ::mysql_use_result(&mysql);
    if (!mysqlResult) {
        throw std::runtime_error{::mysql_error(&mysql)};
    }

    TableResult result;
    unsigned int numFields = ::mysql_num_fields(mysqlResult);

    for (;;) {
        MYSQL_ROW mysqlRow;

        status = ::mysql_fetch_row_start(&mysqlRow, mysqlResult);
        while (status) {
            co_await Awaiter{_loop, mysql, status};
            status = ::mysql_fetch_row_cont(&mysqlRow, mysqlResult, status);
        }

        if (!mysqlRow) {
            break;
        }

        std::vector<std::string> resultRow;
        for (unsigned int i = 0; i < numFields; i++) {
            const char* field = mysqlRow[i];
            if (field) {
                resultRow.emplace_back(field);
            }
            else {
                resultRow.emplace_back("");
            }
        }

        result.rows.push_back(std::move(resultRow));
        result.rowsAffected++;
    }

    if (::mysql_errno(&mysql)) {
        throw std::runtime_error{::mysql_error(&mysql)};
    }

    ::mysql_free_result(mysqlResult);
    ::mysql_close(&mysql);

    co_return result;
}


} // namespace uvw
