/*  This file is the recursive descent parser for the generic CPU commands
 *  Grammar is as follows:
 *
 *  statement -> load_statement | store_statement
 *  load_statement -> load_t identifier_t value_t
 *  store_statement -> store_t identifier_t value_t value_t
 */

#include "parser.h"

bool match(token ** stream, const token * compare,parse_node ** out);
bool statement(token **stream, parse_node **out);
bool load_statement(token **stream, parse_node **out);
bool store_statement(token ** stream, parse_node **out);

/*
 * INPUT:
 *      A string of tokens, s, and a token to be matched in s.
 *
 * OUTPUT:
 *      True/False depending on whether the given token matched the first
 *      token in s.
 *
 * DESCRIPTION:
 *      Determine if token (a string of n characters), matches the first n
 * 			characters in s. If so, advance s pparse the matched characters and 
 * 			return true; otherwise, return false. 
 */
bool match( token** stream, const token* compare, parse_node **out)
{
	printf("matching %s with ",(*stream)->value);
	switch (compare->type)
	{
		case load_t:
			printf("load\n");
			break;
		case store_t:
			printf("store\n");
			break;
		case value_t:
			printf("value\n");
			break;
		case whitespace_t:
			printf("whitespace\n");
			break;
		case identifier_t:
			printf("identifier\n");
			break;
		default:
			printf("other\n");
			break;
	}

	if (*stream != NULL && (*stream)->type == compare->type)
	{
		*out = (parse_node *)malloc(sizeof(parse_node));
		(*out)->p = token_p;
		(*out)->t = (*stream);
		(*out)->children[0] = NULL;
		(*out)->children[1] = NULL;
		(*out)->children[2] = NULL;
		(*out)->children[3] = NULL;
		*stream = (*stream)->next;
		return true;
	}
	return false;
}

bool statement( token **stream, parse_node **out)
{
	token *orig = *stream;
	printf("CALLING STATEMENT\n");


	parse_node *load_parse;
	parse_node *store_parse;

	if (	load_statement(stream,&load_parse) )
	{
		*out = (parse_node*)malloc(sizeof(parse_node));
		(*out)->t = NULL;
		(*out)->p = statement_p;
		(*out)->children[0] = load_parse;
		(*out)->children[1] = NULL;
		(*out)->children[2] = NULL;
		(*out)->children[3] = NULL;
		printf("RETURN TRUE\n");
		return true;
	} else if  (store_statement(stream,&store_parse)) {
		*out = (parse_node*)malloc(sizeof(parse_node));
		(*out)->t = NULL;
		(*out)->p = statement_p;
		(*out)->children[0] = store_parse;
		(*out)->children[1] = NULL;
		(*out)->children[2] = NULL;
		(*out)->children[3] = NULL;
		printf("RETURN TRUE\n");
		return true;
	} else {
		*stream = orig;
		printf("RETURN FALSE\n");
		return false;
	}
}
		
bool load_statement( token **stream, parse_node **out)
{
	printf("CALLING load_statement\n");
	token *orig = *stream;

	token load, identifier, value;
	load.type = load_t;
	identifier.type = identifier_t;
	value.type = value_t;

	parse_node *load_parse;
	parse_node *id_parse;
	parse_node *value_parse;
	if (	match(stream,&load,&load_parse)
		&&	match(stream,&identifier,&id_parse)
		&&	match(stream,&value,&value_parse) )
	{
		*out = (parse_node*)malloc(sizeof(parse_node));
		(*out)->t = NULL;
		(*out)->p = load_p;
		(*out)->children[0] = load_parse;
		(*out)->children[1] = id_parse;
		(*out)->children[2] = value_parse;
		(*out)->children[3] = NULL;
		printf("RETURN TRUE\n");
		return true;
	} else {
		*stream = orig;
		printf("RETURN FALSE\n");
		return false;
	}
}
		
	

bool store_statement( token **stream, parse_node ** out)
{
	printf("CALLING store_statement\n");
	token *orig = *stream;

	token store, identifier, value;
	store.type = store_t;
	identifier.type = identifier_t;
	value.type = value_t;
	

	parse_node *store_parse;
	parse_node *id_parse;
	parse_node *val_1;
	parse_node *val_2;
	if (   match(stream,&store,&store_parse) 
		&& match(stream,&identifier,&id_parse)
		&& match(stream,&value,&val_1) 
		&& match(stream,&value,&val_2 ) )
	{
		*out = (parse_node*)malloc(sizeof(parse_node));
		//(*out)->t = orig;
		(*out)->t = NULL;
		(*out)->p = store_p;
		(*out)->children[0] = store_parse;
		(*out)->children[1] = id_parse;
		(*out)->children[2] = val_1;
		(*out)->children[3] = val_2;
		printf("RETURN TRUE\n");
		return true;
	} else {
		*stream = orig;
		printf("RETURN FALSE\n");
		return false;
	}
}

void parse_walk(parse_node * in)
{
	if (in == NULL)
		return;
	switch (in->p) {
		case store_p:
			printf("Doing store stuff\n");
			break;
		case load_p:
			printf("Doing load stuff\n");
			break;
		case statement_p:
			printf("Doing statement stuff\n");
			break;
		case token_p:
			switch (in->t->type) {
				case identifier_t:
					printf("Doing identifier stuff\n");
					break;
				case value_t:
					printf("Doing value stuff\n");
					break;
			}
			break;
	}
	for (int i=0;i<4;i++)
		if (in->children[i])
			parse_walk(in->children[i]);
}

int test()
{
    char string[] = "store memory 0x661120 0xdeadbeef";
    char *s = string;

	//convert to token stream
    token *p = parse(&s);

	token *j = p;
    while (j!=NULL)
    {
		  printf("%s\n",j->value);
		  j = j->next;
    }

	//convert token stream to parse tree
	parse_node *a;

	if (statement(&p,&a))
		printf("true\n");

	parse_walk(a);
    
    return 1;
}
