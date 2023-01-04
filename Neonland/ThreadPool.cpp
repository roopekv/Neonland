#include "ThreadPool.hpp"
#include <memory>

const uint32_t ThreadPool::ThreadCount = std::max(1u, std::thread::hardware_concurrency()) - 1;

ThreadPool::ThreadPool()
: destructing{false} {
    if (ThreadCount > 0) {
        threads.reserve(ThreadCount);
        
        for (int i = 0; i < ThreadCount; i++) {
            threads.push_back(std::thread(&ThreadPool::ThreadLoop, this));
        }
    }
    
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock lock(jobMutex);
        destructing = true;
    }
    
    jobCondition.notify_all();
    
    for (auto& thread : threads) {
        thread.join();
    }
}

ThreadPool& ThreadPool::GetInstance() {
    static ThreadPool instance;
    return instance;
}

std::future<void> ThreadPool::SubmitJob(std::function<void()> func) {
    auto task = std::make_shared<std::packaged_task<void()>>(func);
    
    std::function<void()> job = [task] { (*task)(); };
    
    if (ThreadCount > 0) {
        std::unique_lock lock(jobMutex);
        jobs.push(job);
    }
    else {
        job();
    }
    
    jobCondition.notify_one();
    
    return task->get_future();
}

void ThreadPool::ThreadLoop() {
    while (true) {
        std::function<void()> job;
        
        {
            std::unique_lock lock(jobMutex);
            jobCondition.wait(lock, [this] {
                return !jobs.empty() || destructing;
            });
            
            if (destructing && jobs.empty()) {
                return;
            }
            
            job = jobs.front();
            jobs.pop();
        }
        
        job();
    }
}
