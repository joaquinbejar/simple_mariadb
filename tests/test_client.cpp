//
// Created by Joaquin Bejar Garcia on 1/2/23.
//

#include <simple_logger/logger.h>
#include <simple_config/config.h>
#include <simple_mariadb/client.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>



// ---------------------------------------------------------------------------------------------------
using simple_mariadb::client::MariaDBManager;
using simple_mariadb::client::is_insert_or_replace_query_correct;

// Helper function to save the current environment variable
std::string get_env_var(const std::string &key) {
    char *val = getenv(key.c_str());
    return val == NULL ? std::string() : std::string(val);
}

// Helper function to restore the environment variable
void set_env_var(const std::string &key, const std::string &value) {
    if (value.empty()) {
        unsetenv(key.c_str());
    } else {
        setenv(key.c_str(), value.c_str(), 1);
    }
}

simple_mariadb::config::MariaDBConfig get_default_config() {
    // Save current environment
    auto old_hostname = get_env_var("MARIADB_HOSTNAME");
    auto old_user = get_env_var("MARIADB_USER");
    auto old_password = get_env_var("MARIADB_PASSWORD");
    auto old_port = get_env_var("MARIADB_PORT");
    auto old_database = get_env_var("MARIADB_DATABASE");
    auto old_multi_insert = get_env_var("MARIADB_MULTI_INSERT");

    // Set new environment
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_USER", "user", 1);
    setenv("MARIADB_PASSWORD", "password", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    setenv("MARIADB_MULTI_INSERT", "true", 1);

    simple_mariadb::config::MariaDBConfig config;
    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());

    // Restore old environment
    set_env_var("MARIADB_HOSTNAME", old_hostname);
    set_env_var("MARIADB_USER", old_user);
    set_env_var("MARIADB_PASSWORD", old_password);
    set_env_var("MARIADB_PORT", old_port);
    set_env_var("MARIADB_DATABASE", old_database);
    set_env_var("MARIADB_MULTI_INSERT", old_multi_insert);

    return config;
}


simple_mariadb::config::MariaDBConfig get_empty_config() {
    auto old_hostname = get_env_var("MARIADB_HOSTNAME");
    auto old_user = get_env_var("MARIADB_USER");
    auto old_password = get_env_var("MARIADB_PASSWORD");
    auto old_port = get_env_var("MARIADB_PORT");
    auto old_database = get_env_var("MARIADB_DATABASE");
    auto old_multi_insert = get_env_var("MARIADB_MULTI_INSERT");

    unsetenv("MARIADB_HOSTNAME");
    unsetenv("MARIADB_USER");
    unsetenv("MARIADB_PASSWORD");
    unsetenv("MARIADB_PORT");
    unsetenv("MARIADB_DATABASE");
    unsetenv("MARIADB_MULTI_INSERT");
    simple_mariadb::config::MariaDBConfig config;
    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());
    set_env_var("MARIADB_HOSTNAME", old_hostname);
    set_env_var("MARIADB_USER", old_user);
    set_env_var("MARIADB_PASSWORD", old_password);
    set_env_var("MARIADB_PORT", old_port);
    set_env_var("MARIADB_DATABASE", old_database);
    set_env_var("MARIADB_MULTI_INSERT", old_multi_insert);
    return config;
}

simple_mariadb::config::MariaDBConfig get_env_config() {
    simple_mariadb::config::MariaDBConfig config;
    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());
    return config;
}

TEST_CASE("Testing MariaDBManager installation", "[queue]") {

    SECTION("Fail connection to localhost") {
        simple_mariadb::config::MariaDBConfig config = get_default_config();
        REQUIRE(config.validate());
        config.logger->send<simple_logger::LogLevel::DEBUG>(config.to_string());
        REQUIRE(config.to_string() == "MariaDBConfig{hostname=localhost, port=3306, user=user, password=password, datab"
                                      "ase=database, multi_insert=1, autoreconnect=true, tcpkeepalive=true, connecttime"
                                      "out=30, sockettimeout=10000, uri=jdbc:mariadb://localhost:3306/database}");
        MariaDBManager dbManager(config);
    }
}


TEST_CASE("Testing MariaDBManager installation valid connection", "[queue]") {
    simple_mariadb::config::MariaDBConfig config = get_env_config();

    SECTION("Valid Connection") {
        REQUIRE(config.validate());
        MariaDBManager dbManager(config);
        REQUIRE(dbManager.is_connected());
    }

    SECTION("Valid Connection and running thread") {
        REQUIRE(config.validate());
        MariaDBManager dbManager(config);
        REQUIRE(dbManager.is_connected());
        REQUIRE(dbManager.is_thread_running());
        dbManager.stop();
        REQUIRE_FALSE(dbManager.is_thread_running());
    }

}


TEST_CASE("Testing ThreadQueue single-insert functionality", "[queue]") {

    simple_mariadb::config::MariaDBConfig config = get_env_config();
    config.multi_insert = false;
    MariaDBManager dbManager(config);
    auto id = common::key_generator();
    REQUIRE(dbManager.is_connected());
    REQUIRE(dbManager.is_thread_running());

    SECTION("Select data") {
        REQUIRE(dbManager.enqueue("INSERT INTO table_name (column1, column2) VALUES ('" + id + "', 'value2');") == true);
//      m_insert not implemented yet
        dbManager.stop(true);
    }
}

//
//TEST_CASE("Testing ThreadQueue multi-insert with fails functionality", "[queue]") {
//    setenv("MARIADB_MULTI_INSERT", "true", 1);
//    setenv("LOGLEVEL", "debug", 1);
//    simple_mariadb::config::MariaDBConfig config;
//    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());
//    MariaDBManager dbManager(config);
//    auto id = common::key_generator();
//
//
//    SECTION("Testing enqueue multi queries method") {
//        for (int i = 0; i < 10; ++i) {
//            for (int j = 0; j < 10; ++j) {
//                REQUIRE(dbManager.enqueue(
//                        "INSERT INTO table_name (column1, column2) VALUES ('value_for_" + id + "', 'value" +
//                        std::to_string(i) + "_"+std::to_string(j) +"');") == true);
//            }
//            REQUIRE(dbManager.enqueue( // This query wrong syntax should fail
//                    "INSERT INTO table_name (column1, column2) VALUES ('value_for_" + id + "', value" +
//                    std::to_string(i) + ");") == true);
//        }
//        REQUIRE(dbManager.queue_size() > 0);
//        std::this_thread::sleep_for(std::chrono::seconds(30));
//        auto result = dbManager.select("SELECT * FROM table_name where column1 = 'value_for_" + id + "';");
//        REQUIRE(!result.empty());  // Assume that the table is not empty
//        REQUIRE(result.size() == 100);
//    }
//}
//
//TEST_CASE("Testing ThreadQueue select types functionality", "[queue]") {
//    setenv("MARIADB_MULTI_INSERT", "true", 1);
//    setenv("LOGLEVEL", "debug", 1);
//    simple_mariadb::config::MariaDBConfig config;
////    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());
//    MariaDBManager dbManager(config);
//
//    SECTION("Testing enqueue multi queries method") {
//        auto id = common::key_generator();
//            for (int j = 0; j < 100; ++j) {
//                REQUIRE(dbManager.enqueue(
//                        "INSERT INTO table_name2 (name , dates, number, f) VALUES ('" + id + "', '2020-10-12'," +
//                        std::to_string(j) + ", 2.2);") == true);
//            }
//
//        REQUIRE(dbManager.queue_size() > 0);
//        std::this_thread::sleep_for(std::chrono::seconds(10));
//        auto result = dbManager.select("SELECT * FROM table_name2 where name = '" + id + "';");
//        REQUIRE(!result.empty());  // Assume that the table is not empty
//        REQUIRE(result.size() == 100);
//        for (int i = 0; i < 100; ++i) {
//            REQUIRE(result[i]["name"] == id);
//            REQUIRE(result[i]["dates"] == "2020-10-12");
//            REQUIRE(result[i]["number"] == std::to_string(i));
//            REQUIRE(result[i]["f"] == "2.2");
//
//        }
//    }
//
//    SECTION("Testing updates ") {
//        auto id = common::key_generator();
//        for (int j = 0; j < 100; ++j) {
//            REQUIRE(dbManager.enqueue(
//                    "INSERT INTO table_name2 (name , dates, number, f) VALUES ('" + id + "', '2020-10-12'," +
//                    std::to_string(j) + ", 2.2);") == true);
//        }
//
//        for (int j = 0; j < 100; ++j) {
//            REQUIRE(dbManager.enqueue(
//                    "INSERT INTO table_name2 (name , dates, number, f) VALUES ('" + id + "', '2020-10-12'," +
//                    std::to_string(j) + ", 2.2) ON CONFLICT (name,number) DO UPDATE SET f = 3.3;") == true);
//        }
//
//        REQUIRE(dbManager.queue_size() > 0);
//        std::this_thread::sleep_for(std::chrono::seconds(10));
//        auto result = dbManager.select("SELECT * FROM table_name2 where name = '" + id + "';");
//        REQUIRE(!result.empty());  // Assume that the table is not empty
//        REQUIRE(result.size() == 100);
//        for (int i = 0; i < 100; ++i) {
//            REQUIRE(result[i]["name"] == id);
//            REQUIRE(result[i]["dates"] == "2020-10-12");
//            REQUIRE(result[i]["number"] == std::to_string(i));
//            REQUIRE(result[i]["f"] == "3.3");
//
//        }
//    }
//}