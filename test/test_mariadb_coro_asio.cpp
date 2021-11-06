#include "mariadb_coro_asio.h"

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all.hpp>

#include <gtest/gtest.h>


#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif


namespace {


inline cppcoro::task<> run_loop(boost::asio::io_context& ioContext) {
    ioContext.run();
    co_return;
}


} // Anonymous namespace


TEST(MariaDBCoro, SHOW_STATUS) {
    boost::asio::io_context ioContext;

    asio::MariaDBCoro client{ ioContext, "127.0.0.1", "root", "password", "" };

    auto task = [&client]() -> cppcoro::task<TableResult> {
        TableResult result = co_await client.query("SHOW STATUS;");
        co_return result;
    };
    auto [result, _] = cppcoro::sync_wait(cppcoro::when_all(
            task(),
            run_loop(ioContext)));


    EXPECT_GE(result.rows.size(), 1);
    EXPECT_GE(result.rowsAffected, 1);

    for (const std::vector<std::string>& row : result.rows) {
        for (const std::string& col : row) {
            std::cout << col << ' ';
        }
        std::cout << std::endl;
    }
}

