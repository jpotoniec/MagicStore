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
			typedef std::deque<std::pair<std::string,Node*>> Children;
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
                return _label[0]!='?';
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
			static Node* fromTriples(const std::deque<Triple>& triples);
			void dump(std::ostream& o, const std::string& prefix="") const;
            void sort()
            {
                for(auto ch:_children)
                    ch.second->sort();
                std::sort(_children.begin(), _children.end(), [](const std::pair<std::string,Node*>& a, const std::pair<std::string,Node*>& b)->bool{return a.second->height()>b.second->height();});
            }
		private:
			explicit Node(const std::string& label)
				:_parent(NULL),_label(label)
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
			Node *_parent;
			std::string _parentProperty;
			Children _children;
			std::string _label;
            size_t height() const
            {
                if(_children.empty())
                    return 0;
                else
                    return _children[0].second->height()+1;
            }
	};
}

#endif //TREEPATTERNHPP
