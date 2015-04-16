#define KERNEL 1
#include "FindArgs.h"
#include "BinaryHelpers.h"

ulong read64(global const uchar* data, uint pos)
{
    ulong val;
    uchar *ptr=(uchar*)&val;
    for(int i=0;i<8;++i)
        ptr[i]=data[pos+i];
    return val;
}

void kernel kfind(global const uchar* data, global const FindArgs* params, global ulong2 *result)
{
    FindArgs args=params[get_global_id(0)];
    size d=find(data+args.begin, args.end-args.begin, args.value);
    ulong b=-1;
    ulong e=-1;
    if(d!=-1)
    {
        b=read64(data+args.begin,d);
        d+=8;
        skip(data+args.begin,&d);
        e=read64(data+args.begin,d);
    }
    result[get_global_id(0)].x=b;
    result[get_global_id(0)].y=e;
}
