const int MAX_TOKEN_LEN = 50;
enum node_type {load_t,store_t,value_t,whitespace_t,identifier_t};

struct token
{
    node_type type;
    char value[MAX_TOKEN_LEN];
    int len;
    token* next;
};

class token_stream
{
	token * head;
	token * gobble();
	bool puke(token * bile);
};

extern void token_print(token *t);
extern token *parse(char *s[]);
