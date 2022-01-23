#include <gmock/gmock.h>
#include <boost/asio.hpp>


#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif


using namespace std::chrono_literals;


boost::asio::awaitable<void> timer(std::chrono::milliseconds duration) {
    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::system_timer timer{executor};
    timer.expires_after(duration);

    co_await timer.async_wait(boost::asio::use_awaitable);
};



TEST(asio_deadline_timer, _) {/*
    auto timer = [](std::chrono::milliseconds duration) -> boost::asio::awaitable<void> {
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::system_timer timer{executor};
        timer.expires_after(duration);

        co_await timer.async_wait(boost::asio::use_awaitable);
    };
*/
    boost::asio::io_context ioContext;

    auto start = std::chrono::system_clock::now();

    boost::asio::co_spawn(ioContext, timer(150ms), boost::asio::detached);
    ioContext.run();

    auto duration = std::chrono::system_clock::now() - start;

    EXPECT_TRUE(duration >= 100ms);
    EXPECT_TRUE(duration <= 200ms);
}

