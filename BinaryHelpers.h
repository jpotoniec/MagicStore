#ifndef BINARYHELPERS_H
#define BINARYHELPERS_H

#ifdef KERNEL
typedef uchar uint8_t;
typedef uint uint32_t;
#define GLOBAL_SPACE global
#define CONST_SPACE constant
#else
#define GLOBAL_SPACE
#define CONST_SPACE
#endif

const CONST_SPACE size_t InvalidPosition=-1;

inline uint8_t len(uint32_t value)
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

#ifndef KERNEL
inline void write(uint8_t *where, size_t &wlen, uint32_t val)
{
    size_t _len=len(val);
    switch(_len)
    {
        case 3:
            where[wlen+3]=static_cast<uint8_t>((val>>22)&0xff);
        case 2:
            where[wlen+2]=static_cast<uint8_t>((val>>14)&0xff);
        case 1:
            where[wlen+1]=static_cast<uint8_t>((val>>6)&0xff);
        case 0:
            where[wlen+0]=static_cast<uint8_t>((val>>0)&0x3f)|(static_cast<uint8_t>(_len)<<6);
    }
    wlen+=_len+1;
}
#endif

inline void skip(GLOBAL_SPACE const uint8_t *where, size_t *position)
{
    uint8_t len=(((where[*position])>>6)&0b11);
    *position+=len+1;
}

inline uint32_t read(GLOBAL_SPACE const uint8_t *where, size_t *position)
{
    uint8_t len=(((where[*position])>>6)&0b11);
    uint32_t result=0;
    switch(len)
    {
        case 3:
            result|=((where[*position+3])<<22);
        case 2:
            result|=((where[*position+2])<<14);
        case 1:
            result|=((where[*position+1])<<6);
        case 0:
            result|=((where[*position+0]&0x3f)<<0);
    }
    *position+=len+1;
    return result;
}

#ifdef __cplusplus
inline void skip(const uint8_t *where, size_t &position)
{
    skip(where, &position);
}
inline uint32_t read(const uint8_t *where, size_t &position)
{
    return read(where, &position);
}
#endif

inline size_t find(GLOBAL_SPACE const uint8_t* where, size_t n, uint32_t value)
{
    uint8_t length=len(value);
    for(size_t i=0;i<n;i+=sizeof(size_t))
    {
        uint8_t len=(((where[i])>>6)&0b11);
        if(len==length)
        {
            uint32_t c=read(where, &i);
            if(c==value)
                return i;
        }
        else
            i+=len+1;
    }
    return InvalidPosition;
}

#endif // BINARYHELPERS_H
