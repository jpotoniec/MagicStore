#ifndef FINDARGS_H
#define FINDARGS_H

#ifdef __cplusplus
#include <CL/cl.hpp>
#endif

typedef struct FindArgs
{
#ifdef __cplusplus
    FindArgs(cl_uint b, cl_uint e, cl_uint val)
        :begin(b),end(e),value(val)
    {

    }
#endif
    uint begin;
    uint end;
    uint value;
    uint wbegin;
    uint wend;
} FindArgs;

#endif // FINDARGS_H
