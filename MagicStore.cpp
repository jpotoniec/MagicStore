#include "Query.hpp"
#include "Triple.hpp"
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
		return a.p().compare(b.p())<0;
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
			processor(i, data.data[i]);
			std::cout<<"\tFound at position "<<i<<": "<<std::hex<<data.data[i]<<std::dec<<"\n";
		}

}

void Search(const Data& data, const Query& query, std::deque<uint32_t>& triples)
{
	Search(data, query, [&triples](uint32_t i,uint32_t val) {triples.push_back(val);});
}

#if 0
void Search(const Data& data, const Query& q1, const Query& q2, std::deque<uint32_t>& result)
{
	std::deque<uint32_t> triples;
	Search(data, q1, triples);
	for(auto t:triples)
	{
		Query q(q2);
		q.mask=std::numeric_limits<uint32_t>::max();
		q.value=(t&(~std::max(q1.mask, q2.mask)))|(q2.value);
		Search(data, q, result);
	}
}
#endif

Triple decode(uint32_t triple, const Data& data)
{
	std::string s=data.codes->DecodeLabelLeft(triple);
	std::string o=data.codes->DecodeLabelRight(triple);
	return Triple(s,"",o);
}

int main(int argc, char **argv)
{
	Triples triples;
	for(int i=1;i<argc;i++)
	{
		std::cout<<"Loading "<<argv[i]<<"\n";
		LoadTriples(argv[i], triples);
	}
	std::cout<<"# of triples"<<triples.size()<<"\n";
	Data data=Compress(triples);
	Query q1=Query::Build(data,"",a,ub+"GraduateStudent");
	Query q2=Query::Build(data,"", ub+"takesCourse", "http://www.Department0.University0.edu/GraduateCourse0");
	std::deque<uint32_t> result;
	Search(data, q1, /*q2,*/ result);
	std::cout<<"Found "<<result.size()<<" triples\n";
	if(result.size()<50)
		for(uint32_t i:result)
			std::cout<<decode(i, data)<<"\n";
	return 0;
}
