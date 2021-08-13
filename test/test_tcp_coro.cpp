#include "run_loop.h"
#include "tcp_coro.h"

#include <gtest/gtest.h>

#include <cppcoro/when_all.hpp>
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


TEST(TCPCoro, shutdown) {
    auto loop = uvw::Loop::create();
    auto listener = loop->resource<uvw::TCPHandle>();

    bool isShutdowned = false;
    auto onConnect = [&](const auto&, uvw::TCPHandle& listener) {
        auto client = listener.loop().resource<uvw::TCPHandle>();

        client->once<uvw::EndEvent>([&](const auto&, uvw::TCPHandle& client) {
            isShutdowned = true;
            client.close();
        });

        listener.accept(*client);
        client->read();

        listener.close();
    };
    listener->once<uvw::ListenEvent>(onConnect);

    listener->bind("127.0.0.1", 8791);
    listener->listen();


    TCPCoro socket{*loop};
    auto task = [&]() -> cppcoro::task<> {
        co_await socket.connect("127.0.0.1", 8791);
        co_await socket.shutdown();
    };


    cppcoro::sync_wait(cppcoro::when_all_ready(
                           task(),
                           run_loop(*loop)));

    ASSERT_TRUE(isShutdowned);
}


TEST(TCPCoro, shutdownError) {
    auto loop = uvw::Loop::create();
    auto listener = loop->resource<uvw::TCPHandle>();

    TCPCoro socket{*loop};
    bool isThrown = false;
    auto task = [&]() -> cppcoro::task<> {
        try {
            co_await socket.shutdown();
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


TEST(TCPCoro, read) {
    auto loop = uvw::Loop::create();
    auto listener = loop->resource<uvw::TCPHandle>();

    auto onConnect = [&](const auto&, uvw::TCPHandle& listener) {
        auto client = listener.loop().resource<uvw::TCPHandle>();

        client->once<uvw::WriteEvent>([&](const auto&, uvw::TCPHandle& client) {
            client.close();
        });

        listener.accept(*client);

        auto data = std::unique_ptr<char[]>{ new char[5] {'d', 'a', 't', 'a', 0} };
        client->write(std::move(data), 5);

        listener.close();
    };
    listener->once<uvw::ListenEvent>(onConnect);

    listener->bind("127.0.0.1", 8791);
    listener->listen();


    TCPCoro socket{*loop};
    auto task = [&]() -> cppcoro::task<std::tuple<std::unique_ptr<char[]>, unsigned int>> {
        co_await socket.connect("127.0.0.1", 8791);
        std::tuple<std::unique_ptr<char[]>, unsigned int> result = co_await socket.read();
        co_return result;
    };


    auto [result, _] = cppcoro::sync_wait(cppcoro::when_all(
                           task(),
                           run_loop(*loop)));

    auto& [data, length] = result;

    ASSERT_TRUE(data);
    ASSERT_STREQ(data.get(), "data");
    ASSERT_EQ(length, 5);
}
