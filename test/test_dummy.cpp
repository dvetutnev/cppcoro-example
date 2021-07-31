#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>

#include <gtest/gtest.h>


#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif


cppcoro::task<int> create_coro(int i) {
    co_return i * 2;
}

TEST(Dummy, _) {
    auto coro = create_coro(21);
    int result = cppcoro::sync_wait(coro);
    EXPECT_EQ(result, 42);
}
