#define KERNEL
#include "Types.h"

__kernel void sort(__global uint32_t *data, size length, size n, size k)
{
    size a,b,c,x;
    size id=get_global_id(0);
    bool dir=(id&(1<<(n-1)));
    x=(id>>(k-1))*(1<<k);
    c=id&((1<<(k-1))-1);
    a=c+x;
//    a+=(id&((1<<k)-1));
    b=a+(1<<(k-1));
//    printf("id=%d a=%d b=%d dir=%d c=%d x=%d\n",id,a,b,dir,c,x);
    if(dir)
    {
        size x=b;
        b=a;
        a=x;
    }
    if(data[a]>data[b])
    {
        uint32_t x=data[b];
        data[b]=data[a];
        data[a]=x;
    }
}
