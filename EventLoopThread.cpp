#include "EventLoopThread.h"    
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),const std::string &name = std::string())
: loop_(nullptr),
  exiting_(false),
  thread_(std::bind(&EventLoopThread::threadFunc,this),name),
  mutex_(),
  cond_(),
  callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();  //等待线程结束
    }
}

//调用startloop的时候 start开新线程执行的是一个新的线程
//下面loop初始化好了才能通知回来本来这个线程

EventLoop* EventLoopThread::startLoop(){
    thread_.start(); //启动底层的新线程

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr){
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}
 
//下面这个方法是在单独的线程里面运行的
// one loop per thread
void EventLoopThread::threadFunc(){
    EventLoop loop; // 创建一个独立的EventLoop对象，每个线程都有一个独立的EventLoop对象

    if(callback_){
        callback_(&loop); //回调函数
    }

    {
        // 唤醒主线程            
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();  
    }

    loop.loop(); //启动事件循环 Eventloop loop => Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
