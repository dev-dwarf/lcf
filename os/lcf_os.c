#include "lcf_os.h"

#ifdef OS_WINDOWS
#include "lcf_win32.c"
#endif
#if OS_LINUX || OS_MAC
#include "lcf_posix.c"
#include "lcf_crt.c"
#endif

/* Generic OS Utils */
s32 os_FileWasWritten(str filepath, u64* last_write_time) {
    s32 result = 0;
    
    os_FileInfo file = os_GetFileInfo(0, filepath);
    if (file.os_flags > 0) {
        if (*last_write_time != file.written) {
            *last_write_time = file.written;
            result = 1;
        }
    }

    return result;
}

