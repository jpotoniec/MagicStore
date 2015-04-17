#include "TreePattern.hpp"
#include <map>
#include <stdexcept>
#include <rasqal.h>
namespace TreePattern
{
    static bool isVar(rasqal_literal *l)
    {
        return l->type==RASQAL_LITERAL_VARIABLE;
	}

    static std::string varName(rasqal_literal *l)
    {
        return reinterpret_cast<const char*>(rasqal_literal_as_variable(l)->name);
    }

    static std::string uri(rasqal_literal *l)
    {
        return std::string(reinterpret_cast<const char*>(rasqal_literal_as_counted_string(l, NULL,0,NULL)));
    }

    Node* Node::fromSPARQL(const std::string& sparql)
	{
        auto world=rasqal_new_world();
        rasqal_world_open(world);
        auto query=rasqal_new_query(world,NULL,NULL);
        rasqal_query_prepare(query, reinterpret_cast<const unsigned char*>(sparql.c_str()), NULL);
		std::map<std::string,Node*> vars;
        for(int i=0;;++i)
        {
            auto t=rasqal_query_get_triple(query,i);
            if(t==NULL)
                break;
            assert(isVar(t->subject));
            std::string var=varName(t->subject);
            if(vars.find(var)==vars.end())
                vars[var]=new Node(var, false);
        }
        for(int i=0;;++i)
        {
            auto t=rasqal_query_get_triple(query,i);
            if(t==NULL)
                break;
            Node *o;
            if(isVar(t->object))
            {
                auto ov=varName(t->object);
                auto i=vars.find(ov);
                if(i!=vars.end())
                    o=i->second;
                else
                    o=new Node(ov, false);
            }
            else
                o=new Node(uri(t->object), true);
            o->parent(vars[varName(t->subject)], uri(t->predicate));
		}
        rasqal_free_query(query);
        rasqal_free_world(world);
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
