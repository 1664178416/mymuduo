#include "TcpServer.h"
#include "Logger.h"

EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpServer is null \n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}


TcpServer::TcpServer(EventLoop *loop,const InetAddress &listenAddr,const std::string &nameArg,Option option)
    :loop_(CheckLoopNotNull(loop)),
    name_(nameArg),
    ipPort_(listenAddr.toIpPort()),
    acceptor_(new Acceptor(loop,listenAddr,option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop,name_)),
    connectionCallback_(),
    messageCallback_(),
    writeCompleteCallback_(),
    nextConnId_(1)    //连接的id
{
    //当有新用户连接时，会执行TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
        //占位符，connfd和peerAddr    
}

void TcpServer::newConnection(int sockfd,const InetAddress &peerAddr){

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