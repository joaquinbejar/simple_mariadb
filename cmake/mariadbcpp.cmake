include(FetchContent)
FetchContent_Declare(mariadbcpp
        GIT_REPOSITORY https://github.com/joaquinbejar/mariadb-connector-cpp.git
        GIT_TAG v0.1.0
        BUILD_ALWAYS 1
)
FetchContent_Populate(mariadbcpp)

if(NOT EXISTS "${MARIADBCPP_LIB}")
    message(STATUS "mariadbcpp library not found, building it")
    execute_process(
            COMMAND ${CMAKE_COMMAND} -DMARIADB_LINK_DYNAMIC=OFF -DWITH_UNIT_TESTS=OFF -S ${mariadbcpp_SOURCE_DIR} -B ${mariadbcpp_BINARY_DIR}
            WORKING_DIRECTORY ${mariadbcpp_SOURCE_DIR}
            COMMAND_ECHO STDOUT
            COMMAND_ECHO STDERR
            RESULT_VARIABLE result
    )
    if (result)
        message(FATAL_ERROR "Error Configuring mariadbcpp ${result}")
    endif ()

    execute_process(
            COMMAND ${CMAKE_COMMAND} --build ${mariadbcpp_BINARY_DIR} --target mariadbcpp
            WORKING_DIRECTORY ${mariadbcpp_SOURCE_DIR}
            COMMAND_ECHO STDOUT
            COMMAND_ECHO STDERR
            RESULT_VARIABLE result
    )
    if (result)
        message(FATAL_ERROR "Error building mariadbcpp ${result}")
    endif ()
else()
    message(STATUS "mariadbcpp library found")
endif()

set(MARIADBCPP_HEADER ${mariadbcpp_SOURCE_DIR}/include CACHE INTERNAL "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(MARIADBCPP_LIB
        NAMES mariadbcpp
        PATHS ${mariadbcpp_BINARY_DIR}
        REQUIRED
        )
set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".dylib") # Reset to default

set(MARIADBCPP_INCLUDE ${mariadbcpp_SOURCE_DIR}/include CACHE INTERNAL "")
if (CMAKE_DEBUG)
    message(STATUS "simple_mariadb/cmake mariadbcpp_SOURCE_DIR ${mariadbcpp_SOURCE_DIR}")
    message(STATUS "simple_mariadb/cmake MARIADBCPP_LIB ${MARIADBCPP_LIB}")
    message(STATUS "simple_mariadb/cmake MARIADBCPP_INCLUDE ${MARIADBCPP_INCLUDE}")
endif ()