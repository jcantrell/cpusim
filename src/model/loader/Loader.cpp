#include "model/cpu/SignedMorsel.h"
#include "model/cpu/UnsignedMorsel.h"
#include "model/cpu/cpu.h"
#include <iomanip>
#include <fstream>
#include "model/loader/Loader.h"

void
Loader::quote(unsigned char y, unsigned char z)
{
  if ((UnsignedMorsel(y).pb(8) | UnsignedMorsel(z)) == UnsignedMorsel(1))
    quoted_flag = true;
}

void
Loader::loc(unsigned char y, unsigned char z)
{
  UnsignedMorsel address(UnsignedMorsel(y).pb(56));
  UnsignedMorsel offset(0);
  char c;
  for (unsigned i=0; i<4u*z; i++) 
  {
    in.get(c); offset.pb(8); offset = offset | UnsignedMorsel(static_cast<unsigned char>(c));
  }
  lambda = address + offset;
}

void
Loader::skip(unsigned char y, unsigned char z)
{
  UnsignedMorsel address(UnsignedMorsel(y).pb(56));
  lambda += ((y<<8) | z);
}

void
Loader::fixo(cpu& c, unsigned char y, unsigned char z)
{
  union tetra_union tetra;
  SignedMorsel tmp(y);
  tmp.pb(24);
  if (z==2) {
    in.read(tetra.ar, 4);
    tmp = tmp + SignedMorsel(tetra.num);
  }
  tmp.pb(32);
  in.read(tetra.ar, 4);
  tmp |= SignedMorsel(tetra.num);

  SignedMorsel curLoc(lambda);
  for (int i=7;i>=0;i--) {
    c.load((tmp+i).asUnsignedMorsel(), (curLoc&0xFF).asUnsignedMorsel());
    curLoc = curLoc >> 8;
  }
}

void
Loader::fixr(cpu& mycpu, unsigned char y, unsigned char z)
{
  SignedMorsel delta(y);
  delta.resize(64);
  delta = (delta.pb(8))|z;
  SignedMorsel address;
  address = SignedMorsel(lambda) + (-delta<<2);
  address.resize(64);
  SignedMorsel curLoc(lambda);
  for (int i=1;i>=0;i--) {
    mycpu.load((address+i+2).asUnsignedMorsel(), (delta&0xFF).asUnsignedMorsel());
    delta = delta >> 8;
  }
  for (int j=0;j<2;j++) mycpu.load((address+j).asUnsignedMorsel(), 0x00);
}

void
Loader::fixrx(cpu& c, unsigned char y, unsigned char z)
{
  SignedMorsel tet(0);
  char d;
  unsigned i=0;
  for (;i<4u; i++) 
  {
    in.get(d);
    tet.pb(8);
    tet = tet | SignedMorsel(static_cast<unsigned char>(d));
  }
  tet.resize(8*4);
  SignedMorsel delta;
  delta = ((tet & 0x1000000) != 0) ? ((tet&0xFFFFFF)-(SignedMorsel(1).pb(z))) : tet;
  c.load( (SignedMorsel(lambda)+(-delta<<2)).asUnsignedMorsel(),((tet>>24)&0xFF).asUnsignedMorsel() );
  c.load( ((SignedMorsel(lambda)+(-delta<<2))+1).asUnsignedMorsel(),((tet>>16)&0xFF).asUnsignedMorsel() );
  c.load( ((SignedMorsel(lambda)+(-delta<<2))+2).asUnsignedMorsel(),((tet>>8)&0xFF).asUnsignedMorsel() );
  c.load( ((SignedMorsel(lambda)+(-delta<<2))+3).asUnsignedMorsel(),(tet&0xFF).asUnsignedMorsel() );
/*
 delta=(tet>=0x1000000? (tet&0xffffff)-(1<<j): tet);
 mmo_load(incr(cur_loc,-delta<<2),tet);
*/
}

void
Loader::file(unsigned char y, unsigned char z)
{
  //Y = file number, Z = tetra count of bytes of filename
  union tetra_union tetra;
  filenum = static_cast<unsigned>(y);
  string n="";
  for (;z>0;z--) {
    if (in.eof()) return;
    in.read(tetra.ar,4);
    for (int i=0;i<in.gcount();i++)
      n+=tetra.ar[i];
  }
  fn=n;
}

void
Loader::line(unsigned char y, unsigned char z)
{
  linenum = (static_cast<unsigned long>(y)<<8) | static_cast<unsigned long>(z);
}

void
Loader::spec(cpu& c)
{
  //YZ = type. Subsequent tetras, until next loader operation
  //other than lop_quote, comprise the special data.
  union tetra_union tetra;
  while (in.read(tetra.ar,4) && !((tetra.ar[0] == static_cast<char>(mmo_escape)) && !(tetra.ar[1] == lop_quote))
)
  {
  }

  //TODO: we shouldn't be calling step here, thats loadobject's job
  unsigned char w = static_cast<unsigned char>(tetra.ar[0]);
  unsigned char x = static_cast<unsigned char>(tetra.ar[1]);
  unsigned char y = static_cast<unsigned char>(tetra.ar[2]);
  unsigned char z = static_cast<unsigned char>(tetra.ar[3]);
  if (in.gcount() == 4) 
  {
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
//  cout << "pre: ";
  if (z>0) {
    if (in.eof()) return;
    in.read(tetra.ar,4);
    timestamp = tetra.num;
    z--;
  }
  for (;z > 0;z--) {
    if (in.eof()) return;
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
  UnsignedMorsel rG(6);
  mycpu.regs(UnsignedMorsel(rG),UnsignedMorsel(z));
  for (unsigned int i=0;(256-mycpu.regs(rG))*2u>i;i+=2)
  {
    in.read(t1.ar,4);
    in.read(t2.ar,4);
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
    m3.resize(64);

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
          file(y,z);
          break;

        case lop_line:
          line(y,z);
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
      mycpu.load(lambda+0,static_cast<long long unsigned int>(w));
      mycpu.load(lambda+1,static_cast<long long unsigned int>(x));
      mycpu.load(lambda+2,static_cast<long long unsigned int>(y));
      mycpu.load(lambda+3,static_cast<long long unsigned int>(z));
      lambda = (lambda/4)*4+4;
      }
	  }
}
