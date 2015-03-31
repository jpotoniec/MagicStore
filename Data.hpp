#ifndef DATAHPP
#define DATAHPP

#include "Compressor.hpp"
#include <cstdint>
#include <cstdlib>
#include <map>

typedef std::map<std::string,size_t> Deltas;

struct Data
{
	Codes* codes;
	Deltas deltas;
	uint32_t *data;
	size_t n;
};

#if 0
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

inline uint64_t convert(const Code& c, bool reversed=false)
{
	if(reversed)
		return convert(c.rbegin(), c.rend());
	else
		return convert(c.begin(), c.end());
}
#endif


#endif //DATAHPP
