#include "lcf/lcf.h"
#include "lcf/lcf.c"

#include <stdio.h>
#include <process.h>

// TODO(lcf) bug in win32_GetFileInfo that makes src path wrong in the below

// NOTE(lcf): should be run with "build" as working dir

#define CODE_ROOT "C:\\Code\\"
#define FLAGS "-GR- -Oi -Zi"
#define DISABLED_WARNINGS "-wd4201 -wd4100 -wd4189 -wd4244 -wd4456 -wd4457 -wd4245"

int main(void) {
    Arena *a = Arena_scratch();

    str search = strl("..\\programs\\*.c");
    os_FileSearchIter(a, search, src) {
        str name = str_cut(src.name, 2);
        str exe_file = strf(a, "%.*s.exe", name.len, name.str);
        os_FileInfo exe = os_GetFileInfo(a, exe_file);

        if (exe.written < src.written) {
            str cmd = strf(a, "cl ..\\programs\\%.*s -I %s -I %sraylib\\src -nologo %s -W4 %s -link \"%sraylib\\raylib.lib\"", (s32)src.name.len, src.name.str, CODE_ROOT, CODE_ROOT, FLAGS, DISABLED_WARNINGS, CODE_ROOT);
            printf("\nbuilding programs\\%.*s: \n\t%.*s\n", (s32)src.name.len, src.name.str, (s32)cmd.len, cmd.str);
            fflush(0);
            system(cmd.str);
        }
    }
}
