#include "scheduler_asio.h"

#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/detached.hpp>

#include <gtest/gtest.h>


TEST(BoostASIO, Scheduler) {
    boost::asio::io_context ioContext;
    Scheduler scheduler{ioContext};

    bool reachedPointA = false;
    bool reachedPointB = false;

    auto process = [&]() -> boost::asio::awaitable<void> {
            reachedPointA = true;
            co_await scheduler.schedule();
            reachedPointB = true;
            co_return;
    };

    auto check = [&]() -> boost::asio::awaitable<void> {
            EXPECT_TRUE(reachedPointA);
            EXPECT_FALSE(reachedPointB);
            co_return;
    };

    auto task = [&]() -> boost::asio::awaitable<void> {
            using namespace boost::asio::experimental::awaitable_operators;

            co_await(process() && check());
            co_return;
    };

    boost::asio::co_spawn(ioContext, task(), boost::asio::detached);

    ioContext.run();

    EXPECT_TRUE(reachedPointA);
    EXPECT_TRUE(reachedPointB);
}
