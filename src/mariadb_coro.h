#pragma once


#include <string>
#include <string_view>
#include <vector>
#include <mutex>

#include <cppcoro/task.hpp>
#include <uvw/loop.h>


struct TableResult {
    /// Number of affected rows.
    std::uint64_t rowsAffected = 0;

    /// Returned rows.
    std::vector<std::vector<std::string>> rows{ };
};


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

    static std::once_flag _mysqlLibInitFlag;
};
