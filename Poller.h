#pragma once

#include<vector> 
#include<unordered_map>

#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

//muduo库中多路事件分发器的核心IO复用模块
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;
    //基类中抽象类保留了统一的接口，派生类中有不同的自定义实现 ，实现在EventLoop中统一的接口来实现不同的IO复用机制
    //为所有的IO复用机制保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    //移除Channel时必须在一个IO线程中执行
    virtual void removeChannel(Channel* channel) = 0;
    
    //判断参数channel是否在当前poller当中
    bool hasChannel(Channel* channel) const; 

    // EventLoop可以通过该接口获取默认的IO复用的具体实现
    //这个不能到Poller.cpp里面实现，因为Poller是一个抽象类，不能实例化
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    //map的key:sockfd,value:sockfd所属的channel
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_; //fd到Channel*的映射
private:
    EventLoop* ownerLoop_; //定义Poller所属的事件循环EventLoop
};