//
// Created by Joaquin Bejar Garcia on 7/11/23.
//

#include "simple_mariadb/client.h"

namespace simple_mariadb::client {

    bool is_insert_or_replace_query_correct(const std::string &query) {
        return std::regex_match(query, QUERYREGEX);
    }

    MariaDBManager::MariaDBManager(simple_mariadb::config::MariaDBConfig &config) : m_config(config),
                                                                                    m_queue_thread(
                                                                                            &MariaDBManager::run,
                                                                                            this) {
        m_config.logger->send<simple_logger::LogLevel::DEBUG>("MariaDBManager constructor");
        if (!m_config.validate()) {
            throw std::runtime_error("MariaDBConfig is not valid");
        }
        this->get_connection(m_conn_insert);
        this->get_connection(m_conn_select);
    }


    bool MariaDBManager::m_is_connected(std::shared_ptr<sql::Connection> &conn) {
        if (conn == nullptr) {
            m_config.logger->send<simple_logger::LogLevel::DEBUG>("Connection is not defined");
            return false;
        }
        if (conn->isClosed()) {
            m_config.logger->send<simple_logger::LogLevel::DEBUG>(
                    "Connection is not open on host: " + std::string(conn->getHostname()));
            return false;
        }
        return conn->isValid();
    }

    void MariaDBManager::get_connection(std::shared_ptr<sql::Connection> &conn) {
        if (this->m_is_connected(conn)) {
            m_config.logger->send<simple_logger::LogLevel::DEBUG>("MariaDB client already connected");
            return;
        }
        try {
            sql::SQLString url(m_config.uri);
            sql::Properties properties(m_config.get_options());
            conn = std::shared_ptr<sql::Connection>(m_driver->connect(url, properties));

            if (this->m_is_connected(conn)) {
                m_config.logger->send<simple_logger::LogLevel::DEBUG>(
                        "MariaDB is now connected to: " + std::string(conn->getHostname())
                        + " with network timeout: " + std::to_string(conn->getNetworkTimeout()));
                return;
            }
        } catch (sql::SQLException &e) {
            this->m_config.logger->send<simple_logger::LogLevel::ERROR>(
                    "MariaDB client connection failed. Error code: " + std::string(e.what()));
        }
    }

    MariaDBManager::~MariaDBManager() {
        this->stop();
        if (m_queue_thread.joinable()) {
            m_queue_thread.join();
        }
    }

    std::vector<std::map<std::string, std::string>> MariaDBManager::select(const std::string &query) {
        return std::vector<std::map<std::string, std::string>>();
    }

    bool MariaDBManager::enqueue(const std::string &query) {
        if (is_insert_or_replace_query_correct(query)) {
            return m_queries.enqueue(query);
        }
        return false;
    }

    size_t MariaDBManager::queue_size() {
        return m_queries.size();
    }

    void MariaDBManager::stop(bool force) {
        if (force) {
            m_queue_thread_is_running = false;
            std::string wipe_out_queries;
            while (m_queries.size() > 0) { // TODO: do this in common::ThreadQueue
                m_queries.dequeue_blocking(wipe_out_queries);
            }
            return;
        }
        while (m_queries.size() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        m_queue_thread_is_running = false;
    }

    void MariaDBManager::run() {
        m_queue_thread_is_running = true;
        get_connection(m_conn_insert);
        while (m_queue_thread_is_running) {
            m_logger->send<simple_logger::LogLevel::DEBUG>("Queue thread running");
            if (!m_is_connected(m_conn_insert)) {
                m_logger->send<simple_logger::LogLevel::ERROR>("Thread Connection to database failed: " + m_config.uri);
                // sleep for 1 second
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                get_connection(m_conn_insert);
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
//                                enqueue(query);
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
//        m_logger->send<simple_logger::LogLevel::DEBUG>("Queue thread stopped");
    }

    bool MariaDBManager::m_insert(const std::string &query) {
        return false;
    }

    bool MariaDBManager::m_insert_multi(const std::vector<std::string> &queries) {
        return false;
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
        return this->m_is_connected(m_conn_insert) && this->m_is_connected(m_conn_select);
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

}
