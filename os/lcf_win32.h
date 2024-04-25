#ifndef LCF_WIN32
#define LCF_WIN32 "1.0.0"

#include "../base/lcf_base.h"

#define NOSHOWWINDOW
#define NOGDI
#define NOUSER
#define NO
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Helper macros */
#undef ASSERT
#define ASSERT(C) STATEMENT( if(!(C)) __debugbreak();)
#define HR(HRESULT_PROC) STATEMENT(HRESULT hr = (HRESULT_PROC); ASSERT(SUCCEEDED(hr)););

#ifdef __cplusplus
#define SAFE_RELEASE(ComObj) do { if (ComObj) { (ComObj)->Release(); (ComObj) = 0; }} while(0);
#else
#define SAFE_RELEASE(ComObj) do { if (ComObj) { (ComObj)->lpVtbl->Release(ComObj); (ComObj) = 0; }} while(0);
#endif


/* Helpers */
internal void win32_ReadBlock(HANDLE file, void* block, u64 block_size);
internal s64 win32_WriteBlock(HANDLE file, StrList data);

/* File Iters */
struct win32_FileSearch {
    WIN32_FIND_DATA fd;
    HANDLE handle;
    str searchdir;
};
typedef struct win32_FileSearch win32_FileSearch;
#endif

