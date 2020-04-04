#ifndef CPU_H
#define CPU_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <unordered_map>
#include <iostream>
#include <string>
#include "model/cpu/UnsignedMorsel.h"
#include <boost/unordered_map.hpp>

#define MAX_STR_LEN 512
#define MAX_ARGS	32
#define MAX_MEM_SIZE	1024

using namespace std;

struct sflags
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
	sflags flags;
	unsigned i;
};


class cpu {
	private:
		UnsignedMorsel address_size;
		boost::unordered_map<UnsignedMorsel, UnsignedMorsel> ram;
		boost::unordered_map<UnsignedMorsel, UnsignedMorsel> registers;
		UnsignedMorsel ip;
		struct sflags status;
		union flagsint flagint;

	public:
		const unsigned int byte_size;  
		cpu(unsigned int byte_in, UnsignedMorsel address_in, unsigned int reg_count);
		//~cpu();

// Control methods
		void memdump(std::ostream& os = std::cout);
		UnsignedMorsel load(UnsignedMorsel address, UnsignedMorsel value);
		UnsignedMorsel& view(UnsignedMorsel address);
		UnsignedMorsel getip();
		UnsignedMorsel setip(UnsignedMorsel in);
		string toString();
		virtual void step(unsigned int inst);
    UnsignedMorsel regs(UnsignedMorsel address);
    UnsignedMorsel regs(UnsignedMorsel address, UnsignedMorsel value);
    int loadimage(string filename);
    //virtual void loadobject(string filename) = 0;

// CPU instructions
/*
		int add(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst);
		int sub(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst);
		int mul(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst);
		int div(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst);
		int land(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst);
		int lor(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst);
		int lnot(UnsignedMorsel a, UnsignedMorsel dst);
		int lxor(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst);
		int lshift(UnsignedMorsel a, UnsignedMorsel b);
		int rshift(UnsignedMorsel a, UnsignedMorsel b);
*/
};
#endif
