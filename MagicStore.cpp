#include "Triple.hpp"
#include "TreePattern.hpp"
#include "BinaryTriples.hpp"
#include "Timer.hpp"
#include <iostream>
#include <map>
#include <queue>
#include <iterator>
#include <cassert>
#include <algorithm>
#include <sstream>

const std::string ub="http://swat.cse.lehigh.edu/onto/univ-bench.owl#";
const std::string rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#";
const std::string a=rdf+"type";

Triples operator+(Triples t, const Triple& x)
{
	t.push_back(x);
	return t;
}

Triples operator+(const Triple& x, const Triple& y)
{
	Triples t;
	t.push_back(x);
	t.push_back(y);
	return t;
}

std::string resolve(const std::string& input)
{
    std::map<std::string,std::string> prefixes;
    if(input[0]=='?')
        return input;
    if(input=="a")
        return rdf+"type";
    prefixes["ub:"]=ub;
    for(auto p:prefixes)
        if(input.find(p.first)==0)
            return p.second+input.substr(p.first.length());
    return input;
}

typedef std::unique_ptr<BinaryTriples> PBinaryTriples;

#if 0
PBinaryTriples LoadDir(const std::string& dirname, PCodes soCodes, PCodes pCodes, const std::string& format)
{
    DIR *d=opendir(dirname.c_str());
    dirent *de;
    PBinaryTriples result;
    while((de=readdir(d))!=NULL)
    {
        std::string file(de->d_name);
        file=dirname+"/"+file;
        if(file.find(".owl")==file.length()-4)
        {
            Triples triples;
            std::cout<<"Loading "<<file<<std::endl;
            LoadTriples(file, [&triples](const Triple& t)->void {triples.push_back(t);}, format);
            if(result!=NULL)
            {
                BinaryTriples bt;
                bt.fill(soCodes,pCodes,triples);
                PBinaryTriples x(new BinaryTriples());
                x->merge(*result, bt);
                result=std::move(x);
            }
            else
            {
                result=PBinaryTriples(new BinaryTriples());
                result->fill(soCodes,pCodes,triples);
            }
        }
    }
    closedir(d);
    return result;
}
#endif

class Adder
{
public:
    void operator()(int x)
    {
//        std::cout<<x<<"\n";
        if(i==end)
            throw std::runtime_error("Did not expect anything, got "+std::to_string(x));
        if(x!=*i)
            throw std::runtime_error("Expected "+std::to_string(*i)+", got "+std::to_string(x));
        i++;
    }
    explicit Adder(const std::deque<int>& c)
        :i(c.cbegin()),end(c.cend())
    {

    }
    ~Adder()
    {
        if(i!=end)
            throw std::runtime_error("Finished before "+std::to_string(*i));
    }
private:
    std::deque<int>::const_iterator i,end;
};

#include "Merger.hpp"
void testMerge()
{
    std::deque<int>
            a({1,2,3}),
            b({4,5,6}),
            c({1,2,3,4,5,6}),
            d({1,3,5}),
            e({2,4,6}),
            f({2,4,5,6}),
            g({4,6}),
            h({1,2,3,4,5});
    auto comparator=[](int x,int y)->int{return x-y;};
    merge<int>(JavaIterator<int>(a),
          JavaIterator<int>(b),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(b),
          JavaIterator<int>(a),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(a),
          JavaIterator<int>(c),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(c),
          JavaIterator<int>(a),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(d),
          JavaIterator<int>(e),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(e),
          JavaIterator<int>(d),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(a),
          JavaIterator<int>(a),
          Adder(a),
          comparator);
    merge<int>(JavaIterator<int>(a),
          JavaIterator<int>(f),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(f),
          JavaIterator<int>(a),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(c),
          JavaIterator<int>(g),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(g),
          JavaIterator<int>(c),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(h),
          JavaIterator<int>(g),
          Adder(c),
          comparator);
    merge<int>(JavaIterator<int>(g),
          JavaIterator<int>(h),
          Adder(c),
          comparator);
}

class Loader
{
public:
    Loader(PCodes soCodes,PCodes pCodes)
        :soCodes(soCodes),pCodes(pCodes)
    {

    }

    void operator()(const Triple& t)
    {
        triples.push_back(BinaryTriple((*soCodes)[t.s()], (*pCodes)[t.p()], (*soCodes)[t.o()]));
        if(triples.size()>1e6)
            merge();
    }
    PBinaryTriples result()
    {
        merge();
        return std::move(res);
    }

private:
    void merge()
    {
        if(triples.empty())
            return;
        std::cout<<"Merge of "<<triples.size()<<" triples ";
        std::cout.flush();
        if(res!=NULL)
        {
            BinaryTriples bt;
            bt.fill(soCodes,pCodes,triples);
            PBinaryTriples x(new BinaryTriples());
            x->merge(*res, bt);
            res=std::move(x);
        }
        else
        {
            res=PBinaryTriples(new BinaryTriples());
            res->fill(soCodes,pCodes,triples);
        }
        triples.clear();
        std::cout<<"done"<<std::endl;
    }
    PBinaryTriples res;
    RawBinaryTriples triples;
    PCodes soCodes,pCodes;
};

class Coder
{
public:
    void operator()(const Triple& t)
    {
        soCodes->inc(t.s());
        pCodes->inc(t.p());
        soCodes->inc(t.o());
    }
    Coder()
        :soCodes(new Codes()),pCodes(new Codes())
    {

    }
    PCodes soCodes;
    PCodes pCodes;
};

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char */*url*/,
                          const char */*method*/, const char */*version*/,
                          const char */*upload_data*/,
                          size_t */*upload_data_size*/, void **/*con_cls*/)
{
    std::string page;
    try
    {
        BinaryTriples& bt(*reinterpret_cast<BinaryTriples*>(cls));
        auto query=TreePattern::Query::fromSPARQL(MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND,"query"));

        std::ostringstream s;
        std::string var=query.variable();
        s<<"<?xml version=\"1.0\"?>\n"
            <<"<sparql xmlns=\"http://www.w3.org/2005/sparql-results#\">\n"
            <<"<head>\n"
            <<"<variable name=\""<<var<<"\"/>\n"
            <<"</head>\n"
           <<"<results>\n";
        if(query.isCount())
        {
            s<<"<result><binding name=\""<<var<<"\">"
            <<"<literal datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"
            <<bt.count(query.where())
            <<"</literal></binding></result>\n";
        }
        else
        {
            std::deque<std::string> answer = bt.answer(query.where());
            for(auto& a:answer)
                s<<"<result><binding name=\""<<var<<"\"><uri>"<<a<<"</uri></binding></result>\n";
        }
        s<<"</results>\n</sparql>\n";
        page=s.str();
    }
    catch(const std::exception& e)
    {
        //TODO: MalformedQuery and QueryRequestRefused, are bound to HTTP status codes 400 Bad Request and 500 Internal Server Error, respectively [HTTP].
        page=e.what();
    }

    struct MHD_Response *response;
    int ret;

    response = MHD_create_response_from_buffer (page.length(),
                                                (void*) page.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "application/sparql-results+xml");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
}

void listen(BinaryTriples& bt)
{
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, 12345, NULL, NULL,
                               &answer_to_connection, &bt, MHD_OPTION_END);
    if (NULL == daemon)
        return;
    for(;;)
        sleep(1000);
}

int main(int argc, char **argv)
{
    if(argc==1)
    {
        return 1;
    }
    if(argv[1][0]=='c')
    {
        std::string format(argv[3]);
        std::cout<<"Generating codes to file "<<argv[2]<<", the input format is "<<format<<"\n";
        auto coder=Coder();
        for(int i=4;i<argc;++i)
        {
            std::cout<<"Generating codes from "<<argv[i]<<"\n";
            LoadTriples(argv[i], coder, format);
        }
        PCodes soCodes=coder.soCodes;
        PCodes pCodes=coder.pCodes;
        soCodes->compress();
        pCodes->compress();
        std::ofstream f(argv[2]);
        soCodes->save(f);
        pCodes->save(f);
        f.close();
    }
    if(argv[1][0]=='l')
    {
        std::cout<<"Codes in "<<argv[2]<<"\n";
        PCodes soCodes(new Codes());
        PCodes pCodes(new Codes());
        {
            std::ifstream f(argv[2]);
            soCodes->load(f);
            pCodes->load(f);
            f.close();
        }
        std::string format(argv[4]);
        std::cout<<"Loading to file "<<argv[3]<<", the input format is "<<format<<"\n";
        Loader loader(soCodes,pCodes);
        for(int i=5;i<argc;++i)
        {
            std::cout<<"Doing magic about "<<argv[i]<<"\n";
            LoadTriples(argv[i], loader, format);
        }
        PBinaryTriples bt=loader.result();
        std::ofstream f(argv[3]);
        bt->save(f);
        f.close();
    }
    if(argv[1][0]=='q')
    {
        std::cout<<"Querying dataset "<<argv[2]<<"\n";
        std::ifstream f(argv[2]);
        BinaryTriples bt;
        bt.load(f);
        f.close();
        listen(bt);
    }
    return 0;
}
