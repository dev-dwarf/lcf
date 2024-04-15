#include "lcf/lcf.h"
#include "lcf/lcf.c"

#include "raylib.h"

#pragma comment(lib, "raylib.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shell32.lib")

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

struct val { char s[8]; };

static struct val
make(s32 x)
{
    struct val v = {{0}};
    s32 len = 1 + (x>9) + (x>99) + (x>999) + (x>9999) + (x>99999) +
              (x>999999) + (x>9999999);
    switch (len) {
        case 8: v.s[7] = '0' + x%10; x /= 10;  // fallthrough
        case 7: v.s[6] = '0' + x%10; x /= 10;  // fallthrough
        case 6: v.s[5] = '0' + x%10; x /= 10;  // fallthrough
        case 5: v.s[4] = '0' + x%10; x /= 10;  // fallthrough
        case 4: v.s[3] = '0' + x%10; x /= 10;  // fallthrough
        case 3: v.s[2] = '0' + x%10; x /= 10;  // fallthrough
        case 2: v.s[1] = '0' + x%10; x /= 10;  // fallthrough
        case 1: v.s[0] = '0' + x%10;
    }
    return v;
}


static u64
hash(struct val v)
{
    u64 h;
    memcpy(&h, v.s, 8);
    return h * 1111111111111111111;
}

int main() {

    Arena *a = Arena_create();

    ARENA_SESSION(a) {
        Table *t = Table_create(a, 4000);
        ASSERT(t->exp == 12);
    }

#define N 512
    Table *t = Table_create(a, N);

    for (s64 i = 1; i < 4*N; i++) {
        struct val v = make(i);
        u64 h = hash(v);
        Table_insert(t, h, (void*)i);
    }

    for (s32 i = 4*N; i > 0; i--) {
        struct val v = make(i);
        u64 h = hash(v);
        u64 d = (u64) Table_lookup(t, h);
        
        if (i >= N) {
            ASSERT(d == 0);
        } else {
            ASSERT(d == i);
        }
    }
}
