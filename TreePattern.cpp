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

    Query Query::fromSPARQL(const std::string& sparql)
	{
        auto world=rasqal_new_world();
        rasqal_world_open(world);
        auto query=rasqal_new_query(world,NULL,NULL);
        rasqal_query_prepare(query, reinterpret_cast<const unsigned char*>(sparql.c_str()), NULL);
        if(rasqal_query_get_verb(query)!=RASQAL_QUERY_VERB_SELECT)
            throw std::runtime_error("Only SELECT is supported");
        auto seq=rasqal_query_get_bound_variable_sequence(query);
        std::string rootVar,topVar;
        bool count=false;
        for(int i=0;;++i)
        {
              auto var=reinterpret_cast<rasqal_variable*>(raptor_sequence_get_at(seq, i));
              if(var==NULL)
                  break;
              if(i>=2)
                  throw std::runtime_error("Only single variable in head is supported");
              topVar=reinterpret_cast<const char*>(var->name);
              if(var->expression!=NULL)
              {
                  if(var->expression->op!=RASQAL_EXPR_COUNT)
                      throw std::runtime_error("Only COUNT operator is supported");
                  if(var->expression->arg1==NULL)
                      throw std::runtime_error("Huh?");
                  if(var->expression->arg1->op!=RASQAL_EXPR_LITERAL)
                      throw std::runtime_error("Unrecognized argument for COUNT");
                  rootVar=varName(var->expression->arg1->literal);
                  count=true;
              }
              else
                  rootVar=topVar;
        }
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
        Node *root=vars[rootVar];
        if(root==NULL)
            throw std::runtime_error("Can not find the root. Check pots.");
        if(root->parent())
            throw std::runtime_error("Root has a parent");
        root->sort();
        return Query(root, topVar, count);
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
