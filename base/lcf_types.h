/** ************************************
  LCF, Created (September 02, 2022)

  Description:
  Basic types, constants and macros.
  
************************************ **/
#if !defined(LCF_TYPES)
#define LCF_TYPES "1.0.0"

/** Convenience Macros               **/
/* Keywords */
#define global static
#define local static
#define internal static
#define c_linkage extern "C"
#define and &&
#define or ||
#define not !
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* #if OS_WINDOWS */
/* #pragma section(".roglob", read) */
/* #define read_only __declspec(allocate(".roglob")) */
/* #else */
/* /\* TODO(rjf): figure out if this benefit is possible on non-Windows *\/ */
/* #define read_only */
/* #endif */
#define read_only 

/* Bits/Flags */
#define TEST_FLAG(fl,fi) ((fl)&(fi))
#define SET_FLAG(fl,fi) ((fl)|=(fi))
#define UNSET_FLAG(fl,fi) ((fl)&=~(fi))
#define TOGGLE_FLAG(fl,fi) ((fl)^=(fi))

/* Math  */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x,a,b) (((x)<(a))?(a):((b)<(x))?(b):(x))
#define CLAMPTOP(a,b) MIN(a,b)
#define CLAMPBOTTOM(a,b) MAX(a,b)
#define ISPOWER2(x) (x & (x-1) == 0)

/* Assertion */
#define STATEMENT(S) do { S } while(0)

#undef ASSERT
#if LCF_DISABLE_ASSERT
    #define ASSERT(C)
    #define ASSERT_STATIC(C,label)
    #define NotImplemented 
    #define InvalidPath 
#else
    #define ASSERT_KILL() (*(int*)0=0)
#define ASSERT(C) STATEMENT( if (!(C)) { ASSERT_KILL(); })
    #define ASSERT_STATIC(C,label) u8 static_assert_##label[(C)?(-1):(1)]
    #define NotImplemented Assert(!"Not Implemented")
    #define InvalidPath Assert(!"Invalid Path")
#endif

/* Misc */
#define ARRAY_LENGTH(A) (sizeof(A)/sizeof(*(A)))
#define PTR_TO_INT(P) (unsigned long long)((char*)p - (char*)0)
#define INT_TO_PTR(I) (void*)((char*)0 + (n))
#define MEMBER(T,M) (((T*)0)->m)
#define MEMBER_OFFSET(T,M) PTR_TO_INT(&MEMBER(T,M))
#define MACRO_EXPAND(S) #S
#define _MACRO_CONCAT(S1,S2) S1##S2
#define MACRO_CONCAT(S1,S2) _MACRO_CONCAT(S1,S2)
#define MACRO_VAR(var) var##__FILE__##__LINE__
#define SWAP(T,a,b) do{ T t__ = a; a = b; b = t__; }while(0)


/** ******************************** **/


/** Portable, Abbreviated Primitive Types  **/
#include <stdint.h>
#define TYPE_MIN(t,val) read_only global t MACRO_CONCAT(t,_MIN) = ((t) val)
#define TYPE_MAX(t,val) read_only global t MACRO_CONCAT(t,_MAX) = ((t) val)

/* Signed Int */
typedef int8_t i8;   TYPE_MIN(i8, 0x80);        TYPE_MAX(i8,0x7F);
typedef int16_t i16; TYPE_MIN(i16, 0x8000);     TYPE_MAX(i16,0x7FFF);
typedef int32_t i32; TYPE_MIN(i32, 0x800000);   TYPE_MAX(i32,0x7FFFFF);
typedef int64_t i64; TYPE_MIN(i64, 0x80000000); TYPE_MAX(i64,0x7FFFFFFF);

/* Unsigned Int */
typedef uint8_t u8;   TYPE_MAX(u8, 0xFF);
typedef uint16_t u16; TYPE_MAX(u16, 0xFFFF);
typedef uint32_t u32; TYPE_MAX(u32, 0xFFFFFF);
typedef uint64_t u64; TYPE_MAX(u64, 0xFFFFFFFF);

#undef TYPE_MIN
#undef TYPE_MAX

/* Bool/Bits */
typedef u8 b8;
typedef u16 b16;
typedef i32 b32;
typedef i64 b64;

/* Floating Point */
typedef float f32; 
read_only global f32 f32_MIN = -3.4028234664e+38;
read_only global f32 f32_MAX = 3.4028234664e+38;
read_only global f32 f32_EPSILON = 5.96046448e-8;
read_only global f32 f32_TAU = 6.28318530718f;
read_only global f32 f32_PI = 3.14159265359f;

read_only global u32 SignF32 = 0x80000000;
read_only global u32 ExponentF32 = 0x7F800000;
read_only global u32 MantissaF32 = 0x7FFFFF;

f32 f32_pos_inf(void);
f32 f32_neg_inf(void);
f32 f32_abs(f32 f);
f32 f32_sign(f32 f); /* NOTE(lcf): sign of an IEEE754 float is never 0. */

typedef double f64; global f64 f64_epsilon = 2.220446e-16;
f64 f64_pos_inf(void);
f64 f64_neg_inf(void);
f64 f64_abs(f64 f);
f64 f64_sign(f64 f);

/** ******************************** **/
#endif