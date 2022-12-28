#include "lcf_posix.h"

global b32 posix_got_page_size;
global u64 posix_page_size;
u64 posix_GetPageSize() {
    if (!posix_got_page_size) {
        posix_page_size = sysconf(_SC_PAGESIZE);
        posix_got_page_size = true;
    }
    return posix_page_size;
}

LCF_MEMORY_RESERVE_MEMORY(posix_Reserve) {
    return mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, (off_t) 0);
}

LCF_MEMORY_COMMIT_MEMORY(posix_Commit) {
    b32 result = mprotect(memory, size, PROT_READ | PROT_WRITE);
    return result == 0; /* NOTE(lcf) 0 is success */
}

LCF_MEMORY_FREE_MEMORY(posix_Decommit) {
    mprotect(memory, size, PROT_NONE);
    madvise(memory, size, MADV_DONTNEED);
}

LCF_MEMORY_FREE_MEMORY(posix_Free) {
    munmap(memory, size);
}
