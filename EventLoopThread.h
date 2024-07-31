#pragma once 

#include "noncopyable.h"
#include "Thread.h"

//信号量、互斥锁、智能指针等都是C++11的不使用系统的了

#include<functional>
#include<mutex>
#include<string>
#include<condition_variable>

class EventLoop; // 前向声明

class EventLoopThread : noncopyable
{
public:
    //线程初始化回调
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop();
private:    

    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};