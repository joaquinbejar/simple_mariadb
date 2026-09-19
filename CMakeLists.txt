cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(simple_mariadb VERSION 0.1.6)

set(GITUSER "$ENV{GITUSER}")
set(GITPASS "$ENV{GITPASS}")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${mariadbcpp_SOURCE_DIR}/libmariadb/libmariadb)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH})

include(FetchContent)
# Catch2 and nlohmann_json only arrive transitively through simple_logger;
# declare them first so these pins win (FetchContent honours the first
# declaration). common_cpp v0.1.5 pins nlohmann_json v3.11.2, whose
# cmake_minimum_required is rejected by CMake >= 4.0.
FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.16.0
)
FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)
include(cmake/mariadb.cmake)
include(cmake/mariadbcpp.cmake)
include(cmake/simple_logger.cmake)

find_package(OpenSSL REQUIRED)
if (APPLE)
    # libmariadb is built against the system zlib on macOS (see cmake/mariadb.cmake).
    set(SIMPLE_MARIADB_ZLIB z)
endif ()


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
    message(STATUS "simple_mariadb MARIADBCLIENT_LIB: ${MARIADBCLIENT_LIB}")
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
        OpenSSL::SSL OpenSSL::Crypto
        ${SIMPLE_MARIADB_ZLIB})

set(SIMPLE_MARIADB_TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
option(NO_SIMPLE_MARIADB_TESTS "simple_mariadb Disable simple_mariadb tests" OFF)
if (NOT NO_SIMPLE_MARIADB_TESTS)
    add_subdirectory(tests)
endif ()

