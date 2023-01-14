/* TODO: redo OS ryan style, have os header for common things and then define each one seperately. */

#ifndef LCF_WIN32
#define LCF_WIN32 "1.0.0"

#include "../base/lcf_base.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Helper macros */
#undef ASSERT
#define ASSERT(C) STATEMENT( if(!(C)) __debugbreak();)
#define HR(HRESULT_PROC) STATEMENT(HRESULT hr = (HRESULT_PROC); ASSERT(SUCCEEDED(hr)););
#define SAFE_RELEASE(ComObj) do { if (ComObj) { (ComObj)->Release(); (ComObj) = 0; }} while(0);

/* Helpers */
internal void win32_ReadBlock(HANDLE file, void* block, u64 block_size);
internal s64 win32_WriteBlock(HANDLE file, Str8List data);
#endif

