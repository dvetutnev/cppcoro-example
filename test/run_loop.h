#pragma once


#include <cppcoro/task.hpp>
#include <uvw/loop.h>


#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif


inline cppcoro::task<> run_loop(std::shared_ptr<uvw::Loop> loop) {
    loop->run();
    co_return;
}
