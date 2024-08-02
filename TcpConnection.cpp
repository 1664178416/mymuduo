#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"


#include<functional>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<strings.h>
#include<netinet/tcp.h>
#include<sys/socket.h>
#include<string>


EventLoop* CheckLoopNotNull_Connection(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnection is null \n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}


TcpConnection::TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr)
    :loop_(CheckLoopNotNull_Connection(loop)),
    name_(name),
    state_(kConnecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop,sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024) //64M
    {
        //给channel设置相应的回调函数，poller给channel通知感兴趣的事件发生了，channel会回调相应的操作函数
        channel_->setReadCallback(
            std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
        channel_->setWriteCallback(
            std::bind(&TcpConnection::handleWrite,this));
        channel_->setCloseCallback(
            std::bind(&TcpConnection::handleClose,this));
        channel_->setErrorCallback(
            std::bind(&TcpConnection::handleError,this));
        LOG_INFO("TcpConnection::ctor[%s] at %p fd=%d\n",
                  name_.c_str(),this,sockfd);
        socket_->setKeepAlive(true);
    }
TcpConnection::~TcpConnection(){
    LOG_INFO("TcpConnection::dtor[%s] at %p fd=%d state=%d\n",
              name_.c_str(),this,channel_->fd(),(int)state_);
}

//发送数据，应用写的快，内核发送慢，需要把待发送数据写入缓冲区，而且设置了水位回调
void TcpConnection::sendInLoop(const void* data,size_t len){
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fautlError = false;  
    //之前调用过connection的shutdown，不能进行发送
    if(state_ == kDisconnected){
        LOG_ERROR("disconnected, give up writing\n");
        return;
    }
    //如果channel没有关注POLLOUT事件，并且缓冲区没有待发送数据，尝试直接发送数据
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0){
        nwrote = ::write(channel_->fd(),data,len);
        if(nwrote >= 0){
            remaining = len - nwrote;
            //如果数据发送完毕，执行写完回调，不用给channel再设置epollout事件了
            if(remaining == 0 && writeCompleteCallback_){
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_,shared_from_this()));
            }
        }
        else{  //出错
            nwrote = 0;
            if(errno != EWOULDBLOCK){
                LOG_ERROR("TcpConnection::sendInLoop");
                //如果是管道破裂或者连接重置，不再尝试发送
                if(errno == EPIPE || errno == ECONNRESET){
                    fautlError = true;
                }
            }
        }
    }
    //如果数据没有发送完毕，或者channel关注了POLLOUT事件，将数据写入缓冲区
    //注册epollout事件，poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel，调用writeCallback_回调方法
    //也就是调用TcpConnection::handleWrite()方法,把发送缓冲区数据全部发送完成
    if(!fautlError && remaining > 0){
        //目前发送缓冲区剩余待发送的数据长度
        size_t oldLen = outputBuffer_.readableBytes();
        //如果超过高水位标记，执行高水位回调
        if(oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_){
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_,shared_from_this(),oldLen+remaining));
        }
        outputBuffer_.append((char*)(data)+nwrote,remaining); //将剩余的数据写入缓冲区
        if(!channel_->isWriting()){
            channel_->enableWriting();  //一定要注册channel的写事件，否则不会给channel通知epollout
        }
    }
}

void TcpConnection::send(const std::string &buf){
    if(state_ == kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(buf.c_str(),buf.size());
        }
        else{
            
            loop_->runInLoop(std::bind(
                &TcpConnection::sendInLoop,this,buf.c_str(),buf.size()
            ));
        }
    }
}

void TcpConnection::handleRead(Timestamp receiveTime){
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(),&saveErrno);
    if(n > 0){
        //已连接连接的用户有可读事件发生，调用用户传入的回调操作onMessage
        //shared_from_this()获取当前对象的智能指针，避免对象提前析构
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if(n == 0) 
        handleClose();
    else{
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}
void TcpConnection::handleWrite(){
    if(channel_->isWriting()){
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(),&saveErrno);
        if(n > 0){
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0){
                //数据发送完毕，取消关注POLLOUT事件
                channel_->disableWriting();
                if(writeCompleteCallback_){
                    //唤醒loop_对应的thread线程，执行回调，写完了执行回调
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_,shared_from_this()));
                }
                if(state_ == kDisconnecting){
                    shutdownInLoop();
                }
            }
        }
        else{
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }
    else{
        LOG_ERROR("Connection fd = %d is down, no more writing\n",channel_->fd());
    }

}
void TcpConnection::handleClose(){
    LOG_INFO("fd = %d state = %d\n",channel_->fd(),(int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr); //执行连接关闭的调用
    closeCallback_(connPtr);  //关闭连接的调用
}
void TcpConnection::handleError(){
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    //获取socket的错误码
    //getsockopt()用于获取socket的选项值
    
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}


void TcpConnection::connectEstablished()  //连接建立
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading(); //向poller注册channel的pollin事件
    //新连接建立，执行回调
    connectionCallback_(shared_from_this());
}
void TcpConnection::connectDestroyed()  //连接销毁
{
    if(state_ == kConnected){
        setState(kDisconnected);
        channel_->disableAll(); //把channel所有感兴趣事件，从poller中del掉
        //连接断开，执行回调
        connectionCallback_(shared_from_this());
    }
    channel_->remove(); //把channel从poller中del掉
}
    

void TcpConnection::shutdown(){
    if(state_ == kConnected){
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
    }
}
void TcpConnection::shutdownInLoop(){
    if(!channel_->isWriting()){  //说明outputBuffer_中的数据已经发送完毕
        socket_->shutdownWrite();
    }
}