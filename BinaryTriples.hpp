#ifndef BINARYTRIPLESHPP
#define BINARYTRIPLESHPP

#include "Triple.hpp"
#include "Compressor.hpp"
#include "TreePattern.hpp"
#include <cstdint>
#include <memory>
#include <fstream>
#include <boost/noncopyable.hpp>

typedef std::unique_ptr<class AbstractIterator> PAbstractIterator;

inline int compare(uint32_t a, uint32_t b)
{
    if(a<b)
        return -1;
    else if(a>b)
        return 1;
    else
        return 0;
}

class BinaryTriple
{
public:
    BinaryTriple(uint32_t s,uint32_t p, uint32_t o)
        :_s(s),_p(p),_o(o)
    {

    }
    uint32_t s() const
    {
        return _s;
    }
    uint32_t p() const
    {
        return _p;
    }
    uint32_t o() const
    {
        return _o;
    }
    static int compare(const BinaryTriple& a, const BinaryTriple& b)
    {
        int pc=::compare(a.p(),b.p());
        if(pc!=0)
            return pc;
        int oc=::compare(a.o(),b.o());
        if(oc!=0)
            return oc;
        int sc=::compare(a.s(),b.s());
        return sc;
    }
    friend bool operator<(const BinaryTriple& a, const BinaryTriple& b)
    {
        return BinaryTriple::compare(a,b)<0;
    }
private:
    uint32_t _s,_p,_o;
};

typedef std::deque<BinaryTriple> RawBinaryTriples;

class BinaryTriple;

class BinaryTriples : private boost::noncopyable
{
	public:
        typedef std::pair<size_t,size_t> Address;
        static const std::pair<size_t,size_t> invalid;        
        void fill(PCodes soCodes, PCodes pCodes,RawBinaryTriples& triples);
        void merge(const BinaryTriples& a, const BinaryTriples& b);
        std::deque<std::string> answer(const TreePattern::Node* query) const;
        bool ask(const TreePattern::Node* query, uint32_t s) const;
        BinaryTriples();
        ~BinaryTriples();
        void save(std::ofstream& f) const;
        void load(std::ifstream& file);
	private:                
        Address level2For1(uint32_t p) const;
        Address level3For2(const Address& a, uint32_t s) const;
        Address level3For12(uint32_t l1, uint32_t l2) const;
        PAbstractIterator iteratorForQuery(const TreePattern::Node* query) const;
        std::deque<uint32_t> answerCodes(const TreePattern::Node* query) const;
        void add(uint32_t s, uint32_t p, uint32_t o);
        void add(const BinaryTriple& t);
        void finish();
        void dump(const BinaryTriple& t) const;

        friend class TripleIterator;

        PCodes soCodes,pCodes;
		//id[1-4 bajty],pozycja w subjects[4 bajty]
        uint8_t *level1;
        size_t len1;
		//id[1-4 bajty],pozycja w objects[4 bajty]
        uint8_t *level2;
        size_t len2;
		//id[1-4 bajty]
        uint8_t *level3;
        size_t len3;
        uint32_t prevP,prevO;
};

#endif //BINARYTRIPLESHPP
