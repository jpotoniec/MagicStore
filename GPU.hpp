#ifndef GPU_HPP
#define GPU_HPP

#if USE_GPU

#include "FindArgs.h"
#include <deque>
#include <string>
#include <boost/noncopyable.hpp>

class GPU : private boost::noncopyable
{
public:
    GPU();
    ~GPU();
    void setData(uint8_t *data, size length);
    std::deque<Address> find(std::vector<FindArgs> &requests) const;
    void test();
    void sort(uint32_t *p_input, size arraySize) const;
    void testSort();
private:
    cl::Program load(const std::string& text);
    cl::Device dev;
    cl::Context context;
    cl::Program prog;
    cl::Buffer *bufData;
    const uint8_t *data;
    size length;
    cl::Kernel kfind,ksort;
};

#endif

#endif // GPU_HPP
