if (CMAKE_DEBUG)
    message(STATUS "simple_mariadb/tests SIMPLE_COLOR_INCLUDE: ${SIMPLE_COLOR_INCLUDE}")
    message(STATUS "simple_mariadb/tests SIMPLE_CONFIG_INCLUDE: ${SIMPLE_CONFIG_INCLUDE}")
    message(STATUS "simple_mariadb/tests SIMPLE_LOGGER_INCLUDE: ${SIMPLE_LOGGER_INCLUDE}")
    message(STATUS "simple_mariadb/tests NLOHMANN_JSON_INCLUDE: ${NLOHMANN_JSON_INCLUDE}")
    message(STATUS "simple_mariadb/tests COMMON_INCLUDE: ${COMMON_INCLUDE}")
    message(STATUS "simple_mariadb/tests MARIADBCPP_HEADER: ${MARIADBCPP_HEADER}")
    message(STATUS "simple_mariadb/tests MARIADBCPP_LIB: ${MARIADBCPP_LIB}")
endif ()

add_executable(test_client test_client.cpp)
target_include_directories(test_client
        PRIVATE
        ${SIMPLE_MARIADB_INCLUDE}
        ${SIMPLE_COLOR_INCLUDE}
        ${SIMPLE_CONFIG_INCLUDE}
        ${SIMPLE_LOGGER_INCLUDE}
        ${NLOHMANN_JSON_INCLUDE}
        ${COMMON_INCLUDE}
        ${MARIADBCPP_HEADER}
)

target_link_libraries(test_client PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test_client PRIVATE
        simple_logger
        simple_config
        common
        simple_mariadb
        ${MARIADBC_LIB}
        ${MARIADBCPP_LIB}
)


add_executable(test_config test_config.cpp)
target_include_directories(test_config
        PRIVATE
        ${SIMPLE_MARIADB_INCLUDE}
        ${SIMPLE_COLOR_INCLUDE}
        ${SIMPLE_CONFIG_INCLUDE}
        ${SIMPLE_LOGGER_INCLUDE}
        ${NLOHMANN_JSON_INCLUDE}
        ${COMMON_INCLUDE}
        ${MARIADBCPP_HEADER}
)

target_link_libraries(test_config PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test_config PRIVATE
        simple_logger
        simple_config
        common
        simple_mariadb
        ${MARIADBC_LIB}
        ${MARIADBCPP_LIB}
)