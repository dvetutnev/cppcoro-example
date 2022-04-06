add_executable(tests
    test_dummy.cpp
    test_mariadb_coro_uvw.cpp
    test_mariadb_coro_asio.cpp
    test_tcp_coro.cpp
    test_timer_coro_asio.cpp
    test_schedule.cpp)

target_link_libraries(tests
    ${PROJECT_NAME}
    GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(tests)
