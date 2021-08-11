#include "run_loop.h"
#include "tcp_coro.h"

#include <gtest/gtest.h>
#include <cppcoro/when_all_ready.hpp>
#include <cppcoro/sync_wait.hpp>


TEST(TCPCoro, connect) {
    auto loop = uvw::Loop::create();
    auto listener = loop->resource<uvw::TCPHandle>();

    bool isConnected = false;
    auto onConnect = [&](const auto&, uvw::TCPHandle& handle) {
        isConnected = true;
        handle.close();
    };
    listener->once<uvw::ListenEvent>(onConnect);

    listener->bind("127.0.0.1", 8791);
    listener->listen();


    TCPCoro socket{*loop};
    auto task = [&]() -> cppcoro::task<> {
        co_await socket.connect("127.0.0.1", 8791);
    };


    cppcoro::sync_wait(cppcoro::when_all_ready(
                           task(),
                           run_loop(*loop)));

    ASSERT_TRUE(isConnected);
}


TEST(TCPCoro, connectError) {
    auto loop = uvw::Loop::create();
    auto listener = loop->resource<uvw::TCPHandle>();

    TCPCoro socket{*loop};
    bool isThrown = false;
    auto task = [&]() -> cppcoro::task<> {
        try {
            co_await socket.connect("127.0.0.1", 8791);
        }
        catch (const TCPCoroException& ex) {
            isThrown = true;
            std::cout << ex.what() << std::endl;
        }
    };


    cppcoro::sync_wait(cppcoro::when_all_ready(
                           task(),
                           run_loop(*loop)));

    ASSERT_TRUE(isThrown);
}
