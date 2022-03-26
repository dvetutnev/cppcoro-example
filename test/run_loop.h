#pragma once


#include <cppcoro/task.hpp>
#include <uvw/loop.h>


inline cppcoro::task<> run_loop(uvw::Loop& loop) {
    loop.run();
    co_return;
}
