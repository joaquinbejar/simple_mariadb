//
// Created by Joaquin Bejar Garcia on 7/11/23.
//

#include "simple_mariadb/config.h"

namespace simple_mariadb::config {

    bool MariaDBConfig::validate() {
        if (m_hostname.empty()) {
            logger->send<simple_logger::LogLevel::ERROR>("Hostname is empty");
            return false;
        }
        if (!common::ip::is_a_valid_port(m_port)) {
            logger->send<simple_logger::LogLevel::ERROR>("Port is not valid: " + std::to_string(m_port));
            return false;
        }
        if (m_user.empty()) {
            logger->send<simple_logger::LogLevel::ERROR>("User is empty");
            return false;
        }
        if (m_password.empty()) {
            logger->send<simple_logger::LogLevel::ERROR>("Password is empty");
            return false;
        }
        if (m_database.empty()) {
            logger->send<simple_logger::LogLevel::ERROR>("Database is empty");
            return false;
        }
        if (checker_time <= 0) {
            logger->send<simple_logger::LogLevel::ERROR>("Checker time is not valid: " + std::to_string(checker_time));
            return false;
        }

        return true;
    }

    json MariaDBConfig::to_json() const {
        json j;
        j["hostname"] = m_hostname;
        j["port"] = m_port;
        j["user"] = m_user;
        j["password"] = m_password;
        j["dbname"] = m_database;
        j["autoreconnect"] = m_autoreconnect;
        j["tcpkeepalive"] = m_tcpkeepalive;
        j["connecttimeout"] = m_connecttimeout;
        j["sockettimeout"] = m_sockettimeout;
        j["multi_insert"] = multi_insert;
        j["checker_time"] = checker_time;
        j["queue_size"] = queue_size;
        j["queue_timeout"] = queue_timeout;

        return j;
    }

    void MariaDBConfig::from_json(const json &j) {
        try {
            m_hostname = j.at("hostname").get<std::string>();
            m_port = j.at("port").get<int>();
            m_user = j.at("user").get<std::string>();
            m_password = j.at("password").get<std::string>();
            m_database = j.at("dbname").get<std::string>();
            m_autoreconnect = j.at("autoreconnect").get<std::string>();
            m_tcpkeepalive = j.at("tcpkeepalive").get<std::string>();
            m_connecttimeout = j.at("connecttimeout").get<std::string>();
            m_sockettimeout = j.at("sockettimeout").get<std::string>();
            multi_insert = j.at("multi_insert").get<bool>();
            checker_time = j.at("checker_time").get<int>();
            uri = "jdbc:mariadb://" + m_hostname + ":" + std::to_string(m_port) + "/" + m_database;
            queue_size = j.at("queue_size").get<int>();
            queue_timeout = j.at("queue_timeout").get<int>();

        } catch (std::exception &e) {
            logger->send<simple_logger::LogLevel::CRITICAL>("Error parsing MariaDBConfig: " + std::string(e.what()));
            throw simple_config::ConfigException(e.what());
        }
    }

    std::string MariaDBConfig::to_string() const {
        return R"({"MariaDBConfig":)" + this->get_basic_string() + "}";
    }

    std::map<sql::SQLString, sql::SQLString> MariaDBConfig::get_options() {
        return {
                {"user",           m_user},
                {"password",       m_password},
                {"autoReconnect",  m_autoreconnect},
                {"tcpKeepAlive",   m_tcpkeepalive},
                {"connectTimeout", m_connecttimeout},
                {"socketTimeout",  m_sockettimeout}
        };
    }

}