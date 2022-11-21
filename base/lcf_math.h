/** ************************************
  LCF, Created (September 02, 2022)

  Description:
  Math stuff; constants, types, functions, macros.
  
************************************ **/

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

f32 f32_deg2rad(f32 a);
f64 f64_deg2rad(f64 a);
f32 f32_rad2deg(f32 a);
f64 f64_rad2deg(f64 a);
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

/** Vector and Matrix Types          **/
union vec4f32 { 
    struct { f32 x, y, z, w; }; 
    struct { f32 r, g, b, a; }; 
    f32 components[4]; 
}; typedef union vec4f32 vec4f32;

union vec4f64 { 
    struct { f64 x, y, z, w; }; 
    struct { f64 r, g, b, a; }; 
    f64 components[4]; 
}; typedef union vec4f64 vec4f64;

union vec4s32 { 
    struct { s32 x, y, z, w; }; 
    struct { s32 r, g, b, a; }; 
    s32 components[4]; 
}; typedef union vec4s32 vec4s32;

union vec4s64 { 
    struct { s64 x, y, z, w; }; 
    struct { s64 r, g, b, a; }; 
    s64 components[4]; 
}; typedef union vec4s64 vec4s64;

typedef vec4f32 vec4;

union vec3f32 { 
    struct { f32 x,y,z; }; 
    struct { f32 r,g,b; }; 
    f32 components[3]; 
}; typedef union vec3f32 vec3f32;

union vec3f64 { 
    struct { f64 x,y,z; }; 
    struct { f64 r,g,b; }; 
    f64 components[3]; 
}; typedef union vec3f64 vec3f64;

union vec3s32 { 
    struct { s32 x,y,z; }; 
    struct { s32 r,g,b; }; 
    s32 components[3]; 
}; typedef union vec3s32 vec3s32;

union vec3s64 { 
    struct { s64 x,y,z; }; 
    struct { s64 r,g,b; }; 
    s64 components[3]; 
}; typedef union vec3s64 vec3s64;

typedef vec3f32 vec3;

union vec2f32 {        
    struct { f32 x,y; }; 
    struct { f32 u,v; }; 
    struct { f32 w,h; };
    struct { f32 l,r; }; 
    f32 components[2]; 
}; typedef union vec2f32 vec2f32;

union vec2f64 {        
    struct { f64 x,y; }; 
    struct { f64 u,v; }; 
    struct { f64 w,h; };
    struct { f64 l,r; }; 
    f64 components[2]; 
}; typedef union vec2f64 vec2f64;

union vec2s32 {        
    struct { s32 x,y; }; 
    struct { s32 u,v; }; 
    struct { s32 w,h; };
    struct { s32 l,r; }; 
    s32 components[2]; 
}; typedef union vec2s32 vec2s32;

union vec2s64 {        
    struct { s64 x,y; }; 
    struct { s64 u,v; }; 
    struct { s64 w,h; };
    struct { s64 l,r; }; 
    s64 components[2]; 
}; typedef union vec2s64 vec2s64;

typedef vec2f32 vec2;

union mat4f32 { 
    struct { vec4f32 x,y,z,w; };
    struct { vec4f32 vec[4]; };
    struct { f32 _comps[4*3]; vec3f32 translation; }; 
    f32 m[4][4];
    f32 components[4*4];
}; typedef union mat4f32 mat4f32;

union mat4f64 { 
    struct { vec4f64 x,y,z,w; };
    struct { vec4f64 vec[4]; };
    struct { f64 _comps[4*3]; vec3f64 translation; }; 
    f64 m[4][4];
    f64 components[4*4];
}; typedef union mat4f64 mat4f64;

typedef mat4f32 mat4;

union mat2f32 {              
    struct { vec2f32 x,y; };
    struct { vec2f32 vec[2]; };
    f32 m[2][2];
    f32 components[2*2];
}; typedef union mat2f32 mat2f32;

union mat2f64 {              
    struct { vec2f64 x,y; };
    struct { vec2f64 vec[2]; };
    f64 m[2][2];
    f64 components[2*2];
}; typedef union mat2f64 mat2f64;

typedef mat2f32 mat2;

/** ******************************** **/

#endif
