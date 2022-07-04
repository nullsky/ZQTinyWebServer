//
// Created by jackqzhou on 2022/6/24.
//

#include "sql_connection_pool.h"
#include "log.h"

ConnectPool::ConnectPool() {
    max_conn_ = 0;
    cur_conn_ = 0;
    free_conn_ = 0;
}

ConnectPool::~ConnectPool() {
    destoryPool();
}

void
ConnectPool::init(string url, string user, string password, string db_name, int port, int max_conn, int close_log) {
    url_ = url;
    user_ = user;
    password_ = password;
    db_name_ = db_name;
    port_ = port;
    max_conn_ = max_conn;
    close_log_ = close_log;

    for (int i = 0; i < max_conn; i++) {
        MYSQL *conn = nullptr;
        conn = mysql_init(conn);
        if (!conn) {
            LOG_ERROR("mysql error.");
            exit(1);
        }
        conn = mysql_real_connect(conn, url.c_str(), user.c_str(), password.c_str(), db_name.c_str(), port, nullptr, 0);
        if (!conn) {
            LOG_ERROR("mysql error, error fun: mysql_real_connect");
            exit(1);
        }
        conn_list.push_back(conn);
        ++free_conn_;
    }

    reserve_ = Sem(free_conn_);
}

int ConnectPool::getFreeConn() {
    return this->free_conn_;
}

//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *ConnectPool::getConnnection() {
    MYSQL *conn = nullptr;
    if (conn_list.empty()) {
        return nullptr;
    }

    reserve_.wait();

    lock_.lock();
    conn = conn_list.front();
    conn_list.pop_front();
    --free_conn_;
    ++cur_conn_;
    lock_.unlock();

    return conn;
}

bool ConnectPool::releaseConnection(MYSQL *conn) {
    if (conn == nullptr) {
        return false;
    }

    lock_.lock();
    conn_list.push_back(conn);
    ++free_conn_;
    --cur_conn_;
    lock_.unlock();

    reserve_.post();

    return true;
}

void ConnectPool::destoryPool() {
    lock_.lock();
    if (!conn_list.empty()) {
        list<MYSQL *>::iterator it;
        for (it = conn_list.begin(); it != conn_list.end(); ++it) {
            mysql_close(*it);
        }
        cur_conn_ = 0;
        free_conn_ = 0;
        conn_list.clear();
    }
    lock_.unlock();
}

ConnectionRAII::ConnectionRAII(MYSQL **conn, ConnectPool *connect_pool) {
    *conn = connect_pool->getConnnection();
    conn_ = *conn;
    pool_ = connect_pool;
}

ConnectionRAII::~ConnectionRAII() {
    pool_->releaseConnection(conn_);
}
