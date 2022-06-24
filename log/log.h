//
// Created by jackqzhou on 2022/6/21.
//

#ifndef ZQTINYWEBSERVER_LOG_H
#define ZQTINYWEBSERVER_LOG_H

#include <stdio.h>
#include <string>
#include "block_queue.h"

using namespace std;

const int MAX_LEN = 128;

class Log {
public:
    static Log* get_instance() {
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread(void *) {
        Log::get_instance()->async_write_log();
    }

    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 500000, int max_queue_size = 0);
    void write_log(int level, const char *format, ...);
    void flush();

private:
    Log();
    virtual ~Log();

    void async_write_log() {
        string single_line;
        while (log_queue_->pop(single_line)) {
            mutex_.lock();
            fputs(single_line.c_str(), fp_);
            mutex_.unlock();
        }
    }

    char dir_name_[MAX_LEN]; // 路径名
    char log_name_[MAX_LEN]; // 日志文件名
    int max_split_lines_;    // 日志最大行数
    int log_buf_size_; // 日志缓冲区大小
    long long count_; // 日志行数记录
    int today_; // 记录当前的天数
    FILE *fp_; // 文件指针
    char *buf_;
    BlockQueue<string> *log_queue_; // log_queue
    bool is_async_; // 是否异步标志
    Locker mutex_;
    int close_log_; // 关闭日志
};

// 日志开关对外暴露给不同模块 此处要求后续模块close_log_变量强耦合
#define LOG_DEBUG(format, ...) if(0 == close_log_) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == close_log_) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == close_log_) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == close_log_) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif //ZQTINYWEBSERVER_LOG_H
