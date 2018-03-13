#ifndef KH_TYPES_H

#include <stdint.h>
#include <limits.h>
#include <float.h>

typedef intptr_t smm;
typedef uintptr_t umm;

typedef int8_t  i8; // signed char
typedef int16_t i16; // signed short
typedef int32_t i32; // signed long
typedef int64_t i64; // signed long long
typedef int8_t  s8; // signed char
typedef int16_t s16; // signed short
typedef int32_t s32; // signed long
typedef int64_t s64; // signed long long
typedef i32 b32;

typedef uint8_t  u8; // unsigned char 
typedef uint16_t u16; // unsigned short
typedef uint32_t u32; // unsigned long
typedef uint64_t u64; // unsigned long long

typedef size_t usize;

typedef float f32;
typedef double f64;

#define KH_GLOBAL static
#define KH_INTERN static
#define KH_PERSIST static
#define KH_INLINE inline

#define F32_MIN FLT_MIN
#define F32_MAX FLT_MAX
#define I16_MIN INT_MIN
#define I16_MAX INT_MAX
#define U16_MAX UINT_MAX
#define I32_MIN LONG_MIN
#define I32_MAX LONG_MAX
#define U32_MAX ULONG_MAX

#define kb(x) ((x) << 10)
#define mb(x) ((x) << 20)
#define gb(x) ((x) << 30)
#define tb(x) ((x)*(1LL << 40))

#define kilobytes(x) kb(x)
#define megabytes(x) mb(x)
#define gigabytes(x) gb(x)
#define terabytes(x) tb(x)

#define KH_TYPES_H
#endif