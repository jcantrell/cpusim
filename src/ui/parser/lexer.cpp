/*  This is the lexer for cpusim
 *  Keywords:
 *      load:load
 *      store:store
 *  Identifiers:
 *      Any string not a keyword or hex value
 *      Built ins: memory, registers
 *  Values:
 *      Any (0 or more) hex digits, lower or uppercase, preceded by "0x"
 *
 *  load -> "load" 
 *  store -> "store"
 *  value -> 0x[0-9a-fA-F]+
 *  whitespace -> [' ''\t']+
 *
 *  step 1: convert from string to list of tokens
 *  step 2: eliminate whitespace tokens
 *  
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

/*
const int MAX_TOKEN_LEN = 50;
enum node_type {load_t,store_t,value_t,whitespace_t};

struct token 
{
    node_type type;
    char value[MAX_TOKEN_LEN];
    int len;
    token* next;
};
*/

//void token_print(token *t);
bool match(char *s[], const char* in);
bool load(char *s[], token* t);
bool store(char *s[], token* t);
bool whitespace(char *s[], token* t);
bool value(char *s[],token* t);
bool identifier(char *s[], token* t);
//token *parse(char *s[]);

void token_print(token* t)
{
    printf("Token:\n");
    switch (t->type)
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
            break;
    }
    printf(":%s:\n",t->value);
    printf("%d\n",t->len);
    printf("\n");
}

bool match(char *s[], const char* in)
{
    size_t l = strlen(in);
    if (strncmp(*s,in,l) == 0)
    {
        *s = *s + l;
        return true;
    }
    return false;
}

bool load(char *s[], token* t)
{
    char *orig = *s;
    t->type = load_t;
    t->len = 0;

    if (match(s,"load"))
    {
        t->len = 4;
        strcpy(t->value,"load");
        return true;
    }
    return false;
}

bool store(char *s[], token *t)
{
    char *orig = *s;
    t->type = store_t;
    t->len = 0;
    if (match(s,"store"))
    {
        t->len = 5;
        strcpy(t->value,"store");
        return true;
    }
    return false;
}


/*
 * INPUT: string, token to be loaded
 * OUTPUT:  bool indicating whether whitespace was found;
 *      if so, return whitespace_t token containing whitespace
 */
bool whitespace(char *s[], token* t)
{
    char *orig = *s;
    t->type = whitespace_t;
    t->len = 0;
    bool flag = false;

    while (match(s," ") || match(s,"\t"))
    {
        flag = true;
    }

    if (!flag) 
    {
        *s = orig;
        return false;
    }

    //load token with whitespace
    char *i = orig;
    while (flag && i != *s)
    {
        t->value[t->len] = *i;
        t->len = t->len + 1;
        i++;
    }
    
    return true;
}

bool identifier(char *s[], token *t)
{
	char *orig = *s;
	t->type = identifier_t;
	t->len = 0;
	bool flag = false;

	while (match(s,"m") || match(s,"e") || match(s,"o") || match(s,"r")
			|| match(s,"y") || match(s,"g") || match(s,"i") || match(s,"s")
			|| match(s,"t")
	)
	{
		flag = true;
	}
	
	if (!flag)
	{
		*s = orig;
		return false;
	}

	char *i = orig;
	while (flag && i != *s)
	{
		t->value[t->len] = *i;
		t->len = t->len + 1;
		i++;
	}
	return true;
}
	
bool value(char *s[], token* t)
{
    char *orig = *s;
    t->type = value_t;
    t->len = 0;
    bool flag = false;

    if (match(s,"0x"))
    {
        while (
            match(s,"0") || match(s,"1") || match(s,"2") || match(s,"3") || 
            match(s,"4") || match(s,"5") || match(s,"6") || match(s,"7") || 
            match(s,"8") || match(s,"9") || match(s,"a") || match(s,"b") || 
            match(s,"c") || match(s,"d") || match(s,"e") || match(s,"f") || 
            match(s,"A") || match(s,"B") || match(s,"C") || match(s,"D") || 
            match(s,"E") || match(s,"F")
        )
        {
            flag = true;
        }
    }
    if (!flag)
    {
        *s = orig;
        return false;
    }

    //load token with value string
    char *i = orig;
    while (flag && i != *s)
    {
        t->value[t->len] = *i;
        t->len = t->len + 1;
        i++;
    }
	return true;
}

/*	INPUT: string to be parsed
 *	OUTPUT: linked list of tokens
 */
token *parse(char *s[])
{
    bool flag = true;
    bool flag2 = true;
    token *head = static_cast<token*>(malloc( sizeof(token) ));
    token *c = head;
    c->next = NULL;

    while (flag)
    {
        if (	whitespace(s,c) 
			|| 	load(s,c) 
			|| 	store(s,c) 
			|| 	value(s,c)
			||	identifier(s,c))
        {
            c->next = static_cast<token*>(malloc(sizeof(token)));
            c = c->next;
            c->next = NULL;
            flag2 = true;
        } else {
            flag = false;
        }
    }
    
    if (flag2)
    {
    /* We created an extra token on the end of the list which doesn't end
        up getting used - now we delete that token */
        c = head;
        while (c->next != NULL && c->next->next != NULL)
            c = c->next;
        if (c->next != NULL)
        {
            free(c->next);
            c->next = NULL;
        }

		// Now strip out whitespace_t's because most languages
		// don't care about whitespace
		c = head;
		while (c != NULL)
		{
			if (c->next != NULL && c->next->type == whitespace_t)
				c->next = c->next->next;
			c = c->next;	
		}
        return head;
    } else {
        free(head);
        return NULL;
    }
}
