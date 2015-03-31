#include "Triple.hpp"
#include <raptor2.h>

static void print_triple(void* user_data, raptor_statement* triple) 
{
	Triples &triples(*static_cast<Triples*>(user_data));
	if(triple->object->type!=RAPTOR_TERM_TYPE_URI)
		return;
	std::string s=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->subject->value.uri, NULL)));
	std::string p=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->predicate->value.uri, NULL)));
	std::string o=std::string(reinterpret_cast<char*>(raptor_uri_as_counted_string(triple->object->value.uri, NULL)));
	triples.push_back(Triple(s,p,o));
}

void LoadTriples(const std::string& file, Triples& result, const std::string& format)
{
	raptor_world *world = NULL;
	raptor_parser* rdf_parser = NULL;
	unsigned char *uri_string;
	raptor_uri *uri, *base_uri;

	world = raptor_new_world();

	rdf_parser = raptor_new_parser(world, format.c_str());

	raptor_parser_set_statement_handler(rdf_parser, &result, print_triple);

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

