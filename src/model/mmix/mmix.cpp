#include "mmix.h"
#include <bitset>
#include <math.h>
#include <iomanip>

mmix::mmix(int byte_size, Address address_size) : cpu(byte_size, address_size,32)  , target(getip()+stepsize)
{
  register_stack_top = 0;
}

bool mmix::N(double x, double f, unsigned int e, float epsilon)
{
  double d = fabs(x-f);
  return (d <= ((1<<(e-1022))*epsilon));
}

void mmix::wideMult(Morsel a, Morsel b,
    Morsel *carry, Morsel *result)
{
  Morsel mask;mask = 0x00000000FFFFFFFF;

/* Multiply two 64-bit unsigned integers
 * by splitting them into 32-bit chunks
 * and using the grid method
 * AB * CD
 * -------------
 * | A*C | B*C |
 * |------------
 * | A*D | B*D |
 * -------------
 */

  Morsel ac = (a>>32) * (b>>32);
  Morsel ad = (a&mask) * (b>>32);
  Morsel bc = (a>>32) * (b&mask);
  Morsel bd = (a&mask) * (b&mask);
  Morsel o1 = bd & mask; // o1 = bd % 2^32
  Morsel t1 = (bd>>32) + (bc&mask) + (ad&mask);
  Morsel o2 = t1 & mask; // o2 = t1 % 2^32
  Morsel t2 = (t1>>32) + (bc>>32) + (ad>>32) + (ac&mask);
  Morsel o3 = t2 & mask;
  Morsel o4 = (ac>>32) + (t2>>32);

  *carry  = ((o4<<32) | o3);
  *result = ((o2<<32) | o1);
}

void mmix::wideDiv(Morsel numerator_hi,
                   Morsel numerator_lo,
                   Morsel divisor,
                   Morsel *quotient,
                   Morsel *remainder
                  )
{
  Morsel a_hi = numerator_hi;
  Morsel a_lo = numerator_lo;
  Morsel b = divisor;

  // quotient
  Morsel q = a_lo << 1;

  // remainder
  Morsel rem = a_hi;

  Morsel carry = a_lo >> 63;
  Morsel temp_carry = 0;
  int i;

  for(i = 0; i < 64; i++)
  {
    temp_carry = rem >> 63;
    rem <<= 1;
    rem |= carry;
    carry = temp_carry;

    if(carry == 0)
    {
      if(rem >= b)
      {
        carry = 1;
      }
      else
      {
        temp_carry = q >> 63;
        q <<= 1;
        q |= carry;
        carry = temp_carry;
        continue;
      }
    }

    rem -= b;
    rem -= (1 - carry);
    carry = 1;
    temp_carry = q >> 63;
    q <<= 1;
    q |= carry;
    carry = temp_carry;
  }

  *quotient = q;
  *remainder = rem;
}

// This function implements the mmix loader
void
mmix::loadobject(string filename)
{
  std::cout << "loading file " << filename << std::endl;
  std::ifstream in(filename, std::ifstream::binary);
  if (!in)
  {
    printf("No such file\n");
    return;
  }  

  union tetra_union {
    uint32_t num;
    char ar[4];
  };
  union tetra_union tetra;
  unsigned char y,z;

  //uint64_t lambda = 0;
  Address lambda;
  lambda = 0;
  Address address;
  bool quoted_flag = false;
  while (!in.eof())
  {
// lambda = loader_step(in, lambda, tetra[0], tetra[1], tetra[2], tetra[3]);
    in.read(tetra.ar,4);
/*
    std::cout << "loaded " << std::hex << setfill('0')
              << std::setw(8) << tetra.num << "\n";
*/
    std::cout << "Tetra: " 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)tetra.ar[0] 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)tetra.ar[1] 
              << " " 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)tetra.ar[2] 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)tetra.ar[3] 
              << "\n";
    
    //op = tetra.ar[0];
    //x = tetra.ar[1];
    y = tetra.ar[2];
    z = tetra.ar[3];

	  if (((char)mmo_escape == tetra.ar[0]) && !quoted_flag)
	  {
      switch (tetra.ar[1])
      {
        case lop_quote:
          quoted_flag = true;
          break;

        case lop_loc:
          {
          //TODO: load tetra.ar[2] (y) into appropriate byte of address
          //cout << "lop_loc called\n";
          //address = ((uint64_t)y<<56);
          address = (Address(y).resize(64) << 56);
          Address yaddr(0xff);
          Address post;
          post = yaddr<<4;
          Address offset; offset = 0;
          union tetra_union addr1;
          union tetra_union addr2;
          in.read(addr1.ar,4);
          //offset = tetra1.num; // reverse this (little-endian)
          offset = (
              (addr1.ar[0]<<24) 
            | (addr1.ar[1]<<16) 
            | (addr1.ar[2]<<8)
            | (addr1.ar[3])
          );
/*
          cout << "loaded first tetra of address: "
               << std::hex << setfill('0') << std::setw(8)
               << offset << '\n' 

              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)(addr1.ar[0]) 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)(addr1.ar[1])
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)(addr1.ar[2]) 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)(addr1.ar[3]) 
          << endl;
          cout << "end first tetra" << endl;
*/
          if (z==2)
          {
            offset = offset<<32;
            //in >> tetra.num;
            in.read(addr2.ar,4);
            offset = offset & (
              (addr2.ar[0]<<24) 
            | (addr2.ar[1]<<16) 
            | (addr2.ar[2]<<8)
            | (addr2.ar[3])
            );
          }
          lambda = address | offset;
/*
          cout << "address: " 
                << std::hex << setfill('0') << std::setw(8)
                << address << endl
                << "offset: " 
                << std::hex << setfill('0') << std::setw(8)
                << offset << endl
                << "lambda: " 
                << std::hex << setfill('0') << std::setw(8)
                << lambda << endl;
*/
          }
          break;

        case lop_skip: 
          lambda += ((y<<8) & z);
          break;

        case lop_fixo:
          {
          Address address;
          address = 0;
          for (;z!=0;z--)
          {
            in >> tetra.num;
            address = address&((uint64_t)(tetra.num)<<32);
          }
          address += (((uint64_t)y)<<56);
          M(8, address, lambda.asMorsel());
          }
          break;

        case lop_fixr:
        {
          Address delta;
          delta = y;
          delta = (delta<<32)&z;
          //uint64_t delta = ((uint64_t)y<<32)&z;
          address = lambda - 4*delta;
          //cout << "case lop_fixr\n";
          load(address+2,y);
          load(address+3,z);
        }
          break;
        case lop_fixrx:
          {
          union {
            uint64_t num;
            char ch[8];
          } delta;
          in >> delta.num;
          delta.num = 
            ( (delta.ch[0] == 1) ? ((delta.num ^ 0x00FFFFFF)-(1<<z)) : delta.num );
          Address P;
          P = lambda - 4*delta.num;
          M(8, P, lambda.asMorsel());
          }
          break;
        case lop_file:
          {
          //Y = file number, Z = tetra count of bytes of filename
          for (;z>0;z--) {
            in.read(tetra.ar,4);
          }
          }
          break;
        case lop_line:
          {
          //YZ = line number
          }
          break;
        case lop_spec:
          //YZ = type. Subsequent tetras, until next loader operation
          //other than lop_quote, comprise the special data.
          break;
        case lop_pre:
          //Y = mmo format version (currently 1)
          //Z = # of subsequent tetras providing useful info
          //    if Z>0, the first tetra is timestamp of file creation
          {
          for (;z > 0;z--) {
            in.read(tetra.ar,4);
          }
          }
          break;
        case lop_post:
          //load rG with value of Z (must be >= 32)
          //$G,$G+1,...,$255 set to values of next (256-G)*2 tetras
          {
          //cout << "lop_post called\n";
          special_registers[rG] = z;
          for (int i=0;i<=(256-G)*2;i+=2)
          {
            union tetra_union t1;
            union tetra_union t2;
            //in >> t1.num;
            in.read(t1.ar,4);
            //in >> t2.num;
            in.read(t2.ar,4);
            //R(z+i, (unsigned long long)t1.num<<32 & (unsigned long long)t2.num);
            R(Morsel(z+i), Morsel(t1.num)<<32 & Morsel(t2.num));
          }
          }
          break;
        case lop_stab:
          {
          // indicates start of user-defined symbols
          cout << "lop_stab called\n";
          }
          break;
        case lop_end:
          {
          // indicates end of user-defined symbols (and end of mmo file)
          }
          break;
        default:
          //error: invalid loader command
          cout << "Invalid loader op: " << tetra.ar[1];
          return;
      }
    } else {
      quoted_flag = false;
/*
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)tetra.ar[0] 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)tetra.ar[1] 
              << " " 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)tetra.ar[2] 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)tetra.ar[3] 
*/
      M(4, lambda, tetra.num);
      //Morsel v = M(4, lambda);
      //cout << "value at " 
       //       << lambda
       //       << " is "
              //<< view(lambda) 
              //<< view(lambda+1) 
              //<< view(lambda+2) 
              //<< view(lambda+3) 
              //<< "\n";
      lambda = (lambda/4)*4+4;
	  }
  
  }
	
  in.close();
  return;
}

// Get value of memory at given address.
// Use size to determine whether to return byte, wyde, tetra, or octa
Morsel
mmix::M(unsigned int size,
        Address address)
{
  const unsigned int octasize = 8;
  Morsel octabyte[octasize];
  Address base = address % size;

  // Load data into byte array
  for (int i=0;i<octasize;i++)
    octabyte[i] = view(base + i);

  // Copy data in byte array to 64-bit int
  Morsel value = 0;
  for (int i=0;i<octasize;i++)
  {
    value & octabyte[octasize-(i+1)];
    value <<= 8;
  }
  Morsel temp;
  temp = (Morsel(0xFF) << Morsel(size - 1) );
  Morsel temp2;
  temp2 = value & temp;
  //printf("M1: At %llu, Value is: %llu\n", address, value);
  return value;
}

Morsel
mmix::M(unsigned int size, Address address, Morsel value)
{
  const unsigned int octasize = 8;
  Morsel octabyte[octasize];
  Address base;
  base = address - (address % size);
  Morsel value2(value);

  // Load data from ram into byte array
  for (int i=0;i<size;i++)
    octabyte[i] = view(base + i);

  // Copy data in byte array to Morsel for returning
  Morsel retvalue(0);
  for (int i=0;i<size;i++)
  {
    retvalue & octabyte[octasize-(i+1)];
    retvalue <<= 8;
  }
  //cout << "retval: " << retvalue << endl;

  for (int i=0;i<size;i++)
  {
    octabyte[i] = (value2 & Morsel(0xFF)).resize(byte_size);
 //   cout << "chunk of value: " << octabyte[i] << endl;
    value2 = value2 >> 8;
  }
  //Morsel temp;
  //temp = (Morsel(0xFF) << Morsel(size - 1) );
  //retvalue = retvalue & temp;

  // Copy data in byte array to memory
  for (int i=0;i<size;i++)
  {
/*
    cout << "M: calling load with "
              << (base+i)
        << " and "
              << octabyte[i].asString().substr(0,2) 
              << " full octabyte[i]: " << octabyte[i].asString()
              << endl;
*/
    load(base+i,octabyte[i]);
/*
    cout << "M: calling view with "
              << (base+i)
        << " gives " <<
              view(base+i) << endl;
*/
  }
/*
  for (int i=0;i<size;i++)
  {
    cout << " Address: " << (base+i) << " Value: "
              //<< std::hex << setfill('0') << std::setw(2)
              //<< octabyte[i].asString().substr(0,2) << endl;
              << view(base+i) << endl;
  }
*/

  return retvalue;
}

Morsel mmix::R(Address reg)
{
  //return registers[reg];
  return regs(reg);
}

Morsel mmix::R(Address reg, Morsel value)
{
  Morsel ret = regs(reg); // registers[reg];
  //registers[reg] = value;
  regs(reg, value);
  return ret;
}

Morsel mmix::g(Address reg, Morsel value)
{
  Morsel ret = globals[reg];
  globals[reg] = value;
  return ret;
}

Morsel mmix::g(Address reg)
{
  return globals[reg];
}

void mmix::push(Morsel reg)
{
  for (Address i(0);i<=reg;i++)
  {
    M(register_stack_top, R(i));
    register_stack_top++;
  }
}

void mmix::pop(Morsel reg)
{
  for (int i=0;i<=reg;i++)
  {
    R(Address(i), M(8, Morsel(register_stack_top-1)));
    register_stack_top--;
  }
}

void mmix::step(int inst)
{
  typedef void (mmix::*instruction)(void);
  typedef std::map<mmix::inst, instruction> instructions_map;
  instructions_map instructions;
      instructions[TRAP] = &mmix::trap;
      instructions[FCMP] = &mmix::fcmp;
      instructions[FUN] = &mmix::fun;
      instructions[FEQL] = &mmix::feql;
      instructions[FADD] = &mmix::fadd;
      instructions[FIX] = &mmix::fix;
      instructions[FSUB] = &mmix::fsub;
      instructions[FIXU] = &mmix::fixu;
      instructions[FLOT] = &mmix::flot;
      instructions[FLOTI] = &mmix::floti;
      instructions[FLOTU] = &mmix::flotu;
      instructions[FLOTUI] = &mmix::flotui;
      instructions[SFLOT] = &mmix::sflot;
      instructions[SFLOTU] = &mmix::sflotu;
      instructions[SFLOTI] = &mmix::sfloti;
      instructions[SFLOTUI] = &mmix::sflotui;

      instructions[FMUL] = &mmix::fmul;
      instructions[FCMPE] = &mmix::fcmpe;
      instructions[FUNE] = &mmix::fune;
      instructions[FEQLE] = &mmix::fune;
      instructions[FDIV] = &mmix::fune;
      instructions[FSQRT] = &mmix::fune;
      instructions[FREM] = &mmix::fune;
      instructions[FINT] = &mmix::fune;
      instructions[MUL] = &mmix::fune;
      instructions[MULI] = &mmix::fune;
      instructions[MULU] = &mmix::fune;
      instructions[MULUI] = &mmix::fune;
      instructions[DIV] = &mmix::fune;
      instructions[DIVI] = &mmix::fune;
      instructions[DIVU] = &mmix::fune;
      instructions[DIVUI] = &mmix::fune;

      instructions[ADDU] = &mmix::addu;
      instructions[ADD] = &mmix::add;
      instructions[SUB] = &mmix::sub;
      instructions[SUBU] = &mmix::subu;
      instructions[i2ADDU] = &mmix::i2addu;
      instructions[i4ADDU] = &mmix::i4addu;
      instructions[i8ADDU] = &mmix::i8addu;
      instructions[i16ADDU] = &mmix::i16addu;
      instructions[ADDI] = &mmix::addi;
      instructions[SUBI] = &mmix::subi;
      instructions[ADDUI] = &mmix::addui;
      instructions[SUBUI] = &mmix::subui;
      instructions[i2ADDUI] = &mmix::i2addui;
      instructions[i4ADDUI] = &mmix::i4addui;
      instructions[i8ADDUI] = &mmix::i8addui;
      instructions[i16ADDUI] = &mmix::i16addui;

      instructions[NEG] = &mmix::neg;
      instructions[NEGU] = &mmix::negu;
      instructions[SL] = &mmix::sl;
      instructions[SLU] = &mmix::slu;
      instructions[SR] = &mmix::sr;
      instructions[SRU] = &mmix::sru;
      instructions[CMP] = &mmix::cmp;
      instructions[CMPU] = &mmix::cmpu;
      instructions[CMPI] = &mmix::cmpi;
      instructions[CMPUI] = &mmix::cmpui;
      instructions[NEGI] = &mmix::negi;
      instructions[NEGUI] = &mmix::negui;
      instructions[SLI] = &mmix::sli;
      instructions[SLUI] = &mmix::slui;
      instructions[SRI] = &mmix::sri;
      instructions[SRUI] = &mmix::srui;

      instructions[BN] = &mmix::bn;
      instructions[BZ] = &mmix::bz;
      instructions[BP] = &mmix::bp;
      instructions[BOD] = &mmix::bod;
      instructions[BNN] = &mmix::bnn;
      instructions[BNZ] = &mmix::bnz;
      instructions[BNP] = &mmix::bnp;
      instructions[BEV] = &mmix::bev;
      instructions[BNB] = &mmix::bnb;
      instructions[BZB] = &mmix::bzb;
      instructions[BPB] = &mmix::bpb;
      instructions[BODB] = &mmix::bodb;
      instructions[BNNB] = &mmix::bnnb;
      instructions[BNZB] = &mmix::bnzb;
      instructions[BNPB] = &mmix::bnpb;
      instructions[BEVB] = &mmix::bevb;

      instructions[PBNB] = &mmix::pbnb;
      instructions[PBZB] = &mmix::pbzb;
      instructions[PBPB] = &mmix::pbpb;
      instructions[PBODB] = &mmix::pbodb;
      instructions[PBNNB] = &mmix::pbnnb;
      instructions[PBNZB] = &mmix::pbnzb;
      instructions[PBNPB] = &mmix::pbnpb;
      instructions[PBEVB] = &mmix::pbevb;
      instructions[PBN] = &mmix::pbn;
      instructions[PBZ] = &mmix::pbz;
      instructions[PBP] = &mmix::pbp;
      instructions[PBOD] = &mmix::pbod;
      instructions[PBNN] = &mmix::pbnn;
      instructions[PBNZ] = &mmix::pbnz;
      instructions[PBNP] = &mmix::pbnp;
      instructions[PBEV] = &mmix::pbev;

      instructions[CSN] = &mmix::csn;
      instructions[CSZ] = &mmix::csz;
      instructions[CSP] = &mmix::csp;
      instructions[CSOD] = &mmix::csod;
      instructions[CSNN] = &mmix::csnn;
      instructions[CSNZ] = &mmix::csnz;
      instructions[CSNP] = &mmix::csnp;
      instructions[CSEV] = &mmix::csev;
      instructions[CSNI] = &mmix::csni;
      instructions[CSZI] = &mmix::cszi;
      instructions[CSPI] = &mmix::cspi;
      instructions[CSODI] = &mmix::csodi;
      instructions[CSNNI] = &mmix::csnni;
      instructions[CSNZI] = &mmix::csnzi;
      instructions[CSNPI] = &mmix::csnpi;
      instructions[CSEVI] = &mmix::csevi;

      instructions[ZSN] = &mmix::zsn;
      instructions[ZSZ] = &mmix::zsz;
      instructions[ZSP] = &mmix::zsp;
      instructions[ZSOD] = &mmix::zsod;
      instructions[ZSNN] = &mmix::zsnn;
      instructions[ZSNZ] = &mmix::zsnz;
      instructions[ZSNP] = &mmix::zsnp;
      instructions[ZSEV] = &mmix::zsev;
      instructions[ZSNI] = &mmix::zsni;
      instructions[ZSZI] = &mmix::zszi;
      instructions[ZSPI] = &mmix::zspi;
      instructions[ZSODI] = &mmix::zsodi;
      instructions[ZSNNI] = &mmix::zsnni;
      instructions[ZSNZI] = &mmix::zsnzi;
      instructions[ZSNPI] = &mmix::zsnpi;
      instructions[ZSEVI] = &mmix::zsevi;

      instructions[LDB] = &mmix::ldb;
      instructions[LDBU] = &mmix::ldbu;
      instructions[LDW] = &mmix::ldw;
      instructions[LDWU] = &mmix::ldwu;
      instructions[LDT   ] = &mmix::ldt;
      instructions[LDTU  ] = &mmix::ldtu;
      instructions[LDO   ] = &mmix::ldo;
      instructions[LDOU  ] = &mmix::ldou;
      instructions[LDBI  ] = &mmix::ldbi;
      instructions[LDBUI ] = &mmix::ldbui;
      instructions[LDWI  ] = &mmix::ldwi;
      instructions[LDWUI ] = &mmix::ldwui;
      instructions[LDTI  ] = &mmix::ldti;
      instructions[LDTUI ] = &mmix::ldtui;
      instructions[LDOI  ] = &mmix::ldoi;
      instructions[LDOUI ] = &mmix::ldoui;

      instructions[LDHT  ] = &mmix::ldht;
      instructions[LDSF  ] = &mmix::ldsf;
      instructions[LDSFI ] = &mmix::ldsfi;
      instructions[LDHTI ] = &mmix::ldhti;
      instructions[CSWAP] = &mmix::cswap;
      instructions[CSWAPI] = &mmix::cswapi;
      instructions[LDUNC ] = &mmix::cswapi;
      instructions[LDUNCI] = &mmix::cswapi;
      instructions[LDVTS] = &mmix::ldvts;
      instructions[LDVTSI] = &mmix::ldvtsi;
      instructions[PRELD ] = &mmix::preld;
      instructions[PRELDI] = &mmix::preldi;
      instructions[PREGO ] = &mmix::prego;
      instructions[PREGOI] = &mmix::pregoi;
      instructions[GOI   ] = &mmix::goi;
      instructions[GO    ] = &mmix::go;

      instructions[STT] = &mmix::stt;
      instructions[STBI] = &mmix::stbi;
      instructions[STBU] = &mmix::stbu;
      instructions[STBUI] = &mmix::stbui;
      instructions[STW] = &mmix::stw;
      instructions[STWI] = &mmix::stwi;
      instructions[STWU] = &mmix::stwu;
      instructions[STWUI] = &mmix::stwui;
      instructions[STB] = &mmix::stb;
      instructions[STTU] = &mmix::sttu;
      instructions[STO] = &mmix::sto;
      instructions[STOU] = &mmix::stou;
      instructions[STTI] = &mmix::stti;
      instructions[STTUI] = &mmix::sttui;
      instructions[STOI] = &mmix::stoi;
      instructions[STOUI] = &mmix::stoui;

      instructions[STSF] = &mmix::stsf;
      instructions[STSFI] = &mmix::stsfi;
      instructions[STHT] = &mmix::stht;
      instructions[STHTI] = &mmix::sthti;
      instructions[STCO] = &mmix::stco;
      instructions[STCOI] = &mmix::stcoi;
      instructions[STUNC] = &mmix::stunc;
      instructions[STUNCI] = &mmix::stunci;
      instructions[SYNCD] = &mmix::syncd;
      instructions[SYNCDI] = &mmix::syncdi;
      instructions[PREST ] = &mmix::prest;
      instructions[PRESTI] = &mmix::presti;
      instructions[SYNCID] = &mmix::syncid;
      instructions[SYNCIDI] = &mmix::syncidi;
      instructions[PUSHGO] = &mmix::pushgo;
      instructions[PUSHGOI] = &mmix::pushgoi;

      instructions[AND] = &mmix::opcode_AND;
      instructions[OR] = &mmix::opcode_OR;
      instructions[XOR] = &mmix::opcode_xor;
      instructions[ANDN] = &mmix::andn;
      instructions[ORN] = &mmix::orn;
      instructions[NAND] = &mmix::nand;
      instructions[NOR] = &mmix::nor;
      instructions[NXOR] = &mmix::nxor;
      instructions[ORI] = &mmix::ori;
      instructions[ORNI] = &mmix::orni;
      instructions[NORI] = &mmix::nori;
      instructions[XORI] = &mmix::xori;
      instructions[ANDI] = &mmix::andi;
      instructions[ANDNI] = &mmix::andni;
      instructions[NANDI] = &mmix::nandi;
      instructions[NXORI] = &mmix::nxori;

      instructions[MUX] = &mmix::mux;
      instructions[SADD] = &mmix::sadd;
      instructions[BDIF] = &mmix::bdif;
      instructions[WDIF] = &mmix::wdif;
      instructions[TDIF] = &mmix::tdif;
      instructions[ODIF] = &mmix::odif;
      instructions[MOR] = &mmix::mor;
      instructions[MXOR] = &mmix::mxor;
      instructions[BDIFI] = &mmix::bdifi;
      instructions[WDIFI] = &mmix::wdifi;
      instructions[TDIFI] = &mmix::tdifi;
      instructions[MUXI] = &mmix::muxi;
      instructions[SADDI] = &mmix::saddi;
      instructions[MORI] = &mmix::mori;
      instructions[MXORI] = &mmix::mxori;
      instructions[ODIFI] = &mmix::odifi;

      instructions[SETH] = &mmix::seth;
      instructions[SETMH] = &mmix::setmh;
      instructions[SETML] = &mmix::setml;
      instructions[SETL] = &mmix::setl;
      instructions[INCH] = &mmix::inch;
      instructions[INCMH] = &mmix::incmh;
      instructions[INCML] = &mmix::incml;
      instructions[INCL] = &mmix::incl;
      instructions[ORH] = &mmix::orh;
      instructions[ORMH] = &mmix::ormh;
      instructions[ORML] = &mmix::orml;
      instructions[ORL] = &mmix::orl;
      instructions[ANDNH] = &mmix::andnh;
      instructions[ANDNMH] = &mmix::andnmh;
      instructions[ANDNL] = &mmix::andnl;
      instructions[ANDNML] = &mmix::andnml;

      instructions[JMP] = &mmix::jmp;
      instructions[JMPB] = &mmix::jmpb;
      instructions[PUSHJ] = &mmix::pushj;
      instructions[PUSHJB] = &mmix::pushjb;
      instructions[GETA] = &mmix::geta;
      instructions[GETAB] = &mmix::getab;
      instructions[PUT] = &mmix::put;
      instructions[PUTI] = &mmix::puti;
      instructions[POP] = &mmix::pop;
      instructions[RESUME] = &mmix::resume;
      instructions[SAVE] = &mmix::save;
      instructions[UNSAVE] = &mmix::unsave;
      instructions[SYNC] = &mmix::sync;
      instructions[SWYM] = &mmix::swym;
      instructions[GET] = &mmix::get;
      instructions[TRIP] = &mmix::trip;

  //call the given instruction's member method
  (this->*instructions[static_cast<mmix::inst>(inst)])();

  setip( target );
}

void mmix::ldtu()
{
  Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
        R(x, M(4, a) );
}

void mmix::ldtui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, M(4, uy+z) );
}

void mmix::ldo()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (M(8, a)) );
}

void mmix::ldoi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (M(8, uy+z)) );
}

void mmix::ldou()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, M(8, a) );
}

void mmix::ldoui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, M(8, uy+z) );
}

void mmix::ldht()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (M(4, a) << 32));
}

void mmix::ldhti()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (M(4, uy+z) << 32));
}

void mmix::stb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(1, a, R(x));
}

void mmix::stbi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(1, uy+z, R(x));
}

void mmix::stw()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(2, a, R(x));
}

void mmix::stwi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(2, uy+z, R(x));
}

void mmix::stt()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, a, R(x));
}

void mmix::stti()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, uy+z, R(x));
}

void mmix::sto()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, a, R(x));
}

void mmix::stoi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, uy+z, R(x));
}

void mmix::stbu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(1, a, R(x));
}

void mmix::stbui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(1, uy+z, R(x));
}

void mmix::stwu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(2, a, R(x));
}

void mmix::stwui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(2, uy+z, R(x));
}

void mmix::sttu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, a, R(x));
}

void mmix::sttui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, uy+z, R(x));
}

void mmix::stou()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, a, R(x));
}

void mmix::stoui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, uy+z, R(x));
}

void mmix::stht()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, a, ((R(x) & Morsel(0xFFFFFFFF00000000)) >> 16));
}

void mmix::sthti()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, uy+z, ((R(x) & Morsel(0xFFFFFFFF00000000)) >> 16));
}

void mmix::stco()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, a, x);
}

void mmix::stcoi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, uy+z, x);
}

void mmix::add()
{
  Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
       R(x, (R(y)) + R(z));
}

void mmix::addi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) + (z));
}

void mmix::sub()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) - (R(z)));
}

void mmix::subi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) - (z));
}

void mmix::mul()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) * (R(z)));
}

void mmix::muli()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) * (z));
}

void mmix::div()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(z) == 0) ? Morsel(0) : 
              ((R(y)) / (R(z))));
        g(Address(rR), (R(z) == 0) ? R(y) :
              ((R(y)) % (R(z))));
}

void mmix::divi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (z == 0) ? Morsel(0) :
              ((R(y)) / (z)));
        g(Address(rR), (z == 0) ? R(y) :
              ((R(y)) % (z)));
}

void mmix::addu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, a);
}

void mmix::addui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (ux + z) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::subu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y) - R(z)) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::subui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y) - z) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::mulu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        {
        Morsel carry;
        Morsel result;
        wideMult(R(y), R(z), &carry, &result);
        g(Address(rH), carry);
        R(x, result);
        }
 
}

void mmix::mului()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        {
        Morsel carry;
        Morsel result;
        wideMult(R(y), z, &carry, &result);
        g(Address(rH), carry);
        R(x, result);
        }
 
}

void mmix::divu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        {
        Morsel numerator_hi;
        Morsel numerator_lo;
        Morsel divisor;
        Morsel quotient;
        Morsel remainder;

        numerator_hi = g(Address(rD));
        numerator_lo = R(y);
        divisor = R(z);

        if (divisor > numerator_hi)
        {
          wideDiv(numerator_hi, numerator_lo, divisor, &quotient, &remainder);
          R(x, quotient);
          g(Address(rR), remainder);
        } else {
          R(x, numerator_hi);
          g(Address(rR), numerator_lo );
        }
  
        }
}

void mmix::divui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        {
        Morsel numerator_hi;
        Morsel numerator_lo;
        Morsel divisor;
        Morsel quotient;
        Morsel remainder;

        numerator_hi = g(Address(rD));
        numerator_lo = R(y);
        divisor = z;

        if (divisor > numerator_hi)
        {
          wideDiv(numerator_hi, numerator_lo, divisor, &quotient, &remainder);
          R(x, quotient);
          g(Address(rR), remainder);
        } else {
          R(x, numerator_hi);
          g(Address(rR), numerator_lo );
        }
  
        }
}
 
void mmix::i2addu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*2 + R(z)) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i2addui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*2 + z) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i4addu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*4 + R(z)) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i4addui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*4 + z) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i8addu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*8 + R(z)) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i8addui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*8 + z) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i16addu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*16 + R(z)) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i16addui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*16 + z) & Morsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::neg()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, y-R(z));
}

void mmix::negi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, y-z);
}

void mmix::negu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, y - R(z));
}

void mmix::negui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, y - z);
}

void mmix::sl()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) << R(z));
}

void mmix::sli()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) << z);
}

void mmix::slu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) << R(z));
}

void mmix::slui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) << z);
}

void mmix::sr()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) >> R(z) );
}

void mmix::sri()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) >> z );
}

void mmix::sru()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) >> R(z) );
}

void mmix::srui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) >> z );
}

void mmix::cmp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > R(z)) ? 1 : (R(y) < R(z) ? -1 : 0)) );
}

void mmix::cmpi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > z) ? 1 : (R(y) < z ? -1 : 0)) );
}

void mmix::cmpu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > R(z)) ? 1 : (R(y) < R(z) ? -1 : 0)) );
}

void mmix::cmpui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > z) ? 1 : (R(y) < z ? -1 : 0)) );
}

void mmix::csn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : R(x)) );
}

void mmix::csni()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : R(x)) );
}

void mmix::csz()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) == 0) ? R(z) : R(x)) );
}

void mmix::cszi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) == 0) ? z : R(x)) );
}

void mmix::csp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > 0) ? R(z) : R(x)) );
}

void mmix::cspi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > 0) ? z : R(x)) );
}

void mmix::csod()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ( ((R(y) & 0x01) == 1) ? R(z) : R(x)) );
}

void mmix::csodi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ( ((R(y) & 0x01) == 1) ? z : R(x)) );
}

void mmix::csnn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) >= 0) ? R(z) : R(x)) );
}

void mmix::csnni()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) >= 0) ? z : R(x)) );
}

void mmix::csnz()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) != 0) ? R(z) : R(x)) );
}

void mmix::csnzi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) != 0) ? z : R(x)) );
}

void mmix::csnp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) <= 0) ? R(z) : R(x)) );
}

void mmix::csnpi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) <= 0) ? z : R(x)) );
}

void mmix::csev()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ( ((R(y) & 0x01) == 0) ? R(z) : R(x)) );
}

void mmix::csevi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ( ((R(y) & 0x01) == 0) ? z : R(x)) );
}

void mmix::zsn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}

void mmix::zsni()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::zsz()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) == 0) ? R(z) : 0) );
}

void mmix::zszi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) == 0) ? z : 0) );
}

void mmix::zsp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > 0) ? R(z) : 0) );
}

void mmix::zspi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > 0) ? z : 0) );
}

void mmix::zsod()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (((R(y) & 0x01) == 1) ? R(z) : 0) );
}

void mmix::zsodi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (((R(y) & 0x01) == 1) ? z : 0) );
}

void mmix::zsnn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}

void mmix::zsnni()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::zsnz()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}
void mmix::zsnzi()
{
  Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
       R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::zsnp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}

void mmix::zsnpi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::zsev()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}

void mmix::zsevi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::opcode_AND()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) & R(z));
}

void mmix::andi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) & z);
}

void mmix::opcode_OR()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) | R(z));
}

void mmix::ori()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) | z);
}

void mmix::opcode_xor()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) ^ R(z));
}

void mmix::xori()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) ^ z);
}

void mmix::andn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) & ~R(z));
}

void mmix::andni()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) & ~z);
}

void mmix::orn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) | ~R(z));
}

void mmix::orni()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) | ~z);
}

void mmix::nand()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) & R(z)) );
}

void mmix::nandi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) & z) );
}

void mmix::nor()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) | R(z)) );
}

void mmix::nori()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) | z) );
}

void mmix::nxor()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) ^ R(z)) );
}

void mmix::nxori()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) ^ z) );
}

void mmix::mux()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y) & g(Address(rM))) | (R(z) & ~g(Address(rM))) );
}

void mmix::muxi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y) & g(Address(rM))) | (z & ~g(Address(rM))) );
}

void mmix::sadd()
        {Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        Morsel temp( R(y) & ~R(z) );
        R(x, temp.count() );
        //R(x, (new std::bitset<64>( R(y) & ~R(z) ))->count() );
        }
 
void mmix::saddi()
        {Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        Morsel temp( R(y) & ~z );
        R(x, temp.count() );
        //R(x, (new std::bitset<64>( R(y) & ~R(z) ))->count() );
        }
 
void mmix::bdif()
        {Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        Morsel b0 = ((R(y)>> 0)&0xFF) - ((R(z)>> 0)&0xFF);
        Morsel b1 = ((R(y)>> 8)&0xFF) - ((R(z)>> 8)&0xFF);
        Morsel b2 = ((R(y)>>16)&0xFF) - ((R(z)>>16)&0xFF);
        Morsel b3 = ((R(y)>>24)&0xFF) - ((R(z)>>24)&0xFF);
        Morsel b4 = ((R(y)>>32)&0xFF) - ((R(z)>>32)&0xFF);
        Morsel b5 = ((R(y)>>40)&0xFF) - ((R(z)>>40)&0xFF);
        Morsel b6 = ((R(y)>>48)&0xFF) - ((R(z)>>48)&0xFF);
        Morsel b7 = ((R(y)>>56)&0xFF) - ((R(z)>>56)&0xFF);
        b0 = (b0 < 0) ? 0 : b0;
        b1 = (b1 < 0) ? 0 : b1;
        b2 = (b2 < 0) ? 0 : b2;
        b3 = (b3 < 0) ? 0 : b3;
        b4 = (b4 < 0) ? 0 : b4;
        b5 = (b5 < 0) ? 0 : b5;
        b6 = (b6 < 0) ? 0 : b6;
        b7 = (b7 < 0) ? 0 : b7;
        R(x, (b7<<56) & (b6<<48) & (b5<<40) & (b4<<32) & (b3<<24) & (b2<<16)
              & (b1<<8) & (b0<<0) );
        }
 
void mmix::bdifi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
         {
        Morsel b0 = ((R(y)>> 0)&0xFF) - ((uzi>> 0)&0xFF);
        Morsel b1 = ((R(y)>> 8)&0xFF) - ((uzi>> 8)&0xFF);
        Morsel b2 = ((R(y)>>16)&0xFF) - ((uzi>>16)&0xFF);
        Morsel b3 = ((R(y)>>24)&0xFF) - ((uzi>>24)&0xFF);
        Morsel b4 = ((R(y)>>32)&0xFF) - ((uzi>>32)&0xFF);
        Morsel b5 = ((R(y)>>40)&0xFF) - ((uzi>>40)&0xFF);
        Morsel b6 = ((R(y)>>48)&0xFF) - ((uzi>>48)&0xFF);
        Morsel b7 = ((R(y)>>56)&0xFF) - ((uzi>>56)&0xFF);
        b0 = (b0 < 0) ? 0 : b0;
        b1 = (b1 < 0) ? 0 : b1;
        b2 = (b2 < 0) ? 0 : b2;
        b3 = (b3 < 0) ? 0 : b3;
        b4 = (b4 < 0) ? 0 : b4;
        b5 = (b5 < 0) ? 0 : b5;
        b6 = (b6 < 0) ? 0 : b6;
        b7 = (b7 < 0) ? 0 : b7;
        R(x, (b7<<56) & (b6<<48) & (b5<<40) & (b4<<32) & (b3<<24) & (b2<<16)
              & (b1<<8) & (b0<<0) );
        }
 
}

void mmix::wdif()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
          {
        Morsel w0 = ((R(y)>> 0)&0xFFFF) - ((R(z)>> 0)&0xFFFF);
        Morsel w1 = ((R(y)>>16)&0xFFFF) - ((R(z)>>16)&0xFFFF);
        Morsel w2 = ((R(y)>>32)&0xFFFF) - ((R(z)>>32)&0xFFFF);
        Morsel w3 = ((R(y)>>48)&0xFFFF) - ((R(z)>>48)&0xFFFF);
        w0 = (w0 < 0) ? 0 : w0;
        w1 = (w0 < 0) ? 0 : w1;
        w2 = (w0 < 0) ? 0 : w2;
        w3 = (w0 < 0) ? 0 : w3;
        R(x, (w3<<48)&(w2<<32)&(w1<<16)&(w0<<0) );
        }
 
}

void mmix::wdifi()
        {Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
  
        Morsel w0 = ((R(y)>> 0)&0xFFFF) - ((uzi>> 0)&0xFFFF);
        Morsel w1 = ((R(y)>>16)&0xFFFF) - ((uzi>>16)&0xFFFF);
        Morsel w2 = ((R(y)>>32)&0xFFFF) - ((uzi>>32)&0xFFFF);
        Morsel w3 = ((R(y)>>48)&0xFFFF) - ((uzi>>48)&0xFFFF);
        w0 = (w0 < 0) ? 0 : w0;
        w1 = (w0 < 0) ? 0 : w1;
        w2 = (w0 < 0) ? 0 : w2;
        w3 = (w0 < 0) ? 0 : w3;
        R(x, (w3<<48)&(w2<<32)&(w1<<16)&(w0<<0) );
        }
 
void mmix::tdif()
        {Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  
        Morsel t0 = ((R(y)>> 0)&0xFFFFFFFF) - ((R(z)>> 0)&0xFFFFFFFF);
        Morsel t1 = ((R(y)>>32)&0xFFFFFFFF) - ((R(z)>>32)&0xFFFFFFFF);
        t0 = (t0 < 0) ? 0 : t0;
        t1 = (t1 < 0) ? 0 : t1;
        R(x, (t1<<32)&(t0<<0) );
        }
 
void mmix::tdifi()
        {Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
 
        Morsel t0 = ((R(y)>> 0)&0xFFFFFFFF) - ((uzi>> 0)&0xFFFFFFFF);
        Morsel t1 = ((R(y)>>32)&0xFFFFFFFF) - ((uzi>>32)&0xFFFFFFFF);
        t0 = (t0 < 0) ? 0 : t0;
        t1 = (t1 < 0) ? 0 : t1;
        R(x, (t1<<32)&(t0<<0) );
        }
 
void mmix::ofif()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         {
        Morsel u0 = (R(y)) - R(z);
        u0 = (u0 > R(y)) ? 0 : u0;
        }
 
}

void mmix::odifi()
{ Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
        {
        Morsel u0 = (R(y)) - z;
        u0 = (u0 > R(y)) ? 0 : u0;
        }
 
}

void mmix::odif()
{
  return;
}
void mmix::mor()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         {
        Morsel r;
        for (int i=0; i<64; i++)
          for (int j=0; j<64; j++)
          {
            r = (R(z)>>i)&(R(y)>>i);
            for (int k=2; k<=64;k++)
              r = r | ( (R(z)>>(i+8*k)) & (R(y)>>(k+8*j)) );
            r=r&1;
            R(x, (R(x) & (~(1<<(i+8*j)))) | (r<<(i+8*j)) );
          }
        }
}

void mmix::mori()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
        {
        Morsel r;
        for (int i=0; i<64; i++)
          for (int j=0; j<64; j++)
          {
            r = (z>>i)&(R(y)>>i);
            for (int k=2; k<=64;k++)
              r = r | ( (z>>(i+8*k)) & (R(y)>>(k+8*j)) );
            r=r&1;
            R(x, (R(x) & (~(1<<(i+8*j)))) | (r<<(i+8*j)) );
          }
        }
}

void mmix::mxor()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         {
        Morsel r;
        for (int i=0; i<8; i++)
          for (int j=0; j<8; j++)
          {
            r = (R(z)>>i)&(R(y)>>i);
            for (int k=1; k<8;k++)
              r = r ^ ( (R(z)>>(i+8*k)) & (R(y)>>(k+8*j)) );
            r=r&1;
            R(x, (R(x) & (~(1<<(i+8*j)))) | (r<<(i+8*j)) );
          }
        }
 
}

void mmix::mxori()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         {
        Morsel r;
        for (int i=0; i<8; i++)
          for (int j=0; j<8; j++)
          {
            r = (z>>i)&(R(y)>>i);
            for (int k=1; k<8;k++)
              r = r ^ ( (z>>(i+8*k)) & (R(y)>>(k+8*j)) );
            r=r&1;
            R(x, (R(x) & (~(1<<(i+8*j)))) | (r<<(i+8*j)) );
          }
        }
 
}

void mmix::fadd()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
{
        double t = ( *(double*)&uy + *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }

}

void mmix::fsub()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         {
        double t = ( *(double*)&uy - *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
 
}

void mmix::fmul()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         {
        double t = ( *(double*)&uy * *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
 
}

void mmix::fdiv()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         {
        double t = ( *(double*)&uy / *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
 
}

void mmix::frem()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
        {
        double t = fmod( *(double*)&uy , *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
 
}

void mmix::fsqrt()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         {
        double t = sqrt( *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
 
}

void mmix::fint()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, (unsigned long long int)( *(double*)&uz ));
}

void mmix::fcmp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
         R(x, ((*(double*)&uy > *(double*)&uz) ? 1 : 0) 
           - ((*(double*)&uy < *(double*)&uz) ? 1 : 0) );
 
}

void mmix::feql()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
        R(x, *(double*)&uy == *(double*)&uz );
}

void mmix::fun()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
        R(x, fy != fy || fz != fz );
}

void mmix::fcmpe()
        {Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
        int e;
        frexp(fy.asFloat(), &e);
        R(x, (fy > fz && !N(fy.asFloat(), fz.asFloat(), e, frE.asFloat())) -
             (fy < fz && !N(fy.asFloat(), fz.asFloat(), e, frE.asFloat())));
        }
 
void mmix::feqle()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
        int e;
        frexp(fy.asFloat(), &e);
        R(x, N(fy.asFloat(), fz.asFloat(), e, frE.asFloat()));
}

void mmix::fune()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, fy != fy || fz != fz || frE != frE);
}

void mmix::fix()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, fz);
}

void mmix::fixu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, fz);
}

void mmix::flot()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
        R(x, sz);
}

void mmix::flotu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, uz);
}

void mmix::flotui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, z);
}

void mmix::sflot()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, fz);
}

void mmix::sfloti()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, z);
}

void mmix::sflotu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, fz);
}

void mmix::sflotui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, z);
}

void mmix::ldsf()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
  R(x, M(4, a));
}

void mmix::ldsfi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, M(4, uy+z));
}

void mmix::stsf()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
  M(4, a, fx);
}

void mmix::stsfi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  M(4, uy+z, fx);
}

void mmix::floti()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  R(x, z);
}

void mmix::seth()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
 Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
  R(x, ((uyi<<8) & uzi) << 48);
}

void mmix::setmh()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
 Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
   R(x, ((uyi<<8) & uzi) << 32);
}

void mmix::setml()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
   R(x, ((uyi<<8) & uzi));
}

void mmix::setl()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
 
  R(x, ((uyi<<8) & uzi) << 16);
}

void mmix::inch()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
 
  R(x, R(x) + (((uyi<<8) & uzi) << 48) );
}

void mmix::incmh()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
 
  R(x, R(x) + (((uyi<<8) & uzi) << 32) );
}

void mmix::incml()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
 
  R(x, R(x) + (((uyi<<8) & uzi) << 16) );
}

void mmix::incl()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
   R(x, ux + ((uyi<<8) & uzi) );
}

void mmix::orh()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
   R(x, ux | (((uyi<<8) & uzi)<<48) );
}

void mmix::ormh()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
   R(x, ux | (((uyi<<8) & uzi)<<32) );
}

void mmix::orml()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
   R(x, ux | (((uyi<<8) & uzi)<<16) );
}

void mmix::orl()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
         R(x, ux | ((uyi<<8) & uzi) );
}

void mmix::andnh()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
         R(x, ux & ~(((uyi<<8) & uzi)<<48) );
}

void mmix::andnmh()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
   R(x, ux & ~(((uyi<<8) & uzi)<<32) );
}

void mmix::andnml()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
   R(x, ux & ~(((uyi<<8) & uzi)<<16) );
}

void mmix::andnl()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;
   R(x, ux & ~((uyi<<8) & uzi) );
}

void mmix::go()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
        R(Address(x), target.asMorsel());
        target = a;
}

void mmix::bn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sx < 0 ) ? ra : target );
}

void mmix::bz()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( uz == 0 ) ? ra : target );
}

void mmix::bp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz > 0 ) ? ra : target );
}

void mmix::bod()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 1 ) ? ra : target );
}

void mmix::bnn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz >= 0 ) ? ra : target );
}

void mmix::bnz()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz != 0 ) ? ra : target );
}

void mmix::bnp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz <= 0 ) ? ra : target );
}

void mmix::bev()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 0 ) ? ra : target );
}

void mmix::pbn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sx < 0 ) ? ra : target );
}

void mmix::pbz()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( uz == 0 ) ? ra : target );
}

void mmix::pbp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz > 0 ) ? ra : target );
}

void mmix::pbod()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 1 ) ? ra : target );
}

void mmix::pbnn()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz >= 0 ) ? ra : target );
}

void mmix::pbnz()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz != 0 ) ? ra : target );
}

void mmix::pbnp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz <= 0 ) ? ra : target );
}

void mmix::pbev()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 0 ) ? ra : target );
}

void mmix::jmpb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = -1*ra;
}

void mmix::bnb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sx < 0 ) ? -1*ra : target );
}

void mmix::bzb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( uz == 0 ) ? -1*ra : target );
}

void mmix::bpb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz > 0 ) ? -1*ra : target );
}

void mmix::bodb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 1 ) ? -1*ra : target );
}

void mmix::bnnb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz >= 0 ) ? -1*ra : target );
}

void mmix::bnzb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz != 0 ) ? -1*ra : target );
}

void mmix::bnpb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz <= 0 ) ? -1*ra : target );
}

void mmix::bevb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 0 ) ? -1*ra : target );
}

void mmix::pbnb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sx < 0 ) ? -1*ra : target );
}

void mmix::pbzb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( uz == 0 ) ? -1*ra : target );
}

void mmix::pbpb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz > 0 ) ? -1*ra : target );
}

void mmix::pbodb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 1 ) ? -1*ra : target );
}

void mmix::pbnnb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz >= 0 ) ? -1*ra : target );
}

void mmix::pbnzb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz != 0 ) ? -1*ra : target );
}

void mmix::pbnpb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz <= 0 ) ? -1*ra : target );
}

void mmix::pbevb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 0 ) ? -1*ra : target );
}

void mmix::pushj()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  push(x);
  R(Address(rJ), (target+4).asMorsel());
  target = ra;
}

void mmix::pushjb()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel ra = 4*( (uy<<8) & uz);
 
  push(x);
  R(Address(rJ), (-1*(target+4)).asMorsel());
  target = ra;
}

void mmix::pushgo()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
  push(x);
  R(Address(rJ), (target+4).asMorsel());
  target = a;
}

void mmix::pop()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  pop(x);
  target = R(Address(rJ))+4*((y<<8) & z);
}

void mmix::save()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  push(255);
  R(x, register_stack_top);
}
void mmix::unsave()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
 
  pop(255);
  R(x, register_stack_top);
}

void mmix::ldunc()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
  R( x, (M(8, a)) );
}
void mmix::stunc()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
  M(8, a, R(x));
}
void mmix::ldunci()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
  R( x, (M(8, a)) );
}

void mmix::cswap()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
        if (M(8, a) == R(Address(rP))) {
          M(8,a,R(x));
          R(x,1);
        } else {
          R(Address(rP), M(8,a));
          R(x, 0);
        }
 
}
void mmix::cswapi()
{
  return;
}
void mmix::preld()
{
  return;
}
void mmix::preldi()
{
  return;
}
void mmix::prego()
{
  return;
}
void mmix::pregoi()
{
  return;
}
void mmix::goi()
{
  return;
}
void mmix::stunci()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  M(8, uy+z, R(x));
}
void mmix::syncd()
{
  return;
}
void mmix::syncdi()
{
  return;
}
void mmix::syncid()
{
  return;
}
void mmix::prest()
{
  return;
}
void mmix::presti()
{
  return;
}
void mmix::syncidi()
{
  return;
}
void mmix::pushgoi()
{
  return;
}
void mmix::trip()
{
  trap();
}
void mmix::trap()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

        switch (y.asChar()) {
        case Halt:
          return;
          break;
        case Fopen:
          break;
        case Fclose:
          break;
        case Fread:
          break;
        case Fgets:
          break;
        case Fgetws:
          break;
        case Fwrite:
          break;
        case Fputs:
          if (z == 1)          
            for (int i=0; M(1,R(Address(255)) + i)  != '\0'; i++)
            {
              printf( "%c", M(1,R(Address(255)) + i).asChar() );
            }
          break;
        case Fputws:
          break;
        case Fseek:
          break;
        case Ftell:
          break;
        default:
          break;
        }
}
void mmix::resume()
{
}

void mmix::get()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  R(x, g(z));
}

void mmix::put()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  g(x, R(z));
}

void mmix::puti()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  g(x, z);
}

void mmix::geta()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  Morsel ra = 4*( (uy<<8) & uz);
  R(x, ra);
}

void mmix::getab()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  Morsel ra = 4*( (uy<<8) & uz);
  R(x, -1*ra);
}

void mmix::jmp()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  Address target = getip()+stepsize;
  target = Address( (x<<16) & (y<<8) & (z<<0) );
  setip( target );
}

void mmix::ldb()
{ 
  Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 

  Morsel uy = R(y);
  Morsel uz = R(z);

  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
  R(x, M(1, a));
}

void mmix::ldbi()
{
  Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 

  Morsel uy = R(y);
  Morsel uz = R(z);


  R(x, M(1, (uy + z)));
}

void mmix::ldbu()
{
  Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 

  Morsel uy = R(y);
  Morsel uz = R(z);

  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
 
  R(x, M(1, a));
}

void mmix::ldbui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  R(x, M(1, (uy+z)));
}

void mmix::ldw()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );

  R(x, M(2, a));
}

void mmix::ldwi()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  R(x, M(2, uy+z));
}

void mmix::ldwu()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );

  R(x, M(2, a) );
}

void mmix::ldwui()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  R(x, M(2, uy+z) );
}

void mmix::ldt()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );

  R(x, (M(4, a)) );
}

void mmix::ldti()
{Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel ux = R(x); // unsigned values at the given addresses
  Morsel uy = R(y);
  Morsel uz = R(z);
  Morsel sx = R(x); // signed values
  Morsel sy = R(y);
  Morsel sz = R(z);
  Morsel fx = R(x); // signed double values
  Morsel fy = R(y);
  Morsel fz = R(z);
  Morsel frE = R(Address(rE));

  R(x, (M(4, uy+z)) );
}

void mmix::swym()
{
  return;
}

void mmix::ldvts()
{
  return;
}

void mmix::ldvtsi()
{
  return;
}
void mmix::sync()
{
  return;
}
