# Project name
# Dmitriy Vetutnev 2021

cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
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

find_package(GTest REQUIRED)

include_directories(src)

enable_testing()

add_subdirectory(test)
