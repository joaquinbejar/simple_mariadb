
find_path(MARIADBCPP_HEADER
        NAMES mariadb/conncpp.hpp
        PATHS /usr/local/include
        REQUIRED
        )
find_library(MARIADBCPP_LIB
        NAMES mariadbcpp
        PATHS /usr/local/lib/mariadb
        REQUIRED
        )
if (MARIADBCPP_HEADER AND MARIADBCPP_LIB)
    message(STATUS "FOUND MARIADBCPP_HEADER ${MARIADBCPP_HEADER}")
    message(STATUS "FOUND MARIADBCPP_LIB ${MARIADBCPP_LIB}")
endif ()