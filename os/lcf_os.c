#include "lcf_os.h"

#ifdef OS_WINDOWS
#include "lcf_win32.c"
#endif
#if OS_LINUX || OS_MAC
#include "lcf_posix.c"
#include "lcf_crt.c"
#endif

/* Generic OS Utils: */
b32 os_FileWasWritten(str8 filepath, u64* last_write_time) {
    b32 result = 0;
    
    os_FileInfo file = os_GetFileInfo(filepath);
    if (file.os_flags > 0) {
        if (*last_write_time != file.written) {
            *last_write_time = file.written;
            result = 1;
        }
    }

    return result;
}
