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
str8 win32_LoadEntireFile(Arena *arena, str8 path);

/* Helpers */
internal void win32_ReadWholeBlock(HANDLE file, void* data, u64 data_size);

#endif

