#ifndef GPU_HPP
#define GPU_HPP

#if USE_GPU

#include "FindArgs.h"
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL/cl.h>
#undef CL_VERSION_1_2
#include <CL/cl.hpp>
#include <deque>
#include <string>
#include <boost/noncopyable.hpp>

class GPU : private boost::noncopyable
{
public:
    GPU();
    ~GPU();
    void setData(uint8_t *data, size length);
    std::deque<Address> find(std::deque<FindArgs> &requests) const;
    void test();
private:
    cl::Program load(const std::string& text);
    cl::Device dev;
    cl::Context context;
    cl::Program prog;
    cl::Buffer *bufData;
    const uint8_t *data;
    size length;
    cl::Kernel kfind;
};

#endif

#endif // GPU_HPP
