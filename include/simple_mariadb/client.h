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

    enum class InsertType {
        INSERT,
        REPLACE,
        IGNORE
    };

    // map of insert types
    static const std::map<InsertType, std::string> insert_type_map = {
            {InsertType::INSERT,  "INSERT INTO "},
            {InsertType::REPLACE, "REPLACE INTO "},
            {InsertType::IGNORE,  "INSERT IGNORE INTO "}
    };

    // funct to replace in a query the string "INSERT INTO", "REPLACE INTO" or "INSERT IGNORE INTO"
    // with the corresponding string for a given InsertType
    void replace_insert_type(std::string &query, const InsertType &insert_type);


    const std::regex QUERYREGEX(
            R"(^\s*(INSERT|REPLACE)\s+INTO\s+[a-zA-Z_][a-zA-Z_0-9]*\s*\(([^)]+)\)\s*VALUES\s*\(([^)]+)\)\s*;?\s*$)",
            std::regex_constants::icase
    );


    bool is_insert_or_replace_query_correct(const std::string &query);

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
        common::ThreadQueue<std::string> m_queries;
        std::atomic<bool> m_queue_thread_is_running;
        std::atomic<bool> m_checker_thread_is_running;
        std::atomic<size_t > m_error_counter = 0; ///< Counter for errors encountered.
        std::thread m_queue_thread;
        std::thread m_checker_thread;
        bool m_multi_insert = m_config.multi_insert;

    };

}
#endif //SIMPLE_MARIADB_CLIENT_H
