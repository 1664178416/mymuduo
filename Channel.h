#pragma once 

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include<memory>

//one loop per thread
//一个线程有一个Eventloop，一个Eventloop上有一个poller，poller可以监听多个channel，在Reactor模型上对应事件分发器Demultiplex
//EventLoop（事件分发器Demultiplex）包含Channel和poller
//channal包含了sockfd和其感兴趣的event，如epollin，epollout等，还绑定了poller返回的具体事件，向poller里面注册，poller向channal通知，channel调用已经定义好的回调操作
class EventLoop;
class Timestamp;

/*
Channel类是对epoll事件的封装，是EventLoop和Poller的中间层
理解为通道，封装了sockfd和其感兴趣的event，如epollin，epollout等，还绑定了poller返回的具体事件
*/
class Channel : noncopyable
{
public:
//std::function是一个模板类，用于封装任何可以调用的目标，包括普通函数、Lambda表达式、函数指针、以及具有operator()的对象（如函数对象）。std::function的模板参数定义了它可以封装的可调用目标的签名。
//using关键字用于定义类型别名，使得复杂的类型名称可以用更简单的名字表示。这在模板编程中特别有用，可以使代码更清晰易读。
//EventCallback类型别名：定义了一个不接受参数且不返回任何值的函数类型。这意味着任何符合这一签名的函数、Lambda表达式或可调用对象都可以被EventCallback类型的变量所持有和调用。这在事件处理中非常有用，因为它允许将不同的回调函数绑定到特定事件上，而不需要关心这些回调函数的具体实现。
//ReadEventCallback类型别名：定义了一个接受Timestamp类型参数且不返回任何值的函数类型。这允许将需要时间戳信息的回调函数绑定到特定事件上。例如，在处理某些读事件时，可能需要知道事件发生的确切时间，这时就可以使用ReadEventCallback类型的变量来持有这样的回调函数。
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);

    ~Channel();


    // fd得到poller通知以后，处理事件的调用相应的回调方法
    void handleEvent(Timestamp receiveTime);

    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    //防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    //设置在fd上真正发生的事件
    //void set_revents(int revt) { revents_ = revt; }
    int set_revents(int revt) { revents_ = revt; return revt; }
    
    //设置fd相应的事件状态
    //epoll_creat,epoll_ctl,epoll_wait
    void enableReading() { events_ |= kReadEvent; update();}
    void disableReading() { events_ &= ~kReadEvent; update();}
    void enableWriting() { events_ |= kWriteEvent; update();}
    void disableWriting() { events_ &= ~kWriteEvent; update();}
    void  disableAll() { events_ = kNoneEvent; update(); }

    //返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }
     
    EventLoop* owerLoop() { return loop_; }
    void remove();

private:

    void update();
    //受保护的处理handleEvent
    void handleEventWithGuard(Timestamp receiveTime);

    //对不同事件感兴趣的回调函数
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_; //事件循环
    const int fd_; //fd,Poller监听的对象,epoll_ctl
    int events_; //关心的事件,注册fd感兴趣的事件
    int revents_; //poller返回的具体事件
    int index_; //用于poller返回的具体事件

    std::weak_ptr<void> tie_;
    bool tied_;

    //因为channel通道里面能够获知fd最终发生的具体时间revents，所以它负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};