
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

f32 AngleDifference(f32 a, f32 b) {
    f32 d = fmod(b - a, 360);
    return fmod(2*d, 360) - d;
}

f32 AngleLerp(f32 a, f32 b, f32 t) {
    return a + t*AngleDifference(a,b);
}

f32 Approach(f32 x, f32 y, f32 s) {
    if (x < y) {
        return MIN(x + s, y);
    } else {
        return MAX(x - s, y);
    }
}

Vec2 ApproachV2(Vec2 a, Vec2 b, Vec2 s) {
    return (Vec2){
        .x = Approach(a.x, b.x, s.x),
        .y = Approach(a.y, b.y, s.y),
    };
}

Vec3 ApproachV3(Vec3 a, Vec3 b, Vec3 s) {
    return (Vec3){
        .x = Approach(a.x, b.x, s.x),
        .y = Approach(a.y, b.y, s.y),
        .z = Approach(a.z, b.z, s.z),
    };
}

Vec4 ApproachV4(Vec4 a, Vec4 b, Vec4 s) {
    return (Vec4){
        .x = Approach(a.x, b.x, s.x),
        .y = Approach(a.y, b.y, s.y),
        .z = Approach(a.z, b.z, s.z),
        .w = Approach(a.w, b.w, s.w),
    };
}

Vec2 SnapV2(Vec2 v, s32 grid) {
    return (Vec2) {
        .x = round(v.x/grid)*grid,
        .y = round(v.y/grid)*grid,
    };
}

Vec2 FloorV2(Vec2 v, s32 grid) {
    return (Vec2) {
        .x = floor(v.x/grid)*grid,
        .y = floor(v.y/grid)*grid,
    };
}

#endif
