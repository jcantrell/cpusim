#include "model/cpu/cpu.h"
#include <fstream>

cpu::cpu(unsigned int byte_in, UnsignedMorsel address_in, unsigned int reg_count) : byte_size(byte_in)
	{
    flagint.i = 42; //magic number 101010
    //byte_size = byte_in;
    address_size = address_in;
    ip = 0;
    //status = {0};
    status = sflags();
	}


void cpu::step(unsigned int inst)
	{
	}

string cpu::toString()
{
	string s = "";
	s += "Byte width: " + to_string(byte_size) + "\n";
	s += "UnsignedMorsel size: " + address_size.asString() + "\n";
	s += "IP: " + ip.asString() + "\n";
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
	s += "-----------------------------------------------\n";
	return s;
}

// Print hex dump of memory contents to screen
void cpu::memdump(std::ostream& os)
{
  struct lineStruct {
    UnsignedMorsel beg;
    UnsignedMorsel end;
  } ls;
  unsigned lineWidth = (128 + byte_size - 1) / byte_size;

  map<UnsignedMorsel, UnsignedMorsel> mymap;
  for (std::pair<UnsignedMorsel, UnsignedMorsel> element : ram)
    mymap[element.first] = element.second;

  std::vector<UnsignedMorsel> line (lineWidth);
  for (UnsignedMorsel& element : line)
    element.resize(byte_size);

  UnsignedMorsel previous;
  previous = mymap.begin()->first;
  for (std::pair<UnsignedMorsel, UnsignedMorsel> element : mymap)
  {
    ls.beg = (previous - previous % (128/byte_size)) ;
    if ( !(previous / 128 == element.first / 128) )
    {
      //os << (previous - previous % (128/byte_size)) << ": ";
      os << ls.beg << ": ";
      for (unsigned int lineIndex=0;lineIndex<lineWidth;lineIndex++)
      {
        os << line[lineIndex].asString();
        if (lineIndex%2==1 && lineIndex!=(lineWidth-1)){os << " ";}
        line[lineIndex] = UnsignedMorsel(0);

      }
      os << endl;
    }
    line[static_cast<unsigned int>((element.first % lineWidth).asInt())] = element.second;
    previous = element.first;
  }
  os << (previous - previous % (128/ byte_size)) << ": ";
  for (unsigned int lineIndex=0;lineIndex<lineWidth;lineIndex++)
  {
    os << line[lineIndex].asString();
    if (lineIndex%2==1 && lineIndex!=(lineWidth-1)) os << " " ;
    line[lineIndex] = UnsignedMorsel(0);
  }
  os << endl;
}

int cpu::loadimage(string filename)
{
  std::ifstream infile(filename, std::ifstream::binary);
  if (!infile)
  {
    printf("No such file\n");
    return 1;
  }  

  unsigned int i = 0;
  unsigned char inchar;
  while (!infile.eof()) {
    infile >> inchar;
    UnsignedMorsel i_addr(i++);
    //i_addr = i;
    ram[i_addr] = UnsignedMorsel(inchar);
  }
  return 0;
}

UnsignedMorsel cpu::load(UnsignedMorsel address, UnsignedMorsel value)
	{
      value.resize(byte_size);
	    UnsignedMorsel ret = ram[address];
	    ram[address] = value;
	    return ret;
	}

UnsignedMorsel& cpu::view(UnsignedMorsel address)
	{
	    return ram[address];
	}
	
	UnsignedMorsel cpu::getip()
	{
	    return ip;
	}

	UnsignedMorsel cpu::setip(UnsignedMorsel in)
	{
	    ip = in;
			return ip;
	}

/*
	int cpu::add(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst)
	{
	    ram[dst] = ram[a] + ram[b];
	    return 0;
	}
	int cpu::sub(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst)
	{
	    ram[dst] = ram[a] - ram[b];
	    return 0;
	}

	int cpu::mul(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst)
	{
	    ram[dst] = ram[a] * ram[b];
	    return 0;
	}

	int cpu::div(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst)
	{
	    ram[dst] = (ram[b] == 0? UnsignedMorsel(0) : ram[a]/ram[b]);
	    return 0;
	}

	int cpu::land(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst)
	{
	    ram[dst] = ram[a] & ram[b];
	    return 0;
	}

	int cpu::lor(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst)
	{
	    ram[dst] = ram[a] | ram[b];
	    return 0;
	}

	int cpu::lnot(UnsignedMorsel a, UnsignedMorsel dst)
	{
	    ram[dst] = ~ram[a];
	    return 0;
	}

	int cpu::lxor(UnsignedMorsel a, UnsignedMorsel b, UnsignedMorsel dst)
	{
	    ram[dst] = ram[a] ^ ram[b];
	    return 0;
	}

	int cpu::lshift(UnsignedMorsel a, UnsignedMorsel b)
	{
	    ram[a] = ram[a]<<ram[b];
	    return 0;
	}
	int cpu::rshift(UnsignedMorsel a, UnsignedMorsel b)
	{
	    ram[a] = ram[a]>>ram[b];
	    return 0;
	}
*/

UnsignedMorsel cpu::regs(UnsignedMorsel address)
{
    return registers[address];
}

UnsignedMorsel cpu::regs(UnsignedMorsel address, UnsignedMorsel value)
{
    registers[address] = value;
    return registers[address];
}
