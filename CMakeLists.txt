cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(simple_mariadb)

set(GITUSER "$ENV{GITUSER}")
set(GITPASS "$ENV{GITPASS}")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${mariadbcpp_SOURCE_DIR}/libmariadb/libmariadb)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH})

include(cmake/mariadb.cmake)
include(cmake/mariadbcpp.cmake)
include(cmake/simple_logger.cmake)

set(MARIADBCPP_INCLUDE_DIRS "${MARIADBCPP_HEADER}")
set(SIMPLE_MARIADB_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SIMPLE_MARIADB_SOURCE_FILES
        include/simple_mariadb/client.h
        include/simple_mariadb/config.h
        src/config.cpp
        src/client.cpp
)

add_library(simple_mariadb STATIC ${SIMPLE_MARIADB_SOURCE_FILES})
set(SIMPLE_MARIADB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_DEBUG)
    message(STATUS "simple_mariadb SIMPLE_COLOR_INCLUDE: ${SIMPLE_COLOR_INCLUDE}")
    message(STATUS "simple_mariadb SIMPLE_CONFIG_INCLUDE: ${SIMPLE_CONFIG_INCLUDE}")
    message(STATUS "simple_mariadb SIMPLE_LOGGER_INCLUDE: ${SIMPLE_LOGGER_INCLUDE}")
    message(STATUS "simple_mariadb NLOHMANN_JSON_INCLUDE: ${NLOHMANN_JSON_INCLUDE}")
    message(STATUS "simple_mariadb COMMON_INCLUDE: ${COMMON_INCLUDE}")
    message(STATUS "simple_mariadb MARIADBCPP_HEADER: ${MARIADBCPP_HEADER}")
    message(STATUS "simple_mariadb MARIADBCPP_LIB: ${MARIADBCPP_LIB}")
    message(STATUS "simple_mariadb MARIADB_LIB: ${MARIADB_LIB}")
endif ()

target_include_directories(simple_mariadb PUBLIC
        ${SIMPLE_MARIADB_INCLUDE}
        ${SIMPLE_CONFIG_INCLUDE}
        ${SIMPLE_LOGGER_INCLUDE}
        ${NLOHMANN_JSON_INCLUDE}
        ${COMMON_INCLUDE}
        ${MARIADBCPP_HEADER}
)

target_link_libraries(simple_mariadb
        PUBLIC
        pthread
        ${MARIADBCPP_LIB}
        ${MARIADB_LIB}
        simple_config
        simple_logger
        common
)

set(SIMPLE_MARIADB_TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
option(NO_SIMPLE_MARIADB_TESTS "simple_mariadb Disable simple_mariadb tests" OFF)
if (NOT NO_MARIADB_REST_API_TESTS)
    add_subdirectory(tests)
endif ()