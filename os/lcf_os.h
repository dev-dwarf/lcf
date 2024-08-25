#ifndef LCF_OS
#define LCF_OS "1.0.0"

#include "../lcf/base/lcf_context.h"

#define LCF_MEMORY_PROVIDE_MEMORY
#define LCF_MEMORY_reserve os_Reserve
#define LCF_MEMORY_commit os_Commit
#define LCF_MEMORY_decommit os_Decommit
#define LCF_MEMORY_free os_Free
#define LCF_MEMORY_RESERVE_SIZE (MB(256))
#define LCF_MEMORY_COMMIT_SIZE (os_GetPageSize())

#include "../base/lcf_base.h"

/* Call before any other of these functions */
void os_PlatformInit();

/* Virtual Memory */
u64 os_GetPageSize();
void* os_Reserve(upr size);
s32 os_Commit(void *memory, upr size);
void os_Decommit(void *memory, upr size);
void os_Free(void *memory, upr size);

/* File System */
enum os_file_flags {
    OS_IS_FILE = FLAG(0),
    OS_IS_FOLDER = FLAG(1),
    OS_IS_DEVICE = FLAG(2),
    OS_CAN_READ = FLAG(3),
    OS_CAN_WRITE = FLAG(4),
    OS_CAN_EXECUTE = FLAG(5),
};
struct os_FileInfo {
    str name;
    str path;
    /* General */
    u32 os_flags;
    u32 flags;
    u64 bytes;
    /* Times */
    u64 written;
    u64 accessed;
    u64 created;
};
typedef struct os_FileInfo os_FileInfo;
str os_ReadFile(Arena *arena, str filepath);
s32 os_WriteFile(str filepath, StrList text);
s32 os_DeleteFile(str path);
s32 os_CreateDirectory(str path);
os_FileInfo os_GetFileInfo(Arena *arena, str filepath);
s32 os_FileWasWritten(str filepath, u64* last_write_time);

#if OS_WINDOWS
 #include "lcf_win32.h"
#elif OS_LINUX || OS_MAC
 #include "lcf_posix.h"
 #include "lcf_crt.h"
#else
 #error "No os implementation available."
#endif

/* TODO: file searching/iters */
/* for windows reference site.cpp */
/* for linux reference glob.h
   REF: https://chat.openai.com/share/312f3f74-4c0a-4be7-b4e0-ae846657f221 */
struct os_FileSearch {
    #if OS_WINDOWS
        win32_FileSearch data;
    #elif OS_LINUX || OS_MAC
        u8 data[512];
    #endif
};
typedef struct os_FileSearch os_FileSearch;
os_FileSearch* os_BeginFileSearch(Arena *arena, str searchstr);
s32 os_NextFileSearch(Arena *arena, os_FileSearch *search, os_FileInfo *out_file);
void os_EndFileSearch(os_FileSearch *search);
#define os_FileSearchIter(arena, searchstr, file)              \
    os_FileSearch *os_fs##__LINE__ = os_BeginFileSearch(arena, searchstr);         \
    DEFER_IF(os_fs##__LINE__ != 0,  \
               os_EndFileSearch(os_fs##__LINE__))                       \
        for (os_FileInfo file; os_NextFileSearch(arena, os_fs##__LINE__, &file); )    
/* Timing */
u64 os_GetFileTime(void);
u64 os_GetTimeMicroseconds(void);
u64 os_GetTimeCycles(void);

/* Threading */
u64 os_GetThreadID(void);

#endif /* LCF_OS */

