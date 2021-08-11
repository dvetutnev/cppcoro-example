#pragma once

#include <uvw/loop.h>
#include <uvw/tcp.h>

#include <cppcoro/coroutine.hpp>


class TCPCoro
{
public:

    TCPCoro(uvw::Loop&);
    ~TCPCoro();

    class AwaiterConnect;
    AwaiterConnect connect(std::string_view ip, unsigned int port);

    class AwaiterRead;
    AwaiterRead read(); // std::tuple<std::unique_ptr<char>, unsigned int> data = co_wait socket.read();

    class AwaiterWrite;
    AwaiterWrite write(std::unique_ptr<char> data, unsigned int length);

    class AwaiterShutdown;
    AwaiterShutdown shutdown();

private:

    std::shared_ptr<uvw::TCPHandle> _tcpHandle;
};


class TCPCoro::AwaiterConnect
{
public:

    AwaiterConnect(uvw::TCPHandle& tcpHandle, std::string_view ip, unsigned int port);

    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<>);
    constexpr void await_resume() const noexcept {}

private:

    uvw::TCPHandle& _tcpHandle;
    const std::string _ip;
    const unsigned int _port;
};
