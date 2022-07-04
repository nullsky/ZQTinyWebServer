//
// Created by jackqzhou on 2022/6/24.
//

#ifndef ZQTINYWEBSERVER_SQL_CONNECTION_POOL_H
#define ZQTINYWEBSERVER_SQL_CONNECTION_POOL_H

#include <string>
#include <list>
#include <mysql.h>
#include "locker.h"

using namespace std;

class ConnectPool {
public:
    MYSQL *getConnnection(); // 获取数据库连接
    bool releaseConnection(MYSQL *conn); // 释放连接
    int getFreeConn(); // 获取连接
    void destoryPool(); // 销毁连接池

    static ConnectPool *instance() {
        static ConnectPool pool;
        return &pool;
    }

    void init(string url, string user, string password, string db_name, int port, int max_conn, int close_log);

private:
    ConnectPool();
    ~ConnectPool();

    int max_conn_; // 最大连接数
    int cur_conn_; // 当前已经使用的连接数
    int free_conn_; // 当前空闲连接数
    Locker lock_;
    list<MYSQL *> conn_list; // 连接池
    Sem reserve_;

private:
    string url_; // 主机
    int port_;   // 端口
    string user_; // 用户名
    string password_; // 密码
    string db_name_; // 数据库名字

    int close_log_;
};

class ConnectionRAII {
public:
    ConnectionRAII(MYSQL **conn, ConnectPool *connect_pool);
    ~ConnectionRAII();

private:
    MYSQL *conn_;
    ConnectPool *pool_;
};

#endif //ZQTINYWEBSERVER_SQL_CONNECTION_POOL_H
