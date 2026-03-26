#pragma once

// #include <stdfloat>
#include <stdint.h>

/* signed types */
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

/* unsigned types */
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

/* floating point types */
// using f16 = std::float16_t;
using f32 = float;
using f64 = double;
using f128 = long double;

/* boolean types */
using b8 = u8;
using b16 = u16;
using b32 = u32;
using b64 = u64;

/* memory size macros */
#define B(x) (x)
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) (((u64)x) << 40)

/* signed constants */
static const i8 MAX_I8 = 127;
static const i16 MAX_I16 = 32767;
static const i32 MAX_I32 = 2147483647;
static const i64 MAX_I64 = 9223372036854775807;
static const i8 MIN_I8 = -127 - 1;
static const i16 MIN_I16 = -32767 - 1;
static const i32 MIN_I32 = -2147483647 - 1;
static const i64 MIN_I64 = -9223372036854775807 - 1;

/* unsigned constants */
static const u8 MAX_U8 = 0xFF;				   // 255
static const u16 MAX_U16 = 0xFFFF;			   // 65535
static const u32 MAX_U32 = 0xFFFFFFFF;		   // 4294967295
static const u64 MAX_U64 = 0xFFFFFFFFFFFFFFFF; // 18446744073709551615

/* floating point constants */
static const f32 MAX_F32 = 3.402823466e+38f;
static const f32 MIN_F32 = -3.402823466e+38f;
static const f32 SMALLEST_POSITIVE_F32 = 1.1754943508e-38f;
static const f32 EPSILON_F32 = 5.96046448e-8f;
static const f32 PI_F32 = 3.14159265359f;
static const f32 HALF_PI_F32 = 1.5707963267f;
static const f64 MAX_F64 = 1.79769313486231e+308;
static const f64 MIN_F64 = -1.79769313486231e+308;
static const f64 SMALLEST_POSITIVE_F64 = 4.94065645841247e-324;
static const f64 EPSILON_F64 = 1.11022302462515650e-16;