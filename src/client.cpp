//
// Created by Joaquin Bejar Garcia on 7/11/23.
//

#include "simple_mariadb/client.h"

namespace simple_mariadb::client {

    MariaDBManager::MariaDBManager(simple_mariadb::config::MariaDBConfig &config) : m_config(config) {
        m_config.logger->send<simple_logger::LogLevel::DEBUG>("MariaDBManager constructor");
        if (!m_config.validate()) {
            throw std::runtime_error("MariaDBConfig is not valid");
        }
        this->get_connection(m_conn_insert);
        this->get_connection(m_conn_select);
    }


    bool MariaDBManager::is_connected(std::shared_ptr<sql::Connection> &conn) {
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
        if (this->is_connected(conn)) {
            m_config.logger->send<simple_logger::LogLevel::DEBUG>("MariaDB client already connected");
            return;
        }
        try {
            sql::SQLString url(m_config.uri);
            sql::Properties properties(m_config.get_options());

            conn = std::shared_ptr<sql::Connection>(m_driver->connect(url, properties));

            if (this->is_connected(conn)) {
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

}
