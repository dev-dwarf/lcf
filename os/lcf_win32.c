#include "lcf_win32.h"

global b32 win32_got_sys_info;
global SYSTEM_INFO win32_sys_info;
global s64 win32_PerfFreq;

void os_Init() {
    os_GetPageSize();
    QueryPerformanceFrequency((LARGE_INTEGER*) &win32_PerfFreq);
}

u64 os_GetPageSize() {
    if (!win32_got_sys_info) {
        GetSystemInfo(&win32_sys_info);
        win32_got_sys_info = true;
    }
    return (u64) win32_sys_info.dwPageSize;
}

void* os_Reserve(upr size) {
    u64 snapped = size;
    snapped += LCF_MEMORY_RESERVE_SIZE - 1;
    snapped -= snapped & LCF_MEMORY_RESERVE_SIZE;
    return VirtualAlloc(0, snapped, MEM_RESERVE, PAGE_NOACCESS);
}

b32 os_Commit(void *memory, upr size) {
    u64 snapped = size;
    snapped += LCF_MEMORY_COMMIT_SIZE - 1;
    snapped -= snapped & LCF_MEMORY_COMMIT_SIZE;
    void* p = VirtualAlloc(memory, snapped, MEM_COMMIT, PAGE_READWRITE);
    return p != 0;
}

void os_Decommit(void *memory, upr size) {
    VirtualFree(memory, size, MEM_DECOMMIT);
}

void os_Free(void *memory, upr size) {
    VirtualFree(memory, 0, MEM_RELEASE);
}


str8 os_LoadEntireFile(Arena *arena, str8 filepath) {
    str8 fileString = ZERO_STRUCT;

    DWORD desired_access = GENERIC_READ;
    DWORD share_mode = 0;
    SECURITY_ATTRIBUTES security_attributes = {
        (DWORD)sizeof(SECURITY_ATTRIBUTES),
        0,
        0,
    };
    DWORD creation_disposition = OPEN_EXISTING;
    DWORD flags_and_attributes = 0;
    HANDLE template_file = 0;

    HANDLE file = INVALID_HANDLE_VALUE;
        /* TODO: WARN: CreateFileA is not recommended, as file paths can be unicode and have
       other characters. Once we have unicode support use UTF-16 for windows file paths. */
    file = CreateFileA(filepath.str, desired_access, share_mode, &security_attributes,
                       creation_disposition, flags_and_attributes, template_file);

    if (file != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER size_int;
        if (GetFileSizeEx(file, &size_int) && size_int.QuadPart > 0) {
            u64 size = size_int.QuadPart;
            /* NOTE: size+1 to make space for null terminator as default.
               Made this default because often the loaded string will need to be passed
               to other c APIs, making it convenient to not have to add the null later.
             */
            chr8 *data = Arena_take_array(arena, chr8, size+1);
            win32_read_block(file, data, size);
            data[size] = '\0';
            fileString.str = data;
            fileString.len = size;
        }
        CloseHandle(file);
    }
    return fileString;
}

b32 os_WriteFile(str8 filepath, Str8List text) {
    s64 bytesWrittenTotal = 0;
    
    HANDLE file = CreateFileA(filepath.str, FILE_APPEND_DATA | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    
    if (file != INVALID_HANDLE_VALUE) {
        bytesWrittenTotal += win32_WriteBlock(file, text);
        CloseHandle(file);
    }
    
    return bytesWrittenTotal == text.total_len;
}

b32 win32_file_was_written(str8 filepath, u64* last_write_time) {
    u64 new_write_time = win32_get_file_write_time(filepath);
    if (new_write_time != *last_write_time) {
        *last_write_time = new_write_time;
        return true;
    }
    return false;
}

u64 win32_get_file_write_time(str8 filepath) {
    /* FILETIME struct is 64 bits.
       REF: https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
    */
    ASSERTSTATIC(sizeof(FILETIME) == 8, filetimeStruct);
    
    union {
        u64 u;
        FILETIME ft; 
    } write_time;
    write_time.u = 0;

    WIN32_FIND_DATA findData;
    HANDLE findHandle = FindFirstFileA(filepath.str, &findData);
    if (findHandle != INVALID_HANDLE_VALUE) {
        write_time.ft = findData.ftLastWriteTime;
        FindClose(findHandle);
    }

    return write_time.u;
}

u64 os_GetTimeMicroseconds(void) {
    u64 result;
    
    s64 counter;
    QueryPerformanceCounter((LARGE_INTEGER*) &result);
    f64 time_seconds = ((f64) result)/((f64) win32_PerfFreq);
    result = (u64)(time_seconds * 1000000);
    
    return result;
}

u64 os_GetTimeCycles(void) {
    return __rdtsc();
}


u64 os_GetThreadID(void) {
    return GetThreadId(0);
}


internal void win32_ReadBlock(HANDLE file, void* block, u64 block_size) {
    chr8 *ptr = (chr8*) block;
    chr8 *opl = ptr + block_size;
    for (;;) {
        u64 unread = (u64)(opl-ptr);
        DWORD bytes_to_read = (DWORD)(CLAMPTOP(unread, u32_MAX));
        DWORD bytes_read = 0;
        if (!ReadFile(file, ptr, bytes_to_read, &bytes_read, 0)) {
            break;
        }
        ptr += bytes_read;
        if (ptr >= opl) {
            break;
        }
    }
}

internal s64 win32_WriteBlock(HANDLE file, Str8List data) {
    s64 result = 0;
    
    Str8Node* n = text.first;
    for (s64 i = 0; i < text.count; i++, n = n->next) {
        toWrite = (u32) n->str.len;
        written = 0;

        if (!WriteFile(file, n->str.str, toWrite, (LPDWORD) &written, 0)) {
            break;
        }
            
        bytesWrittenTotal += written;

        /* NOTE(lcf): Not sure this is really needed in practice. */
        if (n->str.len > u32_MAX) {
            toWrite = n->str.len >> 32;
            written = 0;
            chr8 *back_half = &(n->str.str[u32_MAX]);
            while (written != toWrite) {
                WriteFile(file, back_half, toWrite, (LPDWORD) &written, 0);
            }
            result += written;
        }
    }

    return result;
}
