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
    REQUIRE(config.uri == "jdbc:mariadb://localhost:3306/");
    REQUIRE_FALSE(config.validate());
}

TEST_CASE("Declare MariaDBConfig with env variables", "[MariaDBConfig]") {
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    simple_mariadb::config::MariaDBConfig config;
    REQUIRE(config.uri == "jdbc:mariadb://localhost:3306/database");
    REQUIRE_FALSE(config.validate());
}

TEST_CASE("Declare MariaDBConfig with env variables full valid", "[MariaDBConfig]") {
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    setenv("MARIADB_USER", "user", 1);
    setenv("MARIADB_PASSWORD", "password", 1);
    simple_mariadb::config::MariaDBConfig config;
    REQUIRE(config.uri == "jdbc:mariadb://localhost:3306/database");
    REQUIRE(config.validate());
}

TEST_CASE("Use Logger", "[MariaDBConfig]") {
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    setenv("MARIADB_USER", "user", 1);
    setenv("MARIADB_PASSWORD", "password", 1);
    simple_mariadb::config::MariaDBConfig config;
    REQUIRE(config.uri == "jdbc:mariadb://localhost:3306/database");
    REQUIRE(config.validate());
    std::shared_ptr<simple_logger::Logger> logger = config.logger;
    logger->send<simple_logger::LogLevel::EMERGENCY>("EMERGENCY message");
}

TEST_CASE("Use to_string", "[MariaDBConfig]") {
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    setenv("MARIADB_USER", "user", 1);
    setenv("MARIADB_PASSWORD", "password", 1);
    simple_mariadb::config::MariaDBConfig config;
    REQUIRE(config.uri == "jdbc:mariadb://localhost:3306/database");
    REQUIRE(config.validate());
    std::string expected_str = R"({"MariaDBConfig":{"autoreconnect":"true","checker_time":30,"connecttimeout":"30","dbname":"database","hostname":"localhost","multi_insert":false,"password":"password","port":3306,"sockettimeout":"10000","tcpkeepalive":"true","user":"user"}})";
    REQUIRE(config.to_string() == expected_str);

}

TEST_CASE("Use to_json", "[MariaDBConfig]") {
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "", 1);
    setenv("MARIADB_USER", "user", 1);
    setenv("MARIADB_PASSWORD", "password", 1);
    simple_mariadb::config::MariaDBConfig config;
    SECTION("from_json method and key as param") {
        json j = config.to_json();
        j["hostname"] = "localhost";
        j["dbname"] = "database";
        j["user"] = "user";
        j["password"] = "password";
        REQUIRE_NOTHROW(config.from_json(j));
        REQUIRE(config.uri == "jdbc:mariadb://localhost:3306/database");
        REQUIRE(config.validate());

    }
}