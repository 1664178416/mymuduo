#pragma once 

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;
class Timestamp;

//指向了TcpConnection对象的智能指针
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>; 
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

using MessageCallback = std::function<void(const TcpConnectionPtr&,Buffer*,Timestamp)>;

using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&,size_t)>;