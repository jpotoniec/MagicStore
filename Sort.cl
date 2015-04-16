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

#define MAX ((uint32_t)-1)

__kernel void merge(__global uint32_t *input, __global uint32_t *output, __global size *positions)
{
    size id=get_global_id(0);
    size i=positions[2*id];
    size ae=positions[2*id+1];
    size j=positions[2*id+1];
    size be=positions[2*id+2];
    size pos=i;
//    printf("[%d] (%d,%d)~(%d,%d) (%d,%d)~(%d,%d)\n",id,i,ae,2*id,2*id+1,j,be,2*id+1,2*id+2);
    while(i<ae || j<be)
    {
        uint32_t a=(i<ae)?input[i]:MAX;
        uint32_t b=(j<be)?input[j]:MAX;
//        printf("[%d] pos=%d i=%d a=%d j=%d b=%d\n",id,pos,i,a,j,b);
        if(a<b)
        {
            output[pos++]=a;
            i++;
        }
        else
        {
            output[pos++]=b;
            j++;
        }
    }
}

__kernel void updatePositions(__global size *positions,size o,size n)
{
    for(size i=0;i<n+1-o%2;++i)
        positions[i]=positions[2*i];
    if(o%2)
        positions[n]=positions[o];
}
