#ifndef TYPES_H
#define TYPES_H

#ifdef KERNEL
typedef uchar uint8_t;
typedef uint uint32_t;
typedef ulong uint64_t;
#define GLOBAL_SPACE global
#define CONST_SPACE constant
#else
#define GLOBAL_SPACE
#define CONST_SPACE
#include <cstdint>
#endif

typedef uint64_t size;

#endif // TYPES_H
