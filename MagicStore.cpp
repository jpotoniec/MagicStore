#include "Triple.hpp"
#include "TreePattern.hpp"
#include "BinaryTriples.hpp"
#include <iostream>
#include <map>
#include <queue>
#include <iterator>
#include <cassert>
#include <algorithm>

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

Triples parseSparql(const std::string& sparql)
{
    Triples result;
    std::string t[3];
    int pos=0;
    for(auto i=0;i<sparql.length();++i)
    {
        char c=sparql[i];
        if(isspace(c))
        {
            if(pos<2 && !t[pos].empty())
                pos++;
            continue;
        }
        if(c=='<')
        {
            i++;
            for(;sparql[i]!='>';++i)
                t[pos]+=sparql[i];
            continue;
        }
        if(c=='.' || c==',' || c==';')
        {
            result.push_back(Triple(resolve(t[0]),resolve(t[1]),resolve(t[2])));
            switch(c)
            {
            case '.':
                pos=0;
                break;
            case ';':
                pos=1;
                break;
            case ',':
                pos=2;
                break;
            }
            for(auto j=pos;j<=2;++j)
                t[j]="";
            continue;
        }
        t[pos]+=c;
    }
    return result;
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
        auto query=TreePattern::Node::fromTriples(parseSparql(argv[3]));
        query->dump(std::cout);
        auto result=bt.answer(query);
        if(result.size()<100)
            for(auto i:result)
                std::cout<<i<<"\n";
        std::cout<<"# of rows: "<<result.size()<<std::endl;
    }
    return 0;
}
