/* xoroshiro 128+
   REF: https://prng.di.unimi.it/xoroshiro128plus.c
*/
typedef struct RNG {
    u64 s[2];
    f64 n64;
    f32 n32;
    u32 has_normal;
} RNG;

inline u64 randu64(RNG *r) {
    u64 s0 = r->s[0];
    u64 s1 = r->s[1];
    u64 out = s0 + s1;

    s1 ^= s0;
    r->s[0] = ((s0 << 24) | (s0 >> 40)) ^ s1 ^ (s1 << 16);
    r->s[1] = ((s1 << 37) | (s1 >> 27));

    return out;
}

inline u32 randu32(RNG *r) {
    return randu64(r) >> 32;
}

inline f32 randf32(RNG *r) { /* [0..1] */
    u32 s = randu64(r) >> 32;
    s = (0x7F << 23) | (s >> 9);
    f32 f = *((f32*) &s);
    return f - 1.0f;
}

inline f32 randf32_range(RNG *r, f32 a, f32 b) {
    f32 m = randf32(r);
    return b*m + a*(1.0f - m);
}

inline f64 randf64(RNG *r) {
    u64 s = randu64(r);
    s = (0x3FFLL << 52) | (s >> 12);
    f64 f = *((f64*) &s);
    return f - 1.0;
}

inline f64 randf64_range(RNG *r, f64 a, f64 b) {
    f64 m = randf64(r);
    return b*m + a*(1.0 - m);
}

inline s32 rands32_range(RNG *r, s32 a, s32 b) {
    s32 m = b - a;
    return (s32)(m*randf32(r)) + a;
}
 
inline b32 randbool(RNG *r) {
    u64 s = randu64(r);
    return *((s64*) &s) >= 0;
}

inline s32 randsign(RNG *r) {
    return randbool(r)? 1 : -1;
}

#include <math.h>
inline f32 rand_normalf32(RNG *r) {
    if (r->has_normal) {
        r->has_normal = false;
        return r->n32;
    }

    f32 x,y,R;

    do {
        /* inlined equivalent to randf32_range(r, -1, 1) to not make an extra randu64 call */
        u64 s = randu64(r);
        u32 sx = (u32) s;
        u32 sy = (u32) (s >> 32);
        sx = ((0x7F << 23) | (sx >> 9));
        sy = ((0x7F << 23) | (sy >> 9));
        x = *((f32*) &sx);
        y = *((f32*) &sy);
        x -= 1.5f; x *= 2.0;
        y -= 1.5f; y *= 2.0; 
        R = x*x + y*y;
    } while (R > 1.0f);

    f32 f = sqrtf(-2.0f*logf(R)/R);
    r->n32 = f*x; r->has_normal = true;
    return f*y;
}

inline f64 rand_normalf64(RNG *r) {
    if (r->has_normal) {
        r->has_normal = false;
        return r->n64;
    }

    f64 x,y,R;

    do { /* Box-Mueller, Marsaglia Polar */
        x = randf64(r)*2.0 - 1.0;
        y = randf64(r)*2.0 - 1.0;
        R = x*x + y*y;
    } while (R > 1.0f);

    f64 f = sqrt(-2.0*log(R)/R);
    r->n64 = f*x; r->has_normal = true;
    return f*y;
}

