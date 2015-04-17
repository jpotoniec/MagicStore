#ifndef TREEPATTERNHPP
#define TREEPATTERNHPP

#include "Triple.hpp"
#include <string>
#include <deque>
#include <algorithm>
#include <cassert>
#include <iosfwd>

namespace TreePattern
{
	class Node
	{
		public:
            typedef std::pair<std::string,Node*> Child;
            typedef std::deque<Child> Children;
			~Node()
			{
				for(auto child:_children)
					delete child.second;
			}
			const std::string& label() const
			{
				return _label;
			}
            bool isDefined() const
            {
                return defined;
            }
			const Node* parent() const
			{
				return _parent;
			}
            const std::string& parentProperty() const
            {
                return _parentProperty;
            }
			const Children& children() const
			{
				return _children;
			}
			bool isRoot() const
			{
                return parent()==NULL;
            }
			void dump(std::ostream& o, const std::string& prefix="") const;
            void sort()
            {
                for(auto ch:_children)
                    ch.second->sort();
                std::sort(_children.begin(), _children.end(), [](const std::pair<std::string,Node*>& a, const std::pair<std::string,Node*>& b)->bool{return a.second->height()>b.second->height();});
            }
		private:
            explicit Node(const std::string& label, bool defined)
                :_parent(NULL),_label(label),defined(defined)
			{
			}
			void parent(Node *parent, const std::string& property)
			{
				assert(parent!=NULL);
				assert(_parent==NULL);
				_parent=parent;
				_parent->_children.push_back(std::make_pair(property,this));
				_parentProperty=property;
			}
            size_t height() const
            {
                if(children().empty())
                    return 0;
                //jeżeli dzieci są posortowane od najwyższego, to nie trzeba przeglądać wszystkich
                return _children[0].second->height()+1;
            }
			Node *_parent;
			std::string _parentProperty;
			Children _children;
			std::string _label;
            bool defined;
            friend class Query;
	};

    class Query
    {
    public:
        static Query fromSPARQL(const std::string& sparql);
        const Node* where() const
        {
            return _where;
        }
        const std::string variable() const
        {
            return _var;
        }
        bool isCount() const
        {
            return _count;
        }
        ~Query()
        {
            delete _where;
        }
    private:
        Query(Node *where, const std::string& var, bool count)
            :_where(where),_var(var),_count(count)
        {
        }
        Node *_where;
        std::string _var;
        bool _count;
    };
}

#endif //TREEPATTERNHPP
