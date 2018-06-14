#include "ui/TextUI.h"
#include "ui/parser/parser.h"

int main()
{
	//TextUI t;
	//t.user_loop();
	//test();
	TextUI u;
	UI* u2 = new TextUI;
	((TextUI*) u2)->user_loop();
	return 0;
}
