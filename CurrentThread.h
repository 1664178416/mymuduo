#include <sys/syscall.h>
#include <unistd.h>

namespace CurrentThread {
    extern __thread int t_cachedTid;  // 声明 t_cachedTid

    void cacheTid();

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();   
        }
        // 如果后面不等于0了那么就会直接返回回去
        return t_cachedTid;
    }
}