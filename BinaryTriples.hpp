#ifndef BINARYTRIPLESHPP
#define BINARYTRIPLESHPP

#include "Triple.hpp"
#include "Compressor.hpp"
#include "TreePattern.hpp"
#include <cstdint>
#include <memory>
#include <fstream>

typedef std::unique_ptr<class AbstractIterator> PAbstractIterator;

class BinaryTriples
{
	public:
        typedef std::pair<size_t,size_t> Address;
        static const std::pair<size_t,size_t> invalid;
		void fill(Triples& triples);
        std::deque<std::string> answer(const TreePattern::Node* query) const;
        bool ask(const TreePattern::Node* query, const BinaryCode& s) const;
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


        Codes soCodes,pCodes;
		//id[1-4 bajty],pozycja w subjects[4 bajty]
        uint8_t *level1;
        size_t len1;
		//id[1-4 bajty],pozycja w objects[4 bajty]
        uint8_t *level2;
        size_t len2;
		//id[1-4 bajty]
        uint8_t *level3;
        size_t len3;
};

#endif //BINARYTRIPLESHPP
