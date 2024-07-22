#pragma once

#include<string>
#include<iostream>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch); //explicit关键字防止隐式转换，更好的控制代码的行为
    //static成员函数不依赖于类的任何实例来执行。这意味着你可以在不创建类的对象的情况下调用该函数。
    //使用static使得可以直接通过类名调用now函数（例如，Timestamp::now()），而不需要先创建一个Timestamp对象
    static Timestamp now();
    //添加const关键字表示这个函数不会修改成员变量的值
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};
