//
// Created by Joaquin Bejar Garcia on 7/11/23.
//

#include "simple_mariadb/client.h"

namespace simple_mariadb::client {

    void replace_insert_type(std::string &query, const InsertType &insert_type) {
        // Use regular expressions for case-insensitive search and to handle extra spaces
        for (const auto &it: insert_type_map) {
            // Construct a regex pattern to match optional spaces at the beginning and within the string
            std::string pattern = "\\s*";  // Optionally match leading spaces
            for (char c: it.second) {
                if (c == ' ') {
                    pattern += "\\s*";  // Replace spaces with regex for optional spaces
                } else {
                    pattern += c;
                }
            }

            std::regex regex_pattern(pattern, std::regex_constants::icase);

            // Search and replace using regex
            if (std::regex_search(query, regex_pattern)) {
                const std::string &replacement = insert_type_map.at(insert_type);
                query = std::regex_replace(query, regex_pattern, replacement, std::regex_constants::format_first_only);
                break;
            }
        }
    }

    bool is_insert_or_replace_query_correct(const std::string &query) {
        return std::regex_match(query, QUERYREGEX);
    }

    MariaDBManager::MariaDBManager(simple_mariadb::config::MariaDBConfig &config) :
            m_config(config),
            m_queue_thread(&MariaDBManager::run, this),
            m_checker_thread(&MariaDBManager::m_run_checker, this) {
        m_logger->send<simple_logger::LogLevel::DEBUG>("MariaDBManager constructor");
        if (!m_config.validate()) {
            this->m_join_threads();
            m_logger->send<simple_logger::LogLevel::ERROR>("MariaDBConfig is not valid");
            throw std::runtime_error("MariaDBConfig is not valid");
        }

        {
            std::lock_guard<std::mutex> lock(m_write_mutex);
            this->m_get_connection(m_conn_write);
        }
        {
            std::lock_guard<std::mutex> lock(m_read_mutex);
            this->m_get_connection(m_conn_read);
        }
        if (!this->is_connected()) {
            throw std::runtime_error("MariaDBManager failed to connect to database");
        }
    }

    bool MariaDBManager::m_is_connected(std::shared_ptr<sql::Connection> &conn) {
        if (conn != nullptr) {
            if (conn->isClosed()) {
                m_logger->send<simple_logger::LogLevel::DEBUG>(
                        "Connection is not open on host: " + std::string(conn->getHostname()));
                return false;
            }
        } else {
//            m_logger->send<simple_logger::LogLevel::DEBUG>("MariaDBManager Connection is not defined");
            return false;
        }
        return conn->isValid();
    }

    void MariaDBManager::m_get_connection(std::shared_ptr<sql::Connection> &conn) {
        if (this->m_is_connected(conn)) {
            m_logger->send<simple_logger::LogLevel::DEBUG>("m_get_connection MariaDB client already connected");
            return;
        }
        try {
            sql::SQLString url(m_config.uri);
            sql::Properties properties(m_config.get_options());
            conn = std::shared_ptr<sql::Connection>(m_driver->connect(url, properties));

            if (this->m_is_connected(conn)) {
                m_logger->send<simple_logger::LogLevel::DEBUG>(
                        "MariaDB is now connected to: " + std::string(conn->getHostname())
                        + " with network timeout: " + std::to_string(conn->getNetworkTimeout()));
                return;
            }
        } catch (sql::SQLException &e) {
            this->m_logger->send<simple_logger::LogLevel::ERROR>(
                    "MariaDB client connection failed. Error code: " + std::string(e.what()));
        }
    }

    MariaDBManager::~MariaDBManager() {
        this->m_join_threads();
    }

    void MariaDBManager::m_join_threads() {
        if (m_checker_thread_is_running or m_queue_thread_is_running)
            this->stop();
        if (m_queue_thread.joinable()) {
            m_queue_thread.join();
        }
        if (m_checker_thread.joinable()) {
            m_checker_thread.join();
        }
    }

    std::vector<std::map<std::string, std::string>> MariaDBManager::select(const std::string &query) {
        // use this->query_to_json and convert json to vector of maps

        std::vector<std::map<std::string, std::string>> result;
        json json_result = this->query_to_json(query);

        std::cout << json_result.dump(4) << std::endl;
        result.reserve(json_result.size());
        for (auto &row: json_result) {
            std::map<std::string, std::string> map_row;
            for (auto &column: row.items()) {
                map_row[column.key()] = column.value();
            }
            result.push_back(map_row);
        }
        return result;
    }

    bool MariaDBManager::enqueue(const std::string &query, bool check_correctness) {
        if (!check_correctness)
            return m_queries.enqueue(query);
        if (is_insert_or_replace_query_correct(query)) {
            return m_queries.enqueue(query);
        } else {
            m_logger->send<simple_logger::LogLevel::ERROR>("Query is not correct: " + query);
        }
        return false;
    }

    size_t MariaDBManager::queue_size() {
        return m_queries.size();
    }

    void MariaDBManager::stop(bool force) {
        m_checker_thread_is_running = false;
        if (force) {
            m_queue_thread_is_running = false;
            std::string wipe_out_queries;
            m_queries.wipeout();
            return;
        }
        while (m_queries.size() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        m_queue_thread_is_running = false;
    }

    void MariaDBManager::run() {
        m_queue_thread_is_running = true;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        {
            std::lock_guard<std::mutex> lock(m_write_mutex);
            this->m_get_connection(m_conn_write);
        }
        while (m_queue_thread_is_running) {
//            m_logger->send<simple_logger::LogLevel::DEBUG>("Queue thread running");
            if (!m_is_connected(m_conn_write)) {
                m_logger->send<simple_logger::LogLevel::INFORMATIONAL>("Thread Connection to database failed: " + m_config.uri);
                // sleep for 1 second
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                m_get_connection(m_conn_write);
            }
            if (m_multi_insert) {
                std::vector<std::string> queries;
                while (m_queries.size() > 0) {
                    m_logger->send<simple_logger::LogLevel::DEBUG>("Queue size: " + std::to_string(m_queries.size()));
                    queries.push_back(m_dequeue());
                }
                if (!queries.empty()) {
                    if (!m_insert_multi(queries)) { // if insert fails, try individual m_insert
                        for (auto &query: queries) {
                            if (!m_insert(query)) // if m_insert fails, log error and discard query
                                m_logger->send<simple_logger::LogLevel::ERROR>(
                                        "DISCARD QUERY: " + query);
                        }
                    }
                }
            } else {
                std::string query = m_dequeue();
                if (!query.empty()) {
                    if (!m_insert(query)) // if m_insert fails, enqueue again
                        enqueue(query);
                }
            }
        }
        m_logger->send<simple_logger::LogLevel::DEBUG>("Queue thread stopped");
    }

    void MariaDBManager::m_run_checker() {
        m_checker_thread_is_running = true;
        std::this_thread::sleep_for(std::chrono::seconds(m_config.checker_time));
        while (m_checker_thread_is_running) {
            m_logger->send<simple_logger::LogLevel::DEBUG>("MariaDBManager Checker thread running");
            if (!this->m_is_connected(m_conn_read)) {
                m_logger->send<simple_logger::LogLevel::INFORMATIONAL>(
                        "MariaDBManager Checker Read Connection to database failed: " + m_config.uri);
                std::lock_guard<std::mutex> lock(m_read_mutex);
                m_get_connection(m_conn_read);
            }
            if (!this->m_is_connected(m_conn_write)) {
                m_logger->send<simple_logger::LogLevel::INFORMATIONAL>(
                        "MariaDBManager Checker Write Connection to database failed: " + m_config.uri);
                std::lock_guard<std::mutex> lock(m_write_mutex);
                m_get_connection(m_conn_read);
            }
            std::this_thread::sleep_for(std::chrono::seconds(m_config.checker_time));
        }
    }

    bool MariaDBManager::m_insert(const std::string &query) {
        if (query.empty()) {
            return true;
        }
        try {
            std::lock_guard<std::mutex> lock(m_write_mutex);
            std::unique_ptr<sql::Statement> stmt(m_conn_write->createStatement());
            stmt->execute(query);
        } catch (sql::SQLException &e) {
            m_error_counter++;
            m_logger->send<simple_logger::LogLevel::ERROR>("INSERT failed: " + std::string(e.what()) + " QUERY: " +
                                                           query);
            return false;
        }
        return true;
    }

    bool MariaDBManager::m_insert_multi(const std::vector<std::string> &queries) {
        bool success = true;
        std::stringstream multi_query;

        try {

            multi_query << "START TRANSACTION;";
            for (const auto &query: queries) {
                if (query.empty())
                    continue;
                if (query.back() == ';') {
                    multi_query << query;
                } else {
                    multi_query << query << ";";
                }
            }
            multi_query << "COMMIT;";

            std::lock_guard<std::mutex> lock(m_write_mutex);
            std::unique_ptr<sql::Statement> stmt(m_conn_write->createStatement());
            stmt->execute(multi_query.str());

        } catch (sql::SQLException &e) {
            // if fails, rollback
            m_logger->send<simple_logger::LogLevel::ERROR>("Multi INSERT failed: " + std::string(e.what()));
            m_conn_write->rollback();
            success = false;
        }
        return success;
    }

    std::string MariaDBManager::m_dequeue() {
        std::string query;
        if (m_queries.dequeue_blocking(query)) { // false if queue is empty or retry limit is reached
            return query;
        } else {
            return {};
        }
    }

    bool MariaDBManager::is_connected() {
        return this->m_is_connected(m_conn_write) && this->m_is_connected(m_conn_read);
    }

    bool MariaDBManager::is_thread_running() {
        if (m_queue_thread.joinable() && m_queue_thread_is_running) {
            return m_queue_thread_is_running;
        }
        return false;
    }

    void MariaDBManager::set_multi_insert(bool multi_insert) {
        m_multi_insert = multi_insert;
    }

    std::unique_ptr<sql::ResultSet> MariaDBManager::query(const std::string &query) {
        const int max_retries = 3; // Max retries for query
        for (int attempt = 0; attempt < max_retries; ++attempt) {
            try {
                if (!this->is_connected()) {
                    m_conn_read->reconnect();
                }
                std::unique_ptr<sql::Statement> _stmnt(m_conn_read->createStatement());
                std::unique_ptr<sql::ResultSet> res(_stmnt->executeQuery(query));
                return res;
            } catch (sql::SQLException &e) {
                m_logger->send<simple_logger::LogLevel::ERROR>("MariadbClient query ERROR: " + std::string(e.what()));
                if (attempt == max_retries - 1) {
                    throw; // last attempt, throw exception
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (attempt + 1))); //  Wait 100ms, 200ms, 300ms
        }
        throw std::runtime_error("Max retries reached for MariaDB query.");
    }

    json MariaDBManager::query_to_json(const std::string &query) {
        return simple_mariadb::client::MariaDBManager::resultset_to_json(*this->query(query));
    }

//    json MariaDBManager::resultset_to_json(sql::ResultSet &res) {
//        json result;
//        auto meta = res.getMetaData();
//        const size_t numColumns = meta->getColumnCount();
//        std::vector<std::string> columnNames;
//        std::vector<sql::DataType> columnTypes;
//
//        for (int i = 1; i <= numColumns; ++i) {
//            columnNames.emplace_back(meta->getColumnName(i));
//            columnTypes.push_back(static_cast<const sql::Types>(meta->getColumnType(i)));
//        }
//
//        while (res.next()) {
//            json row;
//            for (int i = 0; i < numColumns; ++i) {
//                const auto &columnName = columnNames[i];
//                const auto &columnType = columnTypes[i];
//
//                switch (columnType) {
//                    case sql::INTEGER:
//                        row[columnName] = res.getInt(columnName);
//                        break;
//                    case sql::BIGINT:
//                        row[columnName] = res.getInt64(columnName);
//                        break;
//                    case sql::BOOLEAN:
//                        row[columnName] = res.getBoolean(columnName);
//                        break;
//                    case sql::DOUBLE:
//                        row[columnName] = res.getDouble(columnName);
//                        break;
//                    case sql::FLOAT:
//                    case sql::REAL: // REAL is often just a synonym for FLOAT
//                        row[columnName] = res.getFloat(columnName);
//                        break;
//                    case sql::DATE:
//                    case sql::TIME:
//                    case sql::TIME_WITH_TIMEZONE:
//                    case sql::TIMESTAMP:
//                    case sql::TIMESTAMP_WITH_TIMEZONE:
//                        row[columnName] = res.getString(i); // And also the timestamp
//                        break;
//                        // Add cases for other types as necessary
//                    default:
//                        // For all other types, default to string representation
//                        row[columnName] = res.getString(columnName);
//                        break;
//                }
//            }
//            result.push_back(row);
//        }
//        return result;
//    }

    json MariaDBManager::resultset_to_json(sql::ResultSet &res) {
        json result;
        auto meta = res.getMetaData();
        const size_t numColumns = meta->getColumnCount();

        while (res.next()) {
            json row;
            for (size_t i = 1; i <= numColumns; ++i) {
                const std::string columnName = std::string(meta->getColumnName(i));
                const auto columnType = static_cast<sql::DataType>(meta->getColumnType(i));

                if (res.wasNull()) {
                    row[columnName] = nullptr;
                    continue;
                }


                try {
                    switch (columnType) {
                        case sql::INTEGER:
                            row[columnName] = res.getInt(columnName);
                            break;
                        case sql::BIGINT:
                            row[columnName] = res.getInt64(columnName);
                            break;
                        case sql::BOOLEAN:
                            row[columnName] = res.getBoolean(columnName);
                            break;
                        case sql::DOUBLE:
                            row[columnName] = res.getDouble(columnName);
                            break;
                        case sql::FLOAT:
                        case sql::REAL: // REAL is often just a synonym for FLOAT
                            row[columnName] = res.getFloat(columnName);
                            break;
                        case sql::DATE:
                        case sql::TIME:
                        case sql::TIME_WITH_TIMEZONE:
                        case sql::TIMESTAMP:
                        case sql::TIMESTAMP_WITH_TIMEZONE:
                            row[columnName] = res.getString(columnName); // Use getString for date/time types
                            break;
                            // Add cases for other types as necessary
                        default:
                            // For all other types, default to string representation
                            row[columnName] = res.getString(columnName);
                            break;
                    }
                } catch (sql::SQLException &e) {
                    std::cout << "columnName: " << columnName << " type: " << columnType << std::endl;
                    std::cout << "MariaDBManager::resultset_to_json ERROR " << e.what() << std::endl;

                    row[columnName] = nullptr;
                }
            }
            result.push_back(row);
        }
        return result;
    }


    bool MariaDBManager::ping() {
        try {
            std::string query = "select 1;";
            json is_one = this->query_to_json(query);
            return is_one.size() == 1 && is_one[0].contains("1") && is_one[0]["1"] == 1;
        } catch (std::exception &gc) {
            m_logger->send<simple_logger::LogLevel::ERROR>("Ping ERROR: " + std::string(gc.what()));
            return false;
        }
    }

    bool MariaDBManager::drop_table(const std::string &table_name) {
        try {
            std::lock_guard<std::mutex> lock(m_write_mutex);
            std::unique_ptr<sql::Statement> _stmnt(m_conn_write->createStatement());
            _stmnt->execute("DROP TABLE IF EXISTS " + table_name);
            return true;
        } catch (std::exception &gc) {
            m_logger->send<simple_logger::LogLevel::ERROR>("drop_table ERROR: " + table_name + " " + gc.what());
            return false;
        }
    }

    bool
    MariaDBManager::create_table(const std::string &table_name, const std::map<std::string, std::string> &columns) {
        try {
            std::string base_query = "CREATE TABLE IF NOT EXISTS `" + table_name + "` (";
            for (auto &column: columns) {
                base_query += "`" + column.first + "` " + column.second + ",";
            }   // Remove last comma
            base_query.pop_back();
            base_query += ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;";
            {
                std::lock_guard<std::mutex> lock(m_write_mutex);
                std::unique_ptr<sql::Statement> _stmnt(m_conn_write->createStatement());
                _stmnt->execute(base_query);
            }
            return true;
        } catch (std::exception &gc) {
            m_logger->send<simple_logger::LogLevel::ERROR>("create_table ERROR: " + table_name + " " + gc.what());
            return false;
        }
    }

    bool MariaDBManager::add_columns_to_table(const std::string &table_name,
                                              const std::map<std::string, std::string> &columns) {
        try {
            std::string base_query = "ALTER TABLE `" + table_name + "` ";
            for (auto &column: columns) {
                base_query += "ADD `" + column.first + "` " + column.second + ",";
            }   // Remove last comma
            base_query.pop_back();
            base_query += ";";
            {
                std::lock_guard<std::mutex> lock(m_write_mutex);
                std::unique_ptr<sql::Statement> _stmnt(m_conn_write->createStatement());
                _stmnt->execute(base_query);
            }
            return true;
        } catch (std::exception &gc) {
            m_logger->send<simple_logger::LogLevel::ERROR>(
                    "add_columns_to_table ERROR: " + table_name + " " + gc.what());
            return false;
        }
    }

    std::map<std::string, std::string> MariaDBManager::get_table_columns(const std::string &table_name) {
        try {
            std::lock_guard<std::mutex> lock(m_read_mutex);
            std::unique_ptr<sql::Statement> _stmnt(m_conn_read->createStatement());
            std::unique_ptr<sql::ResultSet> res(_stmnt->executeQuery("SHOW COLUMNS FROM " + table_name));
            std::map<std::string, std::string> columns;
            while (res->next()) {
                columns[std::string(res->getString("Field"))] = common::to_upper(
                        std::string(res->getString("Type")));
            }
            return columns;
        } catch (std::exception &gc) {
            m_logger->send<simple_logger::LogLevel::ERROR>("get_table_columns ERROR: " + table_name + " " + gc.what());
            return {};
        }
    }

    bool MariaDBManager::create_index_unique(const std::string &table_name, const std::string &index_name,
                                             const std::vector<std::string> &columns) {
        try {
            std::string base_query = "CREATE UNIQUE INDEX `" + index_name + "` ON `" + table_name + "` (";
            for (auto &column: columns) {
                base_query += "`" + column + "`,";
            }   // Remove last comma
            base_query.pop_back();
            base_query += ");";
            {
                std::lock_guard<std::mutex> lock(m_write_mutex);
                std::unique_ptr<sql::Statement> _stmnt(m_conn_write->createStatement());
                _stmnt->execute(base_query);
            }
            return true;
        } catch (std::exception &gc) {
            m_logger->send<simple_logger::LogLevel::ERROR>(
                    "create_index_unique ERROR: " + table_name + " " + gc.what());
            return false;
        }
    }

    bool MariaDBManager::create_index(const std::string &table_name, const std::string &index_name,
                                      const std::vector<std::string> &columns) {
        try {
            std::string base_query = "CREATE INDEX `" + index_name + "` ON `" + table_name + "` (";
            for (auto &column: columns) {
                base_query += "`" + column + "`,";
            }   // Remove last comma
            base_query.pop_back();
            base_query += ");";
            {
                std::lock_guard<std::mutex> lock(m_write_mutex);
                std::unique_ptr<sql::Statement> _stmnt(m_conn_write->createStatement());
                _stmnt->execute(base_query);
            }
            return true;
        } catch (std::exception &gc) {
            m_logger->send<simple_logger::LogLevel::ERROR>("create_index ERROR: " + table_name + " " + gc.what());
            return false;
        }
    }

    size_t  MariaDBManager::get_error_counter() {
        size_t error_counter = m_error_counter;
        m_error_counter = 0;
        return error_counter;
    }

    void MariaDBManager::clear_queue() {
        m_queries.wipeout();
    }

}

