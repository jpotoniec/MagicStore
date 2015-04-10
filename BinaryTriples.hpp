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


class BinaryTriple
{
public:
    BinaryTriple(const BinaryCode& s,const BinaryCode& p, const BinaryCode& o)
        :_s(s),_p(p),_o(o)
    {

    }
    const BinaryCode& s() const
    {
        return _s;
    }
    const BinaryCode& p() const
    {
        return _p;
    }
    const BinaryCode& o() const
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
    BinaryCode _s,_p,_o;
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
        bool ask(const TreePattern::Node* query, const BinaryCode& s) const;
        BinaryTriples();
        ~BinaryTriples();
        void save(std::ofstream& f) const;
        void load(std::ifstream& file);
	private:        
        const uint8_t* find(const uint8_t* where, size_t n, uint32_t value, uint8_t length, bool index) const;
        Address level2For1(const BinaryCode& p) const;
        Address level3For2(const Address& a, const BinaryCode& s) const;
        Address level3For12(const BinaryCode& l1, const BinaryCode& l2) const;
        PAbstractIterator iteratorForQuery(const TreePattern::Node* query) const;
        std::deque<BinaryCode> answerCodes(const TreePattern::Node* query) const;
        void add(const BinaryCode& s, const BinaryCode& p, const BinaryCode& o);
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
        BinaryCode prevP,prevO;
};

#endif //BINARYTRIPLESHPP
