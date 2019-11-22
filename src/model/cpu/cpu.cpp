#include "model/cpu/cpu.h"
#include <fstream>

cpu::cpu(unsigned int byte_in, Address address_in, unsigned int reg_count) : byte_size(byte_in)
	{
    flagint.i = 42; //magic number 101010
    //byte_size = byte_in;
    address_size = address_in;
    ip = 0;
    //status = {0};
    status = sflags();
	}

cpu::~cpu()
	{
	}

void cpu::step(unsigned int inst)
	{
	}

void cpu::loadobject(string filename)
{
}

string cpu::toString()
{
	string s = "";
	s += "Byte width: " + to_string(byte_size) + "\n";
	s += "Address size: " + address_size.asString() + "\n";
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
  unsigned lineWidth = (128 + byte_size - 1) / byte_size;

  map<Address, UnsignedMorsel> mymap;
  for (std::pair<Address, UnsignedMorsel> element : ram)
    mymap[element.first] = element.second;

  std::vector<UnsignedMorsel> line (lineWidth);
  for (UnsignedMorsel& element : line)
    element.resize(byte_size);

  Address previous;
  previous = mymap.begin()->first;
  for (std::pair<Address, UnsignedMorsel> element : mymap)
  {
    if ( !(previous / 128 == element.first / 128) )
    {
      os << (previous - previous % (128/byte_size)) << ": ";
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

  /*
  infile.seekg(0, infile.end);
  unsigned int file_length = infile.tellg();
  std::cout << "file length: " << int(file_length) << std::endl;
  infile.seekg(0, infile.beg);

  if (file_length > address_size)
    file_length = address_size.asInt();
  */
  //std::cout << "file length: " << int(file_length) << std::endl;

  //char buffer[file_length];
  //infile.read(buffer, file_length);
  //Address i;
  unsigned int i = 0;
  unsigned char inchar;
  while (!infile.eof()) {
    infile >> inchar;
    Address i_addr(i++);
    //i_addr = i;
    ram[i_addr] = UnsignedMorsel(inchar);
  }
  return 0;
}

UnsignedMorsel cpu::load(Address address, UnsignedMorsel value)
	{
      value.resize(byte_size);
	    UnsignedMorsel ret = ram[address];
	    ram[address] = value;
	    return ret;
	}

UnsignedMorsel& cpu::view(Address address)
	{
	    return ram[address];
	}
	
	Address cpu::getip()
	{
	    return ip;
	}

	Address cpu::setip(Address in)
	{
	    ip = in;
			return ip;
	}

	/* Arithmetic operations */
	int cpu::add(Address a, Address b, Address dst)
	{
	    ram[dst] = ram[a] + ram[b];
	    return 0;
	}
	int cpu::sub(Address a, Address b, Address dst)
	{
	    ram[dst] = ram[a] - ram[b];
	    return 0;
	}

	int cpu::mul(Address a, Address b, Address dst)
	{
	    ram[dst] = ram[a] + ram[b];
	    return 0;
	}

	int cpu::div(Address a, Address b, Address dst)
	{
	    ram[dst] = (ram[b] == 0? UnsignedMorsel(0) : ram[a]/ram[b]);
	    return 0;
	}

	/* Bitwise logic operations */
	int cpu::land(Address a, Address b, Address dst)
	{
	    ram[dst] = ram[a] & ram[b];
	    return 0;
	}

	int cpu::lor(Address a, Address b, Address dst)
	{
	    ram[dst] = ram[a] | ram[b];
	    return 0;
	}

	int cpu::lnot(Address a, Address dst)
	{
	    ram[dst] = ~ram[a];
	    return 0;
	}

	int cpu::lxor(Address a, Address b, Address dst)
	{
	    ram[dst] = ram[a] ^ ram[b];
	    return 0;
	}

	int cpu::lshift(Address a, Address b)
	{
	    ram[a] = ram[a]<<ram[b];
	    return 0;
	}
	int cpu::rshift(Address a, Address b)
	{
	    ram[a] = ram[a]>>ram[b];
	    return 0;
	}

UnsignedMorsel cpu::regs(Address address)
{
    return registers[address];
}

UnsignedMorsel cpu::regs(Address address, UnsignedMorsel value)
{
    registers[address] = value;
    return registers[address];
}
