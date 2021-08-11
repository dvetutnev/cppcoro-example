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
    _tcpHandle.connect(_ip, _port);
}
