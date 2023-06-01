
#include "lcf_types.h"

/* Unions to help manipulate bits of floats directly
   TODO(lcf): should these unions be in types.h?
   Consider if these would be useful anywhere else.
 */
union _f32_bits { f32 f; u32 bits; };
union _f64_bits { f64 f; u64 bits; };


f32 f32_abs(f32 f) {
    union _f32_bits x;
    x.f = f;
    x.bits &= 0x7FFFFFFF;
    return x.f;
}

f32 f32_sign(f32 f) {
    union _f32_bits x;
    x.f = f;
    return (x.bits & 0x80000000)? -1.0f : 1.0f;
}

f64 f64_abs(f64 f) {
    union _f64_bits x;
    x.f = f;
    x.bits &= 0x7FFFFFFFFFFFFFFF;
    return x.f;
}

f64 f64_sign(f64 f) {
    union _f64_bits x;
    x.f = f;
    return (x.bits & 0x8000000000000000)? -1.0f : 1.0f;
}
