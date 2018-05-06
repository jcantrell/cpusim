#include <stdio.h>
#include "parser.h"

int main()
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
