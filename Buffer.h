#pragma once 

#include<string>
#include <vector>

//网络库底层的缓冲区类型定义

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;  //预留8字节的空间
    static const size_t kInitialSize = 1024;  //初始大小为1024字节

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + kInitialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend)
    {
    }

    size_t readableBytes() const
    { return writerIndex_ - readerIndex_; }

    size_t writableBytes() const
    { return buffer_.size() - writerIndex_; }

    size_t prependableBytes() const //返回前面空闲的区域
    { return readerIndex_; }


    // buffer_.size - writerIndex_  len
    void ensureWritableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len);  //扩容函数
        }
        
    }

    //把[data,data+len]内存上数据添加到writeable区域
    void append(const char* data,size_t len){
        ensureWritableBytes(len); //确保有足够的空间
        std::copy(data,data+len,beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite(){
        return begin() + writerIndex_;
    }
    const char* beginWrite() const {
        return begin() + writerIndex_;
    }

    //从fd上读取数据
    ssize_t readFd(int fd,int* savedErrno);
private:

    char* begin(){
        //首先调用了迭代器的operator*()方法，获得底层vector第0号位元素，然后再用&取地址，数组的起始地址
        return &*buffer_.begin();
    }
    const char* begin() const{
        return &*buffer_.begin();
    }
    //返回缓冲区中可读数据的起始地址
    const char* peak() const {
        return begin() + readerIndex_;
    }

    //复位操作
    void retrieve(size_t len){
        if(len < readableBytes()){
            readerIndex_ += len;  //应用只读取了可读缓冲区的一部分
        }
        else{
            //如果读完了，重新复位
            retrieveAll();
        }
    }
    void retrieveAll(){
        //重新复位
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    //把onMessage函数上报的buffer数据转成string类型的数据返回
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len){
        std::string result(peak(),len);
        retrieve(len); //把读过的数据清空,读走了，那么readerIndex_也要往后移动
        return result;
    }

    void makeSpace(size_t len){
        //如果可写区域加上前面空闲区域小于len+预留，扩容
        if(writableBytes() + prependableBytes() - kCheapPrepend < len){
            buffer_.resize(writerIndex_ + len);
        }
        else{
            //否则，将可读区域前移到预留区域
            //template <class InputIterator, class OutputIterator>
            //OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result);
            //first: 指向要复制的第一个元素的输入迭代器。
            //last: 指向要复制的最后一个元素之后的输入迭代器。
            //result: 指向目标范围起始位置的输出迭代器。
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
            begin() + writerIndex_,
            begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};