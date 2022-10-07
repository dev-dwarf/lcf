/** ************************************
  LCF, Created (September 02, 2022)

  Description:
  Math stuff; constants, types, functions, macros.
  
************************************ **/

/* TODO: I am no longer happy with the use of macros here. Lets do something better. */

#if !defined(LCF_MATH)
#define LCF_MATH "1.0.0"

/** Constants, Radians, and Conversions **/
#include "lcf_types.h"
global f32 f32_pi  = 3.14159265359f;
global f32 f32_pi_inv = 0.31830988618f;
global f32 f32_tau = 6.28318530718f;
global f32 f32_e = 2.71828182846f;
global f32 f32_golden_ratio = 1.61803398875f;
global f32 f32_golden_ratio_inv = 0.61803398875f;

global f64 f64_pi  = 3.14159265359;
global f64 f64_pi_inv = 0.31830988618;
global f64 f64_tau = 6.28318530718;
global f64 f64_e = 2.71828182846;
global f64 f64_golden_ratio = 1.61803398875;
global f64 f64_golden_ratio_inv = 0.61803398875;

f32 f32_to_radians(f32 a);
f64 f64_to_radians(f64 a);
f32 f32_to_degrees(f32 a);
f64 f64_to_degrees(f64 a);
/** ******************************** **/

/** Floating point math functions    **/
// TODO: truncate, floor, ceil, mod?
f32 f32_lerp(f32 a, f32 b, f32 p);
f64 f64_lerp(f64 a, f64 b, f64 p);
f32 f32_angle_difference(f32 a, f32 b);
f64 f64_angle_difference(f64 a, f64 b);
f32 f32_angle_lerp(f32 a, f32 b, f32 p);
f64 f64_angle_lerp(f64 a, f64 b, f64 p);
f32 f32_radian_difference(f32 a, f32 b);
f64 f64_radian_difference(f64 a, f64 b);
f32 f32_radian_lerp(f32 a, f32 b, f32 p);
f64 f64_radian_lerp(f64 a, f64 b, f64 p);
f32 f32_approach(f32 x, f32 t, f32 s);
f64 f64_approach(f64 x, f64 t, f64 s);
/** ******************************** **/

/** Vec, Rect, Interval Types        **/
#define VEC2(T) v2##T
#define VEC3(T) v3##T
#define VEC4(T) v4##T
#define RECT(T) rc##T
#define INTERVAL(T) iv##T
#define DEF_FOR_ALL_TYPES(DEF) DEF(i32); DEF(i64); DEF(f32); DEF(f64); 
#define DEFVEC2(T) union VEC2(T) {           \
        struct {T x; T y;};                   \
        T v[2];                               \
    }
DEF_FOR_ALL_TYPES(DEFVEC2);
#undef DEFVEC2
#define DEFVEC3(T) union VEC3(T) {          \
        struct { T x; T y; T z; };                     \
        struct { T r; T g; T b; };                     \
        T v[3];                                        \
    }
DEF_FOR_ALL_TYPES(DEFVEC3);
#undef DEFVEC3
#define DEFVEC4(T) union VEC4(T) {                         \
        struct { T x; T y; T z; T w;};                      \
        struct { T r; T g; T b; T a;};                      \
        T v[4];                                             \
    }
DEF_FOR_ALL_TYPES(DEFVEC4);
#undef DEFVEC4
#define DEFRECT(T) union RECT(T) {                                  \
        struct { T x1; T y1; T x2; T y2; };                         \
        struct { VEC2(T) p1; VEC2(T) p2; };   \
    }
DEF_FOR_ALL_TYPES(DEFRECT);
#undef DEFRECT
#define DEFINTERVAL(T) union INTERVAL(T) {       \
        struct { T min; T max; };                 \
        T iv[2];                                  \
    }
DEF_FOR_ALL_TYPES(DEFINTERVAL);
#undef DEFINTERVAL

/** ******************************** **/


/** Vec, Rect, Interval Functions    **/
#define _MAKE(V) make_##V
#define MAKE(V) V _MAKE(V)
#define DEFMAKEV2(T) MAKE(VEC2(T))(T x, T y);
DEF_FOR_ALL_TYPES(DEFMAKEV2);
#undef DEFMAKEV2
#define DEFMAKEV3(T) MAKE(VEC3(T))(T xr, T yg, T zb);
DEF_FOR_ALL_TYPES(DEFMAKEV3);
#undef DEFMAKEV3
#define DEFMAKEV4(T) MAKE(VEC4(T))(T xr, T yg, T zb, T wa);
DEF_FOR_ALL_TYPES(DEFMAKEV4);
#undef DEFMAKEV4
#define DEFMAKERECT(T) MAKE(RECT(T))(T x1, T y1, T x2, T y2);
DEF_FOR_ALL_TYPES(DEFMAKERECT);
#undef DEFMAKERECT
#define DEFMAKEINTERVAL(T) MAKE(INTERVAL(T))(T min, T max);
DEF_FOR_ALL_TYPES(DEFMAKEINTERVAL);
#undef DEFMAKEINTERVAL
#undef _MAKE
#undef MAKE

#define OPS(T,V)                                                     \
    V operator+(const V &a, const V &b);                                \
    V operator-(const V &a, const V &b);                                \
    V operator*(const V &a, const T &b);                                \
    V operator*(const T &a, const V &b);                                
#define DEFOPS(T)                               \
    OPS(T,VEC2(T));                             \
    OPS(T,VEC3(T));                             \
    OPS(T,VEC4(T));                             
DEF_FOR_ALL_TYPES(DEFOPS);
#undef OPS
#undef DEFOPS

#define DOTNAME(V) V##_dot
#define DOT(T,V) \
    T DOTNAME(V)(V a, V b)
#define DEFDOT(T) \
    DOT(T, VEC2(T)); \
    DOT(T, VEC3(T)); \
    DOT(T, VEC4(T)); 
DEF_FOR_ALL_TYPES(DEFDOT);
#undef DOTNAME
#undef DOT
#undef DEFDOT

/** ******************************** **/
#undef VEC2
#undef VEC3
#undef VEC4
#undef RECT
#undef INTERVAL
#undef DEF_FOR_ALL_TYPES

#endif
