#pragma once 

#include "noncopyable.h"


class InetAddress;

//封装socket类
class Socket : noncopyable
{
public:
//创建一个socket描述符,防止隐式转换对象
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    { }

    ~Socket();
    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress &localaddr);
    void listen();
    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);  //设置TCP_NODELAY选项，禁用Nagle算法，不缓冲数据
    void setReuseAddr(bool on);   //设置SO_REUSEADDR选项，允许重用本地地址和端口
    void setReusePort(bool on);   //设置SO_REUSEPORT选项，允许多个进程或线程绑定同一个端口号
    void setKeepAlive(bool on);   //设置SO_KEEPALIVE选项，保活机制，探测空闲连接

private:
    const int sockfd_;
};