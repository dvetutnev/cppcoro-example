add_executable(tests
    test_dummy.cpp)

target_link_libraries(tests
    cppcoro::cppcoro
    uvw::uvw
    date::date
    GTest::gmock_main)

add_test(NAME tests COMMAND tests)

