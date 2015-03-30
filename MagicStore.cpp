#include <iostream>
#include <raptor2.h>
#include <map>
#include <queue>
#include <iterator>
#include <cassert>
#include <algorithm>

class Triple
{
	public:
		Triple(const std::string& s, const std::string& p, const std::string& o)
			:_s(s),_p(p),_o(o)
		{
		}
		const std::string& s() const
		{
			return _s;
		}
		const std::string& p() const
		{
			return _p;
		}
		const std::string& o() const
		{
			return _o;
		}
	private:
		std::string _s,_p,_o;
};

std::ostream& operator<<(std::ostream& o, const Triple& t)
{
	return o<<"(<"<<t.s()<<"> <"<<t.p()<<"> <"<<t.o()<<">)";
}

struct PComparator
{
	bool operator()(const Triple& a, const Triple& b)
	{
		return a.p().compare(b.p())<0;
	}
};

typedef std::map<std::string,int> Stats;
typedef std::deque<Triple> Triples;
Stats stats;

static void print_triple(void* user_data, raptor_statement* triple) 
{
	Triples &triples(*static_cast<Triples*>(user_data));
	if(triple->object->type!=RAPTOR_TERM_TYPE_URI)
		return;
	std::string s=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->subject->value.uri, NULL)));
	std::string p=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->predicate->value.uri, NULL)));
	std::string o=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->object->value.uri, NULL)));
	triples.push_back(Triple(s,p,o));
}

void load(const std::string& file, Triples& result)
{
	raptor_world *world = NULL;
	raptor_parser* rdf_parser = NULL;
	unsigned char *uri_string;
	raptor_uri *uri, *base_uri;

	world = raptor_new_world();

	rdf_parser = raptor_new_parser(world, "rdfxml");

	raptor_parser_set_statement_handler(rdf_parser, &result, print_triple);

	uri_string = raptor_uri_filename_to_uri_string(file.c_str());
	uri = raptor_new_uri(world, uri_string);
	base_uri = raptor_uri_copy(uri);

	raptor_parser_parse_file(rdf_parser, uri, base_uri);

	raptor_free_parser(rdf_parser);

	raptor_free_uri(base_uri);
	raptor_free_uri(uri);
	raptor_free_memory(uri_string);

	raptor_free_world(world);
}

class Node
{
	public:
		Node(const std::string& label, int occurences,const Node *left=NULL,const Node *right=NULL)
			:label(label), occurences(occurences),left(left),right(right)
		{
		}
		const std::string& getLabel() const
		{
			return label;
		}
		int getOccurences() const
		{
			return occurences;
		}
		const Node* getLeft() const
		{
			return left;
		}
		const Node* getRight() const
		{
			return right;
		}
	private:
		std::string label;
		int occurences;
		const Node *left,*right;
};

class NodesOrder
{
	public:
		bool operator()(const Node* a, const Node* b) const
		{
			return a->getOccurences()>b->getOccurences();
		}
};

typedef std::vector<bool> Code;
typedef std::map<std::string,Code> Codes;

void MakeCode(const Node *root, const Code& prefix, Codes& result)
{
	if(root!=NULL)
	{
		if(!root->getLabel().empty())
			result[root->getLabel()]=prefix;
		Code lcode(prefix);
		lcode.push_back(false);
		Code rcode(prefix);
		rcode.push_back(true);
		MakeCode(root->getLeft(),lcode,result);
		MakeCode(root->getRight(),rcode,result);
	}
}

void dump(const Node *root, const std::string& prefix)
{
	if(root!=NULL)
	{
		std::cout<<prefix<<" "<<root->getLabel()<<"\n";
		dump(root->getLeft(), prefix+"0");
		dump(root->getRight(), prefix+"1");
	}
}

Codes Compress(const std::map<std::string,int>& stats)
{
	Codes result;
	std::priority_queue<Node*,std::deque<Node*>,NodesOrder> nodes;
	for(auto i:stats)
		nodes.push(new Node(i.first, i.second));
	Node *root;
	for(;;)
	{
		Node *l=nodes.top();
		nodes.pop();
		if(nodes.empty())
		{
			root=l;
			break;
		}
		Node *r=nodes.top();
		nodes.pop();
		nodes.push(new Node("",l->getOccurences()+r->getOccurences(),l,r));
	}
	MakeCode(root, Code(), result);
	return result;
}

template<typename T>
uint64_t convert(const T& begin, const T& end)
{
	uint64_t result=0;
	for(auto i=begin;i!=end;++i)
	{
		result<<=1;
		if(*i)
			result|=1;
	}
	return result;

}

uint64_t convert(const Code& c, bool reversed=false)
{
	if(reversed)
		return convert(c.rbegin(), c.rend());
	else
		return convert(c.begin(), c.end());
}

typedef std::map<std::string,size_t> Deltas;
struct Data
{
	Codes codes;
	Deltas deltas;
	uint32_t *data;
	size_t n;
};

Data Compress(Triples& triples)
{
	Stats soStats;
	for(auto t:triples)
	{
		soStats[t.s()]++;
		soStats[t.o()]++;
	}
	Codes soCodes=Compress(soStats);
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
		Code &sc(soCodes[i.s()]);
		Code &oc(soCodes[i.o()]);
		int len=sc.size()+oc.size();
		assert(len<=32);	//TODO: przywrócić support dla dłuższych wyrażeń
		Code c;
#if 0
		if(len<=32)
			c.push_back(false);
		else
			c.push_back(true);
#endif
		c.reserve(32);
		c.insert(c.end(), sc.begin(), sc.end());
		c.insert(c.end(), 32-len, false);
		c.insert(c.end(), oc.rbegin(), oc.rend());
		uint64_t code=convert(c);
		data[n++]=static_cast<uint32_t>(code);
	}
	for(auto i:deltas)
		std::cout<<i.first<<" -> "<<i.second<<"\n";
	std::cout<<"n="<<n<<" ("<<(4*n/1024)<<" KB)\n";
	Data result;
	result.codes=soCodes;
	result.deltas=deltas;
	result.data=data;
	result.n=n;
	return result;
}

const std::string ub="http://swat.cse.lehigh.edu/onto/univ-bench.owl#";
const std::string rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#";
const std::string a=rdf+"type";

struct Query
{
	size_t begin;	
	size_t end;
	uint32_t value;
	uint32_t mask;
	Query()
	:begin(0),end(0),value(0),mask(0)
	{
	}
};

std::ostream& operator<<(std::ostream& o, const Code& c)
{
	for(auto i:c)
		o<<(i?"1":"0");
	return o;
}

Query MakeQuery(Data& data, const std::string& s, const std::string& p, const std::string& o)
{
	Query result;
	Deltas::const_iterator i=data.deltas.find(p),j;
	assert(i!=data.deltas.end());
	result.begin=i->second;
	j=i;
	j++;
	if(j==data.deltas.end())
		result.end=data.n;
	else
		result.end=j->second;
	std::cout<<i->first<<" "<<j->first<<"\n";
	if(!s.empty())
	{
		Code c=data.codes[s];
		uint32_t mask=((1<<c.size())-1)<<(32-c.size());
		uint32_t value=convert(c)<<(32-c.size());
		result.mask|=mask;
		result.value|=value;
		std::cout<<s<<" -> "<<c<<" "<<std::hex<<value<<"/"<<mask<<std::dec<<"\n";
	}
	if(!o.empty())
	{
		Code c=data.codes[o];
		uint32_t mask=(1<<c.size())-1;
		uint32_t value=convert(c, true);
		result.mask|=mask;
		result.value|=value;
		std::cout<<o<<" -> "<<c<<" "<<std::hex<<value<<"/"<<mask<<std::dec<<"\n";
	}
	std::cout<<std::hex<<result.value<<"/"<<result.mask<<std::dec<<" in range ["<<result.begin<<","<<result.end<<")\n";
	return result;
}

void Search(const Data& data, const Query& query, std::deque<uint32_t>& triples)
{
	std::cout<<std::hex<<query.value<<"/"<<query.mask<<std::dec<<" in range ["<<query.begin<<","<<query.end<<")\n";
	for(size_t i=query.begin;i<query.end;++i)
		if((data.data[i]&query.mask)==query.value)
		{
			triples.push_back(data.data[i]);
			std::cout<<"\tFound at position "<<i<<": "<<std::hex<<data.data[i]<<std::dec<<"\n";
		}
}

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

Triple decode(uint32_t triple, const Data& data)
{
	std::cout<<"Decoding "<<std::hex<<triple<<std::dec<<"\n";
	std::string s,o;
	for(Codes::const_iterator i=data.codes.begin();i!=data.codes.end();++i)
	{
		Code c=i->second;
		uint32_t v=convert(c);
		size_t len=c.size();
		uint32_t mask=(1<<len)-1;
		if(((triple>>(32-len))&mask)==v)
		{
			std::cout<<"Found subject with code "<<std::hex<<v<<std::dec<<" of length "<<len<<"\n";
			assert(s.empty());
			s=i->first;
			//TODO: zasadniczo tu powinien być break, ale nie chcę tego robić dopóki nie będę w miarę pewien, że assert się nie odpala
		}
	}
	for(Codes::const_iterator i=data.codes.begin();i!=data.codes.end();++i)
	{
		Code c=i->second;
		size_t len=c.size();
		uint32_t v=convert(c,true);
		uint32_t mask=(1<<len)-1;
		if((triple&mask)==v)
		{
			std::cout<<"Found object with code "<<std::hex<<v<<std::dec<<" of length "<<len<<"\n";
			assert(o.empty());
			o=i->first;
		}
	}
	return Triple(s,"",o);
}

int main(int argc, char **argv)
{
	Triples triples;
	for(int i=1;i<argc;i++)
	{
		std::cout<<"Loading "<<argv[i]<<"\n";
		load(argv[i], triples);
	}
	std::cout<<"# of triples"<<triples.size()<<"\n";
	Data data=Compress(triples);
	Query q1=MakeQuery(data,"",a,ub+"GraduateStudent");
	Query q2=MakeQuery(data,"", ub+"takesCourse", "http://www.Department0.University0.edu/GraduateCourse0");
	std::deque<uint32_t> result;
	Search(data, q1, q2, result);
	std::cout<<"Found "<<result.size()<<" triples\n";
	for(uint32_t i:result)
		std::cout<<decode(i, data)<<"\n";
	return 0;
}
