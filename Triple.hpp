#ifndef TRIPLEHPP
#define TRIPLEHPP

#include <string>
#include <ostream>
#include <deque>
#include <raptor2.h>
#include <dirent.h>

class Triple
{
	public:
		Triple(const std::string& s, const std::string& p, const std::string& o)
			:_s(s),_p(p),_o(o)
		{
		}
		const std::string& s() const
		{
			return _s;
		}
		const std::string& p() const
		{
			return _p;
		}
		const std::string& o() const
		{
			return _o;
		}
	private:
		std::string _s,_p,_o;
};

inline std::ostream& operator<<(std::ostream& o, const Triple& t)
{
	return o<<"(<"<<t.s()<<"> <"<<t.p()<<"> <"<<t.o()<<">)";
}

typedef std::deque<Triple> Triples;

template<typename T>
void ProcessTriple(void* user_data, raptor_statement* triple)
{
    if(triple->object->type!=RAPTOR_TERM_TYPE_URI)
        return;
    std::string s=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->subject->value.uri, NULL)));
    std::string p=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->predicate->value.uri, NULL)));
    std::string o=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->object->value.uri, NULL)));
    (*static_cast<T*>(user_data))(Triple(s,p,o));
}

#include <cassert>

template<typename T>
void LoadTriples(const std::string& file, T& fun, const std::string& format="rdfxml")
{
    raptor_world *world = NULL;
    raptor_parser* rdf_parser = NULL;
    unsigned char *uri_string;
    raptor_uri *uri, *base_uri;

    world = raptor_new_world();

    rdf_parser = raptor_new_parser(world, format.c_str());

    assert(rdf_parser!=NULL);

    raptor_parser_set_statement_handler(rdf_parser, &fun, ProcessTriple<T>);

    uri_string = raptor_uri_filename_to_uri_string(file.c_str());
    uri = raptor_new_uri(world, uri_string);
    base_uri = raptor_uri_copy(uri);

    raptor_parser_parse_file(rdf_parser, uri, base_uri);

    raptor_free_parser(rdf_parser);

    raptor_free_uri(base_uri);
    raptor_free_uri(uri);
    raptor_free_memory(uri_string);

    raptor_free_world(world);
}

template<typename T>
void LoadDir(const std::string& dirname, T fun, const std::string& format)
{
    DIR *d=opendir(dirname.c_str());
    dirent *de;
    while((de=readdir(d))!=NULL)
    {
        std::string file(de->d_name);
        file=dirname+"/"+file;
        if(file.find(".owl")==file.length()-4)
            LoadTriples(file, fun, format);
    }
    closedir(d);
}

#endif //TRIPLEHPP
