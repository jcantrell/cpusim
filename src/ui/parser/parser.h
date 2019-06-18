#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

enum parse_type {statement_p,load_p,store_p,token_p};
struct parse_node
{
	parse_type p;
	token * t;
	parse_node *children[4];
};

extern bool statement(token **stream, parse_node **out);
extern void parse_walk(parse_node *in);
extern int test();
