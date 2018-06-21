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

  uint64_t a_hi = 0x61C8864680B583EA;
  uint64_t a_lo = 0x1BB32095CCDD51E4;

// quotient
  uint64_t q = a_lo << 1;

  // remainder
  uint64_t rem = a_hi;

  uint64_t carry = a_lo >> 63;
  uint64_t temp_carry = 0;
  int i;

  for(i = 0; i < 64; i++)
  {
    temp_carry = rem >> 63;
    rem <<= 1;
    rem |= carry;
    carry = temp_carry;

    if(carry == 0)
    {
      if(rem >= b)
      {
        carry = 1;
      }
      else
      {
        temp_carry = q >> 63;
        q <<= 1;
        q |= carry;
        carry = temp_carry;
        continue;
      }
    }

    rem -= b;
    rem -= (1 - carry);
    carry = 1;
    temp_carry = q >> 63;
    q <<= 1;
    q |= carry;
    carry = temp_carry;
  }

  printf("Numberator = %llx%llx\n", (long long unsigned int)a_hi, (long long unsigned int)a_lo);
  printf("divisor = %llx\n", (long long unsigned int)b);
  printf("quotient = %llx\n", (long long unsigned int)q);
  printf("remainder = %llx\n", (long long unsigned int)rem);
  return 0;
}
