#pragma once

#include <uvw/loop.h>
#include <uvw/tcp.h>


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

    std::shared_ptr<uvw::TCPHandle> _handle;
};
