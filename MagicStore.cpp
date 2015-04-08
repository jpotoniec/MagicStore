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

int main(int argc, char **argv)
{
#if 1
    auto query=TreePattern::Node::fromTriples(parseSparql(std::string()
                                                          +"?x a ub:GraduateStudent;"
                                                          +"   ub:takesCourse <http://www.Department0.University0.edu/GraduateCourse0>;"+
                                                          +"   ub:advisor ?y."
                                                          +"?y a ub:AssociateProfessor;"
                                                          +"   ub:teacherOf ?z."
                                                          +"?z a ub:GraduateCourse."
                                                          ));    
#else
    auto query=TreePattern::Node::fromTriples(parseSparql("?x a ub:GraduateStudent; ub:takesCourse <http://www.Department0.University0.edu/GraduateCourse0>."));
#endif
#if 0
	for(int i=1;i<argc;i++)
	{
		std::cout<<"Loading "<<argv[i]<<"\n";
		LoadTriples(argv[i], triples);
	}
#endif
    PBinaryTriples bt;
#if 1
    PCodes soCodes(new Codes());
    PCodes pCodes(new Codes());
    LoadDir("lubm", [&soCodes,&pCodes](const Triple& t)->void {
        soCodes->inc(t.s());
        pCodes->inc(t.p());
        soCodes->inc(t.o());
    }, "rdfxml");
    soCodes->compress();
    pCodes->compress();
    bt=LoadDir("lubm", soCodes, pCodes, "rdfxml");
    std::ofstream f("lubm.bin");
    bt->save(f);
    f.close();
#else
    bt=new BinaryTriples();
    std::ifstream f("lubm.bin");
    bt->load(f);
    f.close();
#endif
    query->dump(std::cout);
    auto result=bt->answer(query);
    for(auto i:result)
        std::cout<<i<<"\n";
    std::cout<<"# of rows: "<<result.size()<<std::endl;
	return 0;
}
