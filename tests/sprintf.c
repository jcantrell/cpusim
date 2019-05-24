#include <stdio.h>

int
main()
{
  char string_out[1000];
  sprintf(string_out,"%s","hello world");
  printf("%s",string_out);
}
