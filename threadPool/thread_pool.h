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
    // 工作线程运行的函数，它不断从工作队列中取出任务并执行之
    static void *worker(void *arg);
    void run();

private:
    int thread_number_; // 线程池的线程数量
    int max_requests_; // 请求队列允许的最大请求数
    pthread_t *threads_; // 描述线程池的数组
    std::list<T *> work_queue_; // 请求队列
    Locker queue_locker_; // 保护请求队列的互斥锁
    Sem queue_state_; // 是否有任务需要处理
    ConnectPool *conn_pool_; // 数据库连接池
    int actor_mode_; // 模式切换
};

template<typename T>
ThreadPool<T>::ThreadPool(int actor_mode, ConnectPool *conn_pool, int thread_number = 8, int max_request = 10000)
    : actor_mode_(actor_mode), conn_pool_(conn_pool), thread_number_(thread_number), max_requests_(max_request), threads_(nullptr) {
    if (thread_number <= 0 || max_request <= 0) {
        throw std::exception();
    }

    threads_ = new pthread_t [thread_number];
    for (int i = 0; i < thread_number; i++) {
        if (pthread_create(threads_ + i, nullptr, worker, this) != 0) {
            delete[] threads_;
            throw std::exception();
        }
        if (pthread_detach(threads_[i])) {
            delete[] threads_;
            throw std::exception();
        }
    }
}

template<typename T>
ThreadPool<T>::~ThreadPool() {
    delete[] threads_;
}

template<typename T>
bool ThreadPool<T>::append(T *request, int state) {
    queue_locker_.lock();
    if (work_queue_.size() >= max_requests_) {
        queue_locker_.unlock();
        return false;
    }

    // 修改事件类型
    request->state_ = state;
    // 添加任务
    work_queue_.push_back(request);
    queue_locker_.unlock();

    // 信号量提醒有任务要处理
    queue_state_.post();
    return true;
}

template<typename T>
bool ThreadPool<T>::append_p(T *request) {
    queue_locker_.lock();
    if (work_queue_.size() >= max_requests_) {
        queue_locker_.unlock();
        return false;
    }

    work_queue_.push_back(request);
    queue_locker_.unlock();
    queue_state_.post();
    return true;
}

template<typename T>
void *ThreadPool<T>::worker(void *arg) {
    ThreadPool<T> *pool = (ThreadPool<T> *)arg;
    pool->run();
    return pool;
}

template<typename T>
void ThreadPool<T>::run() {
    while (1) {
        queue_state_.wait();
        queue_locker_.lock();
        if (work_queue_.empty()) {
            queue_locker_.unlock();
            continue;
        }

        T *request = work_queue_.front();
        work_queue_.pop_front();
        queue_locker_.unlock();
        if (!request) {
            continue;
        }

        if (1 == actor_mode_) {
            // TODO:
        } else {
            // 处理数据库连接
        }
    }
}

#endif //ZQTINYWEBSERVER_THREAD_POOL_H
