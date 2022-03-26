#pragma once


#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/use_awaitable.hpp>


class Scheduler
{
public:
    Scheduler(boost::asio::io_context& ioContext) : _ioContext{ioContext} {}

    auto schedule() {
        auto initiate = [this]<typename Handler>(Handler&& handler) mutable
        {
            boost::asio::post(_ioContext, [handler = std::forward<Handler>(handler)]() mutable
            {
                handler();
            });
        };

        return boost::asio::async_initiate<
                decltype(boost::asio::use_awaitable), void()>(
                    initiate, boost::asio::use_awaitable);
    }

private:
    boost::asio::io_context& _ioContext;
};
