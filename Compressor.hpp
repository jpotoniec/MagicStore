#ifndef COMPRESSORHPP
#define COMPRESSORHPP

#include <boost/noncopyable.hpp>
#include <cstdint>
#include <map>
#include <cassert>
#include <memory>
#include <unordered_set>
#include <iosfwd>

class Node
{
	public:
		Node(const std::string& label, int occurences,const Node *left=NULL,const Node *right=NULL)
			:label(label), occurences(occurences),left(left),right(right)
		{
		}
		const std::string& getLabel() const
		{
			return label;
		}
		int getOccurences() const
		{
			return occurences;
		}
		const Node* getLeft() const
		{
			return left;
		}
		const Node* getRight() const
		{
			return right;
		}
		bool hasChildren() const
		{
			assert((left==NULL && right==NULL) || (left!=NULL && right!=NULL));
			return left!=NULL && right!=NULL;
		}
        ~Node()
        {
            if(left)
                delete left;
            if(right)
                delete right;
        }
        void inc()
        {
            ++occurences;
        }
        static Node* load(std::ifstream& f)
        {
            return new Node(f);
        }
        void save(std::ofstream& f) const;
    private:
        Node(std::ifstream& f);
		std::string label;
        size_t occurences;
		const Node *left,*right;
};

class NodeSetHelper
{
public:
    size_t operator()(const Node *a) const
    {
        return hasher(a->getLabel());
    }
    bool operator()(const Node *a, const Node *b) const
    {
        return a->getLabel()==b->getLabel();
    }
private:
    std::hash<std::string> hasher;
};

template<typename T>
class TBinaryCode
{
	public:
		T value() const {return val;}
		uint8_t length() const {return len;}
		void append(bool v)
		{
			assert(len<sizeof(val)*8);			
			if(v)
                val|=(1<<len);
			len++;
		}
		TBinaryCode(T val, uint8_t len)
			:val(val),len(len)
		{
		}
		TBinaryCode()
			:TBinaryCode(0,0)
		{
		}
		friend bool operator==(const TBinaryCode<T> &a, const TBinaryCode<T> &b)
		{
            return a.val==b.val;
		}
		friend bool operator!=(const TBinaryCode<T> &a, const TBinaryCode<T> &b)
		{
			return !(a==b);
		}
        friend bool operator<(const TBinaryCode<T> &a, const TBinaryCode<T> &b)
        {
            return compare(a,b)<0;
        }
        friend int compare(const TBinaryCode<T> &a, const TBinaryCode<T> &b)
        {
            if(a.val<b.val)
                return -1;
            else if(a.val>b.val)
                return 1;
            return 0;
        }
	private:
		T val;
		uint8_t len;
};

#include <iostream>
template<typename T>
std::ostream& operator<<(std::ostream& o, const TBinaryCode<T>& c)
{
    return o<<std::hex<<c.value()<<"/"<<std::dec<<static_cast<uint32_t>(c.length());
}

typedef TBinaryCode<uint32_t> BinaryCode;

class Codes : private boost::noncopyable
{
	public:
		const BinaryCode& operator[](const std::string& value) const
		{
			return valToCode.find(value)->second;
		}
        const std::string& decode(const BinaryCode& code) const
        {
            return decode(code.value());
        }
        const std::string& decode(uint32_t code) const
        {
            const Node *n=root;
            while(n->hasChildren())
            {
                if(code&1)
                    n=n->getRight();
                else
                    n=n->getLeft();
                code>>=1;
            }
            return n->getLabel();
        }
        Codes()
            :root(NULL)
        {

        }
        ~Codes()
        {
            if(root)
                delete root;
        }
        void inc(const std::string& label);
        void compress();
        size_t size() const
        {
            return valToCode.size();
        }
        void load(std::ifstream& f);
        void save(std::ofstream& f) const;
	private:
		void MakeCode(const Node *root, const BinaryCode& prefix);
		std::map<std::string,BinaryCode> valToCode;
		Node *root;
        std::unordered_set<Node*, NodeSetHelper, NodeSetHelper> input;
};

#endif //COMPRESSORHPP
