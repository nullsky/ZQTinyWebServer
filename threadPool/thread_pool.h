//
// Created by jackqzhou on 2022/7/3.
//

#ifndef ZQTINYWEBSERVER_THREAD_POOL_H
#define ZQTINYWEBSERVER_THREAD_POOL_H

#include "sql_connection_pool.h"
#include <pthread.h>
#include <list>

template<typename T>
class ThreadPool {
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    ThreadPool(int actor_mode, ConnectPool *conn_pool, int thread_number = 8, int max_request = 10000);
    ~ThreadPool();

    bool append(T *request, int state);
    bool append_p(T *request);

private:
    static void *worker(void *arg);
    void run();

private:
    int thread_number_; // 线程池的线程数量
    int max_requests_; // 请求队列允许的最大请求数
    pthread_t *threads_; // 描述线程池的数组
    std::list<T *> work_queue_; // 请求队列
    Sem queue_state_; // 是否有任务需要处理
    ConnectPool *conn_pool_; // 数据库连接池
    int actor_mode_; // 模式切换
};

template<T>
ThreadPool<T>::ThreadPool(int actor_mode, ConnectPool *conn_pool, int thread_number = 8, int max_request = 10000)
    : actor_mode_(actormode), conn_pool_(conn_pool), thread_number_(thread_number), max_requests_(max_request), threads_(nullptr) {
    if (thread_number <= 0 || max_request <= 0) {
        throw std::exception();
    }

    threads_ = new pthread_t [thread_number];
    
}

#endif //ZQTINYWEBSERVER_THREAD_POOL_H
