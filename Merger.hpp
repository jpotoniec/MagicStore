#ifndef MERGER_HPP
#define MERGER_HPP

template<typename V,typename C=std::deque<V>,typename I=typename std::deque<V>::const_iterator>
class JavaIterator
{
public:
    JavaIterator(const C& col)
        :JavaIterator(col.begin(), col.end())
    {

    }
    JavaIterator(const I& begin, const I& end)
        :i(begin),end(end)
    {

    }
    V next()
    {
        return *(i++);
    }
    bool hasNext() const
    {
        return i!=end;
    }
private:
    I i,end;
};

template<typename T,typename Iterator, typename AddFunctor, typename Comparator>
void merge(Iterator a, Iterator b, AddFunctor add, Comparator comp)
{
    T x=a.next();
    T y=b.next();
    for(;;)
    {
        bool an=false, bn=false;
        int c=comp(x,y);
        if(c<0)
        {
            add(x);
            an=true;
        }
        else if(c>0)
        {
            add(y);
            bn=true;
        }
        else
        {
            add(x);
            an=bn=true;
        }
        if(an)
        {
            if(a.hasNext())
                x=a.next();
            else
            {
                if(!bn)
                    add(y);
                break;
            }
        }
        if(bn)
        {
            if(b.hasNext())
                y=b.next();
            else
            {
                add(x);
                break;
            }
        }
    }
    assert(!(a.hasNext() && b.hasNext()));
    while(a.hasNext())
        add(a.next());
    while(b.hasNext())
        add(b.next());
}

#endif // MERGER_HPP
