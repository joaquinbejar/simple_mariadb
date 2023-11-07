//
// Created by Joaquin Bejar Garcia on 20/10/23.
//

#include "simple_mariadb/config.h"
#include <catch2/catch_test_macros.hpp>


// ---------------------------------------------------------------------------------------------------
TEST_CASE("Declare MariaDBConfig", "[MariaDBConfig]") {
    unsetenv("MARIADB_HOSTNAME");
    unsetenv("MARIADB_PORT");
    unsetenv("MARIADB_DATABASE");
    simple_mariadb::config::MariaDBConfig config;
    REQUIRE(config.uri == "jdbc:mariadb://:@localhost:3306/");
    REQUIRE_FALSE(config.validate());
}

TEST_CASE("Declare MariaDBConfig with env variables", "[MariaDBConfig]") {
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    simple_mariadb::config::MariaDBConfig config;
    REQUIRE(config.uri == "jdbc:mariadb://:@localhost:3306/database");
    REQUIRE_FALSE(config.validate());
}

TEST_CASE("Declare MariaDBConfig with env variables full valid", "[MariaDBConfig]") {
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    setenv("MARIADB_USER", "user", 1);
    setenv("MARIADB_PASSWORD", "password", 1);
    simple_mariadb::config::MariaDBConfig config;
    REQUIRE(config.uri == "jdbc:mariadb://user:password@localhost:3306/database");
    REQUIRE(config.validate());
}

TEST_CASE("Use Logger", "[MariaDBConfig]") {
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    setenv("MARIADB_USER", "user", 1);
    setenv("MARIADB_PASSWORD", "password", 1);
    simple_mariadb::config::MariaDBConfig config;
    REQUIRE(config.uri == "jdbc:mariadb://user:password@localhost:3306/database");
    REQUIRE(config.validate());
    std::shared_ptr<simple_logger::Logger> logger = config.logger;
    logger->send<simple_logger::LogLevel::EMERGENCY>("EMERGENCY message");
}