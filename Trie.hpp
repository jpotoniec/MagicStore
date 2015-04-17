#ifndef TRIE_HPP
#define TRIE_HPP

#include <boost/bimap.hpp>

class Trie
{
public:
    class Node
    {
    public:
        ~Node()
        {
            for(auto &ch:children.left)
                delete ch.second;
        }
        std::string label() const
        {
            std::string result;
            for(const Node *n=this;n->parent;n=n->parent)
                result=n->parent->children.right.find(const_cast<Node*>(n))->second+result;
            return result;
        }
    private:
        typedef boost::bimap<char,Node*> Children;
        Children children;
        Node *parent;
        Node(Node *parent)
            :parent(parent)
        {
        }
        friend class Trie;
    };
    Trie()
    {
        root=new Node(NULL);
    }
    ~Trie()
    {
        delete root;
    }
    const Node* find(const std::string& text) const
    {
        Node *n=root;
        for(char c:text)
        {
            auto i=n->children.left.find(c);
            if(i==n->children.left.end())
                return NULL;
            n=i->second;
        }
        return n;
    }
    const Node* find(const std::string& text)
    {
        Node *n=root;
        for(char c:text)
        {
            Node *next;
            auto i=n->children.left.find(c);
            if(i==n->children.left.end())
            {
                next=new Node(n);
                n->children.insert(Node::Children::value_type(c,next));
            }
            else
                next=i->second;
            n=next;
        }
        return n;
    }
    static void test()
    {
        Trie t;
        std::cout<<t.find("aaa")->label()<<"\n";
        std::cout<<t.find("abc")<<"\n";
        std::cout<<t.find("abcc")->label()<<"\n";
        std::cout<<t.find("ab")->label()<<"\n";
        std::cout<<t.find("abc")<<"\n";
        std::cout<<t.find("cda")->label()<<"\n";

    }
private:
    Node* root;
};

#endif // TRIE_HPP
