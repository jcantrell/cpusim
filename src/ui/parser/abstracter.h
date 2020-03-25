#include "ui/parser/parser.h"

enum ast_type {load_a, store_a, identifier_a, value_a};
struct ast_node
{
	ast_type t;
	int value;
};
bool abstracter(parse_node* in,ast_node* out);
