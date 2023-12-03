find_library(MARIADBC_LIB
        NAMES mariadb
        PATHS /usr/local/lib/mariadb
        REQUIRED
        )
if ( MARIADBC_LIB)
    message(STATUS "FOUND MARIADBC_LIB ${MARIADBC_LIB}")
endif ()
