#include "EPollPoller.h"
#include "Logger.h"

#include <errno.h>
#include <unistd.h>

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop* loop)
: Poller(loop),
  epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
  events_(kInitEventListSize){
    if(epollfd_ < 0){
        LOG_FATAL("epoll_create error:%d \n",errno);
    }
}

EPollPoller::~EPollPoller(){
    ::close(epollfd_);
}