#include "mmix.h"
#include <bitset>
#include <math.h>
#include <iomanip>

mmix::mmix(unsigned int byte_size_p, UnsignedMorsel address_size_p) : cpu(byte_size_p, address_size_p,32)  , target(getip()+stepsize)
{
  register_stack_top = 0;
}

bool mmix::N(double x, double f, unsigned int e, float epsilon)
{
  double d = fabs(x-f);
  return (d <= ((1<<(e-1022))*epsilon));
}

void mmix::wideMult(UnsignedMorsel a, UnsignedMorsel b,
    UnsignedMorsel *carry, UnsignedMorsel *result)
{
  UnsignedMorsel mask;mask = 0x00000000FFFFFFFF;

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

  UnsignedMorsel ac = (a>>32) * (b>>32);
  UnsignedMorsel ad = (a&mask) * (b>>32);
  UnsignedMorsel bc = (a>>32) * (b&mask);
  UnsignedMorsel bd = (a&mask) * (b&mask);
  UnsignedMorsel o1 = bd & mask; // o1 = bd % 2^32
  UnsignedMorsel t1 = (bd>>32) + (bc&mask) + (ad&mask);
  UnsignedMorsel o2 = t1 & mask; // o2 = t1 % 2^32
  UnsignedMorsel t2 = (t1>>32) + (bc>>32) + (ad>>32) + (ac&mask);
  UnsignedMorsel o3 = t2 & mask;
  UnsignedMorsel o4 = (ac>>32) + (t2>>32);

  *carry  = ((o4<<32) | o3);
  *result = ((o2<<32) | o1);
}

void mmix::wideDiv(UnsignedMorsel numerator_hi,
                   UnsignedMorsel numerator_lo,
                   UnsignedMorsel divisor,
                   UnsignedMorsel *quotient,
                   UnsignedMorsel *remainder
                  )
{
  UnsignedMorsel a_hi = numerator_hi;
  UnsignedMorsel a_lo = numerator_lo;
  UnsignedMorsel b = divisor;

  // quotient
  UnsignedMorsel q = a_lo << 1;

  // remainder
  UnsignedMorsel rem = a_hi;

  UnsignedMorsel carry = a_lo >> 63;
  UnsignedMorsel temp_carry = 0;
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

// Get value of memory at given address.
// Use size to determine whether to return byte, wyde, tetra, or octa
UnsignedMorsel
mmix::M(unsigned int size,
        UnsignedMorsel address)
{
  const unsigned int octasize = 8;
  UnsignedMorsel octabyte[octasize];
  UnsignedMorsel base = address % size;

  // Load data into byte array
  for (unsigned int i=0;i<octasize;i++)
    octabyte[i] = view(base + i);

  // Copy data in byte array to 64-bit int
  UnsignedMorsel value = 0;
  for (unsigned int i=0;i<octasize;i++)
  {
    value & octabyte[octasize-(i+1)];
    value <<= 8;
  }
  UnsignedMorsel temp;
  temp = (UnsignedMorsel(0xFF) << UnsignedMorsel(size - 1) );
  UnsignedMorsel temp2;
  temp2 = value & temp;
  //printf("M1: At %llu, Value is: %llu\n", address, value);
  return value;
}

UnsignedMorsel
mmix::M(unsigned int size, UnsignedMorsel address, UnsignedMorsel value)
{
  const unsigned int octasize = 8;
  UnsignedMorsel octabyte[octasize];
  UnsignedMorsel base;
  base = address - (address % size);
  UnsignedMorsel value2(value);

  // Load data from ram into byte array
  for (unsigned int i=0;i<size;i++)
    octabyte[i] = view(base + i);

  // Copy data in byte array to Morsel for returning
  UnsignedMorsel retvalue(0);
  for (unsigned int i=0;i<size;i++)
  {
    retvalue & octabyte[octasize-(i+1)];
    retvalue <<= 8;
  }
  //cout << "retval: " << retvalue << endl;

  for (unsigned int i=0;i<size;i++)
  {
    octabyte[i] = (value2 & UnsignedMorsel(0xFF)).resize(byte_size);
 //   cout << "chunk of value: " << octabyte[i] << endl;
    value2 = value2 >> 8;
  }
  //Morsel temp;
  //temp = (Morsel(0xFF) << Morsel(size - 1) );
  //retvalue = retvalue & temp;

  // Copy data in byte array to memory
  for (unsigned int i=0;i<size;i++)
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
    cout << " UnsignedMorsel: " << (base+i) << " Value: "
              //<< std::hex << setfill('0') << std::setw(2)
              //<< octabyte[i].asString().substr(0,2) << endl;
              << view(base+i) << endl;
  }
*/

  return retvalue;
}

UnsignedMorsel mmix::R(UnsignedMorsel reg)
{
  return regs(reg);
}

UnsignedMorsel mmix::R(UnsignedMorsel reg, UnsignedMorsel value)
{
  UnsignedMorsel ret = regs(reg);
  //registers[reg] = value;
  regs(reg, value);
  return ret;
}

UnsignedMorsel mmix::g(UnsignedMorsel reg, UnsignedMorsel value)
{
  UnsignedMorsel ret = globals[reg];
  globals[reg] = value;
  return ret;
}

UnsignedMorsel mmix::g(UnsignedMorsel reg)
{
  return globals[reg];
}

void mmix::push(UnsignedMorsel reg)
{
  for (UnsignedMorsel i(0);i<=reg;i++)
  {
    M(register_stack_top, R(i));
    register_stack_top++;
  }
}

void mmix::pop(UnsignedMorsel reg)
{
  for (unsigned int i=0;i<=reg;i++)
  {
    R(UnsignedMorsel(i), M(8, UnsignedMorsel(register_stack_top-1)));
    register_stack_top--;
  }
}

void mmix::step(unsigned int instr)
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
	cout << "calling step with instr " << instr << endl;
  (this->*instructions[static_cast<mmix::inst>(instr)])();

  setip( target );
}

void mmix::ldtu()
{
  UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );

  R(x, M(4, a) );
}

void mmix::ldtui()
{ UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel uy = R(y);
 
        R(x, M(4, uy+z) );
}

void mmix::ldo()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (M(8, a)) );
}

void mmix::ldoi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (M(8, uy+z)) );
}

void mmix::ldou()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, M(8, a) );
}

void mmix::ldoui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, M(8, uy+z) );
}

void mmix::ldht()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (M(4, a) << 32));
}

void mmix::ldhti()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (M(4, uy+z) << 32));
}

void mmix::stb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(1, a, R(x));
}

void mmix::stbi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(1, uy+z, R(x));
}

void mmix::stw()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(2, a, R(x));
}

void mmix::stwi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(2, uy+z, R(x));
}

void mmix::stt()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, a, R(x));
}

void mmix::stti()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, uy+z, R(x));
}

void mmix::sto()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, a, R(x));
}

void mmix::stoi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, uy+z, R(x));
}

void mmix::stbu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(1, a, R(x));
}

void mmix::stbui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(1, uy+z, R(x));
}

void mmix::stwu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(2, a, R(x));
}

void mmix::stwui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(2, uy+z, R(x));
}

void mmix::sttu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, a, R(x));
}

void mmix::sttui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, uy+z, R(x));
}

void mmix::stou()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, a, R(x));
}

void mmix::stoui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, uy+z, R(x));
}

void mmix::stht()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, a, ((R(x) & UnsignedMorsel(0xFFFFFFFF00000000)) >> 16));
}

void mmix::sthti()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(4, uy+z, ((R(x) & UnsignedMorsel(0xFFFFFFFF00000000)) >> 16));
}

void mmix::stco()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, a, x);
}

void mmix::stcoi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        M(8, uy+z, x);
}

void mmix::add()
{
  UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
       R(x, (R(y)) + R(z));
}

void mmix::addi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) + (z));
}

void mmix::sub()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) - (R(z)));
}

void mmix::subi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) - (z));
}

void mmix::mul()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) * (R(z)));
}

void mmix::muli()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)) * (z));
}

void mmix::div()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(z) == 0) ? UnsignedMorsel(0) : 
              ((R(y)) / (R(z))));
        g(UnsignedMorsel(rR), (R(z) == 0) ? R(y) :
              ((R(y)) % (R(z))));
}

void mmix::divi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (z == 0) ? UnsignedMorsel(0) :
              ((R(y)) / (z)));
        g(UnsignedMorsel(rR), (z == 0) ? R(y) :
              ((R(y)) % (z)));
}

void mmix::addu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, a);
}

void mmix::addui()
{
	UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
	UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
  R(x, (ux + z) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::subu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y) - R(z)) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::subui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y) - z) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::mulu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        {
        UnsignedMorsel carry;
        UnsignedMorsel result;
        wideMult(R(y), R(z), &carry, &result);
        g(UnsignedMorsel(rH), carry);
        R(x, result);
        }
 
}

void mmix::mului()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        {
        UnsignedMorsel carry;
        UnsignedMorsel result;
        wideMult(R(y), z, &carry, &result);
        g(UnsignedMorsel(rH), carry);
        R(x, result);
        }
 
}

void mmix::divu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        {
        UnsignedMorsel numerator_hi;
        UnsignedMorsel numerator_lo;
        UnsignedMorsel divisor;
        UnsignedMorsel quotient;
        UnsignedMorsel remainder;

        numerator_hi = g(UnsignedMorsel(rD));
        numerator_lo = R(y);
        divisor = R(z);

        if (divisor > numerator_hi)
        {
          wideDiv(numerator_hi, numerator_lo, divisor, &quotient, &remainder);
          R(x, quotient);
          g(UnsignedMorsel(rR), remainder);
        } else {
          R(x, numerator_hi);
          g(UnsignedMorsel(rR), numerator_lo );
        }
  
        }
}

void mmix::divui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        {
        UnsignedMorsel numerator_hi;
        UnsignedMorsel numerator_lo;
        UnsignedMorsel divisor;
        UnsignedMorsel quotient;
        UnsignedMorsel remainder;

        numerator_hi = g(UnsignedMorsel(rD));
        numerator_lo = R(y);
        divisor = z;

        if (divisor > numerator_hi)
        {
          wideDiv(numerator_hi, numerator_lo, divisor, &quotient, &remainder);
          R(x, quotient);
          g(UnsignedMorsel(rR), remainder);
        } else {
          R(x, numerator_hi);
          g(UnsignedMorsel(rR), numerator_lo );
        }
  
        }
}
 
void mmix::i2addu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*2 + R(z)) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i2addui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*2 + z) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i4addu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*4 + R(z)) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i4addui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*4 + z) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i8addu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*8 + R(z)) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i8addui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*8 + z) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i16addu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*16 + R(z)) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::i16addui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y)*16 + z) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF));
}

void mmix::neg()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, y-R(z));
}

void mmix::negi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, y-z);
}

void mmix::negu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, y - R(z));
}

void mmix::negui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, y - z);
}

void mmix::sl()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) << R(z));
}

void mmix::sli()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) << z);
}

void mmix::slu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) << R(z));
}

void mmix::slui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) << z);
}

void mmix::sr()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) >> R(z) );
}

void mmix::sri()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) >> z );
}

void mmix::sru()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) >> R(z) );
}

void mmix::srui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) >> z );
}

void mmix::cmp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > R(z)) ? 1 : (R(y) < R(z) ? 1 : 0)) );
}

void mmix::cmpi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        //R(x, ((R(y) > z) ? 1 : (R(y) < z ? -1 : 0)) );
        R(x, ((R(y) > z) ? 1 : (R(y) < z ? 1 : 0)) );
}

void mmix::cmpu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > R(z)) ? 1 : (R(y) < R(z) ? 1 : 0)) );
}

void mmix::cmpui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > z) ? 1 : (R(y) < z ? 1 : 0)) );
}

void mmix::csn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : R(x)) );
}

void mmix::csni()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : R(x)) );
}

void mmix::csz()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) == 0) ? R(z) : R(x)) );
}

void mmix::cszi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) == 0) ? z : R(x)) );
}

void mmix::csp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > 0) ? R(z) : R(x)) );
}

void mmix::cspi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > 0) ? z : R(x)) );
}

void mmix::csod()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ( ((R(y) & 0x01) == 1) ? R(z) : R(x)) );
}

void mmix::csodi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ( ((R(y) & 0x01) == 1) ? z : R(x)) );
}

void mmix::csnn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) >= 0) ? R(z) : R(x)) );
}

void mmix::csnni()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) >= 0) ? z : R(x)) );
}

void mmix::csnz()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) != 0) ? R(z) : R(x)) );
}

void mmix::csnzi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) != 0) ? z : R(x)) );
}

void mmix::csnp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) <= 0) ? R(z) : R(x)) );
}

void mmix::csnpi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) <= 0) ? z : R(x)) );
}

void mmix::csev()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ( ((R(y) & 0x01) == 0) ? R(z) : R(x)) );
}

void mmix::csevi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ( ((R(y) & 0x01) == 0) ? z : R(x)) );
}

void mmix::zsn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}

void mmix::zsni()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::zsz()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) == 0) ? R(z) : 0) );
}

void mmix::zszi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) == 0) ? z : 0) );
}

void mmix::zsp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > 0) ? R(z) : 0) );
}

void mmix::zspi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) > 0) ? z : 0) );
}

void mmix::zsod()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (((R(y) & 0x01) == 1) ? R(z) : 0) );
}

void mmix::zsodi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (((R(y) & 0x01) == 1) ? z : 0) );
}

void mmix::zsnn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}

void mmix::zsnni()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::zsnz()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}
void mmix::zsnzi()
{
  UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
       R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::zsnp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}

void mmix::zsnpi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::zsev()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? R(z) : 0) );
}

void mmix::zsevi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ((R(y) < 0) ? z : 0) );
}

void mmix::opcode_AND()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) & R(z));
}

void mmix::andi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) & z);
}

void mmix::opcode_OR()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) | R(z));
}

void mmix::ori()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) | z);
}

void mmix::opcode_xor()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) ^ R(z));
}

void mmix::xori()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) ^ z);
}

void mmix::andn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) & ~R(z));
}

void mmix::andni()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) & ~z);
}

void mmix::orn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) | ~R(z));
}

void mmix::orni()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, R(y) | ~z);
}

void mmix::nand()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) & R(z)) );
}

void mmix::nandi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) & z) );
}

void mmix::nor()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) | R(z)) );
}

void mmix::nori()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) | z) );
}

void mmix::nxor()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) ^ R(z)) );
}

void mmix::nxori()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, ~( R(y) ^ z) );
}

void mmix::mux()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y) & g(UnsignedMorsel(rM))) | (R(z) & ~g(UnsignedMorsel(rM))) );
}

void mmix::muxi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(x, (R(y) & g(UnsignedMorsel(rM))) | (z & ~g(UnsignedMorsel(rM))) );
}

void mmix::sadd()
        {UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        UnsignedMorsel temp( R(y) & ~R(z) );
        R(x, temp.count() );
        //R(x, (new std::bitset<64>( R(y) & ~R(z) ))->count() );
        }
 
void mmix::saddi()
        {UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        UnsignedMorsel temp( R(y) & ~z );
        R(x, temp.count() );
        //R(x, (new std::bitset<64>( R(y) & ~R(z) ))->count() );
        }
 
void mmix::bdif()
        {UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        UnsignedMorsel b0 = ((R(y)>> 0)&0xFF) - ((R(z)>> 0)&0xFF);
        UnsignedMorsel b1 = ((R(y)>> 8)&0xFF) - ((R(z)>> 8)&0xFF);
        UnsignedMorsel b2 = ((R(y)>>16)&0xFF) - ((R(z)>>16)&0xFF);
        UnsignedMorsel b3 = ((R(y)>>24)&0xFF) - ((R(z)>>24)&0xFF);
        UnsignedMorsel b4 = ((R(y)>>32)&0xFF) - ((R(z)>>32)&0xFF);
        UnsignedMorsel b5 = ((R(y)>>40)&0xFF) - ((R(z)>>40)&0xFF);
        UnsignedMorsel b6 = ((R(y)>>48)&0xFF) - ((R(z)>>48)&0xFF);
        UnsignedMorsel b7 = ((R(y)>>56)&0xFF) - ((R(z)>>56)&0xFF);
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
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
         {
        UnsignedMorsel b0 = ((R(y)>> 0)&0xFF) - ((uzi>> 0)&0xFF);
        UnsignedMorsel b1 = ((R(y)>> 8)&0xFF) - ((uzi>> 8)&0xFF);
        UnsignedMorsel b2 = ((R(y)>>16)&0xFF) - ((uzi>>16)&0xFF);
        UnsignedMorsel b3 = ((R(y)>>24)&0xFF) - ((uzi>>24)&0xFF);
        UnsignedMorsel b4 = ((R(y)>>32)&0xFF) - ((uzi>>32)&0xFF);
        UnsignedMorsel b5 = ((R(y)>>40)&0xFF) - ((uzi>>40)&0xFF);
        UnsignedMorsel b6 = ((R(y)>>48)&0xFF) - ((uzi>>48)&0xFF);
        UnsignedMorsel b7 = ((R(y)>>56)&0xFF) - ((uzi>>56)&0xFF);
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
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
          {
        UnsignedMorsel w0 = ((R(y)>> 0)&0xFFFF) - ((R(z)>> 0)&0xFFFF);
        UnsignedMorsel w1 = ((R(y)>>16)&0xFFFF) - ((R(z)>>16)&0xFFFF);
        UnsignedMorsel w2 = ((R(y)>>32)&0xFFFF) - ((R(z)>>32)&0xFFFF);
        UnsignedMorsel w3 = ((R(y)>>48)&0xFFFF) - ((R(z)>>48)&0xFFFF);
        w0 = (w0 < 0) ? 0 : w0;
        w1 = (w0 < 0) ? 0 : w1;
        w2 = (w0 < 0) ? 0 : w2;
        w3 = (w0 < 0) ? 0 : w3;
        R(x, (w3<<48)&(w2<<32)&(w1<<16)&(w0<<0) );
        }
 
}

void mmix::wdifi()
        {UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
  
        UnsignedMorsel w0 = ((R(y)>> 0)&0xFFFF) - ((uzi>> 0)&0xFFFF);
        UnsignedMorsel w1 = ((R(y)>>16)&0xFFFF) - ((uzi>>16)&0xFFFF);
        UnsignedMorsel w2 = ((R(y)>>32)&0xFFFF) - ((uzi>>32)&0xFFFF);
        UnsignedMorsel w3 = ((R(y)>>48)&0xFFFF) - ((uzi>>48)&0xFFFF);
        w0 = (w0 < 0) ? 0 : w0;
        w1 = (w0 < 0) ? 0 : w1;
        w2 = (w0 < 0) ? 0 : w2;
        w3 = (w0 < 0) ? 0 : w3;
        R(x, (w3<<48)&(w2<<32)&(w1<<16)&(w0<<0) );
        }
 
void mmix::tdif()
        {UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  
        UnsignedMorsel t0 = ((R(y)>> 0)&0xFFFFFFFF) - ((R(z)>> 0)&0xFFFFFFFF);
        UnsignedMorsel t1 = ((R(y)>>32)&0xFFFFFFFF) - ((R(z)>>32)&0xFFFFFFFF);
        t0 = (t0 < 0) ? 0 : t0;
        t1 = (t1 < 0) ? 0 : t1;
        R(x, (t1<<32)&(t0<<0) );
        }
 
void mmix::tdifi()
        {UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
 
        UnsignedMorsel t0 = ((R(y)>> 0)&0xFFFFFFFF) - ((uzi>> 0)&0xFFFFFFFF);
        UnsignedMorsel t1 = ((R(y)>>32)&0xFFFFFFFF) - ((uzi>>32)&0xFFFFFFFF);
        t0 = (t0 < 0) ? 0 : t0;
        t1 = (t1 < 0) ? 0 : t1;
        R(x, (t1<<32)&(t0<<0) );
        }
 
void mmix::ofif()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         {
        UnsignedMorsel u0 = (R(y)) - R(z);
        u0 = (u0 > R(y)) ? 0 : u0;
        }
 
}

void mmix::odifi()
{ UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
        {
        UnsignedMorsel u0 = (R(y)) - z;
        u0 = (u0 > R(y)) ? 0 : u0;
        }
 
}

void mmix::odif()
{
  return;
}
void mmix::mor()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         {
        UnsignedMorsel r;
        for (unsigned int i=0; i<64; i++)
          for (unsigned int j=0; j<64; j++)
          {
            r = (R(z)>>i)&(R(y)>>i);
            for (unsigned int k=2; k<=64;k++)
              r = r | ( (R(z)>>(i+8*k)) & (R(y)>>(k+8*j)) );
            r=r&1;
            R(x, (R(x) & (static_cast<unsigned int>(~(1<<(i+8*j))))) | (r<<(i+8*j)) );
          }
        }
}

void mmix::mori()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
        {
        UnsignedMorsel r;
        for (unsigned int i=0; i<64; i++)
          for (unsigned int j=0; j<64; j++)
          {
            r = (z>>i)&(R(y)>>i);
            for (unsigned int k=2; k<=64;k++)
              r = r | ( (z>>(i+8*k)) & (R(y)>>(k+8*j)) );
            r=r&1;
            R(x, (R(x) & static_cast<unsigned int>(~(1<<(i+8*j)))) | (r<<(i+8*j)) );
          }
        }
}

void mmix::mxor()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         {
        UnsignedMorsel r;
        for (unsigned int i=0; i<8; i++)
          for (unsigned int j=0; j<8; j++)
          {
            r = (R(z)>>i)&(R(y)>>i);
            for (unsigned int k=1; k<8;k++)
              r = r ^ ( (R(z)>>(i+8*k)) & (R(y)>>(k+8*j)) );
            r=r&1;
            R(x, (R(x) & static_cast<unsigned int>(~(1<<(i+8*j)))) | (r<<(i+8*j)) );
          }
        }
 
}

void mmix::mxori()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         {
        UnsignedMorsel r;
        for (unsigned int i=0; i<8; i++)
          for (unsigned int j=0; j<8; j++)
          {
            r = (z>>i)&(R(y)>>i);
            for (unsigned int k=1; k<8;k++)
              r = r ^ ( (z>>(i+8*k)) & (R(y)>>(k+8*j)) );
            r=r&1;
            R(x, (R(x) & static_cast<unsigned int>(~(1<<(i+8*j)))) | (r<<(i+8*j)) );
          }
        }
 
}

void mmix::fadd()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
{
        //double t = ( *(dynamic_cast<double*>(&uy)) + *(dynamic_cast<double*>(&uz)) );
        unsigned long long int t = 3.14159;
        R(x, *(static_cast<unsigned long long int *>(&t) ));
        }

}

void mmix::fsub()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         {
        //double t = ( *(double*)&uy - *(double*)&uz );
        //R(x, *(unsigned long long int *)&t );
        R(x, 92ull);
        }
 
}

void mmix::fmul()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         {
        //double t = ( *(double*)&uy * *(double*)&uz );
        //R(x, *(unsigned long long int *)&t );
        R(x, 92ull);
        }
 
}

void mmix::fdiv()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         {
        //double t = ( *(double*)&uy / *(double*)&uz );
        //R(x, *(unsigned long long int *)&t );
        R(x, 92ull);
        }
 
}

void mmix::frem()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
        {
        //double t = fmod( *(double*)&uy , *(double*)&uz );
        //R(x, *(unsigned long long int *)&t );
        R(x, 92ull);
        }
 
}

void mmix::fsqrt()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         {
        //double t = sqrt( *(double*)&uz );
        //R(x, *(unsigned long long int *)&t );
        R(x, 92ull);
        }
 
}

void mmix::fint()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  //R(x, (unsigned long long int)( *(double*)&uz ));
  R(x, 92ull);
}

void mmix::fcmp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
         //R(x, ((*(double*)&uy > *(double*)&uz) ? 1 : 0) 
           //- ((*(double*)&uy < *(double*)&uz) ? 1 : 0) );
        R(x, 92ull);
 
}

void mmix::feql()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
        //R(x, *(double*)&uy == *(double*)&uz );
      R(x, 92ull);
}

void mmix::fun()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
        R(x, fy != fy || fz != fz );
}

void mmix::fcmpe()
        {UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
        int e;
        frexp(fy.asFloat(), &e);
        //R(x, (fy > fz && !N(fy.asFloat(), fz.asFloat(), e, frE.asFloat())) -
         //    (fy < fz && !N(fy.asFloat(), fz.asFloat(), e, frE.asFloat())));
      R(x, 92ull);
        }
 
void mmix::feqle()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
        int e;
        frexp(fy.asFloat(), &e);
        //R(x, N(fy.asFloat(), fz.asFloat(), e, frE.asFloat()));
        R(x, 92ull);
}

void mmix::fune()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, fy != fy || fz != fz || frE != frE);
}

void mmix::fix()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, fz);
}

void mmix::fixu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, fz);
}

void mmix::flot()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
        R(x, sz);
}

void mmix::flotu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, uz);
}

void mmix::flotui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, z);
}

void mmix::sflot()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, fz);
}

void mmix::sfloti()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, z);
}

void mmix::sflotu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, fz);
}

void mmix::sflotui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, z);
}

void mmix::ldsf()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
  R(x, M(4, a));
}

void mmix::ldsfi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, M(4, uy+z));
}

void mmix::stsf()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
  M(4, a, fx);
}

void mmix::stsfi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  M(4, uy+z, fx);
}

void mmix::floti()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  R(x, z);
}

void mmix::seth()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
 UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
  R(x, ((uyi<<8) & uzi) << 48);
}

void mmix::setmh()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
 UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
   R(x, ((uyi<<8) & uzi) << 32);
}

void mmix::setml()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
   R(x, ((uyi<<8) & uzi));
}

void mmix::setl()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
 
  R(x, ((uyi<<8) & uzi) << 16);
}

void mmix::inch()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
 
  R(x, R(x) + (((uyi<<8) & uzi) << 48) );
}

void mmix::incmh()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
 
  R(x, R(x) + (((uyi<<8) & uzi) << 32) );
}

void mmix::incml()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
 
  R(x, R(x) + (((uyi<<8) & uzi) << 16) );
}

void mmix::incl()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
   R(x, ux + ((uyi<<8) & uzi) );
}

void mmix::orh()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
   R(x, ux | (((uyi<<8) & uzi)<<48) );
}

void mmix::ormh()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
   R(x, ux | (((uyi<<8) & uzi)<<32) );
}

void mmix::orml()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
   R(x, ux | (((uyi<<8) & uzi)<<16) );
}

void mmix::orl()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
         R(x, ux | ((uyi<<8) & uzi) );
}

void mmix::andnh()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
         R(x, ux & ~(((uyi<<8) & uzi)<<48) );
}

void mmix::andnmh()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
   R(x, ux & ~(((uyi<<8) & uzi)<<32) );
}

void mmix::andnml()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
   R(x, ux & ~(((uyi<<8) & uzi)<<16) );
}

void mmix::andnl()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel uxi = x;
  UnsignedMorsel uyi = y;
  UnsignedMorsel uzi = z;
   R(x, ux & ~((uyi<<8) & uzi) );
}

void mmix::go()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
        R(UnsignedMorsel(x), target);
        target = a;
}

void mmix::bn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sx < 0 ) ? ra : target );
}

void mmix::bz()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( uz == 0 ) ? ra : target );
}

void mmix::bp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz > 0 ) ? ra : target );
}

void mmix::bod()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 1 ) ? ra : target );
}

void mmix::bnn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz >= 0 ) ? ra : target );
}

void mmix::bnz()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz != 0 ) ? ra : target );
}

void mmix::bnp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz <= 0 ) ? ra : target );
}

void mmix::bev()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 0 ) ? ra : target );
}

void mmix::pbn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sx < 0 ) ? ra : target );
}

void mmix::pbz()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( uz == 0 ) ? ra : target );
}

void mmix::pbp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz > 0 ) ? ra : target );
}

void mmix::pbod()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 1 ) ? ra : target );
}

void mmix::pbnn()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz >= 0 ) ? ra : target );
}

void mmix::pbnz()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz != 0 ) ? ra : target );
}

void mmix::pbnp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( sz <= 0 ) ? ra : target );
}

void mmix::pbev()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  target = ( ( (uz & 1) == 0 ) ? ra : target );
}

void mmix::jmpb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = -1*ra;
  target = 1*ra;
}

void mmix::bnb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sx < 0 ) ? -1*ra : target );
  target = ( ( sx < 0 ) ? 1*ra : target );
}

void mmix::bzb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( uz == 0 ) ? -1*ra : target );
  target = ( ( uz == 0 ) ? 1*ra : target );
}

void mmix::bpb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sz > 0 ) ? -1*ra : target );
  target = ( ( sz > 0 ) ? 1*ra : target );
}

void mmix::bodb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( (uz & 1) == 1 ) ? -1*ra : target );
  target = ( ( (uz & 1) == 1 ) ? 1*ra : target );
}

void mmix::bnnb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sz >= 0 ) ? -1*ra : target );
  target = ( ( sz >= 0 ) ? 1*ra : target );
}

void mmix::bnzb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sz != 0 ) ? -1*ra : target );
  target = ( ( sz != 0 ) ? 1*ra : target );
}

void mmix::bnpb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sz <= 0 ) ? -1*ra : target );
  target = ( ( sz <= 0 ) ? 1*ra : target );
}

void mmix::bevb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( (uz & 1) == 0 ) ? -1*ra : target );
  target = ( ( (uz & 1) == 0 ) ? 1*ra : target );
}

void mmix::pbnb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sx < 0 ) ? -1*ra : target );
  target = ( ( sx < 0 ) ? 1*ra : target );
}

void mmix::pbzb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( uz == 0 ) ? -1*ra : target );
  target = ( ( uz == 0 ) ? 1*ra : target );
}

void mmix::pbpb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sz > 0 ) ? -1*ra : target );
  target = ( ( sz > 0 ) ? 1*ra : target );
}

void mmix::pbodb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( (uz & 1) == 1 ) ? -1*ra : target );
  target = ( ( (uz & 1) == 1 ) ? 1*ra : target );
}

void mmix::pbnnb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sz >= 0 ) ? -1*ra : target );
  target = ( ( sz >= 0 ) ? 1*ra : target );
}

void mmix::pbnzb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sz != 0 ) ? -1*ra : target );
  target = ( ( sz != 0 ) ? 1*ra : target );
}

void mmix::pbnpb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( sz <= 0 ) ? -1*ra : target );
  target = ( ( sz <= 0 ) ? 1*ra : target );
}

void mmix::pbevb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  //target = ( ( (uz & 1) == 0 ) ? -1*ra : target );
  target = ( ( (uz & 1) == 0 ) ? 1*ra : target );
}

void mmix::pushj()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  push(x);
  R(UnsignedMorsel(rJ), (target+4));
  target = ra;
}

void mmix::pushjb()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel ra = 4*( (uy<<8) & uz);
 
  push(x);
  R(UnsignedMorsel(rJ), (1*(target+4)));
  target = ra;
}

void mmix::pushgo()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
  push(x);
  R(UnsignedMorsel(rJ), (target+4));
  target = a;
}

void mmix::pop()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  pop(x);
  target = R(UnsignedMorsel(rJ))+4*((y<<8) & z);
}

void mmix::save()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  push(255);
  R(x, register_stack_top);
}
void mmix::unsave()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
 
  pop(255);
  R(x, register_stack_top);
}

void mmix::ldunc()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
  R( x, (M(8, a)) );
}
void mmix::stunc()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
  M(8, a, R(x));
}
void mmix::ldunci()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
  R( x, (M(8, a)) );
}

void mmix::cswap()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
        if (M(8, a) == R(UnsignedMorsel(rP))) {
          M(8,a,R(x));
          R(x,1);
        } else {
          R(UnsignedMorsel(rP), M(8,a));
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
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

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
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

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
            for (unsigned int i=0; M(1,R(UnsignedMorsel(255)) + i)  != '\0'; i++)
            {
              printf( "%c", M(1,R(UnsignedMorsel(255)) + i).asChar() );
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
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  R(x, g(z));
}

void mmix::put()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  g(x, R(z));
}

void mmix::puti()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  g(x, z);
}

void mmix::geta()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  UnsignedMorsel ra = 4*( (uy<<8) & uz);
  R(x, ra);
}

void mmix::getab()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  UnsignedMorsel ra = 4*( (uy<<8) & uz);
  //R(x, -1*ra);
  R(x, 1*ra);
}

void mmix::jmp()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

 // UnsignedMorsel 
  //target = getip()+stepsize;
  target = UnsignedMorsel( (x<<16) & (y<<8) & (z<<0) );
  setip( target );
}

void mmix::ldb()
{ 
  UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 

  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);

  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
  R(x, M(1, a));
}

void mmix::ldbi()
{
  UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 

  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);


  R(x, M(1, (uy + z)));
}

void mmix::ldbu()
{
  UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 

  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);

  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );
 
  R(x, M(1, a));
}

void mmix::ldbui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  R(x, M(1, (uy+z)));
}

void mmix::ldw()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );

  R(x, M(2, a));
}

void mmix::ldwi()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  R(x, M(2, uy+z));
}

void mmix::ldwu()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );

  R(x, M(2, a) );
}

void mmix::ldwui()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));

  R(x, M(2, uy+z) );
}

void mmix::ldt()
{UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel ux = R(x); // unsigned values at the given addresses
  UnsignedMorsel uy = R(y);
  UnsignedMorsel uz = R(z);
  UnsignedMorsel sx = R(x); // signed values
  UnsignedMorsel sy = R(y);
  UnsignedMorsel sz = R(z);
  UnsignedMorsel fx = R(x); // signed double values
  UnsignedMorsel fy = R(y);
  UnsignedMorsel fz = R(z);
  UnsignedMorsel frE = R(UnsignedMorsel(rE));
  UnsignedMorsel a = ( (uy + uz) & UnsignedMorsel(0xFFFFFFFFFFFFFFFF) );

  R(x, (M(4, a)) );
}

void mmix::ldti()
{ 
  UnsignedMorsel x = M(1, getip()+1); 
  UnsignedMorsel y = M(1, getip()+2); 
  UnsignedMorsel z = M(1, getip()+3); 
  UnsignedMorsel uy = R(y);
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
