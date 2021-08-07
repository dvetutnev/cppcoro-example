#include "run_loop.h"

#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all_ready.hpp>

#include <uvw.hpp>
#include <gtest/gtest.h>

using namespace std::chrono_literals;


cppcoro::task<int> create_coro(int i) {
    co_return i * 2;
}

TEST(Dummy, _) {
    auto coro = create_coro(21);
    int result = cppcoro::sync_wait(coro);
    EXPECT_EQ(result, 42);
}


TEST(Dummy, _2) {
    auto coro = create_coro(21);
    auto coro2 = create_coro(22);

    auto all = cppcoro::when_all(std::move(coro), std::move(coro2));
    auto [a, b]  = cppcoro::sync_wait(all);

    EXPECT_EQ(a, 42);
    EXPECT_EQ(b, 44);
}


namespace {


struct Awaiter
{
    std::chrono::milliseconds delay;
    std::shared_ptr<uvw::Loop> loop = uvw::Loop::getDefault();

    constexpr bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        std::shared_ptr<uvw::TimerHandle> timer = loop->resource<uvw::TimerHandle>();

        timer->once<uvw::TimerEvent>([handle](uvw::TimerEvent& event, uvw::TimerHandle& timer) {
            handle.resume();
            timer.close();
        });

        timer->start(delay, 0ms);
    }

    constexpr void await_resume() const noexcept {}
};

cppcoro::task<> timer(std::chrono::milliseconds delay) {
    co_await Awaiter{delay};
    co_return;
}


} // Anonymous namespace


TEST(Timer, _) {
    auto start = std::chrono::system_clock::now();

    cppcoro::sync_wait(cppcoro::when_all_ready(
                           timer(150ms),
                           run_loop()));

    auto duration = std::chrono::system_clock::now() - start;

    EXPECT_TRUE(duration >= 100ms);
    EXPECT_TRUE(duration <= 200ms);
}
