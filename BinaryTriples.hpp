#ifndef BINARYTRIPLESHPP
#define BINARYTRIPLESHPP

#include "Triple.hpp"
#include "Compressor.hpp"
#include "TreePattern.hpp"
#include <cstdint>

class BinaryTriples
{
	public:
        static const std::pair<size_t,size_t> invalid;
		void fill(Triples& triples);
        std::deque<std::string> answer(const TreePattern::Node* query) const;
        bool ask(const TreePattern::Node* query, const BinaryCode& s) const;
	private:
        typedef std::pair<size_t,size_t> Address;
        const uint8_t* find(const uint8_t* where, size_t n, uint32_t value, uint8_t length, bool index) const;
        Address subjectsForPredicate(const BinaryCode& p) const;
        Address objectsForSubject(const Address& a, const BinaryCode& s) const;
        Address objectsForSP(const BinaryCode& s, const BinaryCode& p) const;


        Codes *soCodes,*pCodes;
		//id[1-4 bajty],pozycja w subjects[4 bajty]
		uint8_t *predicates;
		size_t pLen;
		//id[1-4 bajty],pozycja w objects[4 bajty]
		uint8_t *subjects;
		size_t sLen;
		//id[1-4 bajty]
		uint8_t *objects;
		size_t oLen;
};

#endif //BINARYTRIPLESHPP
