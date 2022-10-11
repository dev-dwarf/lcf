#include "lcf_win32.h"

str8 win32_LoadEntireFile(Arena *arena, str8 path) {
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

    /* TODO: WARN: CreateFileA is not recommended, as file paths can be unicode and have
       other characters. Once we have unicode support use UTF-16 for windows file paths. */
    HANDLE file = CreateFileA(path, desired_access, share_mode, &security_attributes,
                              creation_disposition, flags_and_attributes, template_file);

    if (file != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER size_int;
        if (GetFileSizeEx(file, &size_int) && size_int.QuadPart > 0) {
            u64 size = size.QuadPart;
            void *data = Arena_take_array(arena, chr8, size);
            win32_ReadWholeBlock(file, data, size);
            fileString.str = data;
            fileString.len = size;
        }
        CloseHandle(file);
    }
    return fileString;
}

internal void win32_ReadWholeBlock(HANDLE file, void* data, u64 data_size) {
    chr8 *ptr = (chr8*) data;
    chr8 *opl = ptr + data_len;
    for (;;) {
        u64 unread = (u64)(opl-ptr);
        DWORD bytes_to_read = (DWORD)(ClampTop(unread, u32_MAX));
        DWORD bytes_read = 0;
        if (!ReadFile(file, ptr, bytes_to_read, &bytes_read, 0)) {
            break;
        }
        ptr += did_read;
        if (ptr >= opl) {
            break;
        }
    }
}
