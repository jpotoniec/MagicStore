#include "GPU.hpp"
#include <stdexcept>
#include <iostream>
#include <iterator>
#include "FindArgs.h"

#if USE_GPU

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


GPU::GPU()
    :bufData(NULL),data(NULL),length(0)
{
    const char *platform=getenv("PLATFORM");
    cl::Platform p=cl::Platform::getDefault();
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    std::clog<<"Available platforms:\n";
    for(auto& a:all_platforms)
        std::clog<<a.getInfo<CL_PLATFORM_NAME>()<<"\n";
    if(platform!=NULL)
    {
        for(auto& a:all_platforms)
        {
            if(platform==a.getInfo<CL_PLATFORM_NAME>())
            {
                p=a;
                break;
            }
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
    kfind=cl::Kernel(prog,"kfind");
}

GPU::~GPU()
{
    if(bufData)
        delete bufData;
}

void GPU::setData(uint8_t *data, size length)
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

std::deque<Address> GPU::find(std::deque<FindArgs> &requests) const
{
    cl::CommandQueue queue(context,dev);
//    std::clog<<"Queue and kernel constructed"<<std::endl;
    std::deque<Address> result;
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
            result.push_back(Address(output[i].s[0],output[i].s[1]));
    }
    delete []output;
    return result;
}

void GPU::testSort()
{
    size n=15;
    uint32_t *data=new uint32_t[n];
    for(int i=0;i<n;++i)
        data[i]=rand()%20;
    for(int i=0;i<n;++i)
        std::cout<<data[i]<<" ";
    std::cout<<"\n";
    sort(data, n);
    for(int i=0;i<n;++i)
        std::cout<<data[i]<<" ";
    std::cout<<"\n";
    for(int i=1;i<n;++i)
        if(data[i-1]>data[i])
            std::cout<<"Zonk@"<<i<<std::endl;
}

extern const char *BitonicSort_cl;

static size zeroCopySizeAlignment (size requiredSize)
{
    // Please refer to Intel Zero Copy Tutorial and OpenCL Performance Guide
    // The following statement rounds requiredSize up to the next 64-byte boundary
    return requiredSize + (~requiredSize + 1) % 64;   // or even shorter: requiredSize + (-requiredSize) % 64
}

void GPU::sort(uint32_t *p_input, size arraySize)
{
    uint32_t *ptr=p_input;
    for(uint8_t i=8*sizeof(arraySize);i>=2;i--)
    {
        size mask=(1<<i);
        if(arraySize&mask)
        {
            sort2(ptr, mask);
            ptr+=mask;
        }
    }
}

//wymaga, żeby arraySize było potęgą dwójki
void GPU::sort2(uint32_t *p_input, size arraySize)
{

    cl::CommandQueue queue(context,dev);
    ksort=cl::Kernel(load(BitonicSort_cl),"BitonicSort");

    cl_int numStages = 0;
    cl_uint temp;

    cl_int stage;
    cl_int passOfStage;

    cl::Buffer inputBuffer(context, CL_MEM_USE_HOST_PTR, zeroCopySizeAlignment(sizeof(uint32_t) * arraySize), p_input);

    for (temp = arraySize; temp > 2; temp >>= 1)
    {
        numStages++;
    }

    cl::make_kernel<cl::Buffer&,cl_uint, cl_uint, cl_uint> sort(ksort);

    for (stage = 0; stage < numStages; stage++)
    {
        for (passOfStage = stage; passOfStage >= 0; passOfStage--)
        {
            // set work-item dimensions
            size_t gsz = arraySize / (2*4);
            size_t global_work_size = ( passOfStage ? gsz : gsz << 1 );    //number of quad items in input array

            sort(cl::EnqueueArgs(queue,global_work_size), inputBuffer, stage, passOfStage, 1);

        }
    }
    queue.finish();
    auto tmp_ptr = queue.enqueueMapBuffer(inputBuffer, true, CL_MAP_READ, 0, sizeof(cl_int) * arraySize);
    if(tmp_ptr!=p_input)
    {
        throw std::runtime_error("clEnqueueMapBuffer failed to return original pointer\n");
    }
    queue.finish();
    queue.enqueueUnmapMemObject(inputBuffer, tmp_ptr);
}

#endif
