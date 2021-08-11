#pragma once

#include <uvw/loop.h>
#include <uvw/tcp.h>

#include <cppcoro/coroutine.hpp>


class TCPCoroException : public std::exception
{
public:
    TCPCoroException(const uvw::ErrorEvent& errorEvent) : _errorEvent{errorEvent} {}
    const char* what() const noexcept override { return _errorEvent.what(); }

private:
    const uvw::ErrorEvent _errorEvent;
};


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
    void await_resume() const;

private:

    uvw::TCPHandle& _tcpHandle;
    const std::string _ip;
    const unsigned int _port;

    std::exception_ptr _exception;
};


class TCPCoro::AwaiterShutdown
{
public:

    AwaiterShutdown(uvw::TCPHandle& tcpHandle);

    constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<>);
    void await_resume() const;

private:

    uvw::TCPHandle& _tcpHandle;

    std::exception_ptr _exception;
};
