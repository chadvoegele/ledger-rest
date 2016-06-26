cmake_minimum_required(VERSION 2.6)

set(PROJECT_NAME "ledger-rest")
project("${PROJECT_NAME}")
set(ledget_rest_VERSION_MAJOR 0)
set(ledget_rest_VERSION_MINOR 1)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(RES_DIR "${PROJECT_SOURCE_DIR}/res")

include(cmake/set_cpp11.cmake)

add_subdirectory(${SRC_DIR})
add_subdirectory(${TEST_DIR})
