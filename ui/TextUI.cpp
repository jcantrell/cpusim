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
	    || (input[i] >= '0' && input[i] <= '9')
      || (input[i] == '/')
      || (input[i] =='.'));
	
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

	if (0==strcmp("create", input))
	{
    if (0==strcmp("subleq", args[1]))
    {
		  mycpu = new subleq(8,MAX_MEM_SIZE);
		  printf("cpu created\n");
    } 
    else if (0==strcmp("mmix", args[1]))
    {
      mycpu = new mmix(8, 65536);
		  printf("cpu created\n");
    }
    else
    {
      continue;
    }
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
		if (mycpu == NULL)
			printf("No cpu\n");
    else
	    mycpu->load(atoi(args[1]), atoi(args[2]));
	}
	else if (0==strcmp("view",args[0]))
	{
		if (mycpu == NULL)
    {
			printf("No cpu\n");
      continue;
    }
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
		if (mycpu == NULL)
			printf("No cpu\n");
    else
	    printf("%x\n",mycpu->getip());
	}
	else if (0==strcmp("setip",args[0]))
	{
		if (mycpu == NULL)
			printf("No cpu\n");
    else
	    mycpu->setip(atoi(args[1]));
	}
	else if (0==strcmp("step",args[0]))
	{
		if (mycpu == NULL)
    {
			printf("No cpu\n");
      continue;
    }

	  int inst = mycpu->view(mycpu->getip());
	  mycpu->step(inst);
	}
	else if (0==strcmp("help",args[0]))
	{
    int command_count = 10;
		string commands[command_count] = {
			"help:    display available commands\n"
			"create <cpu>:  create a new cpu\n"
			"memdump: dump the contents of RAM onto the screen\n"
			"exit:    quit the simulator\n"
			"load <address> <value>: load a value into an address\n"
			"view <address> <address>: view contents of an address range\n"
			"viewip:  view the value of the IP\n"
			"setip:   set the IP\n"
			"step:    execute the current instruction\n"
      "loadimg <path>: load a binary image file\n"
		};
		for (int i=0;i<command_count;i++)
		{
			printf("%s",commands[i].c_str());
		}
	}
	else if (0==strcmp("info",args[0]))
	{
		if (mycpu == NULL)
			printf("No cpu\n");
    else
		  printf("%s",mycpu->toString().c_str());
	}
  else if (0==strcmp("loadimg",args[0]))
  {
		for (int i=0;i<arg_count;i++)
		{
		    printf("    %d :%s:\n", i, args[i]);
		}
		if (mycpu == NULL)
			printf("No cpu\n");
    else
	    mycpu->loadimage(args[1]);
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
		cout << "quitting" << endl;
	return 0;
}
