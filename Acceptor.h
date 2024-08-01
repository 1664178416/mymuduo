#pragma once 

#include "noncopyable.h"
#include "Channel.h"
#include "Socket.h" 

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop   *loop, const InetAddress &listenAddr,bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listenning () const { return listenning_; }
    void listen();
private:
    void heandleRead();
    EventLoop* loop_; // Acceptor用的就是用户定义的那个baseloop，也称作mainloop
    Socket acceptSocket_;  // Acceptor的socket
    Channel acceptChannel_;  // Acceptor的channel
    NewConnectionCallback newConnectionCallback_;  // 有新连接到来时的回调函数
    bool listenning_;  // 是否正在监听
};