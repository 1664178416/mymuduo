#pragma once  //类似#ifndef/#define/#endif，语言级别，而pragma once是编译级别，防止头文件被重复包含


/*
noncopyable被继承后，派生类对象可以正常的构造和析构，但是派生类对象
无法拷贝构造和赋值操作
*/

class noncopyable
{
public:
//拷贝构造跟赋值构造被delete掉，即设置不可拷贝构造和复制，让派生类对象不能拷贝构造和赋值构造
    noncopyable(const noncopyable&) = delete;
    noncopyable & operator=(const noncopyable) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
