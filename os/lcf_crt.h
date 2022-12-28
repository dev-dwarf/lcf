#ifndef LCF_CRT
#define LCF_CRT "1.0.0"

#include <stdio.h>
str8 stdio_load_entire_file(Arena *arena, str8 filepath);
b32 stdio_write_file(str8 filepath, Str8List text);
/* u64 stdio_get_file_write_time(str8 filepath); TODO */


#endif
