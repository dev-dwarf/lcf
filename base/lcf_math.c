#include "lcf_math.h"
#include <math.h>

/** Radians / Degree Conversions     **/
f32 f32_to_radians(f32 a) {
    return a*f32_pi*0.00555555555f;
}
f64 f64_to_radians(f64 a) {
    return a*f64_pi*0.00555555555;
}
f32 f32_to_degrees(f32 a) {
    return a*180.0f*f32_pi_inv;
}
f64 f64_to_degrees(f64 a) {
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
    return fmodf(a+f32_angle_difference(a,b)*p, 360.0f);
}
f64 f64_angle_lerp(f64 a, f64 b, f64 p) {
    return fmod(a+f64_angle_difference(a,b)*p, 360.0);
}
f32 f32_radian_difference(f32 a, f32 b) {
    return fmodf(f32_pi + a - b, f32_tau) - f32_pi;
}
f64 f64_radian_difference(f64 a, f64 b) {
     return fmod(f64_pi + a - b, f64_tau) - f64_pi;   
}
f32 f32_radian_lerp(f32 a, f32 b, f32 p) {
    return fmodf(a+f32_radian_difference(a,b)*p, f32_tau);  
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

/** ******************************** **/
