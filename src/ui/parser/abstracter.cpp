#include "ui/parser/abstracter.h"

bool abstracter(parse_node* in,ast_node* out)
{
	if (in != NULL)
	{
		switch (in->p) {
			case store_p:
				break;
			case load_p:
				break;
			case statement_p:
				break;
			case token_p:
				switch (in->t->type) {
					case identifier_t:
						out->t = identifier_a;
						out->value = static_cast<int>(strtol(in->t->value, NULL, 16));
						break;
					case value_t:
						//convert value token to int
						out->t = value_a;
						out->value = static_cast<int>(strtol(in->t->value, NULL, 16));
						break;
					case load_p:
						break;
					case whitespace_t:
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
		for (int i=0;i<4;i++)
			if (in->children[i])
				abstracter(in->children[i], NULL  );
	} 
	return false;
}
