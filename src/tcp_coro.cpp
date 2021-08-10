#include "tcp_coro.h"


TCPCoro::TCPCoro(uvw::Loop& loop)
    :
      _handle{loop.resource<uvw::TCPHandle>()}
{}


TCPCoro::~TCPCoro() {
    _handle->clear(); // Remove event listeners
    _handle->close();
}
