//muduo库的线程原本是按照Linux底层源代码照写的，但是现在要用c11重写，故可以直接调用thread库（c11封装了   


#pragma once 

#include "noncopyable.h"

#include<iostream>
#include<functional>
#include<thread>
#include<memory>
#include<unistd.h>
#include<string>
#include<atomic>

class Thread : noncopyable
{
public:
    //绑定器和函数对象，C++高级编程
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();  //等待线程结束

    bool started() const { return started_; }   
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }
    static int numCreated() { return numCreated_; }
private:

    void setDefaultName();

    bool started_;
    bool joined_;
    //使用thread库，不再需要c的pthread库
    // pthread_t pthreadId_;
    std::shared_ptr<std::thread> thread_; 
    pid_t tid_;  //线程id
    ThreadFunc func_;
    std::string name_;
    static std::atomic_int numCreated_; //原子操作，防止多线程操作时出现问题，保存的是创建的线程数
};