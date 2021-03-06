#ifndef COMPRESSORHPP
#define COMPRESSORHPP

#include "Trie.hpp"
#include <boost/noncopyable.hpp>
#include <cstdint>
#include <cassert>
#include <memory>
#include <iosfwd>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#if USE_UNORDERED_SET
#include <unordered_set>
#else
#include <set>
#endif

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
#if USE_UNORDERED_SET
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
#else
    bool operator()(const Node *a, const Node *b) const
    {
        return a->getLabel()<b->getLabel();
    }
#endif
};

class Codes : private boost::noncopyable
{
	public:
        uint32_t operator[](const std::string& value) const
        {
            return valToCode.left.find(trie.find(value))->second;
		}
        std::string decode(uint32_t code) const
        {
            return valToCode.right.find(code)->second->label();
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
        void MakeCode(const Node *root, uint32_t prefix, uint8_t len=0);
        typedef boost::bimap<const Trie::Node*,uint32_t> Map;
        Map valToCode;
		Node *root;
        Trie trie;
#if USE_UNORDERED_SET
        std::unordered_set<Node*, NodeSetHelper, NodeSetHelper> input;
#else
        std::set<Node*, NodeSetHelper> input;
#endif
};

typedef std::shared_ptr<Codes> PCodes;

#endif //COMPRESSORHPP
