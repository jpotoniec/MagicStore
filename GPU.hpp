#ifndef GPU_HPP
#define GPU_HPP

#include "BinaryTriples.hpp"
#include "FindArgs.h"
#include <CL/cl.hpp>
#include <deque>
#include <string>

class GPU
{
public:
    class Request
    {
    public:
        Request(size_t start, size_t end, uint32_t value)
            :begin(start),end(end),value(value)
        {
        }
        const size_t begin;
        const size_t end;
        const uint32_t value;
    };

    GPU();
    ~GPU();
    void setData(uint8_t *data, size_t length);
    std::deque<BinaryTriples::Address> find(std::deque<FindArgs> &requests);
    void test();
private:
    cl::Program load(const std::string& text);
    cl::Device dev;
    cl::Context context;
    cl::Program prog;
    cl::Buffer *bufData;
    const uint8_t *data;
    size_t length;
};

#endif // GPU_HPP
