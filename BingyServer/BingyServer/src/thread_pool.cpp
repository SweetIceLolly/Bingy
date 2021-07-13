/*
描述: 线程池实现
作者: 冰棍
文件: thread_pool.cpp
*/

#include "thread_pool.hpp"

void worker(thread_pool *pool);

thread_pool_job::thread_pool_job(std::function<void(void*)> func, void* args) {
    this->func = func;
    this->args = args;
}

void thread_pool::init(const size_t& threadCount) {
    if (threadCount < 1)
        return;

    this->threadCount = threadCount;
    this->workingCount = 0;
    this->stop = false;

    for (size_t i = 0; i < threadCount; ++i)
        std::thread(worker, this).detach();
}

void thread_pool::shutdown(const bool& finishRemainingJobs) {
    this->mutexQueue.lock();
    while (!this->jobs.empty()) {
        if (finishRemainingJobs) {
            // 逐个完成工作队列中剩余的工作
            thread_pool_job currentJob = this->jobs.front();
            if (currentJob.func) {
                currentJob.func(currentJob.args);
            }
        }
        this->jobs.pop();
    }
    this->stop = true;
    this->condNewJob.notify_all();  // 并不是真的有新工作, 而是疏通所有等候中的工作线程, 让他们退出
    this->mutexQueue.unlock();
}

void thread_pool::addJob(const thread_pool_job &newJob) {
    this->mutexQueue.lock();
    this->jobs.push(newJob);
    this->condNewJob.notify_all();
    this->mutexQueue.unlock();
}

void thread_pool::waitForAllJobsDone() {
    std::unique_lock<std::mutex> lock(this->mutexQueue);
    for (;;) {
        // 等到没有工作线程且工作队列为空
        if (this->workingCount != 0 || !this->jobs.empty()) {
            this->condNoJob.wait(lock);
        }
        else {
            break;
        }
    }
    lock.unlock();
}

void worker(thread_pool *pool) {
    for (;;) {
        std::unique_lock lock(pool->mutexQueue);

        // 等待新任务信号 (或者停止信号)
        while (pool->jobs.empty() && !pool->stop) {
            pool->condNewJob.wait(lock);
        }

        // 接收到停止信号, 退出线程
        if (pool->stop) {
            --pool->threadCount;
            pool->condNoJob.notify_one();
            lock.unlock();
            return;
        }

        // 从队列中获取任务然后开始执行
        thread_pool_job currentJob = pool->jobs.front();
        pool->jobs.pop();
        ++pool->workingCount;
        lock.unlock();

        if (currentJob.func) {
            currentJob.func(currentJob.args);
        }

        // 任务完成
        lock.lock();
        --pool->workingCount;
        if (pool->jobs.empty() && !pool->stop && pool->workingCount == 0) {
            // 触发所有任务完成信号
            pool->condNoJob.notify_one();
        }
        lock.unlock();
    }
}
