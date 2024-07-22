#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

#include<sys/epoll.h>

const int Channel::kNoneEvent = 0;
//EPOLLIN：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
//EPOLLOUT：表示对应的文件描述符可以写；
//EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
//EPOLLRDHUP：表示对应的文件描述符的另一端已关闭；
const int Channel::kWriteEvent = EPOLLOUT;
//EPOLLHUP:表示对应的文件描述符被挂断；

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      tied_(false)
{
}

Channel::~Channel(){

}

//channel的tie方法什么时候调用过
void Channel::tie(const std::shared_ptr<void>& obj){
    tie_ = obj;
    tied_ = true;
}

//当改变channle所表示fd的events事件后，update负责在poller里面更改fd相应的事件epoll_ctl
//EventLoop  => Channellist Poller
void Channel::update(){

    //通过Channel所属的EventLoop，调用epoller的相应方法，注册fd的events事件 
    //add code...
    // loop_->updateChannel(this);
}

//在channel所属的EventLoop中，把当前的channel删除
void Channel::remove(){
    //add code
    //loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime){

    LOG_INFO("channel handleEvent revents:%d", revents_);

    if(tied_){
        std::shared_ptr<void> guard = tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }
    else{
        handleEventWithGuard(receiveTime);
    }
}

//根据poller监听的channel发生的具体事件，由channel调用相应的回调函数
void Channel::handleEventWithGuard(Timestamp receiveTime){
    //被挂断同时不可读
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){
        if(closeCallback_) closeCallback_();
    }
    //出错
    if(revents_ & EPOLLERR){
        if(errorCallback_) errorCallback_();
    }
    //可读
    if(revents_ & (EPOLLIN | EPOLLPRI)){
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }
    //可写
    if(revents_ & EPOLLOUT){
        if(writeCallback_){
            writeCallback_();
        }
    }
}