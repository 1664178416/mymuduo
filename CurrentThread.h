#pragma once 

#include<unistd.h>
#include<sys/syscall.h>

namespace CurrentThread
{
    extern __thread int t_cacheTid;

    void cacheTid();

    inline int tid()
    {
        if(__builtin_expect(t_cacheTid == 0,0)){
            cacheTid();   
        }
        //如果后面不等于0了那么就会直接返回回去
        return t_cachedTid;
    }
}