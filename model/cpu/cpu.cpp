#include "model/cpu/cpu.h"
#include <fstream>

cpu::cpu(int byte_in, int address_in, unsigned int reg_count)
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

void cpu::loadobject(string filename)
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
	s += "PF: " + "----------------------------; s += (flagint.flags.pf ? "true" : "false");
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
	
// Print hex dump of memory contents to screen
// TODO: Properly format cells with leading 0xF
// TODO: Summarize long areas of zeroes
void cpu::memdump()
	{
    char string_out[1000];
    char buffer[1000];
    int zero_count = 0;
    bool print_line = true;
	    for (int i=0; i<address_size;i+=16)
	    {
          std::string line_out;
          print_line=true;
          for (int j=0;j<1000;j++)
            string_out[j] = 0;
	        sprintf(buffer, "%x: ", i);
          strcat(string_out,buffer);
          bool line_is_zero = true;
          for (int j=0;j<16 && i+j < address_size;j++)
          {
            line_is_zero = line_is_zero && (ram[i+j] == 0);
            if (j % 2 == 0)
            {
              sprintf(buffer,"%s"," ");
              strcat(string_out,buffer);
            }
	          sprintf(buffer,"%02x", (unsigned char)ram[i+j]);
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
    file_length = address_size;
  std::cout << "file length: " << int(file_length) << std::endl;

  char buffer[file_length];
  infile.read(buffer, file_length);
  for (int i=0;i<file_length;i++)
    ram[i] = buffer[i];
  return 0;
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
    printf("setting ip to %d\n",in);
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

int cpu::regs(int address)
{
    return registers[address];
}

int cpu::regs(int address, int value)
{
    registers[address] = value;
    return registers[address];
}
