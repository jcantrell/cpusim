#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unordered_map>
#include <iostream>

#define MAX_STR_LEN 512
#define MAX_ARGS	32
#define MAX_MEM_SIZE	1024

using namespace std;

struct flags
{
	bool cf;	// Carry flag
	bool pf;	// parity flag
	bool af;	// Auxiliary carry flag
	bool zf;	// Zero flag
	bool sf;	// Sign flag
	bool tf;	// Trap
	bool inf;	// Interrupt
	bool df;	// Direction
	bool of;	// Overflow
};

class cpu {
	private:
	int byte_size;  
	int address_size;
	std::unordered_map<int, int> ram;
	int ip;
	struct flags status;

	public:
		cpu(int byte_in, int address_in);
		~cpu();
		void memdump();
		int load(int address, int value);
		int view(int address);
		int getip();
		int setip(int in);
		int add(int a, int b, int dst);
		int sub(int a, int b, int dst);
		int mul(int a, int b, int dst);
		int div(int a, int b, int dst);
		int land(int a, int b, int dst);
		int lor(int a, int b, int dst);
		int lnot(int a, int dst);
		int lxor(int a, int b, int dst);
		int lshift(int a, int b);
		int rshift(int a, int b);
};
