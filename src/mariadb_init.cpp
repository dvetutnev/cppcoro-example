#include "mariadb_init.h"

#include "mysql.h"

#include <mutex>



namespace {

static std::once_flag mariadbInitFlag;

}


void mariadbInit() {
    auto init = []() {
        if (::mysql_library_init(1, nullptr, nullptr)) {
            throw std::runtime_error{"Fatal: mysql_library_init() returns error"};
        }

        std::atexit([]() { ::mysql_library_end(); });
    };

    std::call_once(mariadbInitFlag, init);
}
