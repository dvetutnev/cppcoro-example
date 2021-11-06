# Training example for cppcoro https://github.com/andreasbuhr/cppcoro
# Dmitriy Vetutnev 2021

cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

include(detect_project_name)
detect_project_name(project_name)
message(STATUS "Project name: ${project_name}")
project(${project_name} CXX)

include(conan)

conan_cmake_autodetect(settings)
conan_cmake_install(
    PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}/conanfile.txt
    BUILD missing
    SETTINGS ${settings})


add_compile_definitions(
    CPPCORO_COMPILER_SUPPORTS_SYMMETRIC_TRANSFER=1
    __cpp_impl_coroutine=true)

find_package(cppcoro REQUIRED)
find_package(uvw REQUIRED)
find_package(Boost REQUIRED)
find_package(mariadb-connector-c REQUIRED)
find_package(GTest REQUIRED)

include_directories(src)

add_library(${PROJECT_NAME}
    src/mariadb_init.cpp
    src/mariadb_coro_uvw.cpp
    src/tcp_coro.cpp)

target_link_libraries(${PROJECT_NAME}
    cppcoro::cppcoro
    uvw::uvw
    Boost::boost
    mariadb-connector-c::mariadb-connector-c)

enable_testing()

add_subdirectory(test)
