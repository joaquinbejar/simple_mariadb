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

        return true;
    }

    json MariaDBConfig::to_json() const {
        json j;
        j["m_hostname"] = m_hostname;
        j["m_port"] = m_port;
        j["m_user"] = m_user;
        j["m_password"] = m_password;
        j["dbname"] = m_database;
        j["autoreconnect"] = m_autoreconnect;
        j["tcpkeepalive"] = m_tcpkeepalive;
        j["connecttimeout"] = m_connecttimeout;
        j["sockettimeout"] = m_sockettimeout;

        return j;
    }

    void MariaDBConfig::from_json(const json &j) {
        try {
            m_hostname = j.at("m_hostname").get<std::string>();
            m_port = j.at("m_port").get<int>();
            m_user = j.at("m_user").get<std::string>();
            m_password = j.at("m_password").get<std::string>();
            m_database = j.at("dbname").get<std::string>();
            m_autoreconnect = j.at("autoreconnect").get<std::string>();
            m_tcpkeepalive = j.at("tcpkeepalive").get<std::string>();
            m_connecttimeout = j.at("connecttimeout").get<std::string>();
            m_sockettimeout = j.at("sockettimeout").get<std::string>();

        } catch (std::exception &e) {
            throw simple_config::ConfigException(e.what());
        }
    }

    std::string MariaDBConfig::to_string() const {
        return (std::string) "MariaDBConfig{" +
               "hostname=" + m_hostname +
               ", port=" + std::to_string(m_port) +
               ", user=" + m_user +
               ", password=" + m_password +
               ", database=" + m_database +
               ", multi_insert=" + std::to_string(multi_insert) +
               ", autoreconnect=" + m_autoreconnect +
               ", tcpkeepalive=" + m_tcpkeepalive +
               ", connecttimeout=" + m_connecttimeout +
               ", sockettimeout=" + m_sockettimeout +
               ", uri=" + uri +
               '}';
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