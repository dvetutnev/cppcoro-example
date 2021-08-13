#include "tcp_coro.h"


TCPCoro::TCPCoro(uvw::Loop& loop)
    :
      _tcpHandle{loop.resource<uvw::TCPHandle>()}
{}


TCPCoro::~TCPCoro() {
    _tcpHandle->clear(); // Remove event listeners
    _tcpHandle->close();
}


TCPCoro::AwaiterConnect TCPCoro::connect(std::string_view ip, unsigned int port) {
    return AwaiterConnect{*_tcpHandle, ip, port};
}


TCPCoro::AwaiterShutdown TCPCoro::shutdown() {
    return AwaiterShutdown{*_tcpHandle};
}


TCPCoro::AwaiterRead TCPCoro::read() {
    return AwaiterRead{*_tcpHandle};
}



TCPCoro::AwaiterConnect::AwaiterConnect(uvw::TCPHandle& tcpHandle, std::string_view ip, unsigned int port)
    :
      _tcpHandle{tcpHandle},
      _ip{ip},
      _port{port}
{}


void TCPCoro::AwaiterConnect::await_suspend(std::coroutine_handle<> coro) {
    _tcpHandle.once<uvw::ConnectEvent>([coro](const auto&, const auto&) {
        coro.resume();
    });

    _tcpHandle.once<uvw::ErrorEvent>([this, coro](const uvw::ErrorEvent& ev, const auto&) {
        _exception = std::make_exception_ptr(TCPCoroException{ev});
        coro.resume();
    });

    _tcpHandle.connect(_ip, _port);
}


void TCPCoro::AwaiterConnect::await_resume() const {
    if (_exception) {
        std::rethrow_exception(_exception);
    }
}



TCPCoro::AwaiterShutdown::AwaiterShutdown(uvw::TCPHandle& tcpHandle)
    :
      _tcpHandle{tcpHandle}
{}


void TCPCoro::AwaiterShutdown::await_suspend(std::coroutine_handle<> coro) {
    _tcpHandle.once<uvw::ShutdownEvent>([coro](const auto&, const auto&) {
        coro.resume();
    });

    _tcpHandle.once<uvw::ErrorEvent>([this, coro](const uvw::ErrorEvent& ev, const auto&) {
        _exception = std::make_exception_ptr(TCPCoroException{ev});
        coro.resume();
    });

    _tcpHandle.shutdown();
}


void TCPCoro::AwaiterShutdown::await_resume() const {
    if (_exception) {
        std::rethrow_exception(_exception);
    }
}



TCPCoro::AwaiterRead::AwaiterRead(uvw::TCPHandle& tcpHandle)
    :
      _tcpHandle{tcpHandle},

      _event{nullptr, 0}
{}


void TCPCoro::AwaiterRead::await_suspend(std::coroutine_handle<> coro) {
    _tcpHandle.once<uvw::DataEvent>([this, coro](uvw::DataEvent& event, const auto&) {
        _event = std::move(event);
        _tcpHandle.stop();

        coro.resume();
    });

    _tcpHandle.once<uvw::ErrorEvent>([this, coro](const uvw::ErrorEvent& ev, const auto&) {
        _exception = std::make_exception_ptr(TCPCoroException{ev});
        coro.resume();
    });

    _tcpHandle.read();
}


std::tuple<std::unique_ptr<char[]>, std::size_t> TCPCoro::AwaiterRead::await_resume() {
    if (_exception) {
        std::rethrow_exception(_exception);
    }

    return std::make_tuple(std::move(_event.data), _event.length);
}

