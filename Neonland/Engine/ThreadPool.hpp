#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool final {
public:
    static const uint32_t ThreadCount;
    
    static ThreadPool& GetInstance();
    
    ThreadPool(const ThreadPool&) = delete;
    void operator=(const ThreadPool&) = delete;
    
    ~ThreadPool();
    
    std::future<void> SubmitJob(std::function<void()> func);
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;
    
    std::mutex jobMutex;
    std::condition_variable jobCondition;
    
    bool destructing;
    
    ThreadPool();
    
    void ThreadLoop();
};
