#ifdef OS_WINDOWS
#include "lcf_win32.c"
#endif
#if OS_LINUX || OS_MAC
#include "lcf_posix.c"
#include "lcf_crt.c"
#endif
