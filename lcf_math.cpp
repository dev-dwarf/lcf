p#include "lcf_math.h"
#include <math.h>

/** Radians / Degree Conversions     **/
f32 f32_radians(f32 a) {
    return a*f32_pi*0.00555555555f;
}
f64 f64_radians(f64 a) {
    return a*f64_pi*0.00555555555;
}
f32 f32_degrees(f32 a) {
    return a*180.0f*f32_pi_inv;
}
f64 f64_degrees(f64 a) {
    return a*180.0*f64_pi_inv;
}
/** ******************************** **/

/** Floating point math functions    **/
f32 f32_lerp(f32 a, f32 b, f32 p){
    return a + (b-a)*p;
}
f64 f64_lerp(f64 a, f64 b, f64 p) {
    return a + (b-a)*p;
}
f32 f32_angle_difference(f32 a, f32 b) {
    return fmod(180.0f + a - b, 360.0f) - 180.0f;
}
f64 f64_angle_difference(f64 a, f64 b) {
     return fmod(180.0 + a - b, 360.0) - 180.0;   
}
f32 f32_angle_lerp(f32 a, f32 b, f32 p) {
    return fmod(a+f32_angle_difference(a,b)*p, 360.0f);
}
f64 f64_angle_lerp(f64 a, f64 b, f64 p) {
    return fmod(a+f64_angle_difference(a,b)*p, 360.0);
}
f32 f32_radian_difference(f32 a, f32 b) {
    return fmod(f32_pi + a - b, f32_tau) - f32_pi;
}
f64 f64_radian_difference(f64 a, f64 b) {
     return fmod(f64_pi + a - b, f64_tau) - f64_pi;   
}
f32 f32_radian_lerp(f32 a, f32 b, f32 p) {
    return fmod(a+f32_radian_difference(a,b)*p, f32_tau);  
}
f64 f64_radian_lerp(f64 a, f64 b, f64 p) {
    return fmod(a+f64_radian_difference(a,b)*p, f64_tau);
}
f32 f32_approach(f32 x, f32 t, f32 s) {
    if (x < t) {
        return CLAMPTOP(x+s,t);
    } else {
        return CLAMPBOTTOM(x-s, t);
    }
}
f64 f64_approach(f64 x, f64 t, f64 s) {
    if (x < t) {
        return CLAMPTOP(x+s,t);
    } else {
        return CLAMPBOTTOM(x-s, t);
    }
}
/** ******************************** **/

/** Vec, Rect, Interval Functions    **/
#define VEC2(T) v2##T
#define VEC3(T) v3##T
#define VEC4(T) v4##T
#define RECT(T) rc##T
#define INTERVAL(T) iv##T
#define DEF_FOR_ALL_TYPES(DEF) DEF(i32); DEF(i64); DEF(f32); DEF(f64); 
#define ADDV2(T, V) V operator+(const V &a, const V &b) {   \
        return { a.x + b.x, a.y + b.y };                    \
    }
#define ADDV3(T, V) V operator+(const V &a, const V &b) {   \
        return { a.x + b.x, a.y + b.y, a.z + b.z };         \
    }
#define ADDV4(T, V) V operator+(const V &a, const V &b) {       \
        return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };  \
    }
#define DEFADD(T)                               \
    ADDV2(T, VEC2(T));                          \
    ADDV3(T, VEC3(T));                          \
    ADDV4(T, VEC4(T)); 
DEF_FOR_ALL_TYPES(DEFADD);
#undef ADDV2
#undef ADDV3
#undef ADDV4
#undef DEFADD

#define SUBV2(T, V) V operator-(const V &a, const V &b) {   \
        return { a.x - b.x, a.y - b.y };                    \
    }
#define SUBV3(T, V) V operator-(const V &a, const V &b) {   \
        return { a.x - b.x, a.y - b.y, a.z - b.z };         \
    }
#define SUBV4(T, V) V operator-(const V &a, const V &b) {       \
        return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };  \
    }
#define DEFSUB(T)                               \
    SUBV2(T, VEC2(T));                          \
    SUBV3(T, VEC3(T));                          \
    SUBV4(T, VEC4(T)); 
DEF_FOR_ALL_TYPES(DEFSUB);
#undef SUBV2
#undef SUBV3
#undef SUBV4
#undef DEFSUB

#define MULV2(T, V) V operator*(const T &a, const V &b) {   \
        return { a * b.x, a * b.y };                        \
    }
#define MULV3(T, V) V operator*(const T &a, const V &b) {   \
        return { a * b.x, a * b.y, a * b.z};                \
    }
#define MULV4(T, V) V operator*(const T &a, const V &b) {   \
        return { a * b.x, a * b.y, a * b.z, a * b.w};       \
    }
#define DEFMUL(T)                               \
    MULV2(T, VEC2(T));                          \
    MULV3(T, VEC3(T));                          \
    MULV4(T, VEC4(T)); 
DEF_FOR_ALL_TYPES(DEFMUL);
#undef MULV2
#undef MULV3
#undef MULV4
#undef DEFMUL

#define MULV2(T, V) V operator*(const V &a, const T &b) {   \
        return { b * a.x, b * a.y };                        \
    }
#define MULV3(T, V) V operator*(const V &a, const T &b) {   \
        return { b * a.x, b * a.y, b * a.z};                \
    }
#define MULV4(T, V) V operator*(const V &a, const T &b) {   \
        return { b * a.x, b * a.y, b * a.z, b * a.w};       \
    }
#define DEFMUL(T)                               \
    MULV2(T, VEC2(T));                          \
    MULV3(T, VEC3(T));                          \
    MULV4(T, VEC4(T)); 
DEF_FOR_ALL_TYPES(DEFMUL);
#undef MULV2
#undef MULV3
#undef MULV4
#undef DEFMUL

#define DOTNAME(V) V##_dot
#define DOTV2(T, V) T DOTNAME(V)(const V &a, const V &b) {  \
        return a.x * b.x + a.y * b.y;                       \
    }
#define DOTV3(T, V) T DOTNAME(V)(const V &a, const V &b) {  \
        return a.x * b.x + a.y * b.y + a.z * b.z;           \
    }
#define DOTV4(T, V) T DOTNAME(V)(const V &a, const V &b) {      \
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;   \
    }
#define DEFDOT(T)                               \
    DOTV2(T, VEC2(T));                          \
    DOTV3(T, VEC3(T));                          \
    DOTV4(T, VEC4(T)); 
DEF_FOR_ALL_TYPES(DEFDOT);
#undef DOTV2
#undef DOTV3
#undef DOTV4
#undef DEFDOT



#undef VEC2
#undef VEC3
#undef VEC4
#undef RECT
#undef INTERVAL
#undef DEF_FOR_ALL_TYPES
/** ******************************** **/
