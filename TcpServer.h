#pragma once 

/*
用户使用muduo编写服务器程序全部包含头文件
*/
#include "noncopyable.h"
#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Logger.h"
#include "Callbacks.h"
#include "Logger.h"
#include "TcpConnection.h"
#include "Buffer.h" 

#include <functional>
#include <memory>
#include <atomic>
#include <unordered_map>


//对外的服务器编程使用的类
class TcpServer : noncopyable
{
public:
    
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop *loop,const InetAddress &listenAddr,const std::string &nameArg,Option option = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb)
    {
        threadInitCallback_ = cb;
    }
    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback &cb)
    {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }

    //设置底层subloop的线程数量
    void setThreadNum(int numThreads);

    //开启服务器
    void start();

private:
    //新连接到来时的回调函数
    void newConnection(int sockfd,const InetAddress &peerAddr);
    //连接断开时的回调函数，从connections_中移除连接
    void removeConnection(const TcpConnectionPtr &conn);
    //连接断开时的回调函数,在loop线程中执行
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string,TcpConnectionPtr>;

    EventLoop *loop_;  //baseloop

    const std::string name_;  //服务器名称
    const std::string ipPort_; //服务器ip和端口
    std::unique_ptr<Acceptor> acceptor_;  //运行在mainloop，Acceptor对象，用于监听新连接
    std::shared_ptr<EventLoopThreadPool> threadPool_;  //线程池对象

    ConnectionCallback connectionCallback_;  //新连接建立时的回调函数
    MessageCallback messageCallback_;  //消息到来时的回调函数
    WriteCompleteCallback writeCompleteCallback_;  //消息发送完毕时的回调函数
    ThreadInitCallback threadInitCallback_;  //loop线程初始化时的回调函数

    std::atomic_int started_;  //标识服务器是否已经启动
    int nextConnId_;  //下一个连接的id
    ConnectionMap connections_;  //保存所有连接
};