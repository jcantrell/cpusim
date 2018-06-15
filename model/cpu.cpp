#include "model/cpu.h"

cpu::cpu(int byte_in, int address_in)
	{
		flagint.i = 42; //magic number 101010
	    byte_size = byte_in;
	    address_size = address_in;
	    ip = 0;
	    status = {0};
	}

cpu::~cpu()
	{
	}

void cpu::step(int inst)
	{
	}

string cpu::toString()
{
	string s = "";
	s += "Byte width: " + to_string(byte_size) + "\n";
	s += "Address size: " + to_string(address_size) + "\n";
	s += "IP: " + to_string(ip) + "\n";
	s += "Flags: " + to_string(flagint.i) + "\n";

	s += "-----------------------------------------------\n";
	s += "| OF | DF | INF | TF | SF | ZF | AF | PF | CF |\n";
	s += "-----------------------------------------------\n";

	s +=  "|  " + string(flagint.flags.of ? "1" : "0");
	s += " |  " + string(flagint.flags.df ? "1" : "0");
	s += " |   " + string(flagint.flags.inf ? "1" : "0");
	s += " |  " + string(flagint.flags.tf ? "1" : "0");
	s += " |  " + string(flagint.flags.sf ? "1" : "0");
	s += " |  " + string(flagint.flags.zf ? "1" : "0");
	s += " |  " + string(flagint.flags.af ? "1" : "0");
	s += " |  " + string(flagint.flags.pf ? "1" : "0");
	s += " |  " + string(flagint.flags.cf ? "1" : "0");
	s += " |\n";
	//s += " | " + DF | INF | TF | SF | ZF | AF | PF | CF |\n";
	s += "-----------------------------------------------\n";

/*
	s += "CF: ";
	s += (flagint.flags.cf ? "true" : "false");
	s += "\n";
	s += "PF: " + "----------------------------;
	s += (flagint.flags.pf ? "true" : "false");
	s += "\n";
	s += "AF: "; 
	s += (flagint.flags.af ? "true" : "false");
	s += "\n";
	s += "ZF: ";
	s += (flagint.flags.zf ? "true" : "false");
	s += "\n";
	s += "SF: "; 
	s += (flagint.flags.sf ? "true" : "false");
	s += "\n";
	s += "TF: "; 
	s += (flagint.flags.tf ? "true" : "false");
	s += "\n";
	s += "INF: "; 
	s += (flagint.flags.inf ? "true" : "false");
	s += "\n";
	s += "DF: "; 
	s += (flagint.flags.df ? "true" : "false"); 
	s += "\n";
	s += "OF: ";
	s += (flagint.flags.of ? "true" : "false"); 
	s += "\n";
*/
	return s;
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
