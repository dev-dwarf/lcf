#include "lcf_win32.h"

global s32 win32_got_sys_info;
global SYSTEM_INFO win32_sys_info;
global s64 win32_PerfFreq;

void os_PlatformInit() {
    Arena_thread_init_scratch();
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

static u64 win32_RoundUpSize(u64 size, u64 snap) {
    return ((size-1) | (snap-1))+1;
}

void* os_Reserve(upr size) {
    return VirtualAlloc(0, win32_RoundUpSize(size, LCF_MEMORY_RESERVE_SIZE), MEM_RESERVE, PAGE_NOACCESS);
}

s32 os_Commit(void *memory, upr size) {
    return !!VirtualAlloc(memory, win32_RoundUpSize(size, LCF_MEMORY_COMMIT_SIZE), MEM_COMMIT, PAGE_READWRITE);
}

void os_Decommit(void *memory, upr size) {
    VirtualFree(memory, size, MEM_DECOMMIT);
}

void os_Free(void *memory, upr size) {
    (void) size;
    VirtualFree(memory, 0, MEM_RELEASE);
}

str os_ReadFile(Arena *arena, str filepath) {
    str fileString = ZERO_STRUCT;

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
            char *data = Arena_take_array(arena, char, size+1);
            win32_ReadBlock(file, data, size);
            data[size] = '\0';
            fileString.str = data;
            fileString.len = size;
        }
        CloseHandle(file);
    }
    return fileString;
}

s32 os_WriteFile(str filepath, StrList text) {
    s64 bytesWrittenTotal = 0;

    HANDLE file;
    SCRATCH_SESSION(scratch) {
        str safe_path = str_make_cstring(scratch.arena, filepath);
        file = CreateFileA(safe_path.str, FILE_APPEND_DATA | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    }
    
    if (file != INVALID_HANDLE_VALUE) {
        bytesWrittenTotal += win32_WriteBlock(file, text);
        CloseHandle(file);
    }
    
    return bytesWrittenTotal == text.total_len;
}

s32 os_DeleteFile(str path) {
    s32 deleted;
    SCRATCH_SESSION(scratch) {
        str safe_path = str_make_cstring(scratch.arena, path);
        // TODO(lcf): handle windows utf16 
        deleted = DeleteFileA((LPCSTR) safe_path.str);
    }
    return deleted != 0; /* anything but 0 is success! */
}

os_FileInfo win32_GetFileInfo(Arena *arena, HANDLE filehandle, WIN32_FIND_DATA fd, str path) {
    os_FileInfo result = ZERO_STRUCT;
    /* FILETIME struct is unsigned 64 bits.
       REF: https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
    */
    ASSERTSTATIC(sizeof(FILETIME) == 8, filetimeStruct);
    union {
        u64 u;
        FILETIME ft;
    } time;
    
    if (filehandle != INVALID_HANDLE_VALUE) {
        if (arena != 0) {
            // TODO(lcf): handle windows utf16 
            result.name = str_copy(arena, str_from_cstring((char*) fd.cFileName));

            char full_path[MAX_PATH];
            GetFullPathNameA(path.str, MAX_PATH, full_path, 0);
            result.path = strf(arena, "%s\\%s", full_path, fd.cFileName);
        }
        result.bytes = ((u64)(fd.nFileSizeHigh) << 32) + fd.nFileSizeLow;
        time.ft = fd.ftLastWriteTime;
        result.written = time.u;
        time.ft = fd.ftLastAccessTime;
        result.accessed = time.u;
        time.ft = fd.ftCreationTime;
        result.created = time.u;

        result.os_flags = fd.dwFileAttributes;
        if ((result.flags & FILE_ATTRIBUTE_NORMAL) ||
            ((result.os_flags & FILE_ATTRIBUTE_DIRECTORY) == 0)) {
            result.flags |= OS_IS_FILE;
        } else {
            result.flags |= OS_IS_FOLDER;
        }

        if (result.os_flags & FILE_ATTRIBUTE_DEVICE) {
            result.flags |= OS_IS_DEVICE;
        }

        if (result.os_flags & FILE_ATTRIBUTE_READONLY) {
            result.flags |= OS_CAN_READ;
        } else {
            result.flags |= OS_CAN_READ | OS_CAN_WRITE;
        }

        u32 unused;
        if (arena != 0 && GetBinaryTypeA(result.path.str, (DWORD*) &unused)) {
            result.flags |= OS_CAN_EXECUTE;
        }
        
    }
    return result;
}

os_FileInfo os_GetFileInfo(Arena *arena, str filepath) {
    os_FileInfo result;
    WIN32_FIND_DATA fd;
    // WARN(lcf): is void* ok here?
    HANDLE handle = FindFirstFileA(filepath.str, (LPWIN32_FIND_DATAA) &fd);
    result = win32_GetFileInfo(arena, handle, fd, strl("."));
    FindClose(handle);
    return result;
}

u64 os_GetFileTime(void) {
    u64 out;
    GetSystemTimeAsFileTime((LPFILETIME) &out);
    return out;
}


u64 os_GetTimeMicroseconds(void) {
    u64 result;
    
    s64 counter;
    QueryPerformanceCounter((LARGE_INTEGER*) &counter);
    f64 time_seconds = ((f64) counter)/((f64) win32_PerfFreq);
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
    char *ptr = (char*) block;
    char *opl = ptr + block_size;
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

internal s64 win32_WriteBlock(HANDLE file, StrList data) {
    s64 result = 0;
    
    StrNode* n = data.first;
    for (s64 i = 0; i < data.count; i++, n = n->next) {
        u32 toWrite = (u32) n->str.len;
        u32 written = 0;

        if (!WriteFile(file, n->str.str, toWrite, (LPDWORD) &written, 0)) {
            break;
        }
            
        result += written;

        /* NOTE(lcf): Not sure this is really needed in practice. */
        if (n->str.len > u32_MAX) {
            toWrite = n->str.len >> 32;
            written = 0;
            char *back_half = &(n->str.str[u32_MAX]);
            while (written != toWrite) {
                WriteFile(file, back_half, toWrite, (LPDWORD) &written, 0);
            }
            result += written;
        }
    }

    return result;
}

os_FileSearch* os_BeginFileSearch(Arena *arena, str searchstr) {
    win32_FileSearch *fs = 0; 
    searchstr = str_trim_whitespace(searchstr);
    if (searchstr.len > 0) {
        ASSERTSTATIC(sizeof(win32_FileSearch) <= sizeof(os_FileSearch), _win32_must_fit);
        fs = (win32_FileSearch*) Arena_take_struct_zero(arena, os_FileSearch);
        str cstr = str_make_cstring(arena, searchstr);
        // WARN(lcf): is void* ok here?
        fs->handle = FindFirstFileA(cstr.str, (LPWIN32_FIND_DATAA) &(fs->fd));

        s32 loc = (s32) str_char_location_backward(searchstr, '/');
        if (loc == -1) {
            loc = (s32) str_char_location_backward(searchstr, '\\');
        }
        fs->searchdir = str_make_cstring(arena, str_first(searchstr, loc));
    }
    return (os_FileSearch*) fs;
}

s32 os_NextFileSearch(Arena *arena, os_FileSearch *os_fs, os_FileInfo *out_file) {
    win32_FileSearch *fs = (win32_FileSearch*) os_fs;
    s32 has_file = 0;
    if (fs->handle != INVALID_HANDLE_VALUE) {
        has_file = true;

        *out_file = win32_GetFileInfo(arena, fs->handle, fs->fd, fs->searchdir);
        
        if (!FindNextFileA(fs->handle, (LPWIN32_FIND_DATAA) &fs->fd)) {
            FindClose(fs->handle);
            fs->handle = INVALID_HANDLE_VALUE;
        }
    }
    return has_file;
}

void os_EndFileSearch(os_FileSearch *os_fs) {
    win32_FileSearch *fs = (win32_FileSearch*) os_fs;
    if (fs && fs->handle != INVALID_HANDLE_VALUE) {
        FindClose(fs->handle);
    }
}



