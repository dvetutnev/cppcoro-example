#include "mariadb_coro.h"
#include <mysql.h>


#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif


cppcoro::task<TableResult> MariaDBCoro::query(std::string_view stmp) {
    co_return TableResult{};
}
