/** ************************************
  LCF, Created (September 02, 2022)

  Description:
  Types, constants, and macros that I like to use to wrap primitives.
  
  Changelog:

************************************ **/
#if !defined(LCF_TYPES)
#define LCF_TYPES

/** Convenience Macros               **/
/* Keywords */
#define global static
#define local static
#define internal static
#define c_linkage extern "C"
#define and &&
#define or ||

/* Math  */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(a,x,b) (((x)<(a))?(a):((b)<(x))?(b):(x))
#define CLAMPTOP(a,b) MIN(a,b)
#define CLAMPBOTTOM(a,b) MAX(a,b)

/* Assertion */
#define STATEMENT(S) do { S } while(0)

#if !defined(ASSERT_KILL)
    #define ASSERT_KILL() (*(int*)0=0)
#endif

#if LCF_DISABLE_ASSERT
    #define ASSERT(C) STATEMENT( if (!(c)) { ASSERTKILL() })
#else
    #define ASSERT(C)
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

/* Sizes */
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)

/* Memory */
/* #define MEMORY_ZERO(p, z) */

/** ******************************** **/


/** Portable, Short Primitive Types  **/
#include <stdint.h>
#define TYPE_MIN(t,val) global t MACRO_CONCAT(t,_min) = ((t) val)
#define TYPE_MAX(t,val) global t MACRO_CONCAT(t,_max) = ((t) val)

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
typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

/* Floating Point */
typedef float f32; global f32 f32_epsilon = 1.1920929e-7f;
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
