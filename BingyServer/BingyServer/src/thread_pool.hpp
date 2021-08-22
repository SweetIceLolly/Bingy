/*
描述: 线程池接口
作者: 冰棍
文件: thread_pool.hpp
*/

#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

class thread_pool_job {
public:
    thread_pool_job(const std::function<void(void *)> &func, void *args);
    std::function<void(void *)> func;
    void *args;
};

class thread_pool {
public:
    std::mutex                  mutexQueue;         // 工作队列锁
    std::condition_variable     condNewJob;         // 新任务信号
    std::condition_variable     condNoJob;          // 所有任务完成信号
    std::queue<thread_pool_job> jobs;               // 工作队列
    bool                        stop;               // 是否停止线程池
    size_t                      workingCount;       // 正在执行工作的线程数
    size_t                      threadCount;        // 总线程数

    size_t                      detachedCount;      // 单独执行的线程数
    std::mutex                  mutexDetached;      // 单独执行的线程数锁

    /**
     * 初始化线程池
     * threadCount: 工作线程数. 默认为系统支持最大线程数
     * maxJobCount: 最大工作数. 0 代表无限制. 默认为无限制
     */
    void init(const size_t &threadCount = std::thread::hardware_concurrency());

    /**
     * 关闭线程池
     * finishRemainingJobs: 是否等待工作队列中所有任务完成. 默认为是
     */
    void shutdown(bool finishRemainingJobs = true);

    /**
     * 往工作队列中添加新任务
     * newJob: 新任务信息
     */
    void addJob(const thread_pool_job &newJob);

    /**
     * 单独创建一条线程来执行指定的任务
     * newJob: 新任务信息
     */
    void addDetachedJob(const thread_pool_job &newJob);

    /**
     * 阻塞调用该函数的线程, 等待工作队列中所有任务完成再继续
     */
    void waitForAllJobsDone();
};
