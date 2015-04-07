#include "Compressor.hpp"
#include <deque>
#include <queue>
#include <iostream>

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
