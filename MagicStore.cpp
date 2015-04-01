#include "Query.hpp"
#include "Triple.hpp"
#include "TreePattern.hpp"
#include "BinaryTriples.hpp"
#include <iostream>
#include <map>
#include <queue>
#include <iterator>
#include <cassert>
#include <algorithm>

typedef std::map<std::string,int> Stats;
Stats stats;

struct PComparator
{
	bool operator()(const Triple& a, const Triple& b)
	{
		int c=a.p().compare(b.p());
		if(c!=0)
			return c<0;
		c=a.s().compare(b.s());
		if(c!=0)
			return c<0;
		c=a.o().compare(b.o());
		return c<0;
	}
};

Data Compress(Triples& triples)
{
	Stats stats;
	for(auto t:triples)
	{
		stats[t.s()]++;
		stats[t.o()]++;
	}
	Codes* codes=new Codes(stats);
#if 0
	for(auto i:soCodes)
		std::cout<<i.first<<" "<<i.second<<"\n";
#endif
	uint32_t *data=new uint32_t[2*triples.size()];
	size_t n=0;
	std::sort(triples.begin(), triples.end(), PComparator());
	std::string prevP="";
	Deltas deltas;
	for(auto i:triples)
	{
		if(i.p()!=prevP)
		{
			prevP=i.p();
			deltas[prevP]=n;
		}
		const BinaryCode &s((*codes)[i.s()]);
		const BinaryCode &o((*codes)[i.o()]);
		assert(s.length()+o.length()<=32);
		data[n++]=s.leftNormal()|o.rightReversed();
	}
	for(auto i:deltas)
		std::cout<<i.first<<" -> "<<i.second<<"\n";
	std::cout<<"n="<<n<<" ("<<(4*n/1024)<<" KB)\n";
	Data result;
	result.codes=codes;
	result.deltas=deltas;
	result.data=data;
	result.n=n;
	return result;
}



const std::string ub="http://swat.cse.lehigh.edu/onto/univ-bench.owl#";
const std::string rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#";
const std::string a=rdf+"type";

template<typename T>
void Search(const Data& data, const Query& query, const T& processor)
{
	for(size_t i=query.begin();i<query.end();++i)
		if(query.Match(data.data[i]))
		{
			std::cout<<"\tFound at position "<<i<<": "<<std::hex<<data.data[i]<<std::dec<<"\n";
			if(!processor(i, data.data[i]))
				return;
		}

}

void Search(const Data& data, const Query& query, std::deque<uint32_t>& triples)
{
	Search(data, query, [&triples](uint32_t i,uint32_t val) {triples.push_back(val); return true;});
}

bool Ask(const Data& data, const Query& query)
{
	bool result=false;
	Search(data, query, [&result](uint32_t i,uint32_t val) {result=true; return false;});
	return result;
}

void Search(const Data& data, const Query& q1, const Query& q2, std::deque<std::pair<uint32_t,uint32_t>>& result)
{
	std::deque<uint32_t> triples;
	Search(data, q1, triples);
	Query q(q2);
	assert(q1.subject().IsValid() || q1.object().IsValid());
	bool decodeSubject=q1.object().IsValid();
	bool encodeSubject=!q2.subject().IsValid();
	std::cout<<"In the loop, will decode "<<(decodeSubject?"subject":"object")<<" and encode "<<(encodeSubject?"subject":"object")<<"\n";
	for(auto t:triples)
	{
		BinaryCode c;
		if(decodeSubject)
			c=data.codes->DecodeLeft(t);
		else
			c=data.codes->DecodeRight(t);
		if(encodeSubject)
			q.subject(c);
		else
			q.object(c);
		Search(data, q, [&result, t](uint32_t i, uint32_t val) {result.push_back(std::make_pair(t, val)); return true;});
	}
}

Triple decode(uint32_t triple, const Data& data)
{
	std::string s=data.codes->DecodeLabelLeft(triple);
	std::string o=data.codes->DecodeLabelRight(triple);
	return Triple(s,"",o);
}

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

#include <dirent.h>
void LoadDir(const std::string& dirname, Triples& triples)
{
    DIR *d=opendir(dirname.c_str());
    dirent *de;
    while((de=readdir(d))!=NULL)
    {
        std::string file(de->d_name);
        file=dirname+"/"+file;
        if(file.find(".owl")==file.length()-4)
            LoadTriples(file, triples);
    }
    closedir(d);
}

int main(int argc, char **argv)
{
#if 1
	auto query=TreePattern::Node::fromTriples(
		Triple("?x",a,ub+"GraduateStudent")
		+Triple("?x",ub+"takesCourse", "http://www.Department0.University0.edu/GraduateCourse0")
		+Triple("?x",ub+"advisor", "?y")
                +Triple("?y",a,ub+"AssociateProfessor")
        +Triple("?y",ub+"teacherOf", "?z")
        +Triple("?z",a, ub+"GraduateCourse"));
	query->dump(std::cout);
#endif
#if 0
    auto query=TreePattern::Node::fromTriples(Triples()+Triple("?x",a,ub+"AssociateProfessor"));
#endif
	Triples triples;
#if 0
	for(int i=1;i<argc;i++)
	{
		std::cout<<"Loading "<<argv[i]<<"\n";
		LoadTriples(argv[i], triples);
	}
#endif
    LoadDir("lubm", triples);
	std::cout<<"# of triples"<<triples.size()<<"\n";
	BinaryTriples bt;
	bt.fill(triples);
#if 0
    auto query=TreePattern::Node::fromTriples(
        Triple("?x",a,ub+"GraduateStudent")
        +Triple("?x",ub+"takesCourse", "http://www.Department0.University0.edu/GraduateCourse0")
        );
#endif
    query->dump(std::cout);
    auto result=bt.answer(query);
    for(auto i:result)
        std::cout<<i<<"\n";
#if 0
	Data data=Compress(triples);
	Query q1=Query::Build(data,"",a,ub+"GraduateStudent");
	Query q2=Query::Build(data,"", ub+"takesCourse", "http://www.Department0.University0.edu/GraduateCourse0");
	Query q3=Query::Build(data,"", ub+"takesCourse", "");
	std::deque<std::pair<uint32_t,uint32_t>> result;
	Search(data, q1, q2, result);
	std::cout<<"Found "<<result.size()<<" triples\n";
//	if(result.size()<50)
		for(auto i:result)
			std::cout<<decode(i.first, data)<<" | "<<decode(i.second, data)<<"\n";
#endif
	return 0;
}
