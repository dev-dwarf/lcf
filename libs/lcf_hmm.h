/* NOTE(lcf) This is a modified version of HandmadeMath, with C++ and
    some other things I dont like removed. Also with shorter naming.
*/
/*
  HandmadeMath.h v2.0.0

  This is a single header file with a bunch of useful types and functions for
  games and graphics. Consider it a lightweight alternative to GLM that works
  both C and C++.

  =============================================================================
  CONFIG
  =============================================================================

  By default, all angles in Handmade Math are specified in radians. However, it
  can be configured to use degrees or turns instead. Use one of the following
  defines to specify the default unit for angles:

    #define LCF_HMM_USE_RADIANS
    #define LCF_HMM_USE_DEGREES
    #define LCF_HMM_USE_TURNS

  Regardless of the default angle, you can use the following functions to
  specify an angle in a particular unit:

    AngleRad(radians)
    AngleDeg(degrees)
    AngleTurn(turns)

  The definitions of these functions change depending on the default unit.

  -----------------------------------------------------------------------------

  Handmade Math ships with SSE (SIMD) implementations of several common
  operations. To disable the use of SSE intrinsics, you must define
  LCF_HMM_NO_SSE before including this file:

    #define LCF_HMM_NO_SSE
    #include "HandmadeMath.h"

  -----------------------------------------------------------------------------

  To use Handmade Math without the C runtime library, you must provide your own
  implementations of basic math functions. Otherwise, HandmadeMath.h will use
  the runtime library implementation of these functions.

  Define LCF_HMM_PROVIDE_MATH_FUNCTIONS and provide your own
  implementations of SINF, COSF, TANF, ACOSF, and SQRTF
  before including HandmadeMath.h, like so:

    #define LCF_HMM_PROVIDE_MATH_FUNCTIONS
    #define SINF MySinF
    #define COSF MyCosF
    #define TANF MyTanF
    #define ACOSF MyACosF
    #define SQRTF MySqrtF
    #include "HandmadeMath.h"

  By default, it is assumed that your math functions take radians. To use
  different units, you must define ANGLE_USER_TO_INTERNAL and
  ANGLE_INTERNAL_TO_USER. For example, if you want to use degrees in your
  code but your math functions use turns:

    #define ANGLE_USER_TO_INTERNAL(a) ((a)*DegToTurn)
    #define ANGLE_INTERNAL_TO_USER(a) ((a)*TurnToDeg)

  =============================================================================

  LICENSE

  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.

  =============================================================================

  CREDITS

  Originally written by Zakary Strange.

  Functionality:
   Zakary Strange (strangezak@protonmail.com && @strangezak)
   Matt Mascarenhas (@miblo_)
   Aleph
   FieryDrake (@fierydrake)
   Gingerbill (@TheGingerBill)
   Ben Visness (@bvisness)
   Trinton Bullard (@Peliex_Dev)
   @AntonDan
   Logan Forman (@dev_dwarf)

  Fixes:
   Jeroen van Rijn (@J_vanRijn)
   Kiljacken (@Kiljacken)
   Insofaras (@insofaras)
   Daniel Gibson (@DanielGibson)
*/

#ifndef LCF_HMM_H
#define LCF_HMM_H

#ifdef LCF_HMM_NO_SSE
# warning "LCF_HMM_NO_SSE is deprecated, use LCF_HMM_NO_SIMD instead"
# define LCF_HMM_NO_SIMD
#endif 

/* let's figure out if SSE is really available (unless disabled anyway)
   (it isn't on non-x86/x86_64 platforms or even x86 without explicit SSE support)
   => only use "#ifdef LCF_HMM__USE_SSE" to check for SSE support below this block! */
#ifndef LCF_HMM_NO_SIMD
# ifdef _MSC_VER /* MSVC supports SSE in amd64 mode or _M_IX86_FP >= 1 (2 means SSE2) */
#  if defined(_M_AMD64) || ( defined(_M_IX86_FP) && _M_IX86_FP >= 1 )
#   define LCF_HMM__USE_SSE 1
#  endif
# else /* not MSVC, probably GCC, clang, icc or something that doesn't support SSE anyway */
#  ifdef __SSE__ /* they #define __SSE__ if it's supported */
#   define LCF_HMM__USE_SSE 1
#  endif /*  __SSE__ */
# endif /* not _MSC_VER */
# ifdef __ARM_NEON
#  define LCF_HMM__USE_NEON 1
# endif /* NEON Supported */
#endif /* #ifndef LCF_HMM_NO_SIMD */

#ifdef LCF_HMM__USE_SSE
# include <xmmintrin.h>
#endif

#ifdef LCF_HMM__USE_NEON
# include <arm_neon.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wfloat-equal"
# if (defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 8)) || defined(__clang__)
#  pragma GCC diagnostic ignored "-Wmissing-braces"
# endif
# ifdef __clang__
#  pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# endif
#endif

#if defined(__GNUC__) || defined(__clang__)
# define DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
# define DEPRECATED(msg) __declspec(deprecated(msg))
#else
# define DEPRECATED(msg)
#endif

#if !defined(LCF_HMM_USE_DEGREES) \
    && !defined(LCF_HMM_USE_TURNS) \
    && !defined(LCF_HMM_USE_RADIANS)
# define LCF_HMM_USE_RADIANS
#endif

#define PI 3.14159265358979323846
#define PI32 3.14159265359f
#define DEG180 180.0
#define DEG18032 180.0f
#define TURNHALF 0.5
#define TURNHALF32 0.5f
#define RadToDeg ((float)(DEG180/PI))
#define RadToTurn ((float)(TURNHALF/PI))
#define DegToRad ((float)(PI/DEG180))
#define DegToTurn ((float)(TURNHALF/DEG180))
#define TurnToRad ((float)(PI/TURNHALF))
#define TurnToDeg ((float)(DEG180/TURNHALF))

#if defined(LCF_HMM_USE_RADIANS)
# define AngleRad(a) (a)
# define AngleDeg(a) ((a)*DegToRad)
# define AngleTurn(a) ((a)*TurnToRad)
#elif defined(LCF_HMM_USE_DEGREES)
# define AngleRad(a) ((a)*RadToDeg)
# define AngleDeg(a) (a)
# define AngleTurn(a) ((a)*TurnToDeg)
#elif defined(LCF_HMM_USE_TURNS)
# define AngleRad(a) ((a)*RadToTurn)
# define AngleDeg(a) ((a)*DegToTurn)
# define AngleTurn(a) (a)
#endif

#if !defined(LCF_HMM_PROVIDE_MATH_FUNCTIONS)
# include <math.h>
# define SINF sinf
# define COSF cosf
# define TANF tanf
# define SQRTF sqrtf
# define ACOSF acosf
#endif

#if !defined(ANGLE_USER_TO_INTERNAL)
# define ANGLE_USER_TO_INTERNAL(a) (ToRad(a))
#endif

#if !defined(ANGLE_INTERNAL_TO_USER)
# if defined(LCF_HMM_USE_RADIANS)
#  define ANGLE_INTERNAL_TO_USER(a) (a)
# elif defined(LCF_HMM_USE_DEGREES)
#  define ANGLE_INTERNAL_TO_USER(a) ((a)*RadToDeg)
# elif defined(LCF_HMM_USE_TURNS)
#  define ANGLE_INTERNAL_TO_USER(a) ((a)*RadToTurn)
# endif
#endif

#define MOD(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define SQUARE(x) ((x) * (x))

typedef union Vec2
{
    struct
    {
        float x, y;
    };

    struct
    {
        float u, v;
    };

    struct
    {
        float width, height;
    };

    float raw[2];
} Vec2;

typedef union Vec3
{
    struct
    {
        float x, y, z;
    };

    struct
    {
        float u, v, w;
    };

    struct
    {
        float r, g, b;
    };

    struct
    {
        Vec2 xy;
        float _Ignored0;
    };

    struct
    {
        float _Ignored1;
        Vec2 yz;
    };

    struct
    {
        Vec2 uv;
        float _Ignored2;
    };

    struct
    {
        float _Ignored3;
        Vec2 vw;
    };

    float raw[3];
} Vec3;

typedef union Vec4
{
    struct
    {
        union
        {
            Vec3 xyz;
            struct
            {
                float x, y, z;
            };
        };

        float w;
    };
    struct
    {
        union
        {
            Vec3 rgb;
            struct
            {
                float r, g, b;
            };
        };

        float a;
    };

    struct
    {
        Vec2 xy;
        float _Ignored0;
        float _Ignored1;
    };

    struct
    {
        float _Ignored2;
        Vec2 yz;
        float _Ignored3;
    };

    struct
    {
        float _Ignored4;
        float _Ignored5;
        Vec2 zw;
    };

    float raw[4];

#ifdef LCF_HMM__USE_SSE
    __m128 SSE;
#endif

#ifdef LCF_HMM__USE_NEON
    float32x4_t NEON;
#endif 

} Vec4;

typedef union Mat2
{
    float raw[2][2];
    Vec2 Columns[2];
} Mat2;

typedef union Mat3
{
    float raw[3][3];
    Vec3 Columns[3];
} Mat3;

typedef union Mat4
{
    float raw[4][4];
    Vec4 Columns[4];
} Mat4;

typedef union Quat
{
    struct
    {
        union
        {
            Vec3 xyz;
            struct
            {
                float x, y, z;
            };
        };

        float w;
    };

    float raw[4];

#ifdef LCF_HMM__USE_SSE
    __m128 SSE;
#endif
#ifdef LCF_HMM__USE_NEON
    float32x4_t NEON;
#endif
} Quat;

typedef signed int Bool;

/*
 * Angle unit conversion functions
 */
static inline float ToRad(float Angle)
{
#if defined(LCF_HMM_USE_RADIANS)
    float Result = Angle;
#elif defined(LCF_HMM_USE_DEGREES)
    float Result = Angle * DegToRad;
#elif defined(LCF_HMM_USE_TURNS)
    float Result = Angle * TurnToRad;
#endif

    return Result;
}

static inline float ToDeg(float Angle)
{
#if defined(LCF_HMM_USE_RADIANS)
    float Result = Angle * RadToDeg;
#elif defined(LCF_HMM_USE_DEGREES)
    float Result = Angle;
#elif defined(LCF_HMM_USE_TURNS)
    float Result = Angle * TurnToDeg;
#endif

    return Result;
}

static inline float ToTurn(float Angle)
{
#if defined(LCF_HMM_USE_RADIANS)
    float Result = Angle * RadToTurn;
#elif defined(LCF_HMM_USE_DEGREES)
    float Result = Angle * DegToTurn;
#elif defined(LCF_HMM_USE_TURNS)
    float Result = Angle;
#endif

    return Result;
}

/*
 * Floating-point math functions
 */

static inline float SinF(float Angle)
{
    return SINF(ANGLE_USER_TO_INTERNAL(Angle));
}

static inline float CosF(float Angle)
{
    return COSF(ANGLE_USER_TO_INTERNAL(Angle));
}

static inline float TanF(float Angle)
{
    return TANF(ANGLE_USER_TO_INTERNAL(Angle));
}

static inline float ACosF(float Arg)
{
    return ANGLE_INTERNAL_TO_USER(ACOSF(Arg));
}

static inline float SqrtF(float Float)
{

    float Result;

#ifdef LCF_HMM__USE_SSE
    __m128 In = _mm_set_ss(Float);
    __m128 Out = _mm_sqrt_ss(In);
    Result = _mm_cvtss_f32(Out);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t In = vdupq_n_f32(Float);
    float32x4_t Out = vsqrtq_f32(In);
    Result = vgetq_lane_f32(Out, 0);
#else
    Result = SQRTF(Float);
#endif

    return Result;
}

static inline float InvSqrtF(float Float)
{

    float Result;

    Result = 1.0f/SqrtF(Float);

    return Result;
}


/*
 * Utility functions
 */

static inline float Lerp(float A, float Time, float B)
{
    return (1.0f - Time) * A + Time * B;
}

static inline float Clamp(float Min, float Value, float Max)
{

    float Result = Value;

    if (Result < Min)
    {
        Result = Min;
    }

    if (Result > Max)
    {
        Result = Max;
    }

    return Result;
}


/*
 * Vector initialization
 */

static inline Vec2 V2(float X, float Y)
{

    Vec2 Result;
    Result.x = X;
    Result.y = Y;

    return Result;
}

static inline Vec3 V3(float X, float Y, float Z)
{

    Vec3 Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;

    return Result;
}

static inline Vec4 V4(float X, float Y, float Z, float W)
{

    Vec4 Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_setr_ps(X, Y, Z, W);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t v = {X, Y, Z, W};
    Result.NEON = v;
#else
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
#endif

    return Result;
}

static inline Vec4 V4V(Vec3 Vector, float W)
{

    Vec4 Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_setr_ps(Vector.x, Vector.y, Vector.z, W);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t v = {Vector.x, Vector.y, Vector.z, W};
    Result.NEON = v;
#else
    Result.xyz = Vector;
    Result.w = W;
#endif

    return Result;
}


/*
 * Binary vector operations
 */

static inline Vec2 AddV2(Vec2 Left, Vec2 Right)
{

    Vec2 Result;
    Result.x = Left.x + Right.x;
    Result.y = Left.y + Right.y;

    return Result;
}

static inline Vec3 AddV3(Vec3 Left, Vec3 Right)
{

    Vec3 Result;
    Result.x = Left.x + Right.x;
    Result.y = Left.y + Right.y;
    Result.z = Left.z + Right.z;

    return Result;
}

static inline Vec4 AddV4(Vec4 Left, Vec4 Right)
{

    Vec4 Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_add_ps(Left.SSE, Right.SSE);
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vaddq_f32(Left.NEON, Right.NEON);
#else
    Result.x = Left.x + Right.x;
    Result.y = Left.y + Right.y;
    Result.z = Left.z + Right.z;
    Result.w = Left.w + Right.w;
#endif

    return Result;
}

static inline Vec2 SubV2(Vec2 Left, Vec2 Right)
{

    Vec2 Result;
    Result.x = Left.x - Right.x;
    Result.y = Left.y - Right.y;

    return Result;
}

static inline Vec3 SubV3(Vec3 Left, Vec3 Right)
{

    Vec3 Result;
    Result.x = Left.x - Right.x;
    Result.y = Left.y - Right.y;
    Result.z = Left.z - Right.z;

    return Result;
}

static inline Vec4 SubV4(Vec4 Left, Vec4 Right)
{

    Vec4 Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_sub_ps(Left.SSE, Right.SSE);
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vsubq_f32(Left.NEON, Right.NEON);
#else
    Result.x = Left.x - Right.x;
    Result.y = Left.y - Right.y;
    Result.z = Left.z - Right.z;
    Result.w = Left.w - Right.w;
#endif

    return Result;
}

static inline Vec2 MulV2(Vec2 Left, Vec2 Right)
{

    Vec2 Result;
    Result.x = Left.x * Right.x;
    Result.y = Left.y * Right.y;

    return Result;
}

static inline Vec2 MulV2F(Vec2 Left, float Right)
{

    Vec2 Result;
    Result.x = Left.x * Right;
    Result.y = Left.y * Right;

    return Result;
}

static inline Vec3 MulV3(Vec3 Left, Vec3 Right)
{

    Vec3 Result;
    Result.x = Left.x * Right.x;
    Result.y = Left.y * Right.y;
    Result.z = Left.z * Right.z;

    return Result;
}

static inline Vec3 MulV3F(Vec3 Left, float Right)
{

    Vec3 Result;
    Result.x = Left.x * Right;
    Result.y = Left.y * Right;
    Result.z = Left.z * Right;

    return Result;
}

static inline Vec4 MulV4(Vec4 Left, Vec4 Right)
{

    Vec4 Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_mul_ps(Left.SSE, Right.SSE);
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vmulq_f32(Left.NEON, Right.NEON);
#else
    Result.x = Left.x * Right.x;
    Result.y = Left.y * Right.y;
    Result.z = Left.z * Right.z;
    Result.w = Left.w * Right.w;
#endif

    return Result;
}

static inline Vec4 MulV4F(Vec4 Left, float Right)
{

    Vec4 Result;

#ifdef LCF_HMM__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.SSE = _mm_mul_ps(Left.SSE, Scalar);
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vmulq_n_f32(Left.NEON, Right);
#else
    Result.x = Left.x * Right;
    Result.y = Left.y * Right;
    Result.z = Left.z * Right;
    Result.w = Left.w * Right;
#endif

    return Result;
}

static inline Vec2 DivV2(Vec2 Left, Vec2 Right)
{

    Vec2 Result;
    Result.x = Left.x / Right.x;
    Result.y = Left.y / Right.y;

    return Result;
}

static inline Vec2 DivV2F(Vec2 Left, float Right)
{

    Vec2 Result;
    Result.x = Left.x / Right;
    Result.y = Left.y / Right;

    return Result;
}

static inline Vec3 DivV3(Vec3 Left, Vec3 Right)
{

    Vec3 Result;
    Result.x = Left.x / Right.x;
    Result.y = Left.y / Right.y;
    Result.z = Left.z / Right.z;

    return Result;
}

static inline Vec3 DivV3F(Vec3 Left, float Right)
{

    Vec3 Result;
    Result.x = Left.x / Right;
    Result.y = Left.y / Right;
    Result.z = Left.z / Right;

    return Result;
}

static inline Vec4 DivV4(Vec4 Left, Vec4 Right)
{

    Vec4 Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_div_ps(Left.SSE, Right.SSE);
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vdivq_f32(Left.NEON, Right.NEON);
#else
    Result.x = Left.x / Right.x;
    Result.y = Left.y / Right.y;
    Result.z = Left.z / Right.z;
    Result.w = Left.w / Right.w;
#endif

    return Result;
}

static inline Vec4 DivV4F(Vec4 Left, float Right)
{

    Vec4 Result;

#ifdef LCF_HMM__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.SSE = _mm_div_ps(Left.SSE, Scalar);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t Scalar = vdupq_n_f32(Right);
    Result.NEON = vdivq_f32(Left.NEON, Scalar);
#else
    Result.x = Left.x / Right;
    Result.y = Left.y / Right;
    Result.z = Left.z / Right;
    Result.w = Left.w / Right;
#endif

    return Result;
}

static inline Bool EqV2(Vec2 Left, Vec2 Right)
{
    return Left.x == Right.x && Left.y == Right.y;
}

static inline Bool EqV3(Vec3 Left, Vec3 Right)
{
    return Left.x == Right.x && Left.y == Right.y && Left.z == Right.z;
}

static inline Bool EqV4(Vec4 Left, Vec4 Right)
{
    return Left.x == Right.x && Left.y == Right.y && Left.z == Right.z && Left.w == Right.w;
}

static inline float DotV2(Vec2 Left, Vec2 Right)
{
    return (Left.x * Right.x) + (Left.y * Right.y);
}

static inline float DotV3(Vec3 Left, Vec3 Right)
{
    return (Left.x * Right.x) + (Left.y * Right.y) + (Left.z * Right.z);
}

static inline float DotV4(Vec4 Left, Vec4 Right)
{

    float Result;

    // NOTE(zak): IN the future if we wanna check what version SSE is support
    // we can use _mm_dp_ps (4.3) but for now we will use the old way.
    // Or a r = _mm_mul_ps(v1, v2), r = _mm_hadd_ps(r, r), r = _mm_hadd_ps(r, r) for SSE3
#ifdef LCF_HMM__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, Right.SSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t NEONMultiplyResult = vmulq_f32(Left.NEON, Right.NEON);
    float32x4_t NEONHalfAdd = vpaddq_f32(NEONMultiplyResult, NEONMultiplyResult);
    float32x4_t NEONFullAdd = vpaddq_f32(NEONHalfAdd, NEONHalfAdd);
    Result = vgetq_lane_f32(NEONFullAdd, 0);
#else
    Result = ((Left.x * Right.x) + (Left.z * Right.z)) + ((Left.y * Right.y) + (Left.w * Right.w));
#endif

    return Result;
}

static inline Vec3 CrossV3(Vec3 Left, Vec3 Right)
{
    Vec3 Result;
    Result.x = (Left.y * Right.z) - (Left.z * Right.y);
    Result.y = (Left.z * Right.x) - (Left.x * Right.z);
    Result.z = (Left.x * Right.y) - (Left.y * Right.x);

    return Result;
}


/*
 * Unary vector operations
 */

static inline float LenSqrV2(Vec2 A)
{
    return DotV2(A, A);
}

static inline float LenSqrV3(Vec3 A)
{
    return DotV3(A, A);
}

static inline float LenSqrV4(Vec4 A)
{
    return DotV4(A, A);
}

static inline float LenV2(Vec2 A)
{
    return SqrtF(LenSqrV2(A));
}

static inline float LenV3(Vec3 A)
{
    return SqrtF(LenSqrV3(A));
}

static inline float LenV4(Vec4 A)
{
    return SqrtF(LenSqrV4(A));
}

static inline Vec2 NormV2(Vec2 A)
{
    return MulV2F(A, InvSqrtF(DotV2(A, A)));
}

static inline Vec3 NormV3(Vec3 A)
{
    return MulV3F(A, InvSqrtF(DotV3(A, A)));
}

static inline Vec4 NormV4(Vec4 A)
{
    return MulV4F(A, InvSqrtF(DotV4(A, A)));
}

/*
 * Utility vector functions
 */

static inline Vec2 LerpV2(Vec2 A, float Time, Vec2 B)
{
    return AddV2(MulV2F(A, 1.0f - Time), MulV2F(B, Time));
}

static inline Vec3 LerpV3(Vec3 A, float Time, Vec3 B)
{
    return AddV3(MulV3F(A, 1.0f - Time), MulV3F(B, Time));
}

static inline Vec4 LerpV4(Vec4 A, float Time, Vec4 B)
{
    return AddV4(MulV4F(A, 1.0f - Time), MulV4F(B, Time));
}

/*
 * SSE stuff
 */

static inline Vec4 LinearCombineV4M4(Vec4 Left, Mat4 Right)
{

    Vec4 Result;
#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0x00), Right.Columns[0].SSE);
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0x55), Right.Columns[1].SSE));
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0xaa), Right.Columns[2].SSE));
    Result.SSE = _mm_add_ps(Result.SSE, _mm_mul_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, 0xff), Right.Columns[3].SSE));
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vmulq_laneq_f32(Right.Columns[0].NEON, Left.NEON, 0);
    Result.NEON = vfmaq_laneq_f32(Result.NEON, Right.Columns[1].NEON, Left.NEON, 1);
    Result.NEON = vfmaq_laneq_f32(Result.NEON, Right.Columns[2].NEON, Left.NEON, 2);
    Result.NEON = vfmaq_laneq_f32(Result.NEON, Right.Columns[3].NEON, Left.NEON, 3);
#else
    Result.x = Left.raw[0] * Right.Columns[0].x;
    Result.y = Left.raw[0] * Right.Columns[0].y;
    Result.z = Left.raw[0] * Right.Columns[0].z;
    Result.w = Left.raw[0] * Right.Columns[0].w;

    Result.x += Left.raw[1] * Right.Columns[1].x;
    Result.y += Left.raw[1] * Right.Columns[1].y;
    Result.z += Left.raw[1] * Right.Columns[1].z;
    Result.w += Left.raw[1] * Right.Columns[1].w;

    Result.x += Left.raw[2] * Right.Columns[2].x;
    Result.y += Left.raw[2] * Right.Columns[2].y;
    Result.z += Left.raw[2] * Right.Columns[2].z;
    Result.w += Left.raw[2] * Right.Columns[2].w;

    Result.x += Left.raw[3] * Right.Columns[3].x;
    Result.y += Left.raw[3] * Right.Columns[3].y;
    Result.z += Left.raw[3] * Right.Columns[3].z;
    Result.w += Left.raw[3] * Right.Columns[3].w;
#endif

    return Result;
}

/*
 * 2x2 Matrices
 */

static inline Mat2 M2(void)
{
    Mat2 Result = {0};
    return Result;
}

static inline Mat2 M2D(float Diagonal)
{

    Mat2 Result = {0};
    Result.raw[0][0] = Diagonal;
    Result.raw[1][1] = Diagonal;

    return Result;
}

static inline Mat2 TransposeM2(Mat2 Matrix)
{

    Mat2 Result = Matrix;

    Result.raw[0][1] = Matrix.raw[1][0];
    Result.raw[1][0] = Matrix.raw[0][1];

    return Result;
}

static inline Mat2 AddM2(Mat2 Left, Mat2 Right)
{

    Mat2 Result;

    Result.raw[0][0] = Left.raw[0][0] + Right.raw[0][0];
    Result.raw[0][1] = Left.raw[0][1] + Right.raw[0][1];
    Result.raw[1][0] = Left.raw[1][0] + Right.raw[1][0];
    Result.raw[1][1] = Left.raw[1][1] + Right.raw[1][1];

    return Result;
}

static inline Mat2 SubM2(Mat2 Left, Mat2 Right)
{

    Mat2 Result;

    Result.raw[0][0] = Left.raw[0][0] - Right.raw[0][0];
    Result.raw[0][1] = Left.raw[0][1] - Right.raw[0][1];
    Result.raw[1][0] = Left.raw[1][0] - Right.raw[1][0];
    Result.raw[1][1] = Left.raw[1][1] - Right.raw[1][1];

    return Result;
}

static inline Vec2 MulM2V2(Mat2 Matrix, Vec2 Vector)
{

    Vec2 Result;

    Result.x = Vector.raw[0] * Matrix.Columns[0].x;
    Result.y = Vector.raw[0] * Matrix.Columns[0].y;

    Result.x += Vector.raw[1] * Matrix.Columns[1].x;
    Result.y += Vector.raw[1] * Matrix.Columns[1].y;

    return Result;
}

static inline Mat2 MulM2(Mat2 Left, Mat2 Right)
{

    Mat2 Result;
    Result.Columns[0] = MulM2V2(Left, Right.Columns[0]);
    Result.Columns[1] = MulM2V2(Left, Right.Columns[1]);

    return Result;
}

static inline Mat2 MulM2F(Mat2 Matrix, float Scalar)
{

    Mat2 Result;

    Result.raw[0][0] = Matrix.raw[0][0] * Scalar;
    Result.raw[0][1] = Matrix.raw[0][1] * Scalar;
    Result.raw[1][0] = Matrix.raw[1][0] * Scalar;
    Result.raw[1][1] = Matrix.raw[1][1] * Scalar;

    return Result;
}

static inline Mat2 DivM2F(Mat2 Matrix, float Scalar)
{

    Mat2 Result;

    Result.raw[0][0] = Matrix.raw[0][0] / Scalar;
    Result.raw[0][1] = Matrix.raw[0][1] / Scalar;
    Result.raw[1][0] = Matrix.raw[1][0] / Scalar;
    Result.raw[1][1] = Matrix.raw[1][1] / Scalar;

    return Result;
}

static inline float DeterminantM2(Mat2 Matrix)
{
    return Matrix.raw[0][0]*Matrix.raw[1][1] - Matrix.raw[0][1]*Matrix.raw[1][0];
}


static inline Mat2 InvGeneralM2(Mat2 Matrix)
{

    Mat2 Result;
    float InvDeterminant = 1.0f / DeterminantM2(Matrix);
    Result.raw[0][0] = InvDeterminant * +Matrix.raw[1][1];
    Result.raw[1][1] = InvDeterminant * +Matrix.raw[0][0];
    Result.raw[0][1] = InvDeterminant * -Matrix.raw[0][1];
    Result.raw[1][0] = InvDeterminant * -Matrix.raw[1][0];

    return Result;
}

/*
 * 3x3 Matrices
 */

static inline Mat3 M3(void)
{
    Mat3 Result = {0};
    return Result;
}

static inline Mat3 M3D(float Diagonal)
{

    Mat3 Result = {0};
    Result.raw[0][0] = Diagonal;
    Result.raw[1][1] = Diagonal;
    Result.raw[2][2] = Diagonal;

    return Result;
}

static inline Mat3 TransposeM3(Mat3 Matrix)
{

    Mat3 Result = Matrix;

    Result.raw[0][1] = Matrix.raw[1][0];
    Result.raw[0][2] = Matrix.raw[2][0];
    Result.raw[1][0] = Matrix.raw[0][1];
    Result.raw[1][2] = Matrix.raw[2][1];
    Result.raw[2][1] = Matrix.raw[1][2];
    Result.raw[2][0] = Matrix.raw[0][2];

    return Result;
}

static inline Mat3 AddM3(Mat3 Left, Mat3 Right)
{

    Mat3 Result;

    Result.raw[0][0] = Left.raw[0][0] + Right.raw[0][0];
    Result.raw[0][1] = Left.raw[0][1] + Right.raw[0][1];
    Result.raw[0][2] = Left.raw[0][2] + Right.raw[0][2];
    Result.raw[1][0] = Left.raw[1][0] + Right.raw[1][0];
    Result.raw[1][1] = Left.raw[1][1] + Right.raw[1][1];
    Result.raw[1][2] = Left.raw[1][2] + Right.raw[1][2];
    Result.raw[2][0] = Left.raw[2][0] + Right.raw[2][0];
    Result.raw[2][1] = Left.raw[2][1] + Right.raw[2][1];
    Result.raw[2][2] = Left.raw[2][2] + Right.raw[2][2];

    return Result;
}

static inline Mat3 SubM3(Mat3 Left, Mat3 Right)
{

    Mat3 Result;

    Result.raw[0][0] = Left.raw[0][0] - Right.raw[0][0];
    Result.raw[0][1] = Left.raw[0][1] - Right.raw[0][1];
    Result.raw[0][2] = Left.raw[0][2] - Right.raw[0][2];
    Result.raw[1][0] = Left.raw[1][0] - Right.raw[1][0];
    Result.raw[1][1] = Left.raw[1][1] - Right.raw[1][1];
    Result.raw[1][2] = Left.raw[1][2] - Right.raw[1][2];
    Result.raw[2][0] = Left.raw[2][0] - Right.raw[2][0];
    Result.raw[2][1] = Left.raw[2][1] - Right.raw[2][1];
    Result.raw[2][2] = Left.raw[2][2] - Right.raw[2][2];

    return Result;
}

static inline Vec3 MulM3V3(Mat3 Matrix, Vec3 Vector)
{

    Vec3 Result;

    Result.x = Vector.raw[0] * Matrix.Columns[0].x;
    Result.y = Vector.raw[0] * Matrix.Columns[0].y;
    Result.z = Vector.raw[0] * Matrix.Columns[0].z;

    Result.x += Vector.raw[1] * Matrix.Columns[1].x;
    Result.y += Vector.raw[1] * Matrix.Columns[1].y;
    Result.z += Vector.raw[1] * Matrix.Columns[1].z;

    Result.x += Vector.raw[2] * Matrix.Columns[2].x;
    Result.y += Vector.raw[2] * Matrix.Columns[2].y;
    Result.z += Vector.raw[2] * Matrix.Columns[2].z;

    return Result;
}

static inline Mat3 MulM3(Mat3 Left, Mat3 Right)
{

    Mat3 Result;
    Result.Columns[0] = MulM3V3(Left, Right.Columns[0]);
    Result.Columns[1] = MulM3V3(Left, Right.Columns[1]);
    Result.Columns[2] = MulM3V3(Left, Right.Columns[2]);

    return Result;
}

static inline Mat3 MulM3F(Mat3 Matrix, float Scalar)
{

    Mat3 Result;

    Result.raw[0][0] = Matrix.raw[0][0] * Scalar;
    Result.raw[0][1] = Matrix.raw[0][1] * Scalar;
    Result.raw[0][2] = Matrix.raw[0][2] * Scalar;
    Result.raw[1][0] = Matrix.raw[1][0] * Scalar;
    Result.raw[1][1] = Matrix.raw[1][1] * Scalar;
    Result.raw[1][2] = Matrix.raw[1][2] * Scalar;
    Result.raw[2][0] = Matrix.raw[2][0] * Scalar;
    Result.raw[2][1] = Matrix.raw[2][1] * Scalar;
    Result.raw[2][2] = Matrix.raw[2][2] * Scalar;

    return Result;
}

static inline Mat3 DivM3F(Mat3 Matrix, float Scalar)
{

    Mat3 Result;

    Result.raw[0][0] = Matrix.raw[0][0] / Scalar;
    Result.raw[0][1] = Matrix.raw[0][1] / Scalar;
    Result.raw[0][2] = Matrix.raw[0][2] / Scalar;
    Result.raw[1][0] = Matrix.raw[1][0] / Scalar;
    Result.raw[1][1] = Matrix.raw[1][1] / Scalar;
    Result.raw[1][2] = Matrix.raw[1][2] / Scalar;
    Result.raw[2][0] = Matrix.raw[2][0] / Scalar;
    Result.raw[2][1] = Matrix.raw[2][1] / Scalar;
    Result.raw[2][2] = Matrix.raw[2][2] / Scalar;

    return Result;
}

static inline float DeterminantM3(Mat3 Matrix)
{

    Mat3 m;
    m.Columns[0] = CrossV3(Matrix.Columns[1], Matrix.Columns[2]);
    m.Columns[1] = CrossV3(Matrix.Columns[2], Matrix.Columns[0]);
    m.Columns[2] = CrossV3(Matrix.Columns[0], Matrix.Columns[1]);

    return DotV3(m.Columns[2], Matrix.Columns[2]);
}

static inline Mat3 InvGeneralM3(Mat3 Matrix)
{

    Mat3 m;
    m.Columns[0] = CrossV3(Matrix.Columns[1], Matrix.Columns[2]);
    m.Columns[1] = CrossV3(Matrix.Columns[2], Matrix.Columns[0]);
    m.Columns[2] = CrossV3(Matrix.Columns[0], Matrix.Columns[1]);

    float InvDeterminant = 1.0f / DotV3(m.Columns[2], Matrix.Columns[2]);

    Mat3 Result;
    Result.Columns[0] = MulV3F(m.Columns[0], InvDeterminant);
    Result.Columns[1] = MulV3F(m.Columns[1], InvDeterminant);
    Result.Columns[2] = MulV3F(m.Columns[2], InvDeterminant);

    return TransposeM3(Result);
}

/*
 * 4x4 Matrices
 */

static inline Mat4 M4(void)
{
    Mat4 Result = {0};
    return Result;
}

static inline Mat4 M4D(float Diagonal)
{

    Mat4 Result = {0};
    Result.raw[0][0] = Diagonal;
    Result.raw[1][1] = Diagonal;
    Result.raw[2][2] = Diagonal;
    Result.raw[3][3] = Diagonal;

    return Result;
}

static inline Mat4 TransposeM4(Mat4 Matrix)
{

    Mat4 Result;
#ifdef LCF_HMM__USE_SSE
    Result = Matrix;
    _MM_TRANSPOSE4_PS(Result.Columns[0].SSE, Result.Columns[1].SSE, Result.Columns[2].SSE, Result.Columns[3].SSE);
#elif defined(LCF_HMM__USE_NEON)
    float32x4x4_t Transposed = vld4q_f32((float*)Matrix.Columns);
    Result.Columns[0].NEON = Transposed.val[0];
    Result.Columns[1].NEON = Transposed.val[1];
    Result.Columns[2].NEON = Transposed.val[2];
    Result.Columns[3].NEON = Transposed.val[3];
#else
    Result.raw[0][0] = Matrix.raw[0][0];
    Result.raw[0][1] = Matrix.raw[1][0];
    Result.raw[0][2] = Matrix.raw[2][0];
    Result.raw[0][3] = Matrix.raw[3][0];
    Result.raw[1][0] = Matrix.raw[0][1];
    Result.raw[1][1] = Matrix.raw[1][1];
    Result.raw[1][2] = Matrix.raw[2][1];
    Result.raw[1][3] = Matrix.raw[3][1];
    Result.raw[2][0] = Matrix.raw[0][2];
    Result.raw[2][1] = Matrix.raw[1][2];
    Result.raw[2][2] = Matrix.raw[2][2];
    Result.raw[2][3] = Matrix.raw[3][2];
    Result.raw[3][0] = Matrix.raw[0][3];
    Result.raw[3][1] = Matrix.raw[1][3];
    Result.raw[3][2] = Matrix.raw[2][3];
    Result.raw[3][3] = Matrix.raw[3][3];
#endif

    return Result;
}

static inline Mat4 AddM4(Mat4 Left, Mat4 Right)
{

    Mat4 Result;

    Result.Columns[0] = AddV4(Left.Columns[0], Right.Columns[0]);
    Result.Columns[1] = AddV4(Left.Columns[1], Right.Columns[1]);
    Result.Columns[2] = AddV4(Left.Columns[2], Right.Columns[2]);
    Result.Columns[3] = AddV4(Left.Columns[3], Right.Columns[3]);

    return Result;
}

static inline Mat4 SubM4(Mat4 Left, Mat4 Right)
{

    Mat4 Result;

    Result.Columns[0] = SubV4(Left.Columns[0], Right.Columns[0]);
    Result.Columns[1] = SubV4(Left.Columns[1], Right.Columns[1]);
    Result.Columns[2] = SubV4(Left.Columns[2], Right.Columns[2]);
    Result.Columns[3] = SubV4(Left.Columns[3], Right.Columns[3]);

    return Result;
}

static inline Mat4 MulM4(Mat4 Left, Mat4 Right)
{

    Mat4 Result;
    Result.Columns[0] = LinearCombineV4M4(Right.Columns[0], Left);
    Result.Columns[1] = LinearCombineV4M4(Right.Columns[1], Left);
    Result.Columns[2] = LinearCombineV4M4(Right.Columns[2], Left);
    Result.Columns[3] = LinearCombineV4M4(Right.Columns[3], Left);

    return Result;
}

static inline Mat4 MulM4F(Mat4 Matrix, float Scalar)
{

    Mat4 Result;
    
    
#ifdef LCF_HMM__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0].SSE = _mm_mul_ps(Matrix.Columns[0].SSE, SSEScalar);
    Result.Columns[1].SSE = _mm_mul_ps(Matrix.Columns[1].SSE, SSEScalar);
    Result.Columns[2].SSE = _mm_mul_ps(Matrix.Columns[2].SSE, SSEScalar);
    Result.Columns[3].SSE = _mm_mul_ps(Matrix.Columns[3].SSE, SSEScalar);
#elif defined(LCF_HMM__USE_NEON)
    Result.Columns[0].NEON = vmulq_n_f32(Matrix.Columns[0].NEON, Scalar);
    Result.Columns[1].NEON = vmulq_n_f32(Matrix.Columns[1].NEON, Scalar);
    Result.Columns[2].NEON = vmulq_n_f32(Matrix.Columns[2].NEON, Scalar);
    Result.Columns[3].NEON = vmulq_n_f32(Matrix.Columns[3].NEON, Scalar);
#else
    Result.raw[0][0] = Matrix.raw[0][0] * Scalar;
    Result.raw[0][1] = Matrix.raw[0][1] * Scalar;
    Result.raw[0][2] = Matrix.raw[0][2] * Scalar;
    Result.raw[0][3] = Matrix.raw[0][3] * Scalar;
    Result.raw[1][0] = Matrix.raw[1][0] * Scalar;
    Result.raw[1][1] = Matrix.raw[1][1] * Scalar;
    Result.raw[1][2] = Matrix.raw[1][2] * Scalar;
    Result.raw[1][3] = Matrix.raw[1][3] * Scalar;
    Result.raw[2][0] = Matrix.raw[2][0] * Scalar;
    Result.raw[2][1] = Matrix.raw[2][1] * Scalar;
    Result.raw[2][2] = Matrix.raw[2][2] * Scalar;
    Result.raw[2][3] = Matrix.raw[2][3] * Scalar;
    Result.raw[3][0] = Matrix.raw[3][0] * Scalar;
    Result.raw[3][1] = Matrix.raw[3][1] * Scalar;
    Result.raw[3][2] = Matrix.raw[3][2] * Scalar;
    Result.raw[3][3] = Matrix.raw[3][3] * Scalar;
#endif

    return Result;
}

static inline Vec4 MulM4V4(Mat4 Matrix, Vec4 Vector)
{
    return LinearCombineV4M4(Vector, Matrix);
}

static inline Mat4 DivM4F(Mat4 Matrix, float Scalar)
{

    Mat4 Result;

#ifdef LCF_HMM__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0].SSE = _mm_div_ps(Matrix.Columns[0].SSE, SSEScalar);
    Result.Columns[1].SSE = _mm_div_ps(Matrix.Columns[1].SSE, SSEScalar);
    Result.Columns[2].SSE = _mm_div_ps(Matrix.Columns[2].SSE, SSEScalar);
    Result.Columns[3].SSE = _mm_div_ps(Matrix.Columns[3].SSE, SSEScalar);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t NEONScalar = vdupq_n_f32(Scalar);
    Result.Columns[0].NEON = vdivq_f32(Matrix.Columns[0].NEON, NEONScalar);
    Result.Columns[1].NEON = vdivq_f32(Matrix.Columns[1].NEON, NEONScalar);
    Result.Columns[2].NEON = vdivq_f32(Matrix.Columns[2].NEON, NEONScalar);
    Result.Columns[3].NEON = vdivq_f32(Matrix.Columns[3].NEON, NEONScalar);
#else
    Result.raw[0][0] = Matrix.raw[0][0] / Scalar;
    Result.raw[0][1] = Matrix.raw[0][1] / Scalar;
    Result.raw[0][2] = Matrix.raw[0][2] / Scalar;
    Result.raw[0][3] = Matrix.raw[0][3] / Scalar;
    Result.raw[1][0] = Matrix.raw[1][0] / Scalar;
    Result.raw[1][1] = Matrix.raw[1][1] / Scalar;
    Result.raw[1][2] = Matrix.raw[1][2] / Scalar;
    Result.raw[1][3] = Matrix.raw[1][3] / Scalar;
    Result.raw[2][0] = Matrix.raw[2][0] / Scalar;
    Result.raw[2][1] = Matrix.raw[2][1] / Scalar;
    Result.raw[2][2] = Matrix.raw[2][2] / Scalar;
    Result.raw[2][3] = Matrix.raw[2][3] / Scalar;
    Result.raw[3][0] = Matrix.raw[3][0] / Scalar;
    Result.raw[3][1] = Matrix.raw[3][1] / Scalar;
    Result.raw[3][2] = Matrix.raw[3][2] / Scalar;
    Result.raw[3][3] = Matrix.raw[3][3] / Scalar;
#endif

    return Result;
}

static inline float DeterminantM4(Mat4 Matrix)
{

    Vec3 C01 = CrossV3(Matrix.Columns[0].xyz, Matrix.Columns[1].xyz);
    Vec3 C23 = CrossV3(Matrix.Columns[2].xyz, Matrix.Columns[3].xyz);
    Vec3 B10 = SubV3(MulV3F(Matrix.Columns[0].xyz, Matrix.Columns[1].w), MulV3F(Matrix.Columns[1].xyz, Matrix.Columns[0].w));
    Vec3 B32 = SubV3(MulV3F(Matrix.Columns[2].xyz, Matrix.Columns[3].w), MulV3F(Matrix.Columns[3].xyz, Matrix.Columns[2].w));

    return DotV3(C01, B32) + DotV3(C23, B10);
}

// Returns a general-purpose inverse of an Mat4. Note that special-purpose inverses of many transformations
// are available and will be more efficient.
static inline Mat4 InvGeneralM4(Mat4 Matrix)
{

    Vec3 C01 = CrossV3(Matrix.Columns[0].xyz, Matrix.Columns[1].xyz);
    Vec3 C23 = CrossV3(Matrix.Columns[2].xyz, Matrix.Columns[3].xyz);
    Vec3 B10 = SubV3(MulV3F(Matrix.Columns[0].xyz, Matrix.Columns[1].w), MulV3F(Matrix.Columns[1].xyz, Matrix.Columns[0].w));
    Vec3 B32 = SubV3(MulV3F(Matrix.Columns[2].xyz, Matrix.Columns[3].w), MulV3F(Matrix.Columns[3].xyz, Matrix.Columns[2].w));

    float InvDeterminant = 1.0f / (DotV3(C01, B32) + DotV3(C23, B10));
    C01 = MulV3F(C01, InvDeterminant);
    C23 = MulV3F(C23, InvDeterminant);
    B10 = MulV3F(B10, InvDeterminant);
    B32 = MulV3F(B32, InvDeterminant);

    Mat4 Result;
    Result.Columns[0] = V4V(AddV3(CrossV3(Matrix.Columns[1].xyz, B32), MulV3F(C23, Matrix.Columns[1].w)), -DotV3(Matrix.Columns[1].xyz, C23));
    Result.Columns[1] = V4V(SubV3(CrossV3(B32, Matrix.Columns[0].xyz), MulV3F(C23, Matrix.Columns[0].w)), +DotV3(Matrix.Columns[0].xyz, C23));
    Result.Columns[2] = V4V(AddV3(CrossV3(Matrix.Columns[3].xyz, B10), MulV3F(C01, Matrix.Columns[3].w)), -DotV3(Matrix.Columns[3].xyz, C01));
    Result.Columns[3] = V4V(SubV3(CrossV3(B10, Matrix.Columns[2].xyz), MulV3F(C01, Matrix.Columns[2].w)), +DotV3(Matrix.Columns[2].xyz, C01));

    return TransposeM4(Result);
}

/*
 * Common graphics transformations
 */

// Produces a right-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline Mat4 Orthographic_RH_NO(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    Mat4 Result = {0};

    Result.raw[0][0] = 2.0f / (Right - Left);
    Result.raw[1][1] = 2.0f / (Top - Bottom);
    Result.raw[2][2] = 2.0f / (Near - Far);
    Result.raw[3][3] = 1.0f;

    Result.raw[3][0] = (Left + Right) / (Left - Right);
    Result.raw[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.raw[3][2] = (Near + Far) / (Near - Far);

    return Result;
}

// Produces a right-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline Mat4 Orthographic_RH_ZO(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    Mat4 Result = {0};

    Result.raw[0][0] = 2.0f / (Right - Left);
    Result.raw[1][1] = 2.0f / (Top - Bottom);
    Result.raw[2][2] = 1.0f / (Near - Far);
    Result.raw[3][3] = 1.0f;

    Result.raw[3][0] = (Left + Right) / (Left - Right);
    Result.raw[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.raw[3][2] = (Near) / (Near - Far);

    return Result;
}

// Produces a left-handed orthographic projection matrix with Z ranging from -1 to 1 (the GL convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline Mat4 Orthographic_LH_NO(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    Mat4 Result = Orthographic_RH_NO(Left, Right, Bottom, Top, Near, Far);
    Result.raw[2][2] = -Result.raw[2][2];

    return Result;
}

// Produces a left-handed orthographic projection matrix with Z ranging from 0 to 1 (the DirectX convention).
// Left, Right, Bottom, and Top specify the coordinates of their respective clipping planes.
// Near and Far specify the distances to the near and far clipping planes.
static inline Mat4 Orthographic_LH_ZO(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    Mat4 Result = Orthographic_RH_ZO(Left, Right, Bottom, Top, Near, Far);
    Result.raw[2][2] = -Result.raw[2][2];

    return Result;
}

// Returns an inverse for the given orthographic projection matrix. Works for all orthographic
// projection matrices, regardless of handedness or NDC convention.
static inline Mat4 InvOrthographic(Mat4 OrthoMatrix)
{
    Mat4 Result = {0};
    Result.raw[0][0] = 1.0f / OrthoMatrix.raw[0][0];
    Result.raw[1][1] = 1.0f / OrthoMatrix.raw[1][1];
    Result.raw[2][2] = 1.0f / OrthoMatrix.raw[2][2];
    Result.raw[3][3] = 1.0f;

    Result.raw[3][0] = -OrthoMatrix.raw[3][0] * Result.raw[0][0];
    Result.raw[3][1] = -OrthoMatrix.raw[3][1] * Result.raw[1][1];
    Result.raw[3][2] = -OrthoMatrix.raw[3][2] * Result.raw[2][2];

    return Result;
}

static inline Mat4 Perspective_RH_NO(float FOV, float AspectRatio, float Near, float Far)
{
    Mat4 Result = {0};

    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml

    float Cotangent = 1.0f / TanF(FOV / 2.0f);
    Result.raw[0][0] = Cotangent / AspectRatio;
    Result.raw[1][1] = Cotangent;
    Result.raw[2][3] = -1.0f;

    Result.raw[2][2] = (Near + Far) / (Near - Far);
    Result.raw[3][2] = (2.0f * Near * Far) / (Near - Far);

    return Result;
}

static inline Mat4 Perspective_RH_ZO(float FOV, float AspectRatio, float Near, float Far)
{
    Mat4 Result = {0};

    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml

    float Cotangent = 1.0f / TanF(FOV / 2.0f);
    Result.raw[0][0] = Cotangent / AspectRatio;
    Result.raw[1][1] = Cotangent;
    Result.raw[2][3] = -1.0f;

    Result.raw[2][2] = (Far) / (Near - Far);
    Result.raw[3][2] = (Near * Far) / (Near - Far);

    return Result;
}

static inline Mat4 Perspective_LH_NO(float FOV, float AspectRatio, float Near, float Far)
{
    Mat4 Result = Perspective_RH_NO(FOV, AspectRatio, Near, Far);
    Result.raw[2][2] = -Result.raw[2][2];
    Result.raw[2][3] = -Result.raw[2][3];

    return Result;
}

static inline Mat4 Perspective_LH_ZO(float FOV, float AspectRatio, float Near, float Far)
{
    Mat4 Result = Perspective_RH_ZO(FOV, AspectRatio, Near, Far);
    Result.raw[2][2] = -Result.raw[2][2];
    Result.raw[2][3] = -Result.raw[2][3];

    return Result;
}

static inline Mat4 InvPerspective_RH(Mat4 PerspectiveMatrix)
{
    Mat4 Result = {0};
    Result.raw[0][0] = 1.0f / PerspectiveMatrix.raw[0][0];
    Result.raw[1][1] = 1.0f / PerspectiveMatrix.raw[1][1];
    Result.raw[2][2] = 0.0f;

    Result.raw[2][3] = 1.0f / PerspectiveMatrix.raw[3][2];
    Result.raw[3][3] = PerspectiveMatrix.raw[2][2] * Result.raw[2][3];
    Result.raw[3][2] = PerspectiveMatrix.raw[2][3];

    return Result;
}

static inline Mat4 InvPerspective_LH(Mat4 PerspectiveMatrix)
{
    Mat4 Result = {0};
    Result.raw[0][0] = 1.0f / PerspectiveMatrix.raw[0][0];
    Result.raw[1][1] = 1.0f / PerspectiveMatrix.raw[1][1];
    Result.raw[2][2] = 0.0f;

    Result.raw[2][3] = 1.0f / PerspectiveMatrix.raw[3][2];
    Result.raw[3][3] = PerspectiveMatrix.raw[2][2] * -Result.raw[2][3];
    Result.raw[3][2] = PerspectiveMatrix.raw[2][3];

    return Result;
}

static inline Mat4 Translate(Vec3 Translation)
{
    Mat4 Result = M4D(1.0f);
    Result.raw[3][0] = Translation.x;
    Result.raw[3][1] = Translation.y;
    Result.raw[3][2] = Translation.z;

    return Result;
}

static inline Mat4 InvTranslate(Mat4 TranslationMatrix)
{
    Mat4 Result = TranslationMatrix;
    Result.raw[3][0] = -Result.raw[3][0];
    Result.raw[3][1] = -Result.raw[3][1];
    Result.raw[3][2] = -Result.raw[3][2];

    return Result;
}

static inline Mat4 Rotate_RH(float Angle, Vec3 Axis)
{
    Mat4 Result = M4D(1.0f);

    Axis = NormV3(Axis);

    float SinTheta = SinF(Angle);
    float CosTheta = CosF(Angle);
    float CosValue = 1.0f - CosTheta;

    Result.raw[0][0] = (Axis.x * Axis.x * CosValue) + CosTheta;
    Result.raw[0][1] = (Axis.x * Axis.y * CosValue) + (Axis.z * SinTheta);
    Result.raw[0][2] = (Axis.x * Axis.z * CosValue) - (Axis.y * SinTheta);

    Result.raw[1][0] = (Axis.y * Axis.x * CosValue) - (Axis.z * SinTheta);
    Result.raw[1][1] = (Axis.y * Axis.y * CosValue) + CosTheta;
    Result.raw[1][2] = (Axis.y * Axis.z * CosValue) + (Axis.x * SinTheta);

    Result.raw[2][0] = (Axis.z * Axis.x * CosValue) + (Axis.y * SinTheta);
    Result.raw[2][1] = (Axis.z * Axis.y * CosValue) - (Axis.x * SinTheta);
    Result.raw[2][2] = (Axis.z * Axis.z * CosValue) + CosTheta;

    return Result;
}

static inline Mat4 Rotate_LH(float Angle, Vec3 Axis)
{
    /* NOTE(lcf): Matrix will be inverse/transpose of RH. */
    return Rotate_RH(-Angle, Axis);
}

static inline Mat4 InvRotate(Mat4 RotationMatrix)
{
    return TransposeM4(RotationMatrix);
}

static inline Mat4 Scale(Vec3 Scale)
{
    Mat4 Result = M4D(1.0f);
    Result.raw[0][0] = Scale.x;
    Result.raw[1][1] = Scale.y;
    Result.raw[2][2] = Scale.z;

    return Result;
}

static inline Mat4 InvScale(Mat4 ScaleMatrix)
{
    Mat4 Result = ScaleMatrix;
    Result.raw[0][0] = 1.0f / Result.raw[0][0];
    Result.raw[1][1] = 1.0f / Result.raw[1][1];
    Result.raw[2][2] = 1.0f / Result.raw[2][2];

    return Result;
}

static inline Mat4 _LookAt(Vec3 F,  Vec3 S, Vec3 U,  Vec3 Eye)
{
    Mat4 Result;

    Result.raw[0][0] = S.x;
    Result.raw[0][1] = U.x;
    Result.raw[0][2] = -F.x;
    Result.raw[0][3] = 0.0f;

    Result.raw[1][0] = S.y;
    Result.raw[1][1] = U.y;
    Result.raw[1][2] = -F.y;
    Result.raw[1][3] = 0.0f;

    Result.raw[2][0] = S.z;
    Result.raw[2][1] = U.z;
    Result.raw[2][2] = -F.z;
    Result.raw[2][3] = 0.0f;

    Result.raw[3][0] = -DotV3(S, Eye);
    Result.raw[3][1] = -DotV3(U, Eye);
    Result.raw[3][2] = DotV3(F, Eye);
    Result.raw[3][3] = 1.0f;

    return Result;
}

static inline Mat4 LookAt_RH(Vec3 Eye, Vec3 Center, Vec3 Up)
{
    Vec3 F = NormV3(SubV3(Center, Eye));
    Vec3 S = NormV3(CrossV3(F, Up));
    Vec3 U = CrossV3(S, F);

    return _LookAt(F, S, U, Eye);
}

static inline Mat4 LookAt_LH(Vec3 Eye, Vec3 Center, Vec3 Up)
{
    Vec3 F = NormV3(SubV3(Eye, Center));
    Vec3 S = NormV3(CrossV3(F, Up));
    Vec3 U = CrossV3(S, F);

    return _LookAt(F, S, U, Eye);
}

static inline Mat4 InvLookAt(Mat4 Matrix)
{
    Mat4 Result;

    Mat3 Rotation = {0};
    Rotation.Columns[0] = Matrix.Columns[0].xyz;
    Rotation.Columns[1] = Matrix.Columns[1].xyz;
    Rotation.Columns[2] = Matrix.Columns[2].xyz;
    Rotation = TransposeM3(Rotation);

    Result.Columns[0] = V4V(Rotation.Columns[0], 0.0f);
    Result.Columns[1] = V4V(Rotation.Columns[1], 0.0f);
    Result.Columns[2] = V4V(Rotation.Columns[2], 0.0f);
    Result.Columns[3] = MulV4F(Matrix.Columns[3], -1.0f);
    Result.raw[3][0] = -1.0f * Matrix.raw[3][0] /
        (Rotation.raw[0][0] + Rotation.raw[0][1] + Rotation.raw[0][2]);
    Result.raw[3][1] = -1.0f * Matrix.raw[3][1] /
        (Rotation.raw[1][0] + Rotation.raw[1][1] + Rotation.raw[1][2]);
    Result.raw[3][2] = -1.0f * Matrix.raw[3][2] /
        (Rotation.raw[2][0] + Rotation.raw[2][1] + Rotation.raw[2][2]);
    Result.raw[3][3] = 1.0f;

    return Result;
}

/*
 * Quaternion operations
 */

static inline Quat Q(float X, float Y, float Z, float W)
{
    Quat Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_setr_ps(X, Y, Z, W);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t v = { X, Y, Z, W };
    Result.NEON = v;
#else
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
#endif

    return Result;
}

static inline Quat QV4(Vec4 Vector)
{
    Quat Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = Vector.SSE;
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = Vector.NEON;
#else
    Result.x = Vector.x;
    Result.y = Vector.y;
    Result.z = Vector.z;
    Result.w = Vector.w;
#endif

    return Result;
}

static inline Quat AddQ(Quat Left, Quat Right)
{
    Quat Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_add_ps(Left.SSE, Right.SSE);
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vaddq_f32(Left.NEON, Right.NEON);
#else

    Result.x = Left.x + Right.x;
    Result.y = Left.y + Right.y;
    Result.z = Left.z + Right.z;
    Result.w = Left.w + Right.w;
#endif

    return Result;
}

static inline Quat SubQ(Quat Left, Quat Right)
{
    Quat Result;

#ifdef LCF_HMM__USE_SSE
    Result.SSE = _mm_sub_ps(Left.SSE, Right.SSE);
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vsubq_f32(Left.NEON, Right.NEON);
#else
    Result.x = Left.x - Right.x;
    Result.y = Left.y - Right.y;
    Result.z = Left.z - Right.z;
    Result.w = Left.w - Right.w;
#endif

    return Result;
}

static inline Quat MulQ(Quat Left, Quat Right)
{
    Quat Result;

#ifdef LCF_HMM__USE_SSE
    __m128 SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.f, -0.f, 0.f, -0.f));
    __m128 SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(0, 1, 2, 3));
    __m128 SSEResultThree = _mm_mul_ps(SSEResultTwo, SSEResultOne);

    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(1, 1, 1, 1)) , _mm_setr_ps(0.f, 0.f, -0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(1, 0, 3, 2));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));

    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.f, 0.f, 0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));

    SSEResultOne = _mm_shuffle_ps(Left.SSE, Left.SSE, _MM_SHUFFLE(3, 3, 3, 3));
    SSEResultTwo = _mm_shuffle_ps(Right.SSE, Right.SSE, _MM_SHUFFLE(3, 2, 1, 0));
    Result.SSE = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t Right1032 = vrev64q_f32(Right.NEON);
    float32x4_t Right3210 = vcombine_f32(vget_high_f32(Right1032), vget_low_f32(Right1032));
    float32x4_t Right2301 = vrev64q_f32(Right3210);
    
    float32x4_t FirstSign = {1.0f, -1.0f, 1.0f, -1.0f};
    Result.NEON = vmulq_f32(Right3210, vmulq_f32(vdupq_laneq_f32(Left.NEON, 0), FirstSign));
    float32x4_t SecondSign = {1.0f, 1.0f, -1.0f, -1.0f};
    Result.NEON = vfmaq_f32(Result.NEON, Right2301, vmulq_f32(vdupq_laneq_f32(Left.NEON, 1), SecondSign));
    float32x4_t ThirdSign = {-1.0f, 1.0f, 1.0f, -1.0f};
    Result.NEON = vfmaq_f32(Result.NEON, Right1032, vmulq_f32(vdupq_laneq_f32(Left.NEON, 2), ThirdSign));
    Result.NEON = vfmaq_laneq_f32(Result.NEON, Right.NEON, Left.NEON, 3);
    
#else
    Result.x =  Right.raw[3] * +Left.raw[0];
    Result.y =  Right.raw[2] * -Left.raw[0];
    Result.z =  Right.raw[1] * +Left.raw[0];
    Result.w =  Right.raw[0] * -Left.raw[0];

    Result.x += Right.raw[2] * +Left.raw[1];
    Result.y += Right.raw[3] * +Left.raw[1];
    Result.z += Right.raw[0] * -Left.raw[1];
    Result.w += Right.raw[1] * -Left.raw[1];

    Result.x += Right.raw[1] * -Left.raw[2];
    Result.y += Right.raw[0] * +Left.raw[2];
    Result.z += Right.raw[3] * +Left.raw[2];
    Result.w += Right.raw[2] * -Left.raw[2];

    Result.x += Right.raw[0] * +Left.raw[3];
    Result.y += Right.raw[1] * +Left.raw[3];
    Result.z += Right.raw[2] * +Left.raw[3];
    Result.w += Right.raw[3] * +Left.raw[3];
#endif

    return Result;
}

static inline Quat MulQF(Quat Left, float Multiplicative)
{
    Quat Result;

#ifdef LCF_HMM__USE_SSE
    __m128 Scalar = _mm_set1_ps(Multiplicative);
    Result.SSE = _mm_mul_ps(Left.SSE, Scalar);
#elif defined(LCF_HMM__USE_NEON)
    Result.NEON = vmulq_n_f32(Left.NEON, Multiplicative);
#else
    Result.x = Left.x * Multiplicative;
    Result.y = Left.y * Multiplicative;
    Result.z = Left.z * Multiplicative;
    Result.w = Left.w * Multiplicative;
#endif

    return Result;
}

static inline Quat DivQF(Quat Left, float Divnd)
{
    Quat Result;

#ifdef LCF_HMM__USE_SSE
    __m128 Scalar = _mm_set1_ps(Divnd);
    Result.SSE = _mm_div_ps(Left.SSE, Scalar);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t Scalar = vdupq_n_f32(Divnd);
    Result.NEON = vdivq_f32(Left.NEON, Scalar);
#else
    Result.x = Left.x / Divnd;
    Result.y = Left.y / Divnd;
    Result.z = Left.z / Divnd;
    Result.w = Left.w / Divnd;
#endif

    return Result;
}

static inline float DotQ(Quat Left, Quat Right)
{
    float Result;

#ifdef LCF_HMM__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, Right.SSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t NEONMultiplyResult = vmulq_f32(Left.NEON, Right.NEON);
    float32x4_t NEONHalfAdd = vpaddq_f32(NEONMultiplyResult, NEONMultiplyResult);
    float32x4_t NEONFullAdd = vpaddq_f32(NEONHalfAdd, NEONHalfAdd);
    Result = vgetq_lane_f32(NEONFullAdd, 0);
#else
    Result = ((Left.x * Right.x) + (Left.z * Right.z)) + ((Left.y * Right.y) + (Left.w * Right.w));
#endif

    return Result;
}

static inline Quat InvQ(Quat Left)
{
    Quat Result;
    Result.x = -Left.x;
    Result.y = -Left.y;
    Result.z = -Left.z;
    Result.w = Left.w;

    return DivQF(Result, (DotQ(Left, Left)));
}

static inline Quat NormQ(Quat q)
{
    /* NOTE(lcf): Take advantage of SSE implementation in NormV4 */
    Vec4 v = (Vec4){q.x, q.y, q.z, q.w};
    v = NormV4(v);

    return (Quat) {v.x, v.y, v.z, v.w};
}

static inline Quat _MixQ(Quat Left, float MixLeft, Quat Right, float MixRight) {
    Quat Result;

#ifdef LCF_HMM__USE_SSE
    __m128 ScalarLeft = _mm_set1_ps(MixLeft);
    __m128 ScalarRight = _mm_set1_ps(MixRight);
    __m128 SSEResultOne = _mm_mul_ps(Left.SSE, ScalarLeft);
    __m128 SSEResultTwo = _mm_mul_ps(Right.SSE, ScalarRight);
    Result.SSE = _mm_add_ps(SSEResultOne, SSEResultTwo);
#elif defined(LCF_HMM__USE_NEON)
    float32x4_t ScaledLeft = vmulq_n_f32(Left.NEON, MixLeft);
    float32x4_t ScaledRight = vmulq_n_f32(Right.NEON, MixRight);
    Result.NEON = vaddq_f32(ScaledLeft, ScaledRight);
#else
    Result.x = Left.x*MixLeft + Right.x*MixRight;
    Result.y = Left.y*MixLeft + Right.y*MixRight;
    Result.z = Left.z*MixLeft + Right.z*MixRight;
    Result.w = Left.w*MixLeft + Right.w*MixRight;
#endif

    return Result;
}

static inline Quat NLerp(Quat Left, float Time, Quat Right)
{
    Quat Result = _MixQ(Left, 1.0f-Time, Right, Time);
    Result = NormQ(Result);

    return Result;
}

static inline Quat SLerp(Quat Left, float Time, Quat Right)
{
    Quat Result;

    float Cos_Theta = DotQ(Left, Right);

    if (Cos_Theta < 0.0f) { /* NOTE(lcf): Take shortest path on Hyper-sphere */
        Cos_Theta = -Cos_Theta;
        Right = (Quat){-Right.x, -Right.y, -Right.z, -Right.w};
    }

    /* NOTE(lcf): Use Normalized Linear interpolation when vectors are roughly not L.I. */
    if (Cos_Theta > 0.9995f) {
        Result = NLerp(Left, Time, Right);
    } else {
        float Angle = ACosF(Cos_Theta);
        float MixLeft = SinF((1.0f - Time) * Angle);
        float MixRight = SinF(Time * Angle);

        Result = _MixQ(Left, MixLeft, Right, MixRight);
        Result = NormQ(Result);
    }

    return Result;
}

static inline Mat4 QToM4(Quat Left)
{
    Mat4 Result;

    Quat NormalizedQ = NormQ(Left);

    float XX, YY, ZZ,
          XY, XZ, YZ,
          WX, WY, WZ;

    XX = NormalizedQ.x * NormalizedQ.x;
    YY = NormalizedQ.y * NormalizedQ.y;
    ZZ = NormalizedQ.z * NormalizedQ.z;
    XY = NormalizedQ.x * NormalizedQ.y;
    XZ = NormalizedQ.x * NormalizedQ.z;
    YZ = NormalizedQ.y * NormalizedQ.z;
    WX = NormalizedQ.w * NormalizedQ.x;
    WY = NormalizedQ.w * NormalizedQ.y;
    WZ = NormalizedQ.w * NormalizedQ.z;

    Result.raw[0][0] = 1.0f - 2.0f * (YY + ZZ);
    Result.raw[0][1] = 2.0f * (XY + WZ);
    Result.raw[0][2] = 2.0f * (XZ - WY);
    Result.raw[0][3] = 0.0f;

    Result.raw[1][0] = 2.0f * (XY - WZ);
    Result.raw[1][1] = 1.0f - 2.0f * (XX + ZZ);
    Result.raw[1][2] = 2.0f * (YZ + WX);
    Result.raw[1][3] = 0.0f;

    Result.raw[2][0] = 2.0f * (XZ + WY);
    Result.raw[2][1] = 2.0f * (YZ - WX);
    Result.raw[2][2] = 1.0f - 2.0f * (XX + YY);
    Result.raw[2][3] = 0.0f;

    Result.raw[3][0] = 0.0f;
    Result.raw[3][1] = 0.0f;
    Result.raw[3][2] = 0.0f;
    Result.raw[3][3] = 1.0f;

    return Result;
}

// This method taken from Mike Day at Insomniac Games.
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
//
// Note that as mentioned at the top of the paper, the paper assumes the matrix
// would be *post*-multiplied to a vector to rotate it, meaning the matrix is
// the transpose of what we're dealing with. But, because our matrices are
// stored in column-major order, the indices *appear* to match the paper.
//
// For example, m12 in the paper is row 1, column 2. We need to transpose it to
// row 2, column 1. But, because the column comes first when referencing
// elements, it looks like M.raw[1][2].
//
// Don't be confused! Or if you must be confused, at least trust this
// comment. :)
static inline Quat M4ToQ_RH(Mat4 M)
{
    float T;
    Quat Q;

    if (M.raw[2][2] < 0.0f) {
        if (M.raw[0][0] > M.raw[1][1]) {

            T = 1 + M.raw[0][0] - M.raw[1][1] - M.raw[2][2];
            Q = (Quat){
                T,
                M.raw[0][1] + M.raw[1][0],
                M.raw[2][0] + M.raw[0][2],
                M.raw[1][2] - M.raw[2][1]
            };
        } else {

            T = 1 - M.raw[0][0] + M.raw[1][1] - M.raw[2][2];
            Q = (Quat){
                M.raw[0][1] + M.raw[1][0],
                T,
                M.raw[1][2] + M.raw[2][1],
                M.raw[2][0] - M.raw[0][2]
            };
        }
    } else {
        if (M.raw[0][0] < -M.raw[1][1]) {

            T = 1 - M.raw[0][0] - M.raw[1][1] + M.raw[2][2];
            Q = (Quat){
                M.raw[2][0] + M.raw[0][2],
                M.raw[1][2] + M.raw[2][1],
                T,
                M.raw[0][1] - M.raw[1][0]
            };
        } else {

            T = 1 + M.raw[0][0] + M.raw[1][1] + M.raw[2][2];
            Q = (Quat){
                M.raw[1][2] - M.raw[2][1],
                M.raw[2][0] - M.raw[0][2],
                M.raw[0][1] - M.raw[1][0],
                T
            };
        }
    }

    Q = MulQF(Q, 0.5f / SqrtF(T));

    return Q;
}

static inline Quat M4ToQ_LH(Mat4 M)
{
    float T;
    Quat Q;

    if (M.raw[2][2] < 0.0f) {
        if (M.raw[0][0] > M.raw[1][1]) {

            T = 1 + M.raw[0][0] - M.raw[1][1] - M.raw[2][2];
            Q = (Quat){
                T,
                M.raw[0][1] + M.raw[1][0],
                M.raw[2][0] + M.raw[0][2],
                M.raw[2][1] - M.raw[1][2]
            };
        } else {

            T = 1 - M.raw[0][0] + M.raw[1][1] - M.raw[2][2];
            Q = (Quat){
                M.raw[0][1] + M.raw[1][0],
                T,
                M.raw[1][2] + M.raw[2][1],
                M.raw[0][2] - M.raw[2][0]
            };
        }
    } else {
        if (M.raw[0][0] < -M.raw[1][1]) {

            T = 1 - M.raw[0][0] - M.raw[1][1] + M.raw[2][2];
            Q = (Quat){
                M.raw[2][0] + M.raw[0][2],
                M.raw[1][2] + M.raw[2][1],
                T,
                M.raw[1][0] - M.raw[0][1]
            };
        } else {

            T = 1 + M.raw[0][0] + M.raw[1][1] + M.raw[2][2];
            Q = (Quat){
                M.raw[2][1] - M.raw[1][2],
                M.raw[0][2] - M.raw[2][0],
                M.raw[1][0] - M.raw[0][2],
                T
            };
        }
    }

    Q = MulQF(Q, 0.5f / SqrtF(T));

    return Q;
}


static inline Quat QFromAxisAngle_RH(Vec3 Axis, float Angle)
{
    Quat Result;

    Vec3 AxisNormalized = NormV3(Axis);
    float SineOfRotation = SinF(Angle / 2.0f);

    Result.xyz = MulV3F(AxisNormalized, SineOfRotation);
    Result.w = CosF(Angle / 2.0f);

    return Result;
}

static inline Quat QFromAxisAngle_LH(Vec3 Axis, float Angle)
{
    return QFromAxisAngle_RH(Axis, -Angle);
}

static inline Vec2 RotateV2(Vec2 V, float Angle)
{
    float sinA = SinF(Angle);
    float cosA = CosF(Angle);

    return (Vec2){V.x * cosA - V.y * sinA, V.x * sinA + V.y * cosA};
}

// implementation from
// https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
static inline Vec3 RotateV3Q(Vec3 V, Quat Q)
{
    Vec3 t = MulV3F(CrossV3(Q.xyz, V), 2);
    return AddV3(V, AddV3(MulV3F(t, Q.w), CrossV3(Q.xyz, t)));
}

static inline Vec3 RotateV3AxisAngle_LH(Vec3 V, Vec3 Axis, float Angle) {
    return RotateV3Q(V, QFromAxisAngle_LH(Axis, Angle));
}

static inline Vec3 RotateV3AxisAngle_RH(Vec3 V, Vec3 Axis, float Angle) {
    return RotateV3Q(V, QFromAxisAngle_RH(Axis, Angle));
}

#endif /* LCF_HMM_H */



