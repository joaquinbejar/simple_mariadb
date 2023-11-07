//
// Created by Joaquin Bejar Garcia on 7/11/23.
//

#ifndef SIMPLE_MARIADB_CLIENT_H
#define SIMPLE_MARIADB_CLIENT_H

#include <mutex>
#include <simple_color/color.h>
#include <simple_config/config.h>
#include <simple_logger/logger.h>
#include <simple_mariadb/config.h>
#include <regex>
#include <common/common.h>

namespace simple_mariadb::client {


    const std::regex QUERYREGEX(
            R"(^\s*INSERT\s+INTO\s+[a-zA-Z_][a-zA-Z_0-9]*\s*\(([^)]+)\)\s*VALUES\s*\(([^)]+)\)\s*(ON\s+CONFLICT\s*\(([^)]+)\)\s*DO\s+UPDATE\s+SET\s*([^;]+))?\s*;?\s*$)",
            std::regex_constants::icase
    );


    bool is_insert_or_replace_query_correct(const std::string &query);

    class MariaDBManager {
    public:
        explicit MariaDBManager(simple_mariadb::config::MariaDBConfig &config);

        ~MariaDBManager();

        std::vector<std::map<std::string, std::string>> select(const std::string &query);

        bool enqueue(const std::string &query);

        size_t queue_size();

        void stop();

        void run();

        bool is_connected(std::shared_ptr<sql::Connection> &conn);

    private:
        sql::Driver *m_driver = sql::mariadb::get_driver_instance();
        void get_connection(std::shared_ptr<sql::Connection> &conn);

        bool m_insert(const std::string &query);

        bool m_insert_multi(const std::vector<std::string> &queries);

        std::string m_dequeue();

        std::shared_ptr<sql::Connection> m_conn_insert;
        std::shared_ptr<sql::Connection> m_conn_select;
        std::mutex m_select_mutex;
        std::mutex m_insert_mutex;

        simple_mariadb::config::MariaDBConfig &m_config;
        std::shared_ptr<simple_logger::Logger> m_logger = m_config.logger;
        common::ThreadQueue<std::string> m_queries;
        std::atomic<bool> m_queue_thread_is_running;
        std::thread m_queue_thread;
        bool m_multi_insert = m_config.multi_insert;

    };

}
#endif //SIMPLE_MARIADB_CLIENT_H
