#ifndef FINDARGS_H
#define FINDARGS_H

#ifndef KERNEL
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL/cl.h>
#undef CL_VERSION_1_2
#include <CL/cl.hpp>
#endif

#include "Types.h"

typedef struct FindArgs
{
#ifdef __cplusplus
    FindArgs(size b, size e, uint32_t val)
        :begin(b),end(e),value(val)
    {

    }
#endif
    size begin;
    size end;
    uint32_t value;
} FindArgs;

#endif // FINDARGS_H
