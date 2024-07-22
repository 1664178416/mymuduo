#include "Timestamp.h"

#include<time.h>

Timestamp::Timestamp():microSecondsSinceEpoch_(0){}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch){}

Timestamp Timestamp::now(){
    return Timestamp(time(NULL));
}

std::string Timestamp::toString() const{
    char buf[128] = {0};
    tm* tm_time = localtime(&microSecondsSinceEpoch_);
    //将格式化的数据写入字符串，它是 sprintf 函数的安全版本，因为它允许指定目标字符串的最大长度，从而避免溢出缓冲区
    snprintf(buf,128, "%4d/%02d/%02d %02d:%02d:%02d",
            tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
            tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    return buf;
}


// int main(){

//     std::cout << Timestamp::now().toString() << std::endl;
//     return 0;
// }