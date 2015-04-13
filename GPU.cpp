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
    dev=devices[0];
    context=cl::Context(dev);
    prog=load(kernel_cl);
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
#if 0
    cl::CommandQueue queue(context,dev);
    cl::make_kernel<cl::Buffer&,cl::Buffer&,cl::Buffer&> find(cl::Kernel(prog,"find"));
    FindArgs a=requests[0];
    cl_uint2 *output=new cl_uint2[a.value];
    cl::Buffer bufOutput(context, CL_MEM_WRITE_ONLY, a.value*sizeof(cl_uint2));
    cl::Buffer bufArgs(context, requests.begin(), requests.end(), true, false);
    find(cl::EnqueueArgs(queue, cl::NDRange(requests.size())), *bufData, bufArgs, bufOutput);
    queue.finish();
    cl::copy(queue, bufOutput, output, output+a.value);
    queue.finish();
    std::deque<BinaryTriples::Address> result;
    for(int i=0;i<a.value;++i)
        result.push_back(BinaryTriples::Address(output[i].s[0], output[i].s[1]));
    std::cout<<"I'm done for!"<<std::endl;
    return result;
#endif
#if 1
    cl::CommandQueue queue(context,dev);
    cl::make_kernel<cl::Buffer&,cl::Buffer&,cl::Buffer&> find(cl::Kernel(prog,"find"));
//    std::clog<<"Queue and kernel constructed"<<std::endl;
    std::deque<BinaryTriples::Address> result;
    cl_ulong2 *output=new cl_ulong2[requests.size()];
    cl::Buffer bufOutput(context, CL_MEM_WRITE_ONLY, requests.size()*sizeof(cl_ulong2));
//    std::clog<<"Output buffer ready"<<std::endl;
    cl::Buffer bufArgs(context, requests.begin(), requests.end(), true, false);
//    std::clog<<"Args buffer ready"<<std::endl;
    find(cl::EnqueueArgs(queue, cl::NDRange(requests.size())), *bufData, bufArgs, bufOutput);
    queue.finish();
//    std::clog<<"Kernels executed"<<std::endl;
    memset(output, 0, sizeof(cl_ulong2)*requests.size());
    cl::copy(queue, bufOutput, output, output+requests.size());
    queue.finish();
    for(size_t i=0;i<requests.size();++i)
    {
        if(output[i].s[0]!=-1)
            result.push_back(BinaryTriples::Address(output[i].s[0],output[i].s[1]));
    }
    return result;
#endif
#if 0
    size_t resultSize=5*requests.size();
    std::clog<<"Result size is "<<resultSize<<" of items"<<std::endl;
    std::deque<uint32_t> result;
    cl_uint *output=new uint32_t[resultSize];
    cl::Buffer bufOutput(context, CL_MEM_WRITE_ONLY, resultSize*sizeof(cl_uint));
    std::clog<<"Output buffer ready"<<std::endl;
    cl::Buffer bufArgs(context, CL_MEM_READ_ONLY, current.size()*sizeof(FindArgs));
    std::clog<<"Args buffer ready"<<std::endl;
    while(!current.empty())
    {
        size_t block=resultSize/current.size();
        std::clog<<"Starting for "<<current.size()<<" items, block size is "<<block<<std::endl;
        for(size_t i=0;i<current.size();++i)
        {
            current[i].wbegin=i*block;
            current[i].wend=(i+1)*block-1;
        }
        cl::copy(queue, current.begin(), current.end(), bufArgs);
        queue.finish();
        std::clog<<"Copied "<<current.size()<<" arguments to buffer"<<std::endl;
        find(cl::EnqueueArgs(queue, cl::NDRange(current.size())), *bufData, bufArgs, bufOutput);
        queue.finish();
        std::clog<<"Kernels executed"<<std::endl;
        cl::copy(queue, bufOutput, output, output+resultSize);
        queue.finish();
        std::clog<<"Copied "<<resultSize<<" output items"<<std::endl;
        next.clear();
        for(auto& c:current)
        {
            for(size_t j=c.wbegin;j<c.wend;++j)
                if(output[j]!=static_cast<cl_uint>(-1))
                    result.push_back(output[j]);
                else
                    break;
            std::clog<<"\t"<<output[c.wend]<<" ? "<<c.end<<"\n";
            if(output[c.wend]<c.end)
            {
                FindArgs newArg(c);
                newArg.begin=output[c.wend];
                next.push_back(newArg);
            }
        }
        std::swap(current, next);
    }
    delete[] output;
    return result;
#endif
}

void GPU::test()
{
#if 0
    try
    {
    int n=100;
    int m=5;
    uint8_t *input=new uint8_t[n];
    for(int i=0;i<n;++i)
        input[i]=i%10;
    std::deque<Request> requests;
    for(int i=0;i<m;++i)
    {
        requests.push_back(Request(i*n/m,(i+1)*n/m-1,i));
    }
    setData(input, n);
    std::clog<<"Data loaded"<<std::endl;
    auto r=find(requests);
    std::clog<<"Found "<<r.size()<<" items"<<std::endl;
    std::copy(r.begin(), r.end(), std::ostream_iterator<uint32_t>(std::clog, " "));
    std::clog<<std::endl;
    }
    catch(const cl::Error& ex)
    {
        std::cerr<<ex.what()<<" "<<ex.err()<<std::endl;
        throw ex;
    }
#endif
}
