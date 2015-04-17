#ifndef TIMER_HPP
#define TIMER_HPP

#include <sys/time.h>

inline uint64_t ustimer()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<uint64_t>(tv.tv_sec)*1e6+tv.tv_usec;
}

#endif // TIMER_HPP
