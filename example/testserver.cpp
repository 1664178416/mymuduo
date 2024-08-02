#include <mymuduo/TcpServer.h>

#include<string>
#include<functional>    
class EchoServer
{
public:
    EchoServer(EventLoop* loop,const InetAddress &addr,
                const std::string &name)
                : server_(loop,addr,name),
                loop_(loop)
                {
                    //注册回调函数
                    server_.setConnectionCallback(
                        std::bind(&EchoServer::onConnection,this,std::placeholders::_1));
                    server_.setMessageCallback(
                        std::bind(&EchoServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
                    //设置合适的loop线程数量
                    server_.setThreadNum(3);
                }
                void start(){
                    server_.start();
                }
private:

    //连接建立或者连接断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected()){
            LOG_INFO("conn up:%s",conn->peerAddress().toIpPort().c_str());
        }
        else{
            LOG_INFO("conn down:%s",conn->peerAddress().toIpPort().c_str());
        }
    }
    //可读写事件的回调
    void onMessage(const TcpConnectionPtr &conn,Buffer *buf,Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();  //关闭写端 
    }
    EventLoop* loop_;
    TcpServer server_;
};

int main(){
    EventLoop loop;
    InetAddress addr(8080);
    EchoServer server(&loop,addr,"EchoServer"); //Acceptor对象，nonblocking listenfd create bind
    server.start();  //listen loopthread listenfd => acceptChannel => mainloop  => newConnection
    loop.loop();  //启动mainloop的Poller
    return 0;
}