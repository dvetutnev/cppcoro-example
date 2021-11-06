#pragma once


#include <vector>
#include <string>


struct TableResult {
    /// Number of affected rows.
    std::uint64_t rowsAffected = 0;

    /// Returned rows.
    std::vector<std::vector<std::string>> rows{ };
};
