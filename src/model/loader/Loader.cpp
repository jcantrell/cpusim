#include "model/cpu/Address.h"
#include "model/cpu/cpu.h"
#include <iomanip>
#include <fstream>
#include "model/loader/Loader.h"

//Loader::Loader() {}
void
Loader::quote()
{
  quoted_flag = true;
}

void
Loader::loc(unsigned char y, unsigned char z)
{
          //TODO: load tetra.ar[2] (y) into appropriate byte of address
          address = (Address(y).resize(64) << 56);
          Address yaddr(0xff);
          Address post;
          post = yaddr<<4;
          Address offset; offset = 0;
          union tetra_union addr1;
          union tetra_union addr2;
          in.read(addr1.ar,4);
          //offset = tetra1.num; // reverse this (little-endian)
          offset = static_cast<uint64_t>(
              (static_cast<uint64_t>(addr1.ar[0])<<24) 
            | (static_cast<uint64_t>(addr1.ar[1])<<16) 
            | (static_cast<uint64_t>(addr1.ar[2])<<8)
            | (static_cast<uint64_t>(addr1.ar[3]))
          );

          if (z==2)
          {
            offset = offset<<32;
            //in >> tetra.num;
            in.read(addr2.ar,4);
          offset = offset & static_cast<uint64_t>(
              (static_cast<uint64_t>(addr1.ar[0])<<24) 
            | (static_cast<uint64_t>(addr1.ar[1])<<16) 
            | (static_cast<uint64_t>(addr1.ar[2])<<8)
            | (static_cast<uint64_t>(addr1.ar[3]))
          );
          }
          lambda = address | offset;
}

void
Loader::skip(unsigned char y, unsigned char z)
{
          lambda += ((y<<8) & z);
}

void
Loader::fixo(cpu& c, unsigned char y, unsigned char z)
{
          Address a;
          a = 0;
          union tetra_union tetra;
          for (;z!=0;z--)
          {
            in >> tetra.num;
            a = a&(static_cast<uint64_t>(tetra.num)<<32);
          }
          a += ((static_cast<uint64_t>(y))<<56);
          //M(8, a, lambda.asUnsignedMorsel());
          c.load(a,lambda.asUnsignedMorsel());
}

void
Loader::fixr(cpu& mycpu, unsigned char y, unsigned char z)
{
          Address delta;
          delta = y;
          delta = (delta<<32)&z;
          //uint64_t delta = ((uint64_t)y<<32)&z;
          address = lambda - static_cast<unsigned int>(4)*delta;
          //cout << "case lop_fixr\n";
          mycpu.load(address+2,y);
          mycpu.load(address+3,z);
}

void
Loader::fixrx(cpu& c, unsigned char z)
{
          union {
            uint64_t num;
            char ch[8];
          } delta;
          in >> delta.num;
          delta.num = 
            ( (delta.ch[0] == 1) ? ((delta.num ^ 0x00FFFFFF)-(static_cast<uint64_t>(1)<<static_cast<uint64_t>(z))) : delta.num );
          Address P;
          P = lambda - 4*delta.num; 
          //M(8, P, lambda.asUnsignedMorsel()); 
          c.load(P, lambda.asUnsignedMorsel()); 
}

void
Loader::file(tetra_union& tetra, unsigned char z)
{
          //Y = file number, Z = tetra count of bytes of filename
          for (;z>0;z--) {
            in.read(tetra.ar,4);
          }
}

void
Loader::line()
{
          //YZ = line number
}

void
Loader::spec()
{
          //YZ = type. Subsequent tetras, until next loader operation
          //other than lop_quote, comprise the special data.
}

void
Loader::pre(tetra_union& tetra, unsigned char z)
{
  //Y = mmo format version (currently 1)
  //Z = # of subsequent tetras providing useful info
  //    if Z>0, the first tetra is timestamp of file creation
  for (;z > 0;z--) {
    in.read(tetra.ar,4);
  }
}

void
Loader::post(cpu& mycpu,unsigned char z)
{
  //load rG with value of Z (must be >= 32)
  //$G,$G+1,...,$255 set to values of next (256-G)*2 tetras
  union tetra_union t1;
  union tetra_union t2;
  //special_registers[rG] = z;
  UnsignedMorsel rG(6);
  mycpu.regs(Address(rG),UnsignedMorsel(z)); // TODO: this should be rG
  for (unsigned int i=0;i<=(256-mycpu.regs(rG))*2u;i+=2)
  {
    in.read(t1.ar,4);
    in.read(t2.ar,4);
    //R(UnsignedMorsel(z+static_cast<unsigned int>(i)), UnsignedMorsel(t1.num)<<32 & UnsignedMorsel(t2.num));
    mycpu.regs(UnsignedMorsel(z+static_cast<unsigned int>(i)), UnsignedMorsel(t1.num)<<32 & UnsignedMorsel(t2.num));
  }
}

void
Loader::stab()
{
          // indicates start of user-defined symbols
          cout << "lop_stab called\n";
}

void
Loader::end()
{
  // indicates end of user-defined symbols (and end of mmo file)
}

void
Loader::loadobject(cpu& mycpu, string filename)
{
  std::cout << "loading file " << filename << std::endl;
  //std::ifstream in(filename, std::ifstream::binary);
  in = std::ifstream(filename, std::ifstream::binary);
  if (!in)
  {
    printf("No such file\n");
    return;
  }  

  union tetra_union tetra;
  unsigned char y,z;

  while (!in.eof())
  {
    in.read(tetra.ar,4);
    std::cout << "Tetra: " 
              << std::hex << setfill('0') << std::setw(2)
              << static_cast<int>(static_cast<unsigned char>(tetra.ar[0]))
              << std::hex << setfill('0') << std::setw(2)
              << static_cast<int>(static_cast<unsigned char>(tetra.ar[1]))
              << " " 
              << std::hex << setfill('0') << std::setw(2)
              << static_cast<int>(static_cast<unsigned char>(tetra.ar[2]))
              << std::hex << setfill('0') << std::setw(2)
              << static_cast<int>(static_cast<unsigned char>(tetra.ar[3]))
              << "\n";
    
    //op = tetra.ar[0];
    //x = tetra.ar[1];
    y = static_cast<unsigned char>(tetra.ar[2]);
    z = static_cast<unsigned char>(tetra.ar[3]);

    if ((static_cast<char>(mmo_escape) == tetra.ar[0]) && !quoted_flag)
	  {
      switch (tetra.ar[1])
      {
        case lop_quote:
          quote();
          break;

        case lop_loc:
          loc(y,z);
          break;

        case lop_skip: 
          skip(y,z);
          break;

        case lop_fixo:
          fixo(mycpu, y,z);
          break;

        case lop_fixr:
          fixr(mycpu, y, z);
          break;

        case lop_fixrx:
          fixrx(mycpu,z);
          break;

        case lop_file:
          file(tetra, z);
          break;

        case lop_line:
          line();
          break;

        case lop_spec:
          spec();
          break;

        case lop_pre:
          pre(tetra, z);
          break;

        case lop_post:
          post(mycpu, z);
          break;

        case lop_stab:
          stab();
          break;

        case lop_end:
          end();
          break;

        default:
          //error: invalid loader command
          cout << "Invalid loader op: " << tetra.ar[1];
          return;
      }
    } else {
      quoted_flag = false;
      //M(4, lambda, tetra.num);
      mycpu.regs(lambda, static_cast<long long unsigned int>(tetra.ar[0]));
      mycpu.regs(lambda, static_cast<long long unsigned int>(tetra.ar[1]));
      mycpu.regs(lambda, static_cast<long long unsigned int>(tetra.ar[2]));
      mycpu.regs(lambda, static_cast<long long unsigned int>(tetra.ar[3]));
      lambda = (lambda/4)*4+4;
	  }
  }
	
  in.close();
  cout << "dumping ram" << endl;
  UnsignedMorsel m1 = mycpu.view(Address(256));
  cout << "m1: " << m1 << endl;
  mycpu.memdump();
  return;
}
