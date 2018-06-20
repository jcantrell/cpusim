#include <stdio.h>

int main()
{
  
  uint64_t b = 0x9E3779B97F4A7C16;

  uint16_t dividend[8];
   dividend[0]= 0x61C8;
   dividend[1]= 0x8646;
   dividend[2]= 0x80B5;
   dividend[3]= 0x83EA;
   dividend[4]= 0x1BB3;
   dividend[5]= 0x2095;
   dividend[6]= 0xCCDD;
   dividend[7]= 0x51E4;

  uint64_t divisor = b;

  unsigned int i = 0;
  uint64_t remainder = 0x0;
  uint16_t out[8];

  for (i=0;i<8;i++)
  {
    printf("r[%d] = %llx\n", i, remainder);
    printf("out[%d] = %llx\n", i, ((remainder << 16) + dividend[i])/divisor);
    out[i] = ((remainder << 16) + dividend[i]) / divisor;
    remainder = ((remainder << 16) + dividend[i]) % divisor;
  }

  printf("Out: %u%u%u%u%u%u%u%u\n", 
      out[0], out[1], out[2], out[3],
      out[4], out[5], out[6], out[7]);
  printf("Hex: %x%x%x%x%x%x%x%x\n", 
      out[0], out[1], out[2], out[3],
      out[4], out[5], out[6], out[7]);
  printf("Hex: %x%x%x%x%x%x%x%x\n", 
    dividend[0],
    dividend[1],
    dividend[2],
    dividend[3],
    dividend[4],
    dividend[5],
    dividend[6],
    dividend[7]
  );
  return 0;
}
