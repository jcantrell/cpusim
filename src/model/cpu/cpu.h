#ifndef CPU_H
#define CPU_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <unordered_map>
#include <iostream>
#include <string>
#include "model/cpu/Address.h"
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
		Address address_size;
		boost::unordered_map<Address, UnsignedMorsel> ram;
		boost::unordered_map<Address, UnsignedMorsel> registers;
		Address ip;
		struct sflags status;
		union flagsint flagint;

	public:
		const unsigned int byte_size;  
		cpu(int byte_in, Address address_in, unsigned int reg_count);
		virtual ~cpu() = 0;

// Control methods
		void memdump(std::ostream& os = std::cout);
		UnsignedMorsel load(Address address, UnsignedMorsel value);
		UnsignedMorsel& view(Address address);
		Address getip();
		Address setip(Address in);
		string toString();
		virtual void step(UnsignedMorsel inst);
    UnsignedMorsel regs(Address address);
    UnsignedMorsel regs(Address address, UnsignedMorsel value);
    int loadimage(string filename);
    virtual void loadobject(string filename);

// CPU instructions
		int add(Address a, Address b, Address dst);
		int sub(Address a, Address b, Address dst);
		int mul(Address a, Address b, Address dst);
		int div(Address a, Address b, Address dst);
		int land(Address a, Address b, Address dst);
		int lor(Address a, Address b, Address dst);
		int lnot(Address a, Address dst);
		int lxor(Address a, Address b, Address dst);
		int lshift(Address a, Address b);
		int rshift(Address a, Address b);
};
#endif
