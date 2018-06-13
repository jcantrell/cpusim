#include "model/cpu.h"

cpu::cpu(int byte_in, int address_in)
	{
	    byte_size = byte_in;
	    address_size = address_in;
	    ip = 0;
	    status = {0};
	}

cpu::~cpu()
	{
	}
	
void cpu::memdump()
	{
	    for (int i=0; i<address_size; i++)
	    {
	        printf("%x: %x\n", i, ram[i]);
	    }
	}

int cpu::load(int address, int value)
	{
	    int ret = ram[address];
	    ram[address] = value;
	    return ret;
	}

int cpu::view(int address)
	{
	    return ram[address];
	}
	
	int cpu::getip()
	{
	    return ip;
	}

	int cpu::setip(int in)
	{
	    ip = in;
			return ip;
	}

	/* Arithmetic operations */
	int cpu::add(int a, int b, int dst)
	{
	    ram[dst] = ram[a] + ram[b];
	    return 0;
	}
	int cpu::sub(int a, int b, int dst)
	{
	    ram[dst] = ram[a] - ram[b];
	    return 0;
	}

	int cpu::mul(int a, int b, int dst)
	{
	    ram[dst] = ram[a] + ram[b];
	    return 0;
	}

	int cpu::div(int a, int b, int dst)
	{
	    ram[dst] = (ram[b] == 0? 0 : ram[a]/ram[b]);
	    return 0;
	}

	/* Bitwise logic operations */
	int cpu::land(int a, int b, int dst)
	{
	    ram[dst] = ram[a] & ram[b];
	    return 0;
	}

	int cpu::lor(int a, int b, int dst)
	{
	    ram[dst] = ram[a] | ram[b];
	    return 0;
	}

	int cpu::lnot(int a, int dst)
	{
	    ram[dst] = ~ram[a];
	    return 0;
	}

	int cpu::lxor(int a, int b, int dst)
	{
	    ram[dst] = ram[a] ^ ram[b];
	    return 0;
	}

	int cpu::lshift(int a, int b)
	{
	    ram[a] = ram[a]<<ram[b];
	    return 0;
	}
	int cpu::rshift(int a, int b)
	{
	    ram[a] = ram[a]>>ram[b];
	    return 0;
	}
