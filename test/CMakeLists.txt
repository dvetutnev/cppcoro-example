add_executable(tests
    test_dummy.cpp)

target_link_libraries(tests
    GTest::gmock_main)

add_test(NAME tests COMMAND tests)

