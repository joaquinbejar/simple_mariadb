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
#include <common/sql_utils.h>

namespace simple_mariadb::client {

    typedef std::string Query;

    class MariaDBManager {
    public:
        explicit MariaDBManager(simple_mariadb::config::MariaDBConfig &config);

        MariaDBManager(const MariaDBManager &other) = delete;

        MariaDBManager &operator=(const MariaDBManager &other) = delete;

        MariaDBManager(MariaDBManager &&other) = delete;

        MariaDBManager &operator=(MariaDBManager &&other) = delete;

        ~MariaDBManager();

        std::vector<std::map<std::string, std::string>> select(const std::string &query);

        bool enqueue(const std::string &query, bool check_correctness = true);

        size_t queue_size();

        void stop(bool force = false);

        void run();

        bool is_connected();

        bool is_thread_running();

        void set_multi_insert(bool multi_insert);

        std::unique_ptr<sql::ResultSet> query(const std::string &query);

        json query_to_json(const std::string &query);

        static json resultset_to_json(sql::ResultSet &res);

        bool drop_table(const std::string &table_name);

        bool create_table(const std::string &table_name, const std::map<std::string, std::string> &columns);

        bool add_columns_to_table(const std::string &table_name, const std::map<std::string, std::string> &columns);

        std::map<std::string, std::string> get_table_columns(const std::string &table_name);

        bool create_index_unique(const std::string &table_name, const std::string &index_name,
                                 const std::vector<std::string> &columns);

        bool create_index(const std::string &table_name, const std::string &index_name,
                          const std::vector<std::string> &columns);

        bool ping();

        size_t get_error_counter();

        void clear_queue();


    private:
        bool m_is_connected(std::shared_ptr<sql::Connection> &conn);

        sql::Driver *m_driver = sql::mariadb::get_driver_instance();

        void m_get_connection(std::shared_ptr<sql::Connection> &conn);

        bool m_insert(const std::string &query);

        bool m_insert_multi(const std::vector<std::string> &queries);

        std::string m_dequeue();

        void m_run_checker();

        void m_join_threads();

        std::shared_ptr<sql::Connection> m_conn_write;
        std::shared_ptr<sql::Connection> m_conn_read;
        std::mutex m_read_mutex;
        std::mutex m_write_mutex;

        simple_mariadb::config::MariaDBConfig &m_config;
        std::shared_ptr<simple_logger::Logger> m_logger = m_config.logger;
        common::ThreadQueueWithMaxSize<Query> m_queries = common::ThreadQueueWithMaxSize<Query>(m_config.queue_size,
                                                                                                m_config.queue_timeout);
        std::atomic<bool> m_queue_thread_is_running;
        std::atomic<bool> m_checker_thread_is_running;
        std::atomic<size_t> m_error_counter = 0; ///< Counter for errors encountered.
        std::thread m_queue_thread;
        std::thread m_checker_thread;
        bool m_multi_insert = m_config.multi_insert;

    };

}
#endif //SIMPLE_MARIADB_CLIENT_H
