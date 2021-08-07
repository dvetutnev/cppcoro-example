#include "mariadb_coro.h"
#include "run_loop.h"

#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all.hpp>

#include <gtest/gtest.h>



TEST(MariaDBCoro, SHOW_STATUS) {
    auto loop = uvw::Loop::create();

    MariaDBCoro client{ *loop, "127.0.0.1", "root", "password", "cppcoro-example" };

    auto task = [&client]() -> cppcoro::task<TableResult> {
        TableResult result = co_await client.query("SHOW STATUS;");
        co_return result;
    };
    auto [result, _] = cppcoro::sync_wait(cppcoro::when_all(
            task(),
            run_loop(*loop)));

    ASSERT_GE(result.rowsAffected, 1);
}

