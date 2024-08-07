#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

#include<memory>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop,const std::string &nameArg)
    :baseloop_(baseloop),
    name_(nameArg),
    started_(false),
    numThreads_(0),
    next_(0)
{
}

    EventLoopThreadPool::~EventLoopThreadPool(){

    }

    void EventLoopThreadPool::start(const ThreadInitCallback &cb ){
        started_ = true;
        for(int i = 0;i < numThreads_;++i){
            char buf[name_.size() + 32];  //线程池名字再加上下标作为底层线程池名字
            snprintf(buf,sizeof buf,"%s%d",name_.c_str(),i);
            EventLoopThread *t = new EventLoopThread(cb,buf);
            threads_.push_back(std::unique_ptr<EventLoopThread>(t));
            loops_.push_back(t->startLoop());  //底层创建线程，绑定一个新的EventLoop并返回该loop的地址
        }

        //如果整个服务器只有一个线程，运行baseloop
        if(numThreads_ == 0 && cb){
            cb(baseloop_);
        }
    }

    //如果工作在多线程种，baseloop默认以轮询的方式分配channel给subloop
    //如果工作在单线程，baseloop就运行在mainloop中
    //轮询
    EventLoop* EventLoopThreadPool::getNextLoop(){
        EventLoop *loop = baseloop_;
        if(!loops_.empty()){
            loop = loops_[next_];
            next_++;
            if(next_ >= loops_.size())
                next_ = 0;
        }
        return loop;
    }

    std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
    {
        if(loops_.empty())
            return std::vector<EventLoop*>(1,baseloop_);
        else
            return loops_;
    }