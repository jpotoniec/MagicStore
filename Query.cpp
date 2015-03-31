#include "Query.hpp"

Query Query::Build(const Data& data, const std::string& s, const std::string& p, const std::string& o)
{
	Query result;
	Deltas::const_iterator i=data.deltas.find(p),j;
	assert(i!=data.deltas.end());
	result._begin=i->second;
	j=i;
	j++;
	if(j==data.deltas.end())
		result._end=data.n;
	else
		result._end=j->second;
	if(!s.empty())
	{
		BinaryCode c=(*data.codes)[s];
		result.s=MaskedValue(c.leftNormal(), c.maskLeft());
	}
	if(!o.empty())
	{
		BinaryCode c=(*data.codes)[o];
		result.o=MaskedValue(c.rightReversed(), c.maskRight());
	}
	return result;
}

