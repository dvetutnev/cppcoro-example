#include "mariadb_coro_uvw.h"
#include "run_loop.h"

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all.hpp>

#include <gtest/gtest.h>



TEST(MariaDBCoro, SHOW_STATUS) {
    auto loop = uvw::Loop::create();

    uvw::MariaDBCoro client{ *loop, "127.0.0.1", "root", "password", "" };

    auto task = [&client]() -> cppcoro::task<TableResult> {
        TableResult result = co_await client.query("SHOW STATUS;");
        co_return result;
    };
    auto [result, _] = cppcoro::sync_wait(cppcoro::when_all(
            task(),
            run_loop(*loop)));


    EXPECT_GE(result.rows.size(), 1);
    EXPECT_GE(result.rowsAffected, 1);

    for (const std::vector<std::string>& row : result.rows) {
        for (const std::string& col : row) {
            std::cout << col << ' ';
        }
        std::cout << std::endl;
    }
}

