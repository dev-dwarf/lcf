#include "lcf_posix.h"

void os_PlatformInit() {
    Arena_thread_init_scratch();
    os_GetPageSize();
}

global u64 posix_page_size;
u64 os_GetPageSize() {
    if (!posix_page_size) {
        posix_page_size = sysconf(_SC_PAGESIZE);
    }
    return posix_page_size;
}

void* os_Reserve(upr size) {
    return mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, (off_t) 0);
}

s32 os_Commit(void *memory, upr size) {
    s32 result = mprotect(memory, size, PROT_READ | PROT_WRITE);
    return result == 0; /* NOTE(lcf) 0 is success */
}

void os_Decommit(void *memory, upr size) {
    mprotect(memory, size, PROT_NONE);
    madvise(memory, size, MADV_DONTNEED);
}

void os_Free(void *memory, upr size) {
    munmap(memory, size);    
}

u64 os_GetThreadID(void) {
    #if OS_LINUX
    ASSERTSTATIC(sizeof(pid_t) <= sizeof(u64), threadIdIs64Bits);
    return gettid();
    #elseif OS_MAC
    ASSERTSTATIC(sizeof(pthread_id_np_t) <= sizeof(u64), threadIdIs64Bits);
    return pthread_getthreadid_np(); 
    #endif
}
