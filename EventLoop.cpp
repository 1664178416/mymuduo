#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>

//防止一个线程创建多个EventLoop  thread_local

__thread EventLoop* t_loopInThisThread = nullptr;

//定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

//创建wakeupfd,用来notify唤醒subReactor处理新来的channel
int createEventfd(){
    //创建一个eventfd,用来在IO线程之间传递消息，非阻塞的且线程安全的
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        //LOG_SYSERR << "Failed in eventfd";
        LOG_FATAL("eventfd error: %d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),  //没有开启循环
      quit_(false), //没有退出
      callingPendingFunctors_(false), //没有需要执行的回调操作
      threadId_(CurrentThread::tid()), //当前loop所在线程的tid
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()), //创建wakeupfd
      wakeupChannel_(new Channel(this, wakeupFd_))  //创建wakeupChannel
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    //判断当前线程是否创建了EventLoop对象
    if(t_loopInThisThread){
        //LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }else{
        t_loopInThisThread = this;
    }
    //设置wakeupChannel的事件类型为读事件，回调函数为handleRead
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    //绑定器跟函数对象
    //每个EventLoop对象都关注wakeupChannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){
    //wakeupChannel_不关注任何事件
    wakeupChannel_->disableAll();
    //wakeupChannel_从Poller中移除
    wakeupChannel_->remove();
    //关闭wakeupfd
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

//每一个sub reactor监听了一个wake up channel，main reactor可以通过给wake up channel发送一个write消息，sub reactor就能感知到wakeupfd_有读事件发生

void EventLoop::handleRead(){
    uint64_t one = 1; //64位无符号整数,用来接收wakeupfd的数据
    ssize_t n = read(wakeupFd_, &one, sizeof one);  //读取wakeupfd
    if(n != sizeof one){
        //LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8 \n", n);
    }
}