#include <stdio.h>
#include <stdint.h>
#include <iomanip>
#include <iostream>

using namespace std;

int main() {
  uint64_t testval;
  char c1 = 0x01;
  char c2 = 0x02;
  char c3 = 0x03;
  char c4 = 0x04;
  char c5 = 0x05;
  char c6 = 0x06;
  char c7 = 0x07;
  char c8 = 0x08;
  testval = (
    (((uint64_t)c1)<<56) |
    (((uint64_t)c2)<<48) |
    (((uint64_t)c3)<<40) |
    (((uint64_t)c4)<<32) |
    (((uint64_t)c5)<<24) |
    (((uint64_t)c6)<<16) |
    (((uint64_t)c7)<<8) |
    (((uint64_t)c8))
  );
  uint64_t testval2 = 0xDEADBEEFCAFEBABE;
  unsigned long long testval3 = testval2;
  printf("testval: %lx\n", testval);
  printf("testval2: %lx\n", testval2);
  printf("testval3: %llx\n", testval3);

  cout << "testval is "
            << std::hex << setfill('0') << std::setw(16)
            << (unsigned long long)(testval);

  return 0;
}
