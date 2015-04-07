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
	Triples triples;
#if 0
	for(int i=1;i<argc;i++)
	{
		std::cout<<"Loading "<<argv[i]<<"\n";
		LoadTriples(argv[i], triples);
	}
#endif
    LoadDir("lubm", [&triples](const Triple& t)->void {triples.push_back(t);});
	std::cout<<"# of triples"<<triples.size()<<"\n";
	BinaryTriples bt;
	bt.fill(triples);
    query->dump(std::cout);
    auto result=bt.answer(query);
    for(auto i:result)
        std::cout<<i<<"\n";
    std::cout<<"# of rows: "<<result.size()<<std::endl;
	return 0;
}
