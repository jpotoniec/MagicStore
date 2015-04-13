#include "FindArgs.h"

#define uint8_t uchar
#define uint32_t uint

inline uint read(global const uchar *where, uint *position)
{
    uchar len=(((where[*position])>>6)&0b11);
    uint result=0;
    switch(len)
    {
        case 3:
            result|=(where[*position+3]<<22);
        case 2:
            result|=(where[*position+2]<<14);
        case 1:
            result|=(where[*position+1]<<6);
        case 0:
            result|=(where[*position+0]&0x3f);
    }
    *position+=len+1;
    return result;
}

inline uchar len(uint value)
{
    if(value<=0x3f)
        return 0;
    else if(value<=0x3fff)
        return 1;
    else if(value<=0x3fffff)
        return 2;
    else
        return 3;
}

uint _find(global const uchar* where, uint n, uint value)
{
    uchar length=len(value);
    for(uint i=0;i<n;i+=4)
    {
        uint c=read(where, &i);
        if(c==value)
            return i;
//        uchar len=(((where[i])>>6)&0b11);
//        if(len==length)
//        {
//            uint c=read(where, &i);
//            if(c==value)
//                return i;
//        }
//        else
//            i+=len+1;
    }
    return -1;
}

void kernel find(global const uchar* data, global const FindArgs* params, global uint2 *result)
{
    FindArgs args=params[get_global_id(0)];
    uint pos=args.begin;
    for(int i=0;i<args.value;++i)
    {
    result[i].x=read(data, &pos);
    pos+=4;
    result[i].y=len(result[i].x);
    }
    #if 0
    uint d=_find(data+args.begin, args.end-args.begin+1, args.value);
    uint pos=args.begin+10002;
    uint b=read(data, &pos);
    uint pos2=0;
    uint e=d;//(data[pos+3]<<24)|(data[pos+2]<<16)|(data[pos+1]<<8)|(data[pos+0]<<0);
//    if(d!=-1)
//    {
//        b=*(uint*)(data+d);
//        read(data,d);
//        e=*(uint*)(data+d);
//    }
    result[get_global_id(0)].x=b;
    result[get_global_id(0)].y=e;
    #endif
}

#if 0
void kernel old_find(global const uchar* data, global const FindArgs* params, global int *result)
{
	FindArgs args=params[get_global_id(0)];
	uint pos=args.wbegin;
	uint i;
	for(i=args.begin;i<=args.end;++i)
	{
		if(data[i]==args.value)
		{
			result[pos]=i;
			pos++;
			if(pos==args.wend)
				break;
		}
	}
	for(;pos<args.wend;++pos)
		result[pos]=-1;
	result[args.wend]=i;
//	printf("Hello, nurse: %d %d %d %d\n", get_global_id(0), args.begin, args.end, args.value);
}

void kernel simple_add(global const int* A, global const int* B, global int* C){
    C[get_global_id(0)]=A[get_global_id(0)]+B[get_global_id(0)];
}
#endif
