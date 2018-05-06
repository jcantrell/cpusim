struct id_node
{
	char * value;
	int index;

	id_node *children[2];
};

void insert(id_node *tree, char *id)
{
	static int leaf_count = 0;
	if (strcmp(tree->value, id))
	{
		if (children[0] = NULL)
		{
			children[0] = (id_node*)malloc(sizeof(id_node));
			children[0]->value = id;
			children[0]->index = leaf_count;
	
}

bool retrieve()
{
}
