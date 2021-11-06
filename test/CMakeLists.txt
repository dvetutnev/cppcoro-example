add_executable(tests
    test_dummy.cpp
    test_mariadb_coro_uvw.cpp
    test_tcp_coro.cpp)

target_link_libraries(tests
    ${PROJECT_NAME}
    GTest::gmock_main)

add_test(NAME tests COMMAND tests)

