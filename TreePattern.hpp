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
	};
}

#endif //TREEPATTERNHPP
