#include "Compressor.hpp"
#include <deque>
#include <queue>
#include <iostream>
#include <fstream>

class NodesOrder
{
	public:
		bool operator()(const Node* a, const Node* b) const
		{
			return a->getOccurences()>b->getOccurences();
		}
};

void Codes::MakeCode(const Node *root, uint32_t prefix, uint8_t len)
{
    assert(len<=30);
	if(root!=NULL)
	{
		if(!root->getLabel().empty())
            valToCode[root->getLabel()]=prefix;
        uint32_t lcode(prefix);
        uint32_t rcode(prefix|(1<<len));
        MakeCode(root->getLeft(),lcode,len+1);
        MakeCode(root->getRight(),rcode,len+1);
	}
}

void Codes::inc(const std::string &label)
{
    Node *n=new Node(label,0);
    auto i=input.insert(n);
    if(!i.second)
        delete n;
    (*i.first)->inc();
}

static size_t countLeaves(const Node *root)
{
    if(!root->getLabel().empty())
        return 1;
    return countLeaves(root->getLeft())+countLeaves(root->getRight());
}

void Codes::compress()
{
	std::priority_queue<Node*,std::deque<Node*>,NodesOrder> nodes;
    for(auto i:input)
        nodes.push(i);
    input.clear();
	for(;;)
	{
        assert(!nodes.empty());
		Node *l=nodes.top();
		nodes.pop();        
		if(nodes.empty())
		{
			root=l;
			break;
		}        
		Node *r=nodes.top();
		nodes.pop();        
		nodes.push(new Node("",l->getOccurences()+r->getOccurences(),l,r));
    }
#if USE_UNORDERED_MAP
    valToCode.rehash(countLeaves(root));
#endif
    MakeCode(root, 0);
}

void Codes::load(std::ifstream& f)
{
    root=Node::load(f);
    MakeCode(root, 0);
}

void Codes::save(std::ofstream& f) const
{
    root->save(f);
}

Node::Node(std::ifstream& f)
{
    std::getline(f, label, '\0');
    f.read(reinterpret_cast<char*>(&occurences), sizeof(occurences));
    char c;
    f>>c;
    if(c=='1')
        left=Node::load(f);
    else
        left=NULL;
    f>>c;
    if(c=='1')
        right=Node::load(f);
    else
        right=NULL;
}

void Node::save(std::ofstream& f) const
{
    f<<label<<'\0';
    f.write(reinterpret_cast<const char*>(&occurences), sizeof(occurences));
    if(left)
    {
        f<<'1';
        left->save(f);
    }
    else
        f<<'0';
    if(right)
    {
        f<<'1';
        right->save(f);
    }
    else
        f<<'0';
}

#if 0
void dump(const Node *root, const std::string& prefix)
{
	if(root!=NULL)
	{
		std::cout<<prefix<<" "<<root->getLabel()<<"\n";
		dump(root->getLeft(), prefix+"0");
		dump(root->getRight(), prefix+"1");
	}
}
#endif
