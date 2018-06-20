#include <stdio.h>

int main()
{
  uint64_t mask = 0x00000000FFFFFFFF;
  uint64_t a = 0xFFFFFFFFFFFFFFFF;
  uint64_t b = 0xFFFFFFFFFFFFFFFF;

/* Multiply two 64-bit unsigned integers
 * by splitting them into 32-bit chunks
 * and using the grid method
 * AB * CD
 * -------------
 * | A*C | B*C |
 * |------------
 * | A*D | B*D |
 * -------------
 */

  uint64_t ac = (a>>32) * (b>>32);
  uint64_t ad = (a&mask) * (b>>32);
  uint64_t bc = (a>>32) * (b&mask);
  uint64_t bd = (a&mask) * (b&mask);
  uint32_t o1 = bd & mask; // o1 = bd % 2^32
  uint64_t t1 = (bd>>32) + (bc&mask) + (ad&mask);
  uint32_t o2 = t1 & mask; // o2 = t1 % 2^32
  uint64_t t2 = (t1>>32) + (bc>>32) + (ad>>32) + (ac&mask);
  uint32_t o3 = t2 & mask;
  uint32_t o4 = (ac>>32) + (t2>>32);
	
  uint64_t carry  = ((((uint64_t) o4)<<32) | o3);
  uint64_t result = ((((uint64_t) o2)<<32) | o1);
  
	
    printf("a: %llu\nb: %llu\n", a, b);
    printf("result: %llu\ncarry: %llu\n", result, carry);
    printf("\n");
    printf("a: %llx\nb: %llx\n", a, b);
    printf("result: %llx\ncarry: %llx\n",  result, carry);
	  return 0;
	}
