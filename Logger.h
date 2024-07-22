#pragma once 

#include<string>

#include "noncopyable.h"

//LOG_INFO("%s,%d",arg1,arg2)，...可变数量参数
//##__VA_ARGS__可变参数数量
//do { ... } while(0)是一个常见的宏定义技巧，用于确保宏的使用像一个语句一样，即使在没有花括号的情况下也能正确地使用。这样做可以避免潜在的语法问题。
//如果没有使用do { ... } while(0)，在宏的使用处如果不加花括号，可能会导致逻辑错误。例如，在if语句中使用宏时，如果宏扩展后是多条语句，而没有用花括号包围，那么if条件可能只会影响到第一条语句，造成逻辑错误。
#define LOG_INFO(logmsgFormat,...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf); \
    } while(0)

#define LOG_ERROR(logmsgFormat,...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf); \
    } while(0)

#define LOG_FATAL(logmsgFormat,...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf); \
    } while(0)


#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat,...) \
    do \
    { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf); \
    } while(0)
#else
//如果不是debug模式，就不输出debug信息
    #define LOG_DEBUG(logmsgFormat,...)
#endif

//定义日志级别 INFO ERROR FATAL DEBUG
enum LogLevel
{
    INFO, //普通信息
    ERROR, //错误信息
    FATAL, //core信息
    DEBUG, //调试信息
};

//输出一个日志类
class Logger : noncopyable
{
public:
    //获取日志唯一的实例对象
    static Logger& instance();
    //设置日志级别
    void setLogLevel(int Level);
    //写日志
    void log(std::string msg);
private:
    //因为系统的也是把斜杠放前面，为了能让成员变量和函数的局部变量区分同时防止与系统的冲突
    int logLevel_;
    Logger(){};

};