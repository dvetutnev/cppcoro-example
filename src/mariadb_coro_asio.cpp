#include "mariadb_coro_asio.h"

#include <mysql.h>
#include <mysqld_error.h>

#include <boost/asio.hpp>


#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif


namespace asio {


namespace {


using tcp = boost::asio::ip::tcp;


auto waitMode = [](int status) -> tcp::socket::wait_type {
    if (status & MYSQL_WAIT_READ) {
        return tcp::socket::wait_read;
    }
    else if (status & MYSQL_WAIT_WRITE) {
        return tcp::socket::wait_write;
    }
    else {
        return tcp::socket::wait_error;
    }
};


boost::asio::awaitable<void> poll(boost::asio::io_context& ioContext, MYSQL& mysql, int status) {
    auto nativeHandle = ::mysql_get_socket(&mysql);
    tcp::socket socket{ioContext, tcp::v4(), nativeHandle};

    co_await socket.async_wait(waitMode(status), boost::asio::use_awaitable);

    socket.release();

    co_return;
}


} // Anonymous namespace


boost::asio::awaitable<TableResult> MariaDBCoro::queryImpl(std::string_view stmp) {
    int err, status;
    MYSQL mysql, *ret;

    ::mysql_init(&mysql);
    ::mysql_optionsv(&mysql, MYSQL_OPT_NONBLOCK, 0);

    status = ::mysql_real_connect_start(&ret, &mysql, _host.c_str(), _user.c_str(), _password.c_str(), _dbName.c_str(), 0, nullptr, 0);
    while (status) {
        co_await poll(_ioContext, mysql, status);
        status = ::mysql_real_connect_cont(&ret, &mysql, status);
    }
    if (!ret) {
        throw std::runtime_error{::mysql_error(&mysql)};
    }

    status = ::mysql_real_query_start(&err, &mysql, stmp.data(), stmp.size());
    while (status) {
        co_await poll(_ioContext, mysql, status);
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
            co_await poll(_ioContext, mysql, status);
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
