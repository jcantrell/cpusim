enum ast_type {load_a, store_a, identifier_a, value_a};
struct ast_node
{
	ast_type t;
	int value;
};
