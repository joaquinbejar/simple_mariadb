//
// Created by Joaquin Bejar Garcia on 1/2/23.
//

#include <simple_logger/logger.h>
#include <simple_config/config.h>
#include <simple_mariadb/client.h>
#include <common/dates.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <string>
#include <map>
#include <random>
#include <chrono>
#include <utility>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// ---------------------------------------------------------------------------------------------------
using simple_mariadb::client::MariaDBManager;
using simple_mariadb::client::::common::sql_utils::is_insert_or_replace_query_correct;
using simple_mariadb::client::::common::sql_utils::InsertType;

simple_mariadb::config::MariaDBConfig global_config;

class CreateAndDestroy {
public:
    std::string table;
    bool delete_it;
    bool table_created_successfully;

    explicit CreateAndDestroy(std::string table_name = "MariaClient_Test_", bool delete_on_destroy = true)
            : table(std::move(table_name)), delete_it(delete_on_destroy), table_created_successfully(false) {
        // Append a random string to the table name
        table += generate_random_string(10);

        // Create the table
        create_table();
    }

    ~CreateAndDestroy() {
        if (delete_it && table_created_successfully) {
            drop_table();
        }
    }

    void create_table() {
        // Use a try-catch block to handle any exceptions and set the flag accordingly
        try {
            simple_mariadb::client::MariaDBManager maria(global_config);
            std::map<std::string, std::string> columns = {{"id", "INT NOT NULL DEFAULT UNIX_TIMESTAMP()"}};
            maria.create_table(table, columns);
            table_created_successfully = true;
        } catch (const std::exception &e) {
            // Handle exceptions, such as logging the error
            table_created_successfully = false;
        }
    }

    void drop_table() const {
        simple_mariadb::client::MariaDBManager maria(global_config);
        maria.drop_table(table);
    }

private:
    static std::string generate_random_string(size_t length) {
        // Use the <random> library to generate a random string of a given length
        const std::string chars = "abcdefghijklmnopqrstuvwxyz";
        std::random_device random_device;
        std::mt19937 generator(random_device());
        std::uniform_int_distribution<> distribution(0, chars.size() - 1);

        std::string random_string;
        for (size_t i = 0; i < length; ++i) {
            random_string += chars[distribution(generator)];
        }

        return random_string;
    }
};

// Helper function to save the current environment variable
std::string get_env_var(const std::string &key) {
    char *val = getenv(key.c_str());
    return val == nullptr ? std::string() : std::string(val);
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
    auto old_checker_time = get_env_var("CHECKER_TIME");

    // Set new environment
    setenv("MARIADB_HOSTNAME", "localhost", 1);
    setenv("MARIADB_USER", "user", 1);
    setenv("MARIADB_PASSWORD", "password", 1);
    setenv("MARIADB_PORT", "3306", 1);
    setenv("MARIADB_DATABASE", "database", 1);
    setenv("MARIADB_MULTI_INSERT", "true", 1);
    setenv("CHECKER_TIME", "30", 1);

    simple_mariadb::config::MariaDBConfig config;
    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());

    // Restore old environment
    set_env_var("MARIADB_HOSTNAME", old_hostname);
    set_env_var("MARIADB_USER", old_user);
    set_env_var("MARIADB_PASSWORD", old_password);
    set_env_var("MARIADB_PORT", old_port);
    set_env_var("MARIADB_DATABASE", old_database);
    set_env_var("MARIADB_MULTI_INSERT", old_multi_insert);
    set_env_var("CHECKER_TIME", old_checker_time);

    return config;
}

simple_mariadb::config::MariaDBConfig get_empty_config() {
    auto old_hostname = get_env_var("MARIADB_HOSTNAME");
    auto old_user = get_env_var("MARIADB_USER");
    auto old_password = get_env_var("MARIADB_PASSWORD");
    auto old_port = get_env_var("MARIADB_PORT");
    auto old_database = get_env_var("MARIADB_DATABASE");
    auto old_multi_insert = get_env_var("MARIADB_MULTI_INSERT");
    auto old_checker_time = get_env_var("CHECKER_TIME");

    unsetenv("MARIADB_HOSTNAME");
    unsetenv("MARIADB_USER");
    unsetenv("MARIADB_PASSWORD");
    unsetenv("MARIADB_PORT");
    unsetenv("MARIADB_DATABASE");
    unsetenv("MARIADB_MULTI_INSERT");
    unsetenv("CHECKER_TIME");
    simple_mariadb::config::MariaDBConfig config;
    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());
    set_env_var("MARIADB_HOSTNAME", old_hostname);
    set_env_var("MARIADB_USER", old_user);
    set_env_var("MARIADB_PASSWORD", old_password);
    set_env_var("MARIADB_PORT", old_port);
    set_env_var("MARIADB_DATABASE", old_database);
    set_env_var("MARIADB_MULTI_INSERT", old_multi_insert);
    set_env_var("CHECKER_TIME", old_checker_time);
    return config;
}

simple_mariadb::config::MariaDBConfig get_env_config() {
    simple_mariadb::config::MariaDBConfig config;
    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());
    return config;
}

TEST_CASE("Testing ::common::sql_utils::is_insert_or_replace_query_correct", "[queue]") {

    SECTION("Right queries") {
        std::vector<std::string> queries = {
                R"(REPLACE INTO `OHLC` (`ticker`, `open`, `high`, `low`, `close`, `transactions`, `otc`, `timestamp`, `volume`, `volume_weighted_price`) VALUES ('ACAQ', 9.4, 9.95, 8, 9.75, 996, 0, 1697486400000, 32962, 9.1733);)",
                R"(INSERT IGNORE INTO `Tickers` (`active`, `cik`, `composite_figi`, `currency_name`, `last_updated_utc`, `locale`, `market`, `name`, `primary_exchange`, `share_class_figi`, `ticker`, `type`) VALUES ('true', '0001914023', 'BBG0190WQ3Q5', 'usd', '2023-12-22T00:00:00Z', 'us', 'stocks', 'Acri Capital Acquisition Corporation Warrant', 'XNAS', NULL, 'ACACW', 'WARRANT');)",
                R"(INSERT INTO Tickers (`active`, `cik`, `composite_figi`, `currency_name`, `last_updated_utc`, `locale`, `market`, `name`, `primary_exchange`, `share_class_figi`, `ticker`, `type`) VALUES ('true', '0001914023', 'BBG0190WQ3Q5', 'usd', '2023-12-22T00:00:00Z', 'us', 'stocks', 'Acri Capital Acquisition Corporation Warrant', 'XNAS', NULL, 'ACACW', 'WARRANT');)",
                R"(insert into Tickers (`active`, `cik`, `composite_figi`, `currency_name`, `last_updated_utc`, `locale`, `market`, `name`, `primary_exchange`, `share_class_figi`, `ticker`, `type`) VALUES ('true', '0001914023', 'BBG0190WQ3Q5', 'usd', '2023-12-22T00:00:00Z', 'us', 'stocks', 'Acri Capital Acquisition Corporation Warrant', 'XNAS', NULL, 'ACACW', 'WARRANT');)",
                R"(replace into `OHLC` (`ticker`, `open`, `high`, `low`, `close`, `transactions`, `otc`, `timestamp`, `volume`, `volume_weighted_price`) VALUES ('ACAQ', 9.4, 9.95, 8, 9.75, 996, 0, 1697486400000, 32962, 9.1733);)",
                R"(REPLACE INTO `OHLC` (`ticker`, `open`, `high`, `low`, `close`, `transactions`, `otc`, `timestamp`, `volume`, `volume_weighted_price`) VALUES ('ZBZX', 25, 25, 25, 25, 0, 0, 1703192400000, 0, 0);)",
                R"(INSERT INTO Tickers (active, `cik`, `composite_figi`, `currency_name`, `last_updated_utc`, `locale`, `market`, `name`, `primary_exchange`, `share_class_figi`, `ticker`, `type`) VALUES ('true', '0001914023', 'BBG0190WQ3Q5', 'usd', '2023-12-22T00:00:00Z', 'us', 'stocks', 'Acri Capital Acquisition Corporation Warrant', 'XNAS', NULL, 'ACACW', 'WARRANT');)"
        };
        for (const auto &query : queries) {
            REQUIRE(::common::sql_utils::is_insert_or_replace_query_correct(query) == true);
        }
    }

    SECTION("Wrong Queries") {
        std::vector<std::string> queries = {
                R"(INSERT INTO (`active`, `cik`, `composite_figi`, `currency_name`, `last_updated_utc`, `locale`, `market`, `name`, `primary_exchange`, `share_class_figi`, `ticker`, `type`) VALUES ('true', '0001914023', 'BBG0190WQ3Q5', 'usd', '2023-12-22T00:00:00Z', 'us', 'stocks', 'Acri Capital Acquisition Corporation Warrant', 'XNAS', NULL, 'ACACW', 'WARRANT');)",
                R"(INSERT INTO Tickers active, `cik`, `composite_figi`, `currency_name`, `last_updated_utc`, `locale`, `market`, `name`, `primary_exchange`, `share_class_figi`, `ticker`, `type`) VALUES ('true', '0001914023', 'BBG0190WQ3Q5', 'usd', '2023-12-22T00:00:00Z', 'us', 'stocks', 'Acri Capital Acquisition Corporation Warrant', 'XNAS', NULL, 'ACACW', 'WARRANT');)"
        };
        for (const auto &query : queries) {
            REQUIRE_FALSE(::common::sql_utils::is_insert_or_replace_query_correct(query) == true);
        }
    }
}


TEST_CASE("Testing MariaDBManager installation", "[queue]") {

    SECTION("Fail connection to localhost") {
        simple_mariadb::config::MariaDBConfig config = get_default_config();
        REQUIRE(config.validate());
        config.logger->send<simple_logger::LogLevel::DEBUG>(config.to_string());
        REQUIRE(config.to_string() == R"({"MariaDBConfig":{"autoreconnect":"true","checker_time":30,"connecttimeout":"30","dbname":"database","hostname":"localhost","multi_insert":true,"password":"password","port":3306,"sockettimeout":"10000","tcpkeepalive":"true","user":"user"}})");
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
        REQUIRE(dbManager.enqueue("INSERT INTO table_name (column1, column2) VALUES ('" + id + "', 'value2');") ==
                true);
        dbManager.stop(true);
    }
}

TEST_CASE("Replaces based on ::common::sql_utils::InsertType::REPLACE", "[replace_insert_type]") {

    SECTION("Insert to Replace") {
        std::string query1 = "INSERT INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "insert into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "INSERT  into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " insert  INTO  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query2, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query3, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query4, ::common::sql_utils::InsertType::REPLACE);
        REQUIRE(query1 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
    }

    SECTION("Replace to Replace") {
        std::string query1 = "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "replace into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "REPLACE  into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " replace  INTO  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query2, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query3, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query4, ::common::sql_utils::InsertType::REPLACE);
        REQUIRE(query1 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
    }

    SECTION("Ignore to Replace") {
        std::string query1 = "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "insert ignore into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "INSERT  ignore into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " insert  IGNORE  into  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query2, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query3, ::common::sql_utils::InsertType::REPLACE);
        replace_insert_type(query4, ::common::sql_utils::InsertType::REPLACE);
        REQUIRE(query1 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)");
    }
}

TEST_CASE("Replaces based on ::common::sql_utils::InsertType::IGNORE", "[replace_insert_type]") {

    SECTION("Insert to IGNORE") {
        std::string query1 = "INSERT INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "insert into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "INSERT  into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " insert  INTO  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query2, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query3, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query4, ::common::sql_utils::InsertType::IGNORE);
        REQUIRE(query1 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
    }

    SECTION("Replace to IGNORE") {
        std::string query1 = "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "replace into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "REPLACE  into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " replace  INTO  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query2, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query3, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query4, ::common::sql_utils::InsertType::IGNORE);
        REQUIRE(query1 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
    }

    SECTION("Ignore to IGNORE") {
        std::string query1 = "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "insert ignore into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "INSERT  ignore into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " insert  IGNORE  into  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query2, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query3, ::common::sql_utils::InsertType::IGNORE);
        replace_insert_type(query4, ::common::sql_utils::InsertType::IGNORE);
        REQUIRE(query1 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)");
    }
}

TEST_CASE("Replaces based on ::common::sql_utils::InsertType::INSERT", "[replace_insert_type]") {

    SECTION("Insert to INSERT") {
        std::string query1 = "INSERT INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "insert into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "INSERT  into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " insert  INTO  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query2, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query3, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query4, ::common::sql_utils::InsertType::INSERT);
        REQUIRE(query1 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
    }

    SECTION("Replace to INSERT") {
        std::string query1 = "REPLACE INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "replace into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "REPLACE  into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " replace  INTO  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query2, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query3, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query4, ::common::sql_utils::InsertType::INSERT);
        REQUIRE(query1 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
    }

    SECTION("Ignore to INSERT") {
        std::string query1 = "INSERT IGNORE INTO table_name (column1, column2) VALUES (value1, value2)";
        std::string query2 = "insert ignore into table_name (column1, column2) VALUES (value1, value2)";
        std::string query3 = "INSERT  ignore into table_name (column1, column2) VALUES (value1, value2)";
        std::string query4 = " insert  IGNORE  into  table_name (column1, column2) VALUES (value1, value2)";
        replace_insert_type(query1, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query2, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query3, ::common::sql_utils::InsertType::INSERT);
        replace_insert_type(query4, ::common::sql_utils::InsertType::INSERT);
        REQUIRE(query1 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query2 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query3 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
        REQUIRE(query4 == "INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
    }
}

TEST_CASE("Testing connection", "[queue]") {
    simple_mariadb::config::MariaDBConfig config;
    config.logger->send<simple_logger::LogLevel::INFORMATIONAL>(config.to_string());
    MariaDBManager dbManager(config);
    REQUIRE(dbManager.is_connected());
}

TEST_CASE("Testing ThreadQueue select types functionality multi", "[queue]") {
    setenv("MARIADB_MULTI_INSERT", "true", 1);
    setenv("LOGLEVEL", "debug", 1);
    size_t size = 10;
    simple_mariadb::config::MariaDBConfig config;
    MariaDBManager dbManager(config);
    REQUIRE(dbManager.is_connected());
    REQUIRE(dbManager.is_thread_running());


    SECTION("Testing enqueue multi queries method") {

        CreateAndDestroy createAndDestroy;
        REQUIRE(createAndDestroy.table_created_successfully);

        std::map<std::string, std::string> columns;
        columns["name"] = "VARCHAR(255) NOT NULL";
        columns["number"] = "INT NOT NULL";
        columns["f"] = "FLOAT NOT NULL";
        columns["dates"] = "DATE NOT NULL";
        bool result_create_columns = dbManager.add_columns_to_table(createAndDestroy.table, columns);
        REQUIRE(result_create_columns);

        auto id = common::key_generator();
        for (int j = 0; j < size; ++j) {
            std::string query = "INSERT INTO " + createAndDestroy.table + " (name , dates, number, f) VALUES ('" + id +
                                "', '2020-10-12'," +
                                std::to_string(j) + ", 2.2);";
            REQUIRE(dbManager.enqueue(query) == true);
        }
        REQUIRE(dbManager.queue_size() > 0);
        size_t timeout = 100;
        while (dbManager.queue_size() > 0) {
            timeout++;
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            if (timeout >= 1000) {
                break;
            }
        }
        REQUIRE(dbManager.queue_size() == 0);
        dbManager.stop();
        std::string select_query = "SELECT * FROM " + createAndDestroy.table + " where `name` = '" + id + "';";
        auto result = dbManager.query_to_json(select_query);
        REQUIRE(!result.empty());  // Assume that the table is not empty
        REQUIRE(result.size() == size);
        for (int i = 0; i < size; ++i) {
            REQUIRE(result[i]["name"] == id);
            REQUIRE(result[i]["dates"] == "2020-10-12");
            REQUIRE(result[i]["number"] == i);
            // aproximate comparison
            REQUIRE_THAT( result[i]["f"],
                          Catch::Matchers::WithinRel(2.2, 0.001)
                          && Catch::Matchers::WithinAbs(2.2, 2.200001) );
        }
    }


    SECTION("Testing updates ") {
        CreateAndDestroy createAndDestroy;
        REQUIRE(createAndDestroy.table_created_successfully);
        std::map<std::string, std::string> columns;
        columns["name"] = "VARCHAR(255) NOT NULL";
        columns["number"] = "INT NOT NULL";
        columns["f"] = "FLOAT NOT NULL";
        columns["dates"] = "DATE NOT NULL";
        bool result_create_columns = dbManager.add_columns_to_table(createAndDestroy.table, columns);
        REQUIRE(result_create_columns);
        std::vector<std::string> columns_unique = {"number"};
        bool result_index = dbManager.create_index_unique(createAndDestroy.table, "id_unique", columns_unique);
        REQUIRE(result_index);
        auto id = common::key_generator();

        for (int j = 0; j < size; ++j) {
            std::string query = "INSERT INTO " + createAndDestroy.table + " (name , dates, number, f) VALUES ('" + id +
                                "', '2020-10-12'," + std::to_string(j) + ", 2.2);";
            REQUIRE(dbManager.enqueue(query) == true);
        }

        for (int j = 0; j < size; ++j) {
            std::string query = "INSERT INTO " + createAndDestroy.table + " (name , dates, number, f) VALUES ('" + id +
                                "', '2020-10-12'," + std::to_string(j) +
                                ", 2.2) ON DUPLICATE KEY UPDATE f = 3.3";
            REQUIRE(dbManager.enqueue(query, false) == true);
        }

        REQUIRE(dbManager.queue_size() > 0);
        size_t timeout = 100;
        while (dbManager.queue_size() > 0) {
            timeout++;
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            if (timeout >= 1000) {
                break;
            }
        }
        REQUIRE(dbManager.queue_size() == 0);


        std::string select_query = "SELECT * FROM " + createAndDestroy.table + " where `name` = '" + id + "';";
        auto result = dbManager.query_to_json(select_query);

        REQUIRE(!result.empty());  // Assume that the table is not empty
        REQUIRE(result.size() == size);
        for (int i = 0; i < size; ++i) {
            REQUIRE(result[i]["name"] == id);
            REQUIRE(result[i]["dates"] == "2020-10-12");
            REQUIRE(result[i]["number"] == i);
            REQUIRE_THAT(result[i]["f"],
                         Catch::Matchers::WithinRel(3.3, 0.001)
                         && Catch::Matchers::WithinAbs(3.3, 3.300001));

        }
    }
}

TEST_CASE("Testing ThreadQueue select types functionality", "[queue]") {
    setenv("MARIADB_MULTI_INSERT", "false", 1);
    setenv("LOGLEVEL", "debug", 1);
    size_t size = 10;
    simple_mariadb::config::MariaDBConfig config;
    MariaDBManager dbManager(config);
    REQUIRE(dbManager.is_connected());
    REQUIRE(dbManager.is_thread_running());


    SECTION("Testing enqueue multi queries method") {

        CreateAndDestroy createAndDestroy;
        REQUIRE(createAndDestroy.table_created_successfully);

        std::map<std::string, std::string> columns;
        columns["name"] = "VARCHAR(255) NOT NULL";
        columns["number"] = "INT NOT NULL";
        columns["f"] = "FLOAT NOT NULL";
        columns["dates"] = "DATE NOT NULL";
        bool result_create_columns = dbManager.add_columns_to_table(createAndDestroy.table, columns);
        REQUIRE(result_create_columns);

        auto id = common::key_generator();
        for (int j = 0; j < size; ++j) {
            std::string query = "INSERT INTO " + createAndDestroy.table + " (name , dates, number, f) VALUES ('" + id +
                                "', '2020-10-12'," +
                                std::to_string(j) + ", 2.2);";
            REQUIRE(dbManager.enqueue(query) == true);
        }
        REQUIRE(dbManager.queue_size() > 0);
        size_t timeout = 100;
        while (dbManager.queue_size() > 0) {
            timeout++;
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            if (timeout >= 1000) {
                break;
            }
        }
        REQUIRE(dbManager.queue_size() == 0);
        dbManager.stop();
        std::string select_query = "SELECT * FROM " + createAndDestroy.table + " where `name` = '" + id + "';";
        auto result = dbManager.query_to_json(select_query);
        REQUIRE(!result.empty());  // Assume that the table is not empty
        REQUIRE(result.size() == size);
        for (int i = 0; i < size; ++i) {
            REQUIRE(result[i]["name"] == id);
            REQUIRE(result[i]["dates"] == "2020-10-12");
            REQUIRE(result[i]["number"] == i);
            // aproximate comparison
            REQUIRE_THAT( result[i]["f"],
                          Catch::Matchers::WithinRel(2.2, 0.001)
                          && Catch::Matchers::WithinAbs(2.2, 2.200001) );
        }
    }


    SECTION("Testing updates ") {
        CreateAndDestroy createAndDestroy;
        REQUIRE(createAndDestroy.table_created_successfully);
        std::map<std::string, std::string> columns;
        columns["name"] = "VARCHAR(255) NOT NULL";
        columns["number"] = "INT NOT NULL";
        columns["f"] = "FLOAT NOT NULL";
        columns["dates"] = "DATE NOT NULL";
        bool result_create_columns = dbManager.add_columns_to_table(createAndDestroy.table, columns);
        REQUIRE(result_create_columns);
        std::vector<std::string> columns_unique = {"number"};
        bool result_index = dbManager.create_index_unique(createAndDestroy.table, "id_unique", columns_unique);
        REQUIRE(result_index);
        auto id = common::key_generator();

        for (int j = 0; j < size; ++j) {
            std::string query = "INSERT INTO " + createAndDestroy.table + " (name , dates, number, f) VALUES ('" + id +
                                "', '2020-10-12'," + std::to_string(j) + ", 2.2);";
            REQUIRE(dbManager.enqueue(query) == true);
        }

        for (int j = 0; j < size; ++j) {
            std::string query = "INSERT INTO " + createAndDestroy.table + " (name , dates, number, f) VALUES ('" + id +
                                "', '2020-10-12'," + std::to_string(j) +
                                ", 2.2) ON DUPLICATE KEY UPDATE f = 3.3";
            REQUIRE(dbManager.enqueue(query, false) == true);
        }

        REQUIRE(dbManager.queue_size() > 0);
        size_t timeout = 100;
        while (dbManager.queue_size() > 0) {
            timeout++;
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            if (timeout >= 1000) {
                break;
            }
        }
        REQUIRE(dbManager.queue_size() == 0);


        std::string select_query = "SELECT * FROM " + createAndDestroy.table + " where `name` = '" + id + "';";
        auto result = dbManager.query_to_json(select_query);

        REQUIRE(!result.empty());  // Assume that the table is not empty
        REQUIRE(result.size() == size);
        for (int i = 0; i < size; ++i) {
            REQUIRE(result[i]["name"] == id);
            REQUIRE(result[i]["dates"] == "2020-10-12");
            REQUIRE(result[i]["number"] == i);
            REQUIRE_THAT(result[i]["f"],
                         Catch::Matchers::WithinRel(3.3, 0.001)
                         && Catch::Matchers::WithinAbs(3.3, 3.300001));

        }
    }
}