#pragma once

#include <stdio.h>

typedef unsigned char u8;
typedef unsigned __INT16_TYPE__ u16;
typedef unsigned __INT32_TYPE__ u32;
typedef unsigned __INT64_TYPE__ u64;

typedef char i8;
typedef char c8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __INT64_TYPE__ i64;

typedef float f32;
typedef double f64;
typedef _Bool b8;

#define true 1
#define false 0

#define NULLPTR ((void *)0)

#define U8_MAX 255
#define I8_MAX 127
#define U16_MAX 65535
#define I16_MAX 32767
#define U32_MAX 4294967295
#define I32_MAX 2147483647
#define U64_MAX 18446744073709551615
#define I64_MAX 9223372036854775807

#define F32_MAX 3.402823466e+38f
#define F64_MAX 1.79769e+308

#define Bit(n) ((n) / 8.0f)
#define KB(n) ((n) / 1000.0f)
#define MB(n) ((n) / 1000000.0f)
#define GB(n) ((n) / 1000000000.0f)

#define KiB(n) ((u64)(n) << 10)
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)

#define PROG_SUCCESS 0
#define PROG_FAIL (-1)

#define MAIN_PROG int

#define EXIT(s)                                                                \
    {                                                                          \
        fflush(stdout);                                                        \
        fflush(stderr);                                                        \
        asm volatile("movl %0, %%edi\n"                                        \
                     "movl $60, %%eax\n"                                       \
                     "syscall"                                                 \
                     :                                                         \
                     : "r"(s)                                                  \
                     : "%eax", "%edi");                                        \
    }
