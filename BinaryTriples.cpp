#include "BinaryTriples.hpp"
#include "Compressor.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <memory>
#include "Merger.hpp"

typedef std::map<std::string,int> Stats;

static inline uint8_t len(uint32_t value)
{
    if(value<=0x3f)
        return 0;
    else if(value<=0x3fff)
        return 1;
    else if(value<=0x3fffff)
        return 2;
    else
        return 3;
}

static void write(uint8_t *where, size_t &wlen, uint32_t val)
{
    size_t _len=len(val);
    switch(_len)
    {
        case 3:
            where[wlen+3]=static_cast<uint8_t>((val>>22)&0xff);
        case 2:
            where[wlen+2]=static_cast<uint8_t>((val>>14)&0xff);
        case 1:
            where[wlen+1]=static_cast<uint8_t>((val>>6)&0xff);
        case 0:
            where[wlen+0]=static_cast<uint8_t>((val>>0)&0x3f)|(static_cast<uint8_t>(_len)<<6);
    }
    wlen+=_len+1;
}

static inline void skip(const uint8_t *where, size_t &position)
{
    uint8_t len=(((where[position])>>6)&0b11);
    position+=len+1;
}

static inline uint32_t read(const uint8_t *where, size_t &position)
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
    return result;
}

const std::pair<size_t,size_t> BinaryTriples::invalid=std::pair<size_t,size_t>(static_cast<size_t>(-1),static_cast<size_t>(-1));

static const uint8_t* find(const uint8_t* where, size_t n, uint32_t value)
{
    uint8_t length=len(value);
    for(size_t i=0;i<n;i+=sizeof(size_t))
    {
        uint8_t len=(((where[i])>>6)&0b11);
        if(len==length)
        {
            uint32_t c=read(where, i);
            if(c==value)
                return where+i;
        }
        else
            i+=len+1;
    }
    return NULL;
}

BinaryTriples::Address BinaryTriples::level2For1(uint32_t p) const
{
    const uint8_t *ptr=find(level1, len1, p);
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

BinaryTriples::Address BinaryTriples::level3For2(const Address& a, uint32_t s) const
{
    size_t len=a.second-a.first;
    const uint8_t *ptr=find(level2+a.first, len, s);
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

BinaryTriples::Address BinaryTriples::level3For12(uint32_t l1, uint32_t l2) const
{
    Address sa=level2For1(l1);
    if(sa!=invalid)
        return level3For2(sa, l2);
    return invalid;
}

BinaryTriples::BinaryTriples()
    :prevP(std::numeric_limits<uint32_t>::max()),prevO(std::numeric_limits<uint32_t>::max())
{
}

BinaryTriples::~BinaryTriples()
{
    delete []level1;
    delete []level2;
    delete []level3;
}

void BinaryTriples::save(std::ofstream& f) const
{
    soCodes->save(f);
    pCodes->save(f);
    f.write(reinterpret_cast<const char*>(&len1), sizeof(len1));
    f.write(reinterpret_cast<const char*>(level1), len1);
    f.write(reinterpret_cast<const char*>(&len2), sizeof(len2));
    f.write(reinterpret_cast<const char*>(level2), len2);
    f.write(reinterpret_cast<const char*>(&len3), sizeof(len3));
    f.write(reinterpret_cast<const char*>(level3), len3);
}

void BinaryTriples::load(std::ifstream& f)
{
    soCodes=PCodes(new Codes());
    pCodes=PCodes(new Codes());
    soCodes->load(f);
    pCodes->load(f);
    f.read(reinterpret_cast<char*>(&len1), sizeof(len1));
    level1=new uint8_t[len1];
    f.read(reinterpret_cast<char*>(level1), len1);
    f.read(reinterpret_cast<char*>(&len2), sizeof(len2));
    level2=new uint8_t[len2];
    f.read(reinterpret_cast<char*>(level2), len2);
    f.read(reinterpret_cast<char*>(&len3), sizeof(len3));
    level3=new uint8_t[len3];
    f.read(reinterpret_cast<char*>(level3), len3);
    std::cout<<"Loaded binary blobs of sizes "<<len1<<"+"<<len2<<"+"<<len3<<" bytes"<<std::endl;
}

void BinaryTriples::add(uint32_t s, uint32_t p, uint32_t o)
{
    if(prevP!=p)
    {
        prevP=p;
        write(level1, len1, p);
        memcpy(level1+len1, &len2, sizeof(len2));
        len1+=sizeof(len2);
        prevO=std::numeric_limits<uint32_t>::max();
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

void BinaryTriples::fill(PCodes soCodes, PCodes pCodes, RawBinaryTriples& triples)
{
    this->soCodes=soCodes;
    this->pCodes=pCodes;
    std::sort(triples.begin(), triples.end());
    size_t psize=0,osize=0,ssize=0;
    uint32_t p(std::numeric_limits<uint32_t>::max()),o(std::numeric_limits<uint32_t>::max());
    for(auto t:triples)
    {
        if(p!=t.p())
        {
            psize++;
            p=t.p();
            o=std::numeric_limits<uint32_t>::max();
        }
        if(o!=t.o())
        {
            osize++;
            o=t.o();
        }
    }
    psize*=(4+sizeof(size_t));
    osize*=(4+sizeof(size_t));
    ssize=4*triples.size();
    level1=new uint8_t[psize];
    len1=0;
    level2=new uint8_t[osize];
    len2=0;
    level3=new uint8_t[ssize];
    len3=0;
    for(auto t:triples)
    {
        add(t);
//        std::cout<<t<<" "<<s<<" "<<p<<" "<<o<<"\n";
    }
    finish();
}

void BinaryTriples::finish()
{
    write(level1, len1, std::numeric_limits<uint32_t>::max());
    memcpy(level1+len1, &len2, sizeof(len2));
    write(level2, len2, std::numeric_limits<uint32_t>::max());
    memcpy(level2+len2, &len3, sizeof(len3));
}

class AbstractIterator
{
public:
    virtual ~AbstractIterator()
    {
    }
    virtual uint32_t next()=0;
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
    uint32_t next()
    {
        uint32_t b=read(data, pos);
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
    uint32_t next()
    {
        uint32_t x=b.next();
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

size_t readSize(const uint8_t *data, size_t &pos)
{
    size_t result;
    memcpy(&result, data+pos, sizeof(size_t));
    pos+=sizeof(size_t);
    return result;
}

class TripleIterator
{
public:
    TripleIterator(const BinaryTriples& bt)
        :bt(bt),p1(0),p2(0),p3(0),n2(0),n3(0),change(false)
    {
        l1=read(bt.level1, p1);
        p2=readSize(bt.level1, p1);
        n2=nextSize(bt.level1, p1);
        l2=read(bt.level2, p2);
        p3=readSize(bt.level2, p2);
        n3=nextSize(bt.level2, p2);
    }
    BinaryTriple next()
    {
        uint32_t l3=read(bt.level3, p3);
        BinaryTriple result(l3,l1,l2);
//        std::cout<<"p: "<<p3<<" "<<p2<<" "<<p1<<" n: "<<n3<<" "<<n2<<" inf\n";
        if(p3==n3)
        {
            if(p2==n2)
            {
                l1=read(bt.level1, p1);
                size_t x=readSize(bt.level1, p1);
                assert(x==p2);
                n2=nextSize(bt.level1, p1);
            }
            l2=read(bt.level2, p2);
            size_t x=readSize(bt.level2, p2);
            assert(x==p3);
            n3=nextSize(bt.level2,p2);
        }
        return result;
    }
    bool hasNext()
    {
        return p3<bt.len3;
    }
private:
    size_t nextSize(const uint8_t *data, size_t n)
    {
        skip(data, n);
        return readSize(data,n);
    }
    static const size_t invalid=std::numeric_limits<size_t>::max();
    const BinaryTriples& bt;
    size_t p1,p2,p3,n2,n3;
    uint32_t l2,l1;
    bool change;
};

/// wymga, żeby oba argumenty były posortowane
template<typename A,typename B>
std::deque<uint32_t> intersect(const A& a, const B& b)
{
    std::deque<uint32_t> result;
    auto i=a.begin();
    auto j=b.begin();
    while(i!=a.end() && j!=b.end())
    {
        if(*i<*j)
            i++;
        else if(*i>*j)
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

std::deque<uint32_t> flatten(PAbstractIterator i)
{
    std::deque<uint32_t> result;
    if(i!=NULL)
    {
        while(i->hasNext())
            result.push_back(i->next());
        if(!i->isSorted())
            std::sort(result.begin(),result.end());
    }
    return result;
}

PAbstractIterator BinaryTriples::iteratorForQuery(const TreePattern::Node* query) const
{
    assert(query->children().empty());
    uint32_t p=(*pCodes)[query->parentProperty()];
    if(query->isDefined())
    {
        uint32_t o=(*soCodes)[query->label()];
        Address a=level3For12(p, o);
        if(a!=invalid)
            return PAbstractIterator(new Iterator<false>(level3, a));
    }
    else
    {
        Address a=level2For1(p);
        if(a!=invalid)
            return PAbstractIterator(new DoubleIterator(level2, a, level3));
    }
    return NULL;
}

#define USE_GPU 1

#if USE_GPU
#include "GPU.hpp"
GPU gpu("NVIDIA CUDA");
#endif

std::deque<uint32_t> BinaryTriples::answerCodes(const TreePattern::Node* query) const
{
#if USE_GPU
    gpu.setData(level2, len2);
#endif
    if(query->children().empty())
        return flatten(iteratorForQuery(query));
    else
    {
        std::deque<uint32_t> subjects=answerCodes(query->children()[0].second);
        for(int i=1;i<query->children().size();++i)
        {
            if(subjects.empty())
                break;
            subjects=intersect(subjects, answerCodes(query->children()[i].second));
        }
        if(!query->isRoot())
        {
            uint32_t p=(*pCodes)[query->parentProperty()];
            std::deque<uint32_t> result;
            Address paddr=level2For1(p);
#if USE_GPU
            std::deque<FindArgs> r;
            for(auto o:subjects)
                r.push_back(FindArgs(paddr.first,paddr.second,o));
            std::deque<Address> gpuaddr = gpu.find(r);
            for(auto oaddr:gpuaddr)
            {
#else
            for(auto o:subjects)
            {
                auto oaddr=level3For2(paddr, o);
#endif
                auto i=Iterator<false>(level3, oaddr);
                while(i.hasNext())
                {
                    uint32_t s=i.next();
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
    TripleIterator t(*this);
    std::deque<uint32_t> codes=answerCodes(query);
    std::deque<std::string> result;
    for(auto b:codes)
        result.push_back(soCodes->decode(b));
    return result;
}

void BinaryTriples::add(const BinaryTriple& t)
{
//    std::cout<<"Adding: ";
//    dump(t);
    add(t.s(),t.p(),t.o());
    assert(t.s()!=std::numeric_limits<uint32_t>::max());
    assert(t.p()!=std::numeric_limits<uint32_t>::max());
    assert(t.o()!=std::numeric_limits<uint32_t>::max());
}

void BinaryTriples::dump(const BinaryTriple& t) const
{
    std::cout<<t.s()<<" "<<t.p()<<" "<<t.o()<<" "<<soCodes->decode(t.s())<<" "<<pCodes->decode(t.p())<<" "<<soCodes->decode(t.o())<<"\n";
}

void BinaryTriples::merge(const BinaryTriples& a, const BinaryTriples& b)
{
    this->soCodes=a.soCodes;
    this->pCodes=a.pCodes;
    level1=new uint8_t[a.len1+b.len1];
    len1=0;
    level2=new uint8_t[a.len2+b.len2];
    len2=0;
    level3=new uint8_t[a.len3+b.len3];
    len3=0;
    ::merge<BinaryTriple>(TripleIterator(a), TripleIterator(b),
                          [this](const BinaryTriple& t)->void{add(t);},
    BinaryTriple::compare);
}
