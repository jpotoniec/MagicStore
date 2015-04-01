#include "BinaryTriples.hpp"
#include "Compressor.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

typedef std::map<std::string,int> Stats;

struct TriplesComparator
{
	bool operator()(const Triple& a, const Triple& b)
	{
		int c=a.p().compare(b.p());
		if(c!=0)
			return c<0;
		c=a.s().compare(b.s());
		if(c!=0)
			return c<0;
		c=a.o().compare(b.o());
		return c<0;
	}
};


void write(uint8_t *where, size_t &wlen, const BinaryCode& c)
{
	assert(c.length()<=30);
    size_t len=(c.length()+2)/8;
    //std::cout<<"For "<<static_cast<uint32_t>(c.length())<<" needed is "<<(len+1)<<" bytes\n";
	uint32_t val=c.value();
	switch(len)
	{
		case 3:
			where[wlen+3]=static_cast<uint8_t>((val>>22)&0xff);
		case 2:
			where[wlen+2]=static_cast<uint8_t>((val>>14)&0xff);
		case 1:
			where[wlen+1]=static_cast<uint8_t>((val>>6)&0xff);
		case 0:
            where[wlen+0]=static_cast<uint8_t>((val>>0)&0x3f)|(static_cast<uint8_t>(len)<<6);
    }
	wlen+=len+1;
}

void skip(const uint8_t *where, size_t &position)
{
    uint8_t len=(((where[position])>>6)&0b11);
    position+=len+1;
}

BinaryCode read(const uint8_t *where, size_t &position)
{
    uint8_t len=(((where[position])>>6)&0b11);
	uint32_t result=0;
	switch(len)
	{
		case 3:
			result|=static_cast<uint32_t>(where[position+3])<<22;
		case 2:
			result|=static_cast<uint32_t>(where[position+2])<<14;
		case 1:
			result|=static_cast<uint32_t>(where[position+1])<<6;
		case 0:
			result|=static_cast<uint32_t>(where[position+0]&0x3f)<<0;
	}
	position+=len+1;
    return BinaryCode(result, 8*(len+1)-2);
}

const std::pair<size_t,size_t> BinaryTriples::invalid=std::pair<size_t,size_t>(static_cast<size_t>(-1),static_cast<size_t>(-1));

const uint8_t* BinaryTriples::find(const uint8_t* where, size_t n, uint32_t value, uint8_t length, bool index) const
{
    for(size_t i=0;i<n;i+=sizeof(size_t))
    {
        BinaryCode c=read(where, i);
        if(c.value()==value)
            return where+i;
        if(index)
        {
            size_t pos;
            memcpy(&pos, where+i, sizeof(size_t));
        }
    }
#if 0
#error Przedwczesna optymalizacja AKA nie działa
    uint8_t len=(length-2)/8;
    for(size_t i=0;i<n;)
    {
        uint8_t x=(where[i]>>6);
        if(x==len && read(where,i).value()==value)
            return where+i;
        else
            i+=x;
        if(index)
            i+=sizeof(size_t);
    }
#endif
    return NULL;
}

BinaryTriples::Address BinaryTriples::subjectsForPredicate(const BinaryCode& p) const
{
    const uint8_t *ptr=find(predicates, pLen, p.value(), p.length(), true);
    if(ptr!=NULL)
    {
        size_t begin=*reinterpret_cast<const size_t*>(ptr);
        ptr+=sizeof(size_t);
        size_t end=sLen;
        if(ptr-predicates<pLen)
        {
            size_t d=0;
            read(ptr,d);
            end=*reinterpret_cast<const size_t*>(ptr+d);
        }
        return std::make_pair(begin,end);
    }
    else
        return invalid;
}

BinaryTriples::Address BinaryTriples::objectsForSubject(const Address& a, const BinaryCode& s) const
{
    size_t len=a.second-a.first;
    const uint8_t *ptr=find(subjects+a.first, len, s.value(), s.length(), true);
    if(ptr!=NULL)
    {
        size_t begin=*reinterpret_cast<const size_t*>(ptr);
        ptr+=sizeof(size_t);
        assert(ptr-subjects<sLen);
        size_t d=0;
        skip(ptr,d);
        size_t end=*reinterpret_cast<const size_t*>(ptr+d);
        return std::make_pair(begin,end);
    }
    else
        return invalid;
}

BinaryTriples::Address BinaryTriples::objectsForSP(const BinaryCode& s, const BinaryCode& p) const
{
    Address sa=subjectsForPredicate(p);
    if(sa!=invalid)
        return objectsForSubject(sa, s);
    return invalid;
}

void BinaryTriples::fill(Triples& triples)
{
	Stats soStats,pStats;    
	std::sort(triples.begin(), triples.end(), TriplesComparator());    
	for(auto t:triples)
	{
		soStats[t.s()]++;
		pStats[t.p()]++;
		soStats[t.o()]++;
	}
    soCodes=new Codes(soStats);
    pCodes=new Codes(pStats);
    predicates=new uint8_t[(4+sizeof(size_t))*pStats.size()];
    memset(predicates, 0, (4+sizeof(size_t))*pStats.size());
	pLen=0;
    subjects=new uint8_t[(4+sizeof(size_t))*pStats.size()*soStats.size()];
	sLen=0;
    objects=new uint8_t[4*triples.size()];
	oLen=0;
	BinaryCode prevP,prevS;
	for(auto t:triples)
	{
		BinaryCode s((*soCodes)[t.s()]);
		BinaryCode p((*pCodes)[t.p()]);
		BinaryCode o((*soCodes)[t.o()]);
		if(prevP!=p)
		{
            std::cout<<t.p()<<" "<<p<<"\n";
			prevP=p;
			write(predicates, pLen, p);            
            memcpy(predicates+pLen, &sLen, sizeof(sLen));
			pLen+=sizeof(sLen);
            std::cout<<"Written "<<static_cast<uint32_t>(p.length())<<"+32 bits, pLen="<<pLen<<"\n";
			prevS=BinaryCode();
		}
		if(prevS!=s)
		{
			prevS=s;
			write(subjects, sLen, s);
            memcpy(subjects+sLen, &oLen, sizeof(oLen));
			sLen+=sizeof(oLen);
		}
		write(objects, oLen, o);
    }
    write(predicates, pLen, BinaryCode(0xffffffff, 30));
    memcpy(predicates+pLen, &sLen, sizeof(sLen));
    write(subjects, sLen, BinaryCode(0xffffffff, 30));
    memcpy(subjects+sLen, &oLen, sizeof(oLen));
	std::cout<<"pLen="<<pLen<<" sLen="<<sLen<<" oLen="<<oLen<<"\n";
}

bool BinaryTriples::ask(const TreePattern::Node* query, const BinaryCode& s) const
{
    for(auto child:query->children())
    {
        auto p=(*pCodes)[child.first];
        std::cout<<soCodes->decode(s)<<" "<<pCodes->decode(p)<<"\n";
        auto addr=objectsForSP(s, p);
        if(child.second->isDefined())
        {
            BinaryCode o=(*soCodes)[child.second->label()];
            std::cout<<"  Looking for "<<soCodes->decode(o)<<" in "<<addr.first<<","<<addr.second<<"\n";
            auto ptr=find(objects+addr.first,addr.second-addr.first,o.value(), o.length(), false);
            std::cout<<"  "<<(ptr?"OK":"FAIL")<<"\n";
            if(ptr==NULL)
                return false;
        }
        else
        {
            bool b=false;
            for(size_t a=addr.first;a<addr.second;)
            {
                BinaryCode o=read(objects, a);
                if(ask(child.second, o))
                {
                    b=true;
                    break;
                }
            }
            if(!b)
                return false;
        }
    }
    return true;
}

std::deque<std::string> BinaryTriples::answer(const TreePattern::Node* query) const
{
#if 0
    for(size_t i=0;i<pLen;i+=sizeof(size_t))
    {
        BinaryCode c=read(predicates, i);
        size_t pos;
        memcpy(&pos, predicates+i, sizeof(size_t));
        std::cout<<pCodes->decode(c)<<" "<<c<<" @"<<pos<<"\n";
    }
#endif
    std::map<BinaryCode,int> candidates;
    bool first=true;
    int n=0;
    for(auto child:query->children())
    {
        auto code=(*pCodes)[child.first];
        auto addr = subjectsForPredicate(code);
        std::cout<<child.first<<" ("<<code<<")"<<" in range ["<<addr.first<<","<<addr.second<<")\n";
        for(size_t pos=addr.first;pos<addr.second;pos+=sizeof(size_t))
        {
            BinaryCode c=read(subjects, pos);
            if(!first && candidates[c]<n)
                continue;
            size_t oBegin;
            memcpy(&oBegin, subjects+pos, sizeof(size_t));
            size_t tmp=pos+sizeof(size_t);
            read(subjects,tmp);
            size_t oEnd;
            memcpy(&oEnd, subjects+tmp, sizeof(size_t));
            if(child.second->isDefined())
            {
                BinaryCode o=(*soCodes)[child.second->label()];
                auto ptr = find(objects+oBegin, oEnd-oBegin, o.value(), o.length(), false);
                if(ptr!=NULL)
                    candidates[c]++;
            }
            else
            {
                for(size_t a=oBegin;a<oEnd;)
                {
                    BinaryCode o=read(objects, a);
                    if(ask(child.second, o))
                    {
                        candidates[c]++;
                        break;
                    }
                }
            }
        }
        first=false;
        n++;
    }
    std::deque<std::string> answer;    
    for(auto c:candidates)
    {
        if(c.second==n)
        {
            std::cout<<soCodes->decode(c.first)<<" "<<c.second<<"\n";
            answer.push_back(soCodes->decode(c.first));
        }
    }
    return answer;
}
