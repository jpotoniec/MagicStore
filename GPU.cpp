#include "GPU.hpp"
#include <stdexcept>
#include <iostream>
#include <iterator>
#include "FindArgs.h"
#include <cassert>
#include "Timer.hpp"
#include <algorithm>

#if USE_GPU

extern const char *kernel_cl;
extern const char *Sort_cl;

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
    prog=load(Sort_cl);
    ksort=cl::Kernel(prog,"sort");
    kmerge=cl::Kernel(prog,"merge");
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

std::deque<Address> GPU::find(std::vector<FindArgs> &requests) const
{
    cl::CommandQueue queue(context,dev);
//    std::clog<<"Queue and kernel constructed"<<std::endl;
    std::deque<Address> result;
    cl_ulong2 *output=new cl_ulong2[requests.size()];
    cl::Buffer bufOutput(context, CL_MEM_WRITE_ONLY, requests.size()*sizeof(cl_ulong2));
//    std::clog<<"Output buffer ready"<<std::endl;
    cl::Buffer bufArgs(context, requests.data(), requests.data()+requests.size(), true, true);
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
    size n=1e8;
    uint32_t *data=new uint32_t[n];
    uint32_t *orig=new uint32_t[n];
    for(int i=0;i<n;++i)
        orig[i]=rand();
#if 0
    for(int i=0;i<n;++i)
        std::cout<<data[i]<<" ";
    std::cout<<"\n";
#endif
    auto t1=ustimer();
    for(int i=0;i<10;++i)
    {
        std::copy(orig, orig+n, data);
        sort(data, n);
    }
    auto t2=ustimer();
    for(int i=0;i<10;++i)
    {
        std::copy(orig, orig+n, data);
        std::sort(data, data+n);
    }
    auto t3=ustimer();
    std::cout<<"gpu: "<<(t2-t1)<<" cpu:"<<(t3-t2)<<std::endl;
#if 0
    for(int i=0;i<n;++i)
        std::cout<<data[i]<<" ";
    std::cout<<"\n";
#endif
    for(int i=1;i<n;++i)
        if(data[i-1]>data[i])
            std::cout<<"Zonk@"<<i<<std::endl;
}

static size zeroCopySizeAlignment (size requiredSize)
{
    // Please refer to Intel Zero Copy Tutorial and OpenCL Performance Guide
    // The following statement rounds requiredSize up to the next 64-byte boundary
    return requiredSize + (~requiredSize + 1) % 64;   // or even shorter: requiredSize + (-requiredSize) % 64
}

void GPU::merge(uint32_t *input, size *positions, size n) const
{
    cl::make_kernel<cl::Buffer&,cl::Buffer&,cl::Buffer&> merge(this->kmerge);
    cl::CommandQueue queue(context,dev);
    size len=positions[n];
    cl::Buffer inputBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t)*len);
    queue.enqueueWriteBuffer(inputBuffer, false, 0, sizeof(uint32_t)*len, input);
    cl::Buffer outputBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t)*len);
    cl::Buffer positionsBuffer(context, CL_MEM_READ_WRITE, sizeof(size)*(n+1));
    if(n%2)
        queue.enqueueCopyBuffer(inputBuffer, outputBuffer, sizeof(uint32_t)*positions[n-1],sizeof(uint32_t)*positions[n-1], (positions[n]-positions[n-1])*sizeof(uint32_t));    //kopiowanie ostatniego fragmentu, który przez pewien czas może nie być mergowany (jak długo jest nieparzysta liczba list)
    while(n>1)
    {
//        std::clog<<n<<": ";
//        std::copy(positions,positions+n+1,std::ostream_iterator<size>(std::clog," "));
//        std::clog<<std::endl;
        queue.enqueueWriteBuffer(positionsBuffer, false, 0, (n+1)*sizeof(size), positions);
        merge(cl::EnqueueArgs(queue,n/2),inputBuffer,outputBuffer,positionsBuffer);
        size o=n;
        n-=n/2;
        for(size i=0;i<n+1-o%2;++i)
            positions[i]=positions[2*i];
        if(o%2)
            positions[n]=positions[o];
        std::swap(inputBuffer,outputBuffer);
    }
    queue.enqueueReadBuffer(inputBuffer, false, 0, sizeof(uint32_t)*len, input);
    queue.finish();
}

void GPU::testMerge()
{
    uint32_t data[]={1,5,10,15,20,
                     16,17,18,
                     2,3,4,
                     6,7,8,9};
    size positions[]={0,5,8,11,15};
    size n=sizeof(positions)/sizeof(positions[0])-1;
    merge(data, positions, n);
    std::copy(data, data+positions[n], std::ostream_iterator<uint32_t>(std::cout," "));
    std::cout<<std::endl;
}

void GPU::sort(uint32_t *p_input, size arraySize) const
{
    cl::make_kernel<cl::Buffer&,size,size,size> sort(this->ksort);
    cl::CommandQueue queue(context,dev);
    size ceiling=1;
    while(ceiling<arraySize)
        ceiling<<=1;
    cl::Buffer inputBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t)*ceiling);
    queue.enqueueWriteBuffer(inputBuffer, false, 0, sizeof(uint32_t)*arraySize, p_input);
    if(ceiling!=arraySize)
    {
        // z jakiejś przyczyny segfaultuje nawet na pocl
        //    queue.enqueueFillBuffer(inputBuffer, std::numeric_limits<uint32_t>::max(), 0, sizeof(uint32_t)*ceiling);
        size rest=ceiling-arraySize;
        uint32_t *pattern=new uint32_t[rest];
        std::fill(pattern, pattern+rest, std::numeric_limits<uint32_t>::max());
        queue.enqueueWriteBuffer(inputBuffer, false, sizeof(uint32_t)*arraySize, rest*sizeof(uint32_t), pattern);
        delete []pattern;
//  ekstremalnie powolne
//        uint32_t pattern=std::numeric_limits<uint32_t>::max();
//        for(size n=arraySize;n<ceiling;++n)
//            queue.enqueueWriteBuffer(inputBuffer, false, n*sizeof(uint32_t), sizeof(pattern), &pattern);
    }
    for(size n=1;(1<<n)<=ceiling;n++)
    {
        for(size k=n; k>=1; k--)
        {
            sort(cl::EnqueueArgs(queue,ceiling/2), inputBuffer, arraySize, n, k);
#if 0
            uint32_t *tmp=new uint32_t[ceiling];
        queue.enqueueReadBuffer(inputBuffer, false, 0, sizeof(uint32_t)*ceiling, tmp);
        queue.finish();
        std::cout<<"n="<<n<<" k="<<k<<": ";
        for(int j=0;j<ceiling;++j)
            std::cout<<tmp[j]<<" ";
        std::cout<<"\n";
        delete []tmp;
#endif
        }
    }
    queue.enqueueReadBuffer(inputBuffer, false, 0, sizeof(uint32_t)*arraySize, p_input);
    queue.finish();
}
#endif
