include(FetchContent)
FetchContent_Declare(mariadbcpp
        GIT_REPOSITORY https://github.com/mariadb-corporation/mariadb-connector-cpp.git
        GIT_TAG 1.1.2
        BUILD_ALWAYS 1
)
FetchContent_Populate(mariadbcpp)

if(NOT EXISTS "${MARIADBCPP_LIB}")
    message(STATUS "mariadbcpp library not found, building it")
    execute_process(
            COMMAND ${CMAKE_COMMAND} -DBUILD_SHARED_LIBS=OFF -S ${mariadbcpp_SOURCE_DIR} -B ${mariadbcpp_BINARY_DIR}
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
find_library(MARIADBCPP_LIB
        NAMES mariadbcpp
        PATHS ${mariadbcpp_BINARY_DIR}
        REQUIRED
        )

set(MARIADBCPP_INCLUDE ${mariadbcpp_SOURCE_DIR}/include CACHE INTERNAL "")
if (CMAKE_DEBUG)
    message(STATUS "simple_mariadb/cmake mariadbcpp_SOURCE_DIR ${mariadbcpp_SOURCE_DIR}")
    message(STATUS "simple_mariadb/cmake MARIADBCPP_LIB ${MARIADBCPP_LIB}")
    message(STATUS "simple_mariadb/cmake MARIADBCPP_INCLUDE ${MARIADBCPP_INCLUDE}")
endif ()