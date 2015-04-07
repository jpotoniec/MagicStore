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

void Codes::MakeCode(const Node *root, const BinaryCode& prefix)
{
	if(root!=NULL)
	{
		if(!root->getLabel().empty())
			valToCode[root->getLabel()]=prefix;
		BinaryCode lcode(prefix);
		lcode.append(false);
		BinaryCode rcode(prefix);
		rcode.append(true);
		MakeCode(root->getLeft(),lcode);
		MakeCode(root->getRight(),rcode);
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
	MakeCode(root, BinaryCode());
}

void Codes::load(std::ifstream& f)
{
    root=Node::load(f);
    MakeCode(root, BinaryCode());
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
