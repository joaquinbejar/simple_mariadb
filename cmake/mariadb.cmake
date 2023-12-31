include(FetchContent)
FetchContent_Declare(mariadb
        GIT_REPOSITORY https://github.com/joaquinbejar/mariadb-connector-c.git
        GIT_TAG v0.1.0
        BUILD_ALWAYS 1
)
FetchContent_Populate(mariadb)

if(NOT EXISTS "${MARIADB_LIB}" OR NOT EXISTS "${MARIADBCLIENT_LIB}")
    message(STATUS "mariadb library not found, building it")
    execute_process(
            COMMAND ${CMAKE_COMMAND} -S ${mariadb_SOURCE_DIR} -B ${mariadb_BINARY_DIR}
            WORKING_DIRECTORY ${mariadb_SOURCE_DIR}
            COMMAND_ECHO STDOUT
            COMMAND_ECHO STDERR
            RESULT_VARIABLE result
    )
    if (result)
        message(FATAL_ERROR "Error Configuring mariadb ${result}")
    endif ()

    execute_process(
            COMMAND ${CMAKE_COMMAND} --build ${mariadb_BINARY_DIR}
            WORKING_DIRECTORY ${mariadb_SOURCE_DIR}
            COMMAND_ECHO STDOUT
            COMMAND_ECHO STDERR
            RESULT_VARIABLE result
    )
    if (result)
        message(FATAL_ERROR "Error building mariadb ${result}")
    endif ()
else()
    message(STATUS "mariadb library found: ${MARIADB_LIB}")
endif()

set(MARIADB_HEADER ${mariadb_SOURCE_DIR}/include CACHE INTERNAL "")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(MARIADB_LIB
        NAMES mariadb
        PATHS ${mariadb_BINARY_DIR}/libmariadb
        REQUIRED
)
find_library(MARIADBCLIENT_LIB
        NAMES mariadbclient
        PATHS ${mariadb_BINARY_DIR}/libmariadb
        REQUIRED
)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".dylib") # Reset to default


set(MARIADB_INCLUDE ${mariadb_SOURCE_DIR}/include CACHE INTERNAL "")
if (CMAKE_DEBUG)
    message(STATUS "simple_mariadb/cmake mariadb_SOURCE_DIR ${mariadb_SOURCE_DIR}")
    message(STATUS "simple_mariadb/cmake MARIADB_LIB ${MARIADB_LIB}")
    message(STATUS "simple_mariadb/cmake MARIADBCLIENT_LIB ${MARIADBCLIENT_LIB}")
endif ()