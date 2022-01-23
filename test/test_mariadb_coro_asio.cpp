#include "mariadb_coro_asio.h"

#include <boost/asio/co_spawn.hpp>
#include <gtest/gtest.h>


#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif


TEST(asio_MariaDBCoro, SHOW_STATUS) {
    boost::asio::io_context ioContext;

    asio::MariaDBCoro client{ ioContext, "127.0.0.1", "root", "password", "" };

    auto task = [&client]() -> boost::asio::awaitable<TableResult> {
        // Send query, wait until buffer overflow
        boost::asio::awaitable<TableResult> query = co_await client.query("SHOW STATUS;");

        // Wait result of sent query
        TableResult result = co_await std::move(query); // Need R-value
        co_return result;
    };

    auto handler = [](std::exception_ptr ex, TableResult result) {
        if (ex) {
            std::rethrow_exception(ex);
        }

        EXPECT_GE(result.rows.size(), 1);
        EXPECT_GE(result.rowsAffected, 1);

        for (const std::vector<std::string>& row : result.rows) {
            for (const std::string& col : row) {
                std::cout << col << ' ';
            }
            std::cout << std::endl;
        }
    };
    boost::asio::co_spawn(ioContext, task(), handler);

    ioContext.run();
}

