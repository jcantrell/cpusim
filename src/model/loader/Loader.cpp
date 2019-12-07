#include "model/cpu/Address.h"
#include "model/cpu/cpu.h"
#include <iomanip>
#include <fstream>
#include "model/loader/Loader.h"

void
Loader::quote(unsigned char y, unsigned char z)
{
  if ((Address(y).pb(8) | Address(z)) == Address(1))
    quoted_flag = true;
}

void
Loader::loc(unsigned char y, unsigned char z)
{
  Address address(Address(y).pb(56));
  Address offset(0);
  char c;
    cout << "loc: ";
  for (unsigned i=0; i<4u*z; i++) 
  {
    in.get(c); offset.pb(8); offset = offset | Address(static_cast<unsigned char>(c));
    cout << std::hex << setfill('0') << std::setw(2)
    << static_cast<int>(static_cast<unsigned char>(c));
  }
  lambda = address + offset;
}

void
Loader::skip(unsigned char y, unsigned char z)
{
  Address address(Address(y).pb(56));
  //UnsignedMorsel t;
  //t = y;
  //t.resize(64);
  //t = (t<<8);
  lambda += ((y<<8) | z);
}

void
Loader::fixo(cpu& c, unsigned char y, unsigned char z)
{
          Address a;
          a = 0;
          union tetra_union tetra;
          for (;z!=0;z--)
          {
            //in >> tetra.num;
            in.read(tetra.ar,4);
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
          Address address;
          //address = lambda - static_cast<unsigned int>(4)*delta;
          address = lambda - 4u*delta;
          //cout << "case lop_fixr\n";
          mycpu.load(address+2,y);
          mycpu.load(address+3,z);
}

void
Loader::fixrx(cpu& c, unsigned char y, unsigned char z)
{
  Address sigma(0);
  char first;
  char d;
  for (unsigned i=0; i<4u; i++) 
  {
    in.get(d); sigma.pb(8); sigma = sigma | Address(static_cast<unsigned char>(d));
    if (i==0) first = d;
  }
  sigma.resize(8*4);
  Address delta(sigma);
  if (first == 1) {
    cout << "sigma: " << sigma << " sigma and: " << (sigma&0xFFFFFF) << endl;
  cout << " address shifted: " << (Address(1u).pb(z)) << endl;
  cout << " diff: " << ((sigma&0xFFFFFF)-(Address(1u).pb(z))) << endl;
    delta = (sigma&0xFFFFFF)-(Address(1u).pb(z));
  }
  cout << "delta: " << delta << endl;
  cout << "delta shift: " << (delta<<2) << endl;
  cout << "loading " << (delta<<2) << " to " << lambda << endl;
  c.load(lambda,(delta<<2).asUnsignedMorsel());

/*
  cout << "fixrx lambda: " << lambda << " y: " << y << " z: " << z;
  cout << " sigma: " << sigma << endl;
  Address P;
  Address psigma(sigma);
  if (first == 1)
    psigma = 4*((Address(sigma) & 0x00FFFFFFu)-(Address(1u).pb(z)));
  Address fx(4*sigma);
  cout << " sigma after: " << sigma << endl;
  fx.resize(32);
  P = lambda - fx; 
  P.resize(32);
  cout << "lambda: " << lambda << " 4x: " << (fx) << endl;
  cout << "P: " << P << endl;
  P.resize(8*4);
  //c.load(P, (c.view(P) & 0xFFFF0000) | (UnsignedMorsel(y)<<8u) | z ); 
  cout << "loading to " << P << " " << sigma << endl;
  c.load(P,sigma.asUnsignedMorsel());
*/
}

void
Loader::file(unsigned char z)
{
  union tetra_union tetra;
          //Y = file number, Z = tetra count of bytes of filename
          for (;z>0;z--) {
            if (in.eof()) return;
            in.read(tetra.ar,4);
          }
}

void
Loader::line()
{
          //YZ = line number
}

void
Loader::spec(cpu& c)
{
  cout << "SPEC CALLED" << endl;
          //YZ = type. Subsequent tetras, until next loader operation
          //other than lop_quote, comprise the special data.
  union tetra_union tetra;
  while (in.read(tetra.ar,4) && !((tetra.ar[0] == static_cast<char>(mmo_escape)) && !(tetra.ar[1] == lop_quote))
)
  {
      cout << "loop read " 
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(tetra.ar[0]))
;
              cout << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(tetra.ar[1]))
;
              cout << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(tetra.ar[2]))
;
              cout << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(tetra.ar[3]))
;
    // check that tetra.ar[0] == mmo_escape if tetra.ar[1] is an opcode other than lop_quote
    //if (tetra.ar[0] == mmo_escape)
      //if tetra.ar[1] == lop_quote
       // break;
  }
  unsigned char w = static_cast<unsigned char>(tetra.ar[0]);
  unsigned char x = static_cast<unsigned char>(tetra.ar[1]);
  unsigned char y = static_cast<unsigned char>(tetra.ar[2]);
  unsigned char z = static_cast<unsigned char>(tetra.ar[3]);
  if (in.gcount() == 4) 
{
  cout << "spec exit on " 
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(w));
              cout << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(x));
              cout << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(y));
              cout << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(z));
  cout << endl;
  step(c,w,x,y,z);
}
}

void
Loader::pre(unsigned char z)
{
  union tetra_union tetra;
  //Y = mmo format version (currently 1)
  //Z = # of subsequent tetras providing useful info
  //    if Z>0, the first tetra is timestamp of file creation
  cout << "pre: ";
  for (;z > 0;z--) {
    if (in.eof()) return;
    in.read(tetra.ar,4);
    cout << std::hex << setfill('0') << std::setw(2)
    << static_cast<int>(static_cast<unsigned char>(tetra.ar[0]));
    cout << std::hex << setfill('0') << std::setw(2)
    << static_cast<int>(static_cast<unsigned char>(tetra.ar[1]));
    cout << std::hex << setfill('0') << std::setw(2)
    << static_cast<int>(static_cast<unsigned char>(tetra.ar[2]));
    cout << std::hex << setfill('0') << std::setw(2)
    << static_cast<int>(static_cast<unsigned char>(tetra.ar[3]));
  }
  cout << endl;
}

void
Loader::post(cpu& mycpu,unsigned char z)
{
  //load rG with value of Z (must be >= 32)
  //$G,$G+1,...,$255 set to values of next (256-G)*2 tetras
  union tetra_union t1;
  union tetra_union t2;
  UnsignedMorsel rG(6);
  mycpu.regs(Address(rG),UnsignedMorsel(z)); // TODO: this should be rG
  for (unsigned int i=0;(256-mycpu.regs(rG))*2u>i;i+=2)
  {
    in.read(t1.ar,4);
    in.read(t2.ar,4);
    // TODO: properly implement endianness
    // reverse bytes bc endianness
    char t;
    t = t1.ar[0]; t1.ar[0] = t1.ar[3]; t1.ar[3] = t; 
    t = t1.ar[1]; t1.ar[1] = t1.ar[2]; t1.ar[2] = t;
    t = t2.ar[0]; t2.ar[0] = t2.ar[3]; t2.ar[3] = t; 
    t = t2.ar[1]; t2.ar[1] = t2.ar[2]; t2.ar[2] = t;
    UnsignedMorsel m1(t1.num);
    m1.resize(m1.size()+32);
    m1<<=32;
    UnsignedMorsel m2(t2.num);
    UnsignedMorsel m3( m1|m2 );

    mycpu.regs(UnsignedMorsel(z+static_cast<unsigned int>(i/2)), m3);
  }
}

void
Loader::stab(unsigned char y, unsigned char z)
{
  if ((UnsignedMorsel(y).pb(8) | UnsignedMorsel(z)) == 0)
    in_stab = true;
}

void
Loader::end()
{
  in_stab = false;
}

void
Loader::setfile(string filename)
{
  in = std::ifstream(filename, std::ifstream::binary);
  if (!in)
  {
    printf("No such file\n");
    return;
  }  
}

void
Loader::loadobject(cpu& mycpu, string filename)
{
  setfile(filename);

  union tetra_union tetra;
  unsigned char w,x,y,z;
 
  while (in.read(tetra.ar,4)) {
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


    w = static_cast<unsigned char>(tetra.ar[0]);
    x = static_cast<unsigned char>(tetra.ar[1]);
    y = static_cast<unsigned char>(tetra.ar[2]);
    z = static_cast<unsigned char>(tetra.ar[3]);
    step(mycpu,w,x,y,z);
  }
	
  in.close();
  return;
}

void
Loader::step(cpu& mycpu, unsigned char w, unsigned char x, unsigned char y, unsigned char z)
{
/*
      cout << "step called: "
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(w))
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(x))
              << " "  
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(y))
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(z))
        << " to " << lambda
              << "\n";
*/
    if ((static_cast<unsigned char>(mmo_escape) == w) && !quoted_flag)
	  {
      switch (x)
      {
        case lop_quote:
          quote(y,z);
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
          fixrx(mycpu, y, z);
          break;

        case lop_file:
          file(z);
          break;

        case lop_line:
          line();
          break;

        case lop_spec:
          spec(mycpu);
          break;

        case lop_pre:
          pre(z);
          break;

        case lop_post:
          post(mycpu, z);
          break;

        case lop_stab:
          stab(y,z);
          break;

        case lop_end:
          end();
          break;

        default:
          cout << "Invalid loader op: " << w;
          return;
      }
    } else {
      if (!in_stab) {
      quoted_flag = false;
      cout << "loading: "
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(w))
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(x))
              << " "  
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(y))
              << std::hex << setfill('0') << std::setw(2)
        << static_cast<int>(static_cast<unsigned char>(z))
        << " to " << lambda
              << "\n";
      mycpu.load(lambda+0,static_cast<long long unsigned int>(w));
      mycpu.load(lambda+1,static_cast<long long unsigned int>(x));
      mycpu.load(lambda+2,static_cast<long long unsigned int>(y));
      mycpu.load(lambda+3,static_cast<long long unsigned int>(z));
      lambda = (lambda/4)*4+4;
      }
	  }
}
