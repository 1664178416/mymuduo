#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h> // Include the appropriate header file for "iovec" type

/*
 
*/

ssize_t Buffer::readFd(int fd,int* savedErrno){
    char extrabuf[65536] = {0};  //栈上的空间 64k

    struct iovec vec[2];  //定义两个缓冲区

    //第一块缓冲区
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    //第二块缓冲区
    vec[1].iov_base = extrabuf; //栈上空间
    vec[1].iov_len = sizeof(extrabuf);
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    //readv能在非连续的缓冲区里面，写入同一个fd上的数据
    const ssize_t n = ::readv(fd,vec,iovcnt);  //从fd上读数据
    if(n < 0) {
        *savedErrno = errno;
    }
    else if(n <= writable)
        writerIndex_ += n;
    else{  //extrabuf也写入了数据
        writerIndex_ = buffer_.size();
        append(extrabuf,n - writable);  //writerIndex_开始写n - writable大小的数据
    }

    return n;
}