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
void os_Init();

/* Virtual Memory */
u64 os_GetPageSize();
void* os_Reserve(upr size);
b32 os_Commit(void *memory, upr size);
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
    /* */
    u32 os_flags;
    u32 flags;
    u64 bytes;
    /* Times */
    u64 written;
    u64 accessed;
};
typedef struct os_file_info os_file_info;
str8 os_LoadEntireFile(Arena *arena, str8 filepath);
b32 os_WriteFile(str8 filepath, Str8List text);
b32 os_DeleteFile(str8 path);
b32 os_CreateDirectory(str8 path);
os_FileInfo os_GetFileInfo(str8 filepath);

b32 os_FileWasWritten(str8 filepath, u64* last_write_time);

/* Timing */
u64 os_GetTimeMicroseconds(void);
u64 os_GetTimeCycles(void);

/* Threading */
u64 os_GetThreadID(void);
    
#if OS_WINDOWS
 #include "lcf_win32.h"
#elseif OS_LINUX || OS_MAC
 #include "lcf_posix.h"
 #include "lcf_crt.h"
#else
 #error "No os implementation available."
#endif

#endif /* LCF_OS */
