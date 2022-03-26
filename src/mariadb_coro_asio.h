#pragma once


#include "table_result.h"
#include "mariadb_init.h"

#include <string_view>

#include <boost/asio/io_context.hpp>
#include <boost/asio/awaitable.hpp>


namespace asio {


class MariaDBCoro
{
public:
    MariaDBCoro(boost::asio::io_context& ioContext,
                std::string_view host,
                std::string_view user,
                std::string_view password,
                std::string_view dbName);

    boost::asio::awaitable< boost::asio::awaitable<TableResult>> query(std::string_view query) {
        co_return queryImpl(query);
    }

private:
    boost::asio::io_context& _ioContext;

    const std::string _host;
    const std::string _user;
    const std::string _password;
    const std::string _dbName;

    boost::asio::awaitable<TableResult> queryImpl(std::string_view query);
};


inline MariaDBCoro::MariaDBCoro(boost::asio::io_context& ioContext,
                         std::string_view host,
                         std::string_view user,
                         std::string_view password,
                         std::string_view dbName)
    :
      _ioContext{ioContext},

      _host{host},
      _user{user},
      _password{password},
      _dbName{dbName}
{
    mariadbInit();
}


} // namspace asio
