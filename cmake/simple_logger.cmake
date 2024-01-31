include(FetchContent)
FetchContent_Declare(simple_logger
        GIT_REPOSITORY https://github.com/joaquinbejar/simple_logger.git
        GIT_TAG v0.1.7
        )
FetchContent_MakeAvailable(simple_logger)

set(SIMPLE_LOGGER_INCLUDE ${simple_logger_SOURCE_DIR}/include CACHE INTERNAL "")
if (CMAKE_DEBUG)
        message(STATUS "simple_mariadb/cmake simple_logger_SOURCE_DIR ${simple_logger_SOURCE_DIR}")
        message(STATUS "simple_mariadb/cmake SIMPLE_LOGGER_INCLUDE ${SIMPLE_LOGGER_INCLUDE}")
endif ()
