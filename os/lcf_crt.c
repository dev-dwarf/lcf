#include "lcf_crt.h"

str os_ReadFile(arena *arena, str filepath) {
    str file_content = {0};

    FILE *file = fopen(filepath.str, "rb");
    if (file != 0) {
        fseek(file, 0, SEEK_END);
        u64 file_len = ftell(file);
        fseek(file, 0, SEEK_SET);
        file_content.str = (chr8*) arena_take(arena, file_len+1);
        if (file_content.str != 0) {
            file_content.len = file_len;
            fread(file_content.str, 1, file_len, file);
            file_content.str[file_content.len] = 0;
        }
        fclose(file);
    }
    return file_content;
}

s32 os_WriteFile(str filepath, StrList text) {
    u64 bytes_written = 0;
    FILE *file = fopen(filepath.str, "wb");
    if (file != 0) {
        StrNode* n = text.first;
        for (s64 i = 0; i < text.count; i++, n = n->next) {
            if ((n->str.len > 0) && (fwrite(n->str.str, n->str.len, 1, file) <= 0)) {
                break;
            }
            bytes_written += n->str.len;
        }
        fclose(file);
    }
    return bytes_written == text.total_len;    
}
s32 os_DeleteFile(str path) {
    s32 result = remove(path.str);
    return result == 0; /* 0 is success */
}

#define SEC_USERR (1 << 8)
#define SEC_USERW (1 << 7)
#define SEC_USERE (1 << 6)
#define SEC_USERALL (SEC_USERR || SEC_USERW || SEC_USERE)
#define SEC_OTHERREAD ((1 << 2))
s32 os_CreateDirectory(str path) {
    s32 result = mkdir(path.str, SEC_USERALL || SEC_OTHERREAD);
    return result >= 0;
}

os_FileInfo os_GetFileInfo(Arena *arena, str filepath) {
    os_FileInfo result = ZERO_STRUCT;
    filepath = str_trim_last_slash(filepath); 
    struct stat filestat;
    if (stat(filepath.str, &crt) == 0) {
        if (arena != 0) {
            result.path = str_copy(arena, filepath);
            u64 loc = str_char_location_backward(filepath, '/');
            if (loc != LCF_STRING_NO_MATCH) {
                result.name = str_skip(filepath, loc+1);
            }
        }
        result.bytes = filestat.st_size;
        result.written = filestat.st_mtime;
        result.accessed = filestat.st_atime;

        result.os_flags = filestat.st_mode;
        if (S_ISREG(filestat.st_mode)) {
            result.flags |= OS_IS_FILE;
        }
        
        if (S_ISDIR(filestat.st_mode)) {
            result.flags |= OS_IS_DIR;
        }
        
        if (S_ISCHR(filestat.st_mode)) {
            result.flags |= OS_IS_DEVICE;
        }
        
        if (filestat.st_mode & SEC_USERR) {
            result.flags |= OS_CAN_READ;
        }
        
        if (filestat.st_mode & SEC_USERW) {
            result.flags |= OS_CAN_WRITE;
        }

        if (filestat.st_mode & SEC_USERE) {
            result.flags |= OS_CAN_EXECUTE;
        }
    }
    
    return result;
}


/* Get __rdtsc support */
#if (COMPILER_GCC || COMPILER_CLANG)
 #include <x86intrin.h>
 #define CYCLE_TIMER __rdtsc
#elseif (COMPILER_CL)
 #include <intrin.h>
 #pragma intrinsic(__rdtsc)
 #define CYCLE_TIMER __rdtsc
#elseif
 #define CYCLE_TIMER NOT_IMPLEMENTED()
#endif

u64 os_GetTimeMicroseconds(void) {
    u64 result = 0;

    struct timespec time;
    if (clock_gettime(CLOCK_REALTIME, &time) == 0) {
        result = time.tv_nsec;
    }
        
    return result;
}

u64 os_GetTimeCycles(void) {
    return CYCLE_TIMER();
}
