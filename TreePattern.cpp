#include "TreePattern.hpp"
#include <map>
#include <stdexcept>
namespace TreePattern
{
	static bool isVar(const std::string& var)
	{
		return var[0]=='?';
	}

	Node* Node::fromTriples(const std::deque<Triple>& triples)
	{
        //TODO: bleh, tu podobno jest jakiś wyciek na danych testowych
		std::map<std::string,Node*> vars;
		for(Triple t:triples)
		{
			assert(isVar(t.s()));
			if(vars.find(t.s())==vars.end())
				vars[t.s()]=new Node(t.s());
        }
		for(Triple t:triples)
		{
            auto i=vars.find(t.o());
            Node *o;
            if(i!=vars.end())
                o=i->second;
            else
                o=new Node(t.o());
			o->parent(vars[t.s()], t.p());
		}
        for(auto v:vars)
			if(v.second->parent()==NULL)
                return v.second;
		throw std::runtime_error("Can not find root, check in your pots");
	}

	void Node::dump(std::ostream& o, const std::string& prefix) const
	{
		o<<prefix<<label()<<"\n";
		std::string x(label().length(),' ');
		for(auto i:children())
		{
			o<<prefix<<x<<i.first<<":\n";
			i.second->dump(o, prefix+x+std::string(i.first.length()+1,' '));
		}
	}
}
