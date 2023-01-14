#if !defined(LCF_TYPES)
#define LCF_TYPES "1.0.0"

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

#ifdef __cplusplus
#define ZERO_STRUCT {}
#else
#define ZERO_STRUCT {0}
#endif

#if COMPILER_CL
#define per_thread __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
#define per_thread __thread
#endif

#if OS_WINDOWS
#pragma section(".roglob", read)
#define read_only __declspec(allocate(".roglob"))
#else
/* TODO(rjf): figure out if this benefit is possible on non-Windows */
#define read_only
#endif

/* Bits/Flags */
#define TEST_FLAG(fl,fi) ((fl)&(fi))
#define SET_FLAG(fl,fi) ((fl)|=(fi))
#define REM_FLAG(fl,fi) ((fl)&=~(fi))
#define TOGGLE_FLAG(fl,fi) ((fl)^=(fi))
#define FLAG(I) (1 << (I))

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
    #define ASSERTM(C, M) ASSERT((M) && (C))
    #define ASSERTSTATIC(C,label) STATEMENT(                           \
        u8 static_assert_##label[(C)?(1):(-1)];                         \
        (void) static_assert_##label; )
    #define NOTIMPLEMENTED() ASSERTM(0, "Not Implemented")
    #define BADPATH(M) ASSERTM(0, M)
#endif

/* Misc */
#define ARRAY_LENGTH(A) (sizeof(A)/sizeof(*(A)))
#define PTR_TO_INT(P) (unsigned long long)((char*)p - (char*)0)
#define INT_TO_PTR(I) (void*)((char*)0 + (n))
#define MEMBER(T,M) (((T*)0)->m)
#define MEMBER_OFFSET(T,M) PTR_TO_INT(&MEMBER(T,M))
#define STRINGIFY(SYMB) #SYMB
#define MACRO_EXPAND(S) #S
#define _MACRO_CONCAT(S1,S2) S1##S2
#define MACRO_CONCAT(S1,S2) _MACRO_CONCAT(S1,S2)
#define MACRO_VAR(var) MACRO_CONCAT(var, __LINE__)
#define SWAP(T,a,b) do{ T t__ = a; a = b; b = t__; }while(0)
#define DEFER_LOOP(start,end) start; for(int ___i = (0); ___i == 0; ___i += 1, end)
    
/** ******************************** **/


/** Portable, Abbreviated Primitive Types  **/
#include <stdint.h>

/* Signed Int */
typedef int8_t s8;
read_only global s8 s8_MAX = 0x7F;
read_only global s8 s8_MIN = -1 - 0x7F;
typedef int16_t s16;
read_only global s16 s16_MAX = 0x7FFF;
read_only global s16 s16_MIN = -1 - 0x7FFF;
typedef int32_t s32;
read_only global s32 s32_MAX = 0x7FFFFFFF;
read_only global s32 s32_MIN = -1 - 0x7FFFFFFF;
typedef int64_t s64;
read_only global s64 s64_MAX = 0x7FFFFFFFFFFFFFFF;
read_only global s64 s64_MIN = -1 - 0x7FFFFFFFFFFFFFFF;

/* Unsigned Int */
typedef uint8_t u8;
read_only global u8 u8_MAX = 0xFF;
read_only global u8 u8_MIN = 0;
typedef uint16_t u16;
read_only global u16 u16_MAX = 0xFFFF;
read_only global u16 u16_MIN = 0;
typedef uint32_t u32;
read_only global u32 u32_MAX = 0xFFFFFFFF;
read_only global u32 u32_MIN = 0;
typedef uint64_t u64;
read_only global u64 u64_MAX = 0xFFFFFFFFFFFFFFFF;
read_only global u64 u64_MIN = 0;

/* Floating Point */
typedef float f32; 
read_only global f32 f32_MIN = (f32)-3.4028234664e+38;
read_only global f32 f32_MAX = (f32) 3.4028234664e+38;
read_only global f32 f32_EPSILON = (f32) 5.96046448e-8;
read_only global f32 f32_TAU = (f32) 6.28318530718;
read_only global f32 f32_PI = (f32) 3.14159265359f;

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

/* Bool/Bits */
typedef u8 b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

/* Pointers */
typedef intptr_t spr;
typedef uintptr_t upr;

    
/** ******************************** **/
#endif
