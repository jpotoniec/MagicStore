#ifndef QUERYHPP
#define QUERYHPP

#include "Data.hpp"
#include <cassert>

class MaskedValue
{
	public:
		MaskedValue(uint32_t v, uint32_t m)
			:v(v),m(m)
		{
		}
		MaskedValue()
			:v(0),m(0)
		{
		}
		bool Match(uint32_t x) const
		{
			return (x&m)==v;
		}
	private:
		uint32_t v,m;
};

class Query
{
	public:
		const size_t begin() const
		{
			return _begin;
		}
		const size_t end() const
		{
			return _end;
		}
		const MaskedValue& subject() const
		{
			return s;
		}
		const MaskedValue& object() const
		{
			return o;
		}
		bool Match(uint32_t value) const
		{
			return s.Match(value) && o.Match(value);
		}
		static Query Build(const Data& data, const std::string& s, const std::string& p, const std::string& o);
	private:
		size_t _begin;	
		size_t _end;
		MaskedValue s,o;
		Query()
		{
		}
};

#endif //QUERYHPP
