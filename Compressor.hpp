#ifndef COMPRESSORHPP
#define COMPRESSORHPP

#include <boost/noncopyable.hpp>
#include <cstdint>
#include <map>
#include <cassert>

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
	private:
		std::string label;
		int occurences;
		const Node *left,*right;
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
		T leftNormal() const
		{
			return val<<(sizeof(T)*8-len);
		}
		T rightReversed() const
		{
			T input(val);
			T output(0);
			for(uint8_t i=0;i<len;++i)
			{
				output<<=1;
				if(input&1)
					output|=1;
				input>>=1;
			}
			return output;
		}
		T maskRight() const
		{
			return (static_cast<T>(1)<<len)-1;
		}
		T maskLeft() const
		{
			return maskRight()<<(sizeof(T)*8-len);
		}
		friend bool operator==(const TBinaryCode<T> &a, const TBinaryCode<T> &b)
		{
			return a.val==b.val && a.len==b.len;
		}
		friend bool operator!=(const TBinaryCode<T> &a, const TBinaryCode<T> &b)
		{
			return !(a==b);
		}
        friend bool operator<(const TBinaryCode<T> &a, const TBinaryCode<T> &b)
        {
            if(a.len!=b.len)
                return a.len<b.len;
            return a.val<b.val;
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
        std::string decode(const BinaryCode& code)
        {
            return decode(code.value());
        }
        std::string decode(uint32_t code)
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
		std::string DecodeLabelRight(uint32_t var) const
		{
			const Node *n=root;
			while(n->hasChildren())
			{
				if(var&0x1)
					n=n->getRight();
				else
					n=n->getLeft();
				var>>=1;
			}
			return n->getLabel();
		}
		std::string DecodeLabelLeft(uint32_t var) const
		{
			const Node *n=root;
			while(n->hasChildren())
			{
				if(var&(1<<31))
					n=n->getRight();
				else
					n=n->getLeft();
				var<<=1;
			}
			return n->getLabel();
		}
		BinaryCode DecodeRight(uint32_t var) const
		{
			uint32_t val=0;
			uint8_t len=0;
			const Node *n=root;
			while(n->hasChildren())
			{
				val<<=1;
				len++;
				if(var&0x1)
				{
					n=n->getRight();
					val|=1;
				}
				else
					n=n->getLeft();
				var>>=1;
			}
			return BinaryCode(val, len);
		}
		BinaryCode DecodeLeft(uint32_t var) const
		{
			uint32_t val=0;
			uint8_t len=0;
			const Node *n=root;
			while(n->hasChildren())
			{
				val<<=1;
				len++;
				if(var&(1<<31))
				{
					n=n->getRight();
					val|=1;
				}
				else
					n=n->getLeft();
				var<<=1;
			}
			return BinaryCode(val, len);
		}
		Codes(const std::map<std::string,int>& stats);
	private:
		void MakeCode(const Node *root, const BinaryCode& prefix);
		std::map<std::string,BinaryCode> valToCode;
		Node *root;
};


#endif //COMPRESSORHPP
