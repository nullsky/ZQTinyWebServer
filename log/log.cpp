//
// Created by jackqzhou on 2022/6/21.
//

#include "log.h"
#include <pthread.h>
#include <string.h>
#include <stdarg.h>

Log::Log() {
    count_ = 0;
    close_log_ = false;
}

Log::~Log() {
    if (fp_) {
        fclose(fp_);
    }
}

//异步需要设置阻塞队列的长度，同步不需要设置
bool Log::init(const char *file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size) {
    //如果设置了max_queue_size,则设置为异步
    if (max_queue_size > 0) {
        is_async_ = true;
        log_queue_ = new BlockQueue<string>(max_queue_size);
        pthread_t tid;
        //flush_log_thread为回调函数,这里表示创建线程异步写日志
        pthread_create(&tid, NULL, flush_log_thread, NULL);
    }
    close_log_ = close_log;
    log_buf_size_ = log_buf_size;
    buf_ = new char [log_buf_size_]();
    max_split_lines_ = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char *p = strrchr(file_name, '/'); // 返回 str 中最后一次出现字符 c 的位置
    char log_full_name[256] = {0};
    if (p == NULL) {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    } else {
        strcpy(log_name_, p + 1);
        strncpy(dir_name_, file_name, p - file_name + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name_, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name_);
    }

    today_ = my_tm.tm_mday;

    fp_ = fopen(log_full_name, "a");
    if (fp_ == NULL) {
        return false;
    }
    return true;
}

void Log::write_log(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};
    switch (level)
    {
        case 0:
            strcpy(s, "[debug]:");
            break;
        case 1:
            strcpy(s, "[info]:");
            break;
        case 2:
            strcpy(s, "[warn]:");
            break;
        case 3:
            strcpy(s, "[erro]:");
            break;
        default:
            strcpy(s, "[info]:");
            break;
    }
    // 写入一个log，对m_count++, m_split_lines最大行数
    mutex_.lock();
    count_++;
    if (today_ != my_tm.tm_mday || count_ % max_split_lines_ == 0) {
        //everyday log
        char new_log[256] = {0};
        fflush(fp_);
        fclose(fp_);
        char tail[16] = {0};
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        if (today_ != my_tm.tm_mday) {
            snprintf(new_log, 255, "%s%s%s", dir_name_, tail, log_name_);
            today_ = my_tm.tm_mday;
            count_ = 0;
        } else {
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name_, tail, log_name_, count_ / max_split_lines_);
        }
        fp_ = fopen(new_log, "a");
    }
    mutex_.unlock();

    va_list valst; // VA_LIST 是在C语言中解决变参问题的一组宏
    va_start(valst, format);
    string log_str;

    mutex_.lock();
    //写入的具体时间内容格式
    int n = snprintf(buf_, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

    int m = vsnprintf(buf_ + n, log_buf_size_ - n - 2, format, valst);
    buf_[n + m] = '\n';
    buf_[n + m + 1] = '\0';
    log_str = buf_;
    mutex_.unlock();

    if (is_async_ && !log_queue_->full()) {
        log_queue_->push(log_str);
    } else {
        mutex_.lock();
        fputs(log_str.c_str(), fp_);
    }
    va_end(valst);
}

void Log::flush(void) {
    mutex_.lock();
    //强制刷新写入流缓冲区
    fflush(fp_);
    mutex_.unlock();
}
