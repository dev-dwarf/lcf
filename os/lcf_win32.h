/** ************************************
  Created (October 06, 2022)

  Description:
  
************************************ **/

#ifndef LCF_WIN32
#define LCF_WIN32 "1.0.0"

#include "../base/lcf_base.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* API */
str8 win32_load_entire_file(Arena *arena, str8 filepath);
u64 win32_get_file_write_time(str8 filepath);

/* Helpers */
internal void win32_read_block(HANDLE file, void* block, u64 block_size);

#endif

