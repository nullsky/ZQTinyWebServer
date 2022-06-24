//
// Created by jackqzhou on 2022/6/12.
//

#ifndef ZQTINYWEBSERVER_BLOCK_QUEUE_H
#define ZQTINYWEBSERVER_BLOCK_QUEUE_H

#include "locker.h"
#include <sys/time.h>

template<typename T>
class BlockQueue {
public:
    BlockQueue(int max_size = 1000) {
        if (max_size < 0) {
            exit(-1);
        }

        array_ = new T[max_size];
        max_size_ = max_size;
        front_ = 0;
        back_ = 0;
        size_ = 0;
    }

    ~BlockQueue() {
        mutex_.lock();
        if (array_ != nullptr) {
            delete [] array_;
        }
        mutex_.unlock();
    }

    bool full() {
        mutex_.lock();
        if (max_size_ > size_) {
            mutex_.unlock();
            return true;
        }
        mutex_.unlock();
        return false;
    }

    bool empty() {
        mutex_.lock();
        if (size_ == 0) {
            mutex_.unlock();
            return true;
        }
        mutex_.unlock();
        return false;
    }

    bool front(T &value) {
        mutex_.lock();
        if (size_ == 0) {
            mutex_.unlock();
            return false;
        }

        value = array_[front_];
        mutex_.unlock();
        return true;
    }

    bool back(T &value) {
        mutex_.lock();
        if (size_ == 0) {
            mutex_.unlock();
            return false;
        }

        value = array_[back_];
        mutex_.unlock();
        return true;
    }

    int size() {
        int tmp = 0;
        mutex_.lock();
        tmp = size_;
        mutex_.unlock();
        return tmp;
    }

    int max_size() {
        int tmp = 0;
        mutex_.lock();
        tmp = max_size_;
        mutex_.unlock();
        return tmp;
    }

    void clear() {
        mutex_.lock();
        size_ = 0;
        front_ = 0;
        back_ = 0;
        mutex_.unlock();
    }

    bool push(const T& item) {
        mutex_.lock();
        if (size_ >= max_size_) {
            cond_.boardcast();
            mutex_.unlock();
            return false;
        }

        int index = (back_ + 1) % max_size_;
        array_[index] = item;
        back_ = index;
        size_++;
        cond_.boardcast();
        mutex_.unlock();
        return true;
    }

    bool pop(T &item) {
        mutex_.lock();
        while (size_ <= 0) {
            if(!cond_.wait(mutex_.get())) {
                mutex_.unlock();
                return false;
            }
        }

        item = array_[front_];
        front_ = (front_ + 1) % max_size_;
        size_--;
        mutex_.unlock();
        return true;
    }

    bool pop(T &item, int ms_timeout) {
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, NULL);
        mutex_.lock();
        if (size_ <= 0) {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if(!cond_.timewait(mutex_.get(), t)) {
                mutex_.unlock();
                return false;
            }
        }

        if (size_ <= 0) {
            mutex_.unlock();
            return false;
        }

        item = array_[front_];
        front_ = (front_ + 1) % max_size_;
        size_--;
        mutex_.unlock();
        return true;
    }

private:
    Locker mutex_;
    Cond cond_;

    T *array_;
    int size_;
    int max_size_;
    int front_;
    int back_;
};

#endif //ZQTINYWEBSERVER_BLOCK_QUEUE_H
