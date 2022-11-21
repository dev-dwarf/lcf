/** ************************************
  Created (October 06, 2022)

  Description:
  
************************************ **/

#ifndef LCF_WIN32
#define LCF_WIN32 "1.0.0"

#define LCF_MEMORY_PROVIDE_MEMORY "win32"
#define LCF_MEMORY_reserve Win32_Reserve
#define LCF_MEMORY_commit Win32_Commit
#define LCF_MEMORY_decommit Win32_Decommit
#define LCF_MEMORY_free Win32_Free
#define LCF_MEMORY_RESERVE_SIZE GB(1)
#define LCF_MEMORY_COMMIT_SIZE (Win32_GetPageSize())

#include "../base/lcf_base.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Helper macros */
#define HR(HRESULT_PROC) STATEMENT(HRESULT hr = (HRESULT_PROC); ASSERT(SUCCEEDED(hr)););

#ifdef _DEBUG
#undef ASSERT
#define ASSERT(C) STATEMENT( if(!(C)) __debugbreak();)
#endif

/* Memory */
u32 Win32_GetPageSize();
LCF_MEMORY_RESERVE_MEMORY(Win32_Reserve);
LCF_MEMORY_COMMIT_MEMORY(Win32_Commit);
LCF_MEMORY_DECOMMIT_MEMORY(Win32_Decommit);
LCF_MEMORY_FREE_MEMORY(Win32_Free);

/* Files */
/* WARN: not handling wide chars in filepaths, meaning technically some files are
   inaccessible using this API. Would mostly be a problem when letting end user
   pass filepaths. */
str8 win32_load_entire_file(Arena *arena, str8 filepath);
u64 win32_get_file_write_time(str8 filepath);
b32 win32_file_was_written(str8 filepath, u64* last_write_time); 

/* Timing */
void win32_init_timing(void); /* Call before using other timing calls */
s64 win32_get_wall_clock(void);
f32 win32_get_seconds_elapsed(s64 start, s64 end);

/* Helpers */
internal void win32_read_block(HANDLE file, void* block, u64 block_size);

#endif

