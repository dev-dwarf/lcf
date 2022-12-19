#include "lcf_win32.h"

global b32 Win32_got_sys_info;
global SYSTEM_INFO Win32_sys_info;

u32 Win32_GetPageSize() {
    if (!Win32_got_sys_info) {
        GetSystemInfo(&Win32_sys_info);
        Win32_got_sys_info = 1;
    }
    return Win32_sys_info.dwPageSize;
}

LCF_MEMORY_RESERVE_MEMORY(Win32_Reserve) {
    u64 snapped = size;
    snapped += LCF_MEMORY_RESERVE_SIZE - 1;
    snapped -= snapped & LCF_MEMORY_RESERVE_SIZE;
    return VirtualAlloc(0, snapped, MEM_RESERVE, PAGE_NOACCESS);
}

LCF_MEMORY_COMMIT_MEMORY(Win32_Commit) {
    u64 snapped = size;
    snapped += LCF_MEMORY_COMMIT_SIZE - 1;
    snapped -= snapped & LCF_MEMORY_COMMIT_SIZE;
    void* p = VirtualAlloc(memory, snapped, MEM_COMMIT, PAGE_READWRITE);
    return p != 0;
}

LCF_MEMORY_DECOMMIT_MEMORY(Win32_Decommit) {
    VirtualFree(memory, size, MEM_DECOMMIT);
}

LCF_MEMORY_FREE_MEMORY(Win32_Free) {
    VirtualFree(memory, 0, MEM_RELEASE);
}

str8 win32_load_entire_file(Arena *arena, str8 filepath) {
    str8 fileString = {0};

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

internal void win32_read_block(HANDLE file, void* block, u64 block_size) {
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

void win32_write_file(str8 filepath, Str8List text) {
    HANDLE file = CreateFileA(filepath.str, FILE_APPEND_DATA | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    ASSERT(file != INVALID_HANDLE_VALUE);
    u32 toWrite = 0;
    u32 written = 0;
    u32 bytesWrittenTotal = 0;
    Str8Node* n = text.first;
    for (u64 i = 0; i < text.count; i++, n = n->next) {
        toWrite = (u32) n->str.len;
        written = 0;

        while (written != toWrite) {
            WriteFile(file, n->str.str, toWrite, (LPDWORD) &written, 0);
        }
            
        bytesWrittenTotal += written;
    }
    ASSERT(bytesWrittenTotal == text.total_len);
    CloseHandle(file);
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

global s64 win32_PerfFreq;
void win32_init_timing(void) {
    QueryPerformanceFrequency((LARGE_INTEGER*) &win32_PerfFreq);
}

f32 win32_get_seconds_elapsed(s64 start, s64 end) {
    f32 result = (((f32)(end-start))/(f32)win32_PerfFreq);
    return result;
}

s64 win32_get_wall_clock(void) {
    s64 result;
    QueryPerformanceCounter((LARGE_INTEGER*) &result);
    return result;
}

