#ifndef CPU_H
#define CPU_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unordered_map>
#include <iostream>
#include <string>

#define MAX_STR_LEN 512
#define MAX_ARGS	32
#define MAX_MEM_SIZE	1024

using namespace std;

struct flags
{
	unsigned
	cf:1,	// Carry flag
	pf:1,	// parity flag
	af:1,	// Auxiliary carry flag
	zf:1,	// Zero flag
	sf:1,	// Sign flag
	tf:1,	// Trap
	inf:1,	// Interrupt
	df:1,	// Direction
	of:1;	// Overflow
};
union flagsint {
	flags flags;
	unsigned i;
};

class cpu {
	private:
	int byte_size;  
	int address_size;
	std::unordered_map<int, int> ram;
	int ip;
	struct flags status;
	union flagsint flagint;

	public:
		cpu(int byte_in, int address_in);
		~cpu();

		void memdump();
		int load(int address, int value);
		int view(int address);
		int getip();
		int setip(int in);
		string toString();
		virtual void step(int inst);

// CPU instructions
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
#endif
