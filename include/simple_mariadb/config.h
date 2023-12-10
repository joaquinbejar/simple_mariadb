//
// Created by Joaquin Bejar Garcia on 7/11/23.
//

#ifndef SIMPLE_MARIADB_CONFIG_H
#define SIMPLE_MARIADB_CONFIG_H

#include <simple_config/config.h>
#include <simple_logger/logger.h>
#include <common/common.h>
#include <common/ip.h>
#include <conncpp.hpp>


using json = nlohmann::json;

namespace simple_mariadb::config {

    template<typename T>
    std::string to_sql_literal(T const &value) {
        static std::string const true_literal = "TRUE";
        static std::string const false_literal = "FALSE";
        static std::ostringstream oss;

        oss.str("");

        if constexpr (std::is_same_v<T, bool>) {
            oss << (value ? true_literal : false_literal);
        } else if constexpr (std::is_same_v<T, int>) {
            oss << value;
        } else if constexpr (std::is_same_v<T, double>) {
            oss << value;
        } else if constexpr (std::is_same_v<T, unsigned>) {
            oss << value;
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            oss << "'" << value << "'";
        } else {
            oss << value;
        }
        return oss.str();
    }

    class MariaDBConfig : public simple_config::Config {
    public:

        bool validate() override;

        [[nodiscard]] json to_json() const override;

        void from_json(const json &j) override;

        [[nodiscard]] std::string to_string() const override;

        bool multi_insert = common::get_env_variable_bool("MARIADB_MULTI_INSERT", false);
        int checker_time = common::get_env_variable_int("CHECKER_TIME", 30);

        std::map<sql::SQLString, sql::SQLString> get_options();

    protected:
        std::string m_database = common::get_env_variable_string("MARIADB_DATABASE", "");
        std::string m_password = common::get_env_variable_string("MARIADB_PASSWORD", "");
        std::string m_hostname = common::get_env_variable_string("MARIADB_HOSTNAME", "localhost");
        std::string m_user = common::get_env_variable_string("MARIADB_USER", "");
        int m_port = common::get_env_variable_int("MARIADB_PORT", 3306);
        std::string m_autoreconnect = common::get_env_variable_string("MARIADB_AUTORECONNECT", "true");
        std::string m_tcpkeepalive = common::get_env_variable_string("MARIADB_TCPKEEPALIVE", "true");
        std::string m_connecttimeout = common::get_env_variable_string("MARIADB_CONNECTTIMEOUT", "30");
        std::string m_sockettimeout = common::get_env_variable_string("MARIADB_SOCKETTIMEOUT", "10000");




    public:
        std::string uri = "jdbc:mariadb://" + m_hostname + ":" + std::to_string(m_port) + "/" + m_database;
        std::shared_ptr<simple_logger::Logger> logger = std::make_shared<simple_logger::Logger>(loglevel);

    };

}

#endif //SIMPLE_MARIADB_CONFIG_H
