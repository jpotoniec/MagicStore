#ifndef TRIPLEHPP
#define TRIPLEHPP

#include <string>
#include <ostream>
#include <deque>

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

inline std::ostream& operator<<(std::ostream& o, const Triple& t)
{
	return o<<"(<"<<t.s()<<"> <"<<t.p()<<"> <"<<t.o()<<">)";
}

typedef std::deque<Triple> Triples;

void LoadTriples(const std::string& file, Triples& result, const std::string& format="rdfxml");

#endif //TRIPLEHPP
