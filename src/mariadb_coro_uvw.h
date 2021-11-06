#pragma once


#include "table_result.h"

#include <string_view>

#include <cppcoro/task.hpp>
#include <uvw/loop.h>


namespace uvw {


class MariaDBCoro
{
public:
    MariaDBCoro(uvw::Loop& loop,
                std::string_view host,
                std::string_view user,
                std::string_view password,
                std::string_view dbName);

    cppcoro::task<TableResult> query(std::string_view);

private:
    uvw::Loop& _loop;

    const std::string _host;
    const std::string _user;
    const std::string _password;
    const std::string _dbName;
};


}
