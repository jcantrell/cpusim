#include "TextUI.h"

int TextUI::user_loop()
{
	char input[MAX_STR_LEN];
	char *args[MAX_ARGS];
	int arg_count=0;
	cpu* mycpu = NULL;

	while (1)
	{
	arg_count = 0;

	printf(":");
	fgets(input, MAX_STR_LEN, stdin);

	int j;
	bool in_word = false;

	/* Scan user input for seperate args */
	for (int i=0; input[i]!='\0'; i++)
	{
	bool letter = ((input[i] >= 'a' && input[i] <= 'z')
	    || (input[i] >= '0' && input[i] <= '9'));
	
	//printf("char is %c %d %s %s\n",input[i],input[i],
	//    (in_word?"true":"false"),(letter?"true":"false"));
	

	// If we find the first letter of a word
	if (!in_word && letter)
	{
	    args[arg_count++] = input+i;
 	   //printf("first letter at %d is %c\n",i,input[i]);
	} 
	else if (in_word && !letter)
	{
	//    printf("putting 0 at %d\n",i);
	    input[i] = '\0';
	}
	in_word = letter;
	}

	//printf("last letter is: %c %d len: %d\n", input[j], input[j], j);
	//printf("last letter is: %c %d len: %d\n", 'T', 78, 1);

/*
	printf("args:\n");
	for (int i=0;i<arg_count;i++)
	printf(":%s:\n",args[i]);
	printf("end args\n");
*/

	if (0==strcmp("create", input))
	{
		mycpu = new subleq(8,MAX_MEM_SIZE);
		printf("cpu created\n");
	} 
	else if (0==strcmp("memdump",input)) 
	{
		if (mycpu == NULL)
			printf("No cpu\n");
		else
			mycpu->memdump();
	} 
	else if (0==strcmp("exit",input))
	{
	    return 0;
	}
	else if (0==strcmp("load",args[0]))
	{
	    mycpu->load(atoi(args[1]), atoi(args[2]));
	}
	else if (0==strcmp("view",args[0]))
	{
	    int address1 = atoi(args[1]);
	    int address2 = address1;

	    if (arg_count == 3)
	    {
	        address2 = atoi(args[2]);
	    }

	    int value;
	    for (int i=address1; i<=address2; i++)
	    {
	        value = mycpu->view(i);
	        printf("%x: %x\n", i, value);
	    }
	}
	else if (0==strcmp("viewip",args[0]))
	{
	    printf("%x\n",mycpu->getip());
	}
	else if (0==strcmp("setip",args[0]))
	{
	    mycpu->setip(atoi(args[1]));
	}
	else if (0==strcmp("step",args[0]))
	{
	    int inst = mycpu->view(mycpu->getip());

	}
	else if (0==strcmp("help",args[0]))
	{
		printf("");
	}
	else if (0==strcmp("info",args[0]))
	{
		printf(mycpu->toString().c_str());
	}
	else
	{
		printf("Unrecognized command\n");
		for (int i=0;i<arg_count;i++)
		{
		    printf("    %d :%s:\n", i, args[i]);
		}
		}
	}
	return 0;
}
