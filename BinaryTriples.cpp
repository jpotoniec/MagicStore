#include "BinaryTriples.hpp"
#include "Compressor.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

typedef std::map<std::string,int> Stats;

struct TriplesComparator
{
    TriplesComparator(const Codes *soCodes, const Codes *pCodes)
        :soCodes(soCodes),pCodes(pCodes)
    {
    }
	bool operator()(const Triple& a, const Triple& b)
    {
        int c=compare((*pCodes)[a.p()], (*pCodes)[b.p()]);
        if(c!=0)
            return c<0;
        c=compare((*soCodes)[a.o()], (*soCodes)[b.o()]);
        if(c!=0)
            return c<0;
        c=compare((*soCodes)[a.s()], (*soCodes)[b.s()]);
        return c<0;
    }
private:
    const Codes *soCodes, *pCodes;
};


void write(uint8_t *where, size_t &wlen, const BinaryCode& c)
{
    assert(c.length()<=30);
    size_t len=(c.length()+1)/8;
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

BinaryTriples::Address BinaryTriples::level2For1(const BinaryCode& p) const
{
    const uint8_t *ptr=find(level1, len1, p.value(), p.length(), true);
    if(ptr!=NULL)
    {
        size_t begin=*reinterpret_cast<const size_t*>(ptr);
        ptr+=sizeof(size_t);
        size_t end=len2;
        if(ptr-level1<len1)
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

BinaryTriples::Address BinaryTriples::level3For2(const Address& a, const BinaryCode& s) const
{
    size_t len=a.second-a.first;
    const uint8_t *ptr=find(level2+a.first, len, s.value(), s.length(), true);
    if(ptr!=NULL)
    {
        size_t begin=*reinterpret_cast<const size_t*>(ptr);
        ptr+=sizeof(size_t);
        assert(ptr-level2<len2);
        size_t d=0;
        skip(ptr,d);
        size_t end=*reinterpret_cast<const size_t*>(ptr+d);
        return std::make_pair(begin,end);
    }
    else
        return invalid;
}

BinaryTriples::Address BinaryTriples::level3For12(const BinaryCode& l1, const BinaryCode& l2) const
{
    Address sa=level2For1(l1);
    if(sa!=invalid)
        return level3For2(sa, l2);
    return invalid;
}

void BinaryTriples::fill(Triples& triples)
{
    Stats soStats,pStats;
    for(auto t:triples)
    {
        soStats[t.s()]++;
        pStats[t.p()]++;
        soStats[t.o()]++;
    }
    soCodes=new Codes(soStats);
    pCodes=new Codes(pStats);
    std::sort(triples.begin(), triples.end(), TriplesComparator(soCodes, pCodes));
    level1=new uint8_t[(4+sizeof(size_t))*pStats.size()];
    memset(level1, 0, (4+sizeof(size_t))*pStats.size());
    len1=0;
    level2=new uint8_t[(4+sizeof(size_t))*pStats.size()*soStats.size()];
    len2=0;
    level3=new uint8_t[4*triples.size()];
    len3=0;
    BinaryCode prevP,prevO;
    for(auto t:triples)
    {
        BinaryCode s((*soCodes)[t.s()]);
        BinaryCode p((*pCodes)[t.p()]);
        BinaryCode o((*soCodes)[t.o()]);
        if(prevP!=p)
        {
            std::cout<<t.p()<<" "<<p<<"\n";
            prevP=p;
            write(level1, len1, p);
            memcpy(level1+len1, &len2, sizeof(len2));
            len1+=sizeof(len2);
            std::cout<<"Written "<<static_cast<uint32_t>(p.length())<<"+32 bits, pLen="<<len1<<"\n";
            prevO=BinaryCode();
        }
        if(prevO!=o)
        {
            prevO=o;
            write(level2, len2, o);
            memcpy(level2+len2, &len3, sizeof(len3));
            len2+=sizeof(len3);
        }
        write(level3, len3, s);
    }
    write(level1, len1, BinaryCode(0xffffffff, 30));
    memcpy(level1+len1, &len2, sizeof(len2));
    write(level2, len2, BinaryCode(0xffffffff, 30));
    memcpy(level2+len2, &len3, sizeof(len3));
    std::cout<<"pLen="<<len1<<" sLen="<<len2<<" oLen="<<len3<<"\n";
}

class AbstractIterator
{
public:
    virtual ~AbstractIterator()
    {
    }
    virtual BinaryCode next()=0;
    virtual bool hasNext() const=0;
    virtual bool isSorted() const
    {
        return false;
    }
};

template<bool index>
class Iterator:public AbstractIterator
{
public:
    Iterator(const uint8_t* data, const BinaryTriples::Address& a)
        :data(data),pos(a.first),end(a.second)
    {
    }
    BinaryCode next()
    {
        BinaryCode b=read(data, pos);
        if(index)
            pos+=sizeof(size_t);
        return b;
    }
    bool hasNext() const
    {
        return pos<end;
    }
    bool isSorted() const
    {
        return true;
    }
private:
    const uint8_t *data;
    size_t pos,end;
};

class DoubleIterator:public AbstractIterator
{
public:
    DoubleIterator(const uint8_t* data1, const BinaryTriples::Address& a, const uint8_t* data2)
        :data1(data1),pos(a.first),end(a.second),data2(data2),b(Iterator<false>(data2, init()))
    {
    }
    bool hasNext() const
    {
        return b.hasNext();
    }
    BinaryCode next()
    {
        BinaryCode x=b.next();
        if(!b.hasNext() && pos<end)
        {
            size_t tmp=nextPos();
            b=Iterator<false>(data2, BinaryTriples::Address(nextBegin2,tmp));
            nextBegin2=tmp;
        }
        return x;
    }
private:
    const uint8_t *data1,*data2;
    size_t pos,end;
    size_t nextBegin2;
    Iterator<false> b;

    size_t nextPos()
    {
        size_t x;
        skip(data1,pos);
        memcpy(&x,data1+pos,sizeof(size_t));
        pos+=sizeof(size_t);
        return x;
    }
    BinaryTriples::Address init()
    {
        size_t pos2=nextPos();
        nextBegin2=nextPos();
        return BinaryTriples::Address(pos2,nextBegin2);
    }
};

/// wymga, żeby oba argumenty były posortowane
template<typename A,typename B>
std::deque<BinaryCode> intersect(const A& a, const B& b)
{
    std::deque<BinaryCode> result;
    auto i=a.begin();
    auto j=b.begin();
    while(i!=a.end() && j!=b.end())
    {
        int c=compare(*i,*j);
        if(c<0)
            i++;
        else if(c>0)
            j++;
        else
        {
            result.push_back(*i);
            i++;
            j++;
        }
    }
    return result;
}

std::deque<BinaryCode> flatten(AbstractIterator *i)
{    
    std::deque<BinaryCode> result;
    if(i!=NULL)
    {
        while(i->hasNext())
            result.push_back(i->next());
        if(!i->isSorted())
            std::sort(result.begin(),result.end());
    }
    return result;
}

//polega na posortowaniu, a DoubleIterator tego nie zapewnia
template<bool i,bool j>
std::deque<BinaryCode> intersect(Iterator<i>* a, Iterator<j>* b)
{
    std::deque<BinaryCode> result;
    BinaryCode aVal=a->next();
    BinaryCode bVal=b->next();
    for(;;)
    {
        bool aInc=false;
        bool bInc=false;
        int c=compare(aVal,bVal);
        std::cout<<aVal<<" "<<bVal<<" "<<c<<"\n";
        if(c<0)
            aInc=true;
        else if(c>0)
            bInc=true;
        else
        {
            aInc=true;
            bInc=true;
            result.push_back(aVal);
        }
        if(aInc)
        {
            if(a->hasNext())
                aVal=a->next();
            else
                break;
        }
        if(bInc)
        {
            if(b->hasNext())
                bVal=b->next();
            else
                break;
        }
    }
    return result;
}

AbstractIterator* BinaryTriples::iteratorForQuery(const TreePattern::Node* query) const
{
    assert(query->children().empty());
    BinaryCode p=(*pCodes)[query->parentProperty()];
    if(query->isDefined())
    {
        BinaryCode o=(*soCodes)[query->label()];
        Address a=level3For12(p, o);
        if(a!=invalid)
            return new Iterator<false>(level3, a);
    }
    else
    {
        Address a=level2For1(p);
        if(a!=invalid)
            return new DoubleIterator(level2, a, level3);
    }
    return NULL;
}

std::deque<BinaryCode> BinaryTriples::answerCodes(const TreePattern::Node* query) const
{
    if(query->children().empty())
        return flatten(iteratorForQuery(query));
    else
    {
        std::deque<BinaryCode> subjects=answerCodes(query->children()[0].second);
        for(int i=1;i<query->children().size();++i)
        {
            if(subjects.empty())
                break;
            subjects=intersect(subjects, answerCodes(query->children()[i].second));
        }
        if(!query->isRoot())
        {
            BinaryCode p=(*pCodes)[query->parentProperty()];
            std::deque<BinaryCode> result;
            for(auto o:subjects)
            {
                auto i=new Iterator<false>(level3, level3For12(p, o));
                while(i->hasNext())
                {
                    BinaryCode s=i->next();
                    result.push_back(s);
                }
            }
            std::sort(result.begin(), result.end());
            return result;
        }
        else
            return subjects;
    }
}

std::deque<std::string> BinaryTriples::answer(const TreePattern::Node* query) const
{    
    std::deque<BinaryCode> codes=answerCodes(query);
    std::deque<std::string> result;
    for(auto b:codes)
        result.push_back(soCodes->decode(b));
    return result;
}
