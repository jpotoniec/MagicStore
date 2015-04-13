#ifndef FINDARGS_H
#define FINDARGS_H

#ifdef __cplusplus
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL/cl.h>
#undef CL_VERSION_1_2
#include <CL/cl.hpp>
#endif

#ifndef __cplusplus
typedef ulong cl_ulong;
typedef uint cl_uint;
#endif

typedef struct FindArgs
{
#ifdef __cplusplus
    FindArgs(cl_ulong b, cl_ulong e, cl_uint val)
        :begin(b),end(e),value(val)
    {

    }
#endif
    cl_ulong begin;
    cl_ulong end;
    cl_uint value;
} FindArgs;

#endif // FINDARGS_H
