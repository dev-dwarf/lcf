
#ifndef LCF_MATH
#define LCF_MATH

#define HANDMADE_MATH_USE_TURNS
#include "../libs/lcf_hmm.h"

typedef union Rect {
    struct {
        f32 x, y;
        f32 w, h;
    };
    struct {
        Vec2 tl;
        Vec2 wh;
    };
    f32 raw[4];
} Rect;

Rect RectFromPoints(Vec2 p0, Vec2 p1) {
    Rect r;
    r.tl = p0;
    r.wh = SubV2(p1, p0);
    return r;
}

Rect RectGrow(Rect r, Vec2 p) {
    r.x = MIN(r.x, p.x);
    r.y = MIN(r.y, p.y);
    r.x = MAX(r.w, p.x - r.x);
    r.y = MAX(r.h, p.y - r.y);
    return r;
}

Rect RectUnion(Rect r1, Rect r2) {
    Rect r;
    r.x = MIN(r1.x, r2.x);
    r.y = MIN(r1.y, r2.y);
    r.w = MAX(r1.x+r1.w, r2.x+r2.w) - r.x;
    r.h = MAX(r1.y+r1.h, r2.y+r2.h) - r.y;
    return r;
}

#endif
