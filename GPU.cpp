#include "GPU.hpp"
#include <stdexcept>
#include <iostream>
#include <iterator>
#include "FindArgs.h"

#if 0
static std::string get_file_contents(const std::string& filename)
{
        std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
        if (in)
        {
                std::string contents;
                in.seekg(0, std::ios::end);
                contents.resize(in.tellg());
                in.seekg(0, std::ios::beg);
                in.read(&contents[0], contents.size());
                in.close();
                return(contents);
        }
        return "";
}
#endif

extern const char *kernel_cl;

cl::Program GPU::load(const std::string& text)
{
        cl::Program prog(context, text, false);
        try
        {
            prog.build("");
            std::cerr<<prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev)<<std::endl;
        }
        catch(cl::Error& err)
        {
            std::cerr<<prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev)<<std::endl;
            throw err;
        }
        return prog;
}


GPU::GPU(const std::string& platform)
    :bufData(NULL),data(NULL),length(0)
{
    cl::Platform p=cl::Platform::getDefault();
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    std::clog<<"Available platforms:\n";
    for(auto& a:all_platforms)
        std::clog<<a.getInfo<CL_PLATFORM_NAME>()<<"\n";
    for(auto& a:all_platforms)
    {
        if(platform==a.getInfo<CL_PLATFORM_NAME>())
        {
            p=a;
            break;
        }
    }
    std::clog<<"Selected platform: "<<p.getInfo<CL_PLATFORM_NAME>()<<"\n";
    std::vector<cl::Device> devices;
    p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    std::clog<<"Devices:\n";
    for(auto& d:devices)
            std::clog<<d.getInfo<CL_DEVICE_NAME>()<<"\n";
    std::clog<<std::endl;
    dev=devices[0];
    context=cl::Context(dev);
    prog=load(kernel_cl);
    kfind=cl::Kernel(prog,"find");
}

GPU::~GPU()
{
    if(bufData)
        delete bufData;
}

void GPU::setData(uint8_t *data, size_t length)
{
    if(this->data!=data)
    {
        this->data=data;
        this->length=length;
        if(bufData)
            delete bufData;
        bufData=new cl::Buffer(context, data, data+length, true, false);
//        bufData=new cl::Buffer(context, CL_MEM_READ_ONLY, length);
//        cl::CommandQueue queue(context,dev);
//        queue.enqueueWriteBuffer(*bufData, false, 0, length, data);
//        queue.finish();
        std::clog<<"Set data to blob of "<<length<<" bytes"<<std::endl;
    }
}

std::deque<BinaryTriples::Address> GPU::find(std::deque<FindArgs> &requests)
{
    cl::CommandQueue queue(context,dev);
//    std::clog<<"Queue and kernel constructed"<<std::endl;
    std::deque<BinaryTriples::Address> result;
    cl_ulong2 *output=new cl_ulong2[requests.size()];
    cl::Buffer bufOutput(context, CL_MEM_WRITE_ONLY, requests.size()*sizeof(cl_ulong2));
//    std::clog<<"Output buffer ready"<<std::endl;
    cl::Buffer bufArgs(context, requests.begin(), requests.end(), true, false);
//    std::clog<<"Args buffer ready"<<std::endl;
    cl::make_kernel<cl::Buffer&,cl::Buffer&,cl::Buffer&> find(kfind);
    find(cl::EnqueueArgs(queue, cl::NDRange(requests.size())), *bufData, bufArgs, bufOutput);
    queue.finish();
//    std::clog<<"Kernels executed"<<std::endl;
    cl::copy(queue, bufOutput, output, output+requests.size());
    queue.finish();
    for(size_t i=0;i<requests.size();++i)
    {
        if(output[i].s[0]!=-1)
            result.push_back(BinaryTriples::Address(output[i].s[0],output[i].s[1]));
    }
    delete []output;
    return result;
}
