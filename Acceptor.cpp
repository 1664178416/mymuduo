#include "Acceptor.h"

#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <error.h>
#include <unistd.h>

static int createNonblockingOrDie()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d,sockets::createNonblockingOrDie:%d\n",__FILE__,__FUNCTION__,__LINE__,errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop   *loop, const InetAddress &listenAddr,bool reuseport)
    : loop_(loop),
      acceptSocket_(createNonblockingOrDie()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false)
{
    acceptSocket_.setReuseAddr(true); //设置SO_REUSEADDR选项，允许重用本地地址和端口
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr); //bind
    //TcpServer::start() Acceptor.listen 有新用户连接，要执行一个回调 （confd => channel => subloop)
    //baseloop => acceptChannel_(listenfd) => acceptChannel_
    acceptChannel_.setReadCallback(std::bind(&Acceptor::heandleRead,this));
}

Acceptor::~Acceptor(){
    acceptChannel_.disableAll();  //Acceptor::acceptChannel_不关注任何事件
    acceptChannel_.remove();  //Acceptor::acceptChannel_从Poller中移除
}

void Acceptor::listen(){
    listenning_ = true;
    acceptSocket_.listen();  //listen
    acceptChannel_.enableReading();  //Acceptor::acceptChannel_关注读事件
}

//listenfd有事件发生了，也就是有新用户连接了
void Acceptor::heandleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0 )
    {
        if(newConnectionCallback_)
        {
            //轮询找到subloop，唤醒分发当前的新客户端的Channel
            newConnectionCallback_(connfd,peerAddr);
        }else{
            ::close(connfd);
        }
    }
    else{
        LOG_ERROR("%s:%s:%d,Acceptor::heandleRead:%d\n",__FILE__,__FUNCTION__,__LINE__,errno);
        //LOG_SYSERR << "in Acceptor::heandleRead";
    }
}