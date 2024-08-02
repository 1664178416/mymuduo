#include "TcpServer.h"


#include<strings.h>


//这个函数在TcpConnection中也有一个，报错也是报的这个重复，故忽略
EventLoop* CheckLoopNotNull_Server(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpServer is null \n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}


TcpServer::TcpServer(EventLoop *loop,const InetAddress &listenAddr,const std::string &nameArg,Option option)
    :loop_(CheckLoopNotNull_Server(loop)),
    name_(nameArg),
    ipPort_(listenAddr.toIpPort()),
    acceptor_(new Acceptor(loop,listenAddr,option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop,name_)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1),   //连接的id
    started_(0)  //服务器是否已经启动
{
    //当有新用户连接时，会执行TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
        //占位符，connfd和peerAddr    
}


TcpServer::~TcpServer(){
    for(auto &item : connections_){
        TcpConnectionPtr conn(item.second);  //局部的智能指针 ，出了右括号可以自动释放new出来的TcpConnection对象资源了
        item.second.reset(); //将TcpConnectionPtr置为空
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed,conn));
    }
}

//设置底层subloop的线程数量
void TcpServer::setThreadNum(int numThreads){
    threadPool_->setThreadNum(numThreads);
}
 
//开启服务器  loop_.loop()
void TcpServer::start(){
    if(started_++ == 0){  //防止一个TcpServer对象被start多次
        threadPool_->start(threadInitCallback_); //启动底层线程池
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));  //在主loop中执行acceptor监听操作
    }
}

//Acceptor有一个新的客户端连接，会执行TcpServer::newConnection回调
void TcpServer::newConnection(int sockfd,const InetAddress &peerAddr){
    //轮询算法，选择一个subloop
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf,sizeof buf,"-%s#%d",ipPort_.c_str(),nextConnId_);
    nextConnId_++;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s\n",name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

    //通过sockfd获取其1绑定的本机ip地址以及端口号信息
    sockaddr_in local;
    ::bzero(&local,sizeof local);
    socklen_t addrlen = sizeof local;
    if(::getsockname(sockfd,(sockaddr*)&local,&addrlen) < 0){
        LOG_ERROR("sockets::getLocalAddr");
    }
    InetAddress localAddr(local);

    //根据sockfd创建一个TcpConnection对象
    TcpConnectionPtr conn(new TcpConnection(
        ioLoop,
        connName,
        sockfd,   // Socket Channel
        localAddr,
        peerAddr));
    connections_[connName] = conn;
    //下面的回调都是用户设置给TcpServer =》 TcpConnection => Channel => Poller => Channel的回调操作
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));  //设置连接断开时的回调函数
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));  //连接建立
}


//连接断开时的回调函数，从connections_中移除连接
void TcpServer::removeConnection(const TcpConnectionPtr &conn){
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}


//poller => channel::closeCallback => TcpConnection::handleClose => TcpServer::removeConnectionInLoop => TcpConnection::connectDestroyed

//连接断开时的回调函数,在loop线程中执行
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn){
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n",name_.c_str(),conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed,conn));  //连接销毁
}