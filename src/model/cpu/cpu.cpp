#include "model/cpu/cpu.h"
#include <fstream>

cpu::cpu(int byte_in, Address address_in, unsigned int reg_count)
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

void cpu::step(Morsel inst)
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
// TODO: Properly format cells with leading 0xF
// TODO: Summarize long areas of zeroes
void cpu::memdump()
	{
    char string_out[1000];
    char buffer[1000];
    int zero_count = 0;
    bool print_line = true;
    cout << "address size is " << address_size;
    //for (int i=0; i<address_size;i++)
	    //printf("%02x", (unsigned char)ram[i]);
      Address i;
	    for (i=0; i<address_size;i+=16)
	    {
        //cout << "i is " << i;
          std::string line_out;
          print_line=true;
          for (int j=0;j<1000;j++)
            string_out[j] = 0;
	        sprintf(buffer, "%s: ", i.asString().c_str());
          strcat(string_out,buffer);
          bool line_is_zero = true;
          Address j;
          for (j=0;j<16 && i+j < address_size;j++)
          {
            line_is_zero = line_is_zero && (ram[i+j] == 0);
            if (j % 2 == 0)
            {
              sprintf(buffer,"%s"," ");
              strcat(string_out,buffer);
            }
	          //sprintf(buffer,"%02x", (unsigned char)ram[i+j]);
	          sprintf(buffer,"%s", ram[i+j].asString().c_str());
            strcat(string_out,buffer);
          }
	        sprintf(buffer,"%s","\n");
          strcat(string_out,buffer);

          zero_count = ( line_is_zero ? zero_count+1 : 0);
          print_line = ( (zero_count > 1) ? false : true );

          if (print_line)
            printf("%s",string_out);
	    }
	}

int cpu::loadimage(string filename)
{
  std::cout << "loading file " << filename << std::endl;
  std::ifstream infile(filename, std::ifstream::binary);
  if (!infile)
  {
    printf("No such file\n");
    return 1;
  }  

  infile.seekg(0, infile.end);
  int file_length = infile.tellg();
  std::cout << "file length: " << int(file_length) << std::endl;
  infile.seekg(0, infile.beg);

  if (file_length > address_size)
    file_length = address_size.asInt();
  std::cout << "file length: " << int(file_length) << std::endl;

  char buffer[file_length];
  infile.read(buffer, file_length);
  //Address i;
  int i;
  for (i=0;i<file_length;i++)
  {
    Address i_addr;
    i_addr = i;
    ram[i_addr] = buffer[i];
  }
  return 0;
}

Morsel cpu::load(Address address, Morsel value)
	{
	    Morsel ret = ram[address];
	    ram[address] = value;
	    return ret;
	}

Morsel& cpu::view(Address address)
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
	    ram[dst] = (ram[b] == 0? Morsel(0) : ram[a]/ram[b]);
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

Morsel cpu::regs(Address address)
{
    return registers[address];
}

Morsel cpu::regs(Address address, Morsel value)
{
    registers[address] = value;
    return registers[address];
}
