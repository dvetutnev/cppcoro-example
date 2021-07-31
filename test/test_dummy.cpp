#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/when_all_ready.hpp>

#include <uvw.hpp>
#include <gtest/gtest.h>


#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif


using namespace std::chrono_literals;


cppcoro::task<int> create_coro(int i) {
    co_return i * 2;
}

TEST(Dummy, _) {
    auto coro = create_coro(21);
    int result = cppcoro::sync_wait(coro);
    EXPECT_EQ(result, 42);
}


struct Awaiter
{
    std::chrono::milliseconds delay;
    std::shared_ptr<uvw::Loop> loop = uvw::Loop::getDefault();

    constexpr bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        auto timer = loop->resource<uvw::TimerHandle>();
        timer->once<uvw::TimerEvent>([handle](const auto&, const auto&) {
            handle.resume();
        });
        timer->start(delay, 0ms);
    }

    constexpr void await_resume() const noexcept {}
};

cppcoro::task<> timer(std::chrono::milliseconds delay) {
    co_await Awaiter{delay};
    co_return;
}

cppcoro::task<> run_loop() {
    uvw::Loop::getDefault()->run();
    co_return;
}

TEST(Timer, _) {
    auto start = std::chrono::system_clock::now();

    cppcoro::sync_wait(cppcoro::when_all_ready(
                           timer(150ms),
                           run_loop()));

    auto duration = std::chrono::system_clock::now() - start;

    EXPECT_TRUE(duration >= 100ms);
    EXPECT_TRUE(duration <= 200ms);
}
