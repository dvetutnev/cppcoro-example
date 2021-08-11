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
