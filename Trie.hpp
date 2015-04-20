#ifndef TRIE_HPP
#define TRIE_HPP

#include <boost/bimap.hpp>

static std::string::size_type findFirstDifference(const std::string& haystack, const std::string& needle)
{
    auto e=std::min(haystack.length(),needle.length());
    for(std::string::size_type i=0;i<e;++i)
        if(haystack[i]!=needle[i])
            return i;
    return e;
}

class Trie
{
public:
    class Node
    {
    public:
        ~Node()
        {
            for(auto &ch:children)
                delete ch;
        }
        std::string label() const
        {
            std::string result;
            for(const Node *n=this;n->parent;n=n->parent)
                result=n->l+result;
            return result;
        }
    private:
        typedef std::vector<Node*> Children;
        std::string l;
        Children children;
        Node *parent;
        Node(Node *parent)
            :parent(parent)
        {
        }
        Node* insert(const std::string& text)
        {
            if(text.empty())
                return this;
            for(Children::size_type k=0;k<children.size();++k)
            {
                auto child=children[k];
                auto i=findFirstDifference(child->l, text);
                if(i==child->l.length())
                {
                    //etykieta węzła pasuje idealnie
                    return child->insert(text.substr(child->l.length()));
                }
                if(i>0)
                {
//                    std::cout<<"Pierwsze "<<i<<" znaków "<<text<<" pasuje do "<<child->l<<"\n";
                    //pasuje pierwsze i znaków
                    Node *n=new Node(this);
                    n->l=child->l.substr(0,i);
                    child->l=child->l.substr(i);
                    child->parent=n;
                    n->children.push_back(child);
                    children[k]=n;
                    if(text.length()>i)
                    {
                        Node *x=new Node(n);
                        x->l=text.substr(i);
                        n->children.push_back(x);
                        return x;
                    }
                    else
                        return n;
                }
            }
            //nic nie pasuje
            Node *child=new Node(this);
            child->l=text;
            children.push_back(child);
            return child;
        }
        void compress()
        {
            l.shrink_to_fit();
            children.shrink_to_fit();
            for(Node *child:children)
                child->compress();
        }
        void dump(std::ostream& s, const std::string& prefix="") const
        {
            s<<prefix<<l<<"\n";
            for(auto a:children)
            {
                a->dump(s,prefix+std::string(l.length(),' '));
            }
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
    const Node* find(std::string text) const
    {
        Node *n=root;
        while(text!="")
        {
            bool found=false;
            for(Node* ch:n->children)
                if(text.substr(0,ch->l.length())==ch->l)
                {
                    n=ch;
                    text=text.substr(ch->l.length());
                    found=true;
                }
            if(!found)
                return NULL;
        }
        return n;
    }
    void dump(std::ostream& s=std::cout, const std::string& prefix="# ") const
    {
        root->dump(s,prefix);
    }
    const Node* insert(const std::string& text)
    {
        return root->insert(text);
    }
    static void test()
    {
        Trie t;
        std::cout<<t.insert("aaa")->label()<<"\n";
        t.dump();
        std::cout<<t.insert("abc")<<"\n";
        t.dump();
        std::cout<<t.insert("abcc")->label()<<"\n";
        t.dump();
        std::cout<<t.insert("ab")->label()<<"\n";
        t.dump();
        std::cout<<t.insert("abc")<<"\n";
        t.dump();
        std::cout<<t.insert("cda")->label()<<"\n";
        t.dump();
        t.insert("http://dbpedia.org/resource/Autism");
        t.insert("http://dbpedia.org/resource/Aristotle");
        t.insert("http://dbpedia.org/resource/Alabama");
        t.insert("http://dbpedia.org/resource/Abraham_Lincoln");
        t.insert("http://dbpedia.org/resource/Abraham_Lincoln__1");
        t.insert("http://dbpedia.org/resource/Abraham_Lincoln__2");
        t.insert("http://dbpedia.org/resource/Abraham_Lincoln__3");
        t.dump();
    }
    void compress()
    {
        root->compress();
    }
private:
    Node* root;
};

#endif // TRIE_HPP
