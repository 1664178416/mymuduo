#include "InetAddress.h"

#include<strings.h>
#include<string.h>

InetAddress::InetAddress(uint16_t port,std::string ip){
    bzero(&addr_,sizeof(addr_));
    addr_.sin_family = AF_INET;
    //htons stands for "Host TO Network Short". 它将主机字节序的16位数（短整型）转换为网络字节序。
    //htonl stands for "Host TO Network Long". 它将主机字节序的32位数（长整型）转换为网络字节序。
    //此处是16位数，所以使用htons 
    addr_.sin_port = htons(port);
    //inet_addr函数接收一个字符串形式的IP地址，并返回一个网络字节序的32位整数，同时该字符串格式必须为点分十进制格式的IP地址
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    //inet_ntop函数将网络字节序的32位整数转换为点分十进制格式的IP地址字符串
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    return buf;
}

//打印ip和port数据
std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    //inet_ntop函数将网络字节序的32位整数转换为点分十进制格式的IP地址字符串
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    int end = strlen(buf);
    //ntohs函数将网络字节序的16位整数转换为主机字节序的16位整数
    uint16_t port = ntohs(addr_.sin_port);
    snprintf(buf+end,sizeof(buf)-end,":%u",port);
    return buf;
}

uint16_t InetAddress::toPort() const {
    //ntohs函数将网络字节序的16位整数转换为主机字节序的16位整数
    return ntohs(addr_.sin_port);
}

#include<iostream>
int main(){
    InetAddress addr(8080);
    std::cout << addr.toIpPort() << std::endl;
    std::cout << addr.toPort() << std::endl;
    std::cout << addr.toIp() << std::endl;
    return 0;
}