
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

static Rect RectFromPoints(Vec2 p0, Vec2 p1) {
    Rect r;
    r.tl = p0;
    r.wh = SubV2(p1, p0);
    return r;
}

static Rect RectGrowP(Rect r, Vec2 p) {
    r.x = MIN(r.x, p.x);
    r.y = MIN(r.y, p.y);
    r.w = MAX(r.w, p.x - r.x);
    r.h = MAX(r.h, p.y - r.y);
    return r;
}

static Rect RectGrow(Rect r1, Rect r2) {
    Rect r;
    r.x = MIN(r1.x, r2.x);
    r.y = MIN(r1.y, r2.y);
    r.w = MAX(r1.x+r1.w, r2.x+r2.w) - r.x;
    r.h = MAX(r1.y+r1.h, r2.y+r2.h) - r.y;
    return r;
}

static Rect RectUnion(Rect r1, Rect r2) {
    Rect r;
    r.x = MIN(r1.x, r2.x);
    r.y = MIN(r1.y, r2.y);
    r.w = MAX(r1.x+r1.w, r2.x+r2.w) - r.x;
    r.h = MAX(r1.y+r1.h, r2.y+r2.h) - r.y;
    return r;
}

static f32 AngleDifference(f32 a, f32 b) {
    f32 d = (f32) fmod(b - a, 360);
    return (f32) fmod(2*d, 360) - d;
}

static f32 Unlerp(f32 a, f32 b, f32 m) {
    return (m - a)/(b - a);
}

static f32 AngleLerp(f32 a, f32 b, f32 t) {
    return a + t*AngleDifference(a,b);
}

static f32 Approach(f32 x, f32 y, f32 s) {
    if (x < y) {
        return MIN(x + s, y);
    } else {
        return MAX(x - s, y);
    }
}

static Vec2 ApproachV2(Vec2 a, Vec2 b, Vec2 s) {
    a.x = Approach(a.x, b.x, s.x);
    a.y = Approach(a.y, b.y, s.y);
    return a;
}

static Vec3 ApproachV3(Vec3 a, Vec3 b, Vec3 s) {
    a.x = Approach(a.x, b.x, s.x);
    a.y = Approach(a.y, b.y, s.y);
    a.z = Approach(a.z, b.z, s.z);
    return a;
}

static Vec4 ApproachV4(Vec4 a, Vec4 b, Vec4 s) {
    a.x = Approach(a.x, b.x, s.x);
    a.y = Approach(a.y, b.y, s.y);
    a.z = Approach(a.z, b.z, s.z);
    a.w = Approach(a.w, b.w, s.w);
    return a;
}

static Vec2 SnapV2(Vec2 v, s32 grid) {
    v.x = (f32) round(v.x/grid)*grid;
    v.y = (f32) round(v.y/grid)*grid;
    return v;
}

static Vec2 SnapFloorV2(Vec2 v, s32 grid) {
    v.x = (f32) floor(v.x/grid)*grid;
    v.y = (f32) floor(v.y/grid)*grid;
    return v;
}

#endif
