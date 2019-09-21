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
  unsigned char op,x,y,z;

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
    
    op = tetra.ar[0];
    x = tetra.ar[1];
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
/*
  // Pre-fetch register numbers, and their values, for convenience
  Morsel x = M(1, getip()+1); 
  Morsel y = M(1, getip()+2); 
  Morsel z = M(1, getip()+3); 
  Morsel uxi = x;
  Morsel uyi = y;
  Morsel uzi = z;

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

  // A <- (u($Y) + u($Z)) mod 2^64
  Morsel a = ( (uy + uz) & Morsel(0xFFFFFFFFFFFFFFFF) );
  Morsel ra = 4*( (uy<<8) & uz);
*/

  static void (*syscalls[])(void) = {
  };
// instruction_map[inst]();
  switch (inst)
  {
      case JMP: // JMP
        jmp();
        break;

      case LDB: // LDB: s($X) <- s(M_1[A])
        ldb();
        break;

      case LDBI: // s($X) <- s(M_1[A])
        ldbi();
        break;

      case LDBU: // LDBU: u($X) <- u(M_1[A])
        ldbu();
        break;

      case LDBUI: // u($X) <- u(M_1[A])
        ldbui();
        break;

      case LDW: // s($X) <- s(M_2[A])
        ldw();
        break;

      case LDWI: // s($X) <- s(M_2[A])
        ldwi();
        break;

      case LDWU: // LDWU: u($X) <- u(M_2[A])
        ldwu();
        break;
  
      case LDWUI: // u($X) <- u(M_2[A])
        ldwui();
        break;

      case LDT: // LDT: s($X) <- s(M_4[A])
        ldt();
        break;

      case LDTI: // s($X) <- s(M_4[A])
        ldti();
        break;

      case LDTU: // u($X) <- u(M_4[A])
        ldtu();
        break;

      case LDTUI: // u($X) <- u(M_4[A])
        ldtui();
        break;

      case LDO: // s($X) <- s(M_8[A])
        ldo();
        break;

      case LDOI: // s($X) <- s(M_8[A])
        ldoi();
        break;

      case LDOU: // u($X) <- u(M_8[A])
        ldou();
        break;

      case LDOUI: // u($X) <- u(M_8[A])
        ldoui();
        break;

      case LDHT: // u($X) <- u(M_4[A]) x 2^32
        ldht();
        break;

      case LDHTI: // u($X) <- u(M_4[A]) x 2^32
        ldhti();
        break;

      case STB: // s(M_1[A]) <- s($X)
        stb();
        break;

      case STBI: // s(M_1[A]) <- s($X)
        stbi();
        break;

      case STW: // s(M_2[A]) <- s($X)
        stw();
        break;

      case STWI: // s(M_2[A]) <- s($X)
        stwi();
        break;
      
      case STT: // s(M_4[A] <- s($X)
        stt();
        break;
      case STTI: // s(M_4[A] <- s($X)
        stti();
        break;
      case STO: // s(M_8[A] <- s($X)
        sto();
        break;
      case STOI: // s(M_8[A] <- s($X)
        stoi();
        break;

      case STBU: // u(M_1[A]) <- u($X) mod 2^8
        stbu();
        break;

      case STBUI: // u(M_1[A]) <- u($X) mod 2^8
        stbui();
        break;
      case STWU: // u(M_2[A]) <- u($X) mod 2^16
        stwu();
        break;
      case STWUI: // u(M_2[A]) <- u($X) mod 2^16
        stwui();
        break;
      case STTU: // u(M_4[A]) <- u($X) mod 2^32
        sttu();
        break;
      case STTUI: // u(M_4[A]) <- u($X) mod 2^32
        sttui();
        break;
      case STOU: // u(M_8[A]) <- u($X)
        stou();
        break;
      case STOUI: // u(M_8[A]) <- u($X)
        stoui();
        break;
      case STHT: // u(M_4[A]) <- floor( u($X) / 2^32 )
        stht();
        break;
      case STHTI: // u(M_4[A]) <- floor( u($X) / 2^32 )
        sthti();
        break;
      case STCO: // u(M_8[A]) <- X
        stco();
        break;
      case STCOI: // u(M_8[A]) <- X
        stcoi();
        break;
      case ADD: // s($X) <- s($Y) + s($Z)
        add();
        break;
      case ADDI: // s($X) <- s($Y) + s(Z)
        addi();
        break;
      case SUB: // s($X) <- s($Y) - s($Z)
        sub();
        break;
      case SUBI: // s($X) <- s($Y) - s(Z)
        subi();
        break;
      case MUL: // s($X) <- s($Y) x s($Z)
        mul();
        break;

      case MULI: // s($X) <- s($Y) x s(Z)
        muli();
        break;

      case DIV: // s($X) <- floor(s($Y) / s($Z)) such that ($Z != 0)
                // and s(rR) <- s($Y) mod s($Z)
        div();
       break;

      case DIVI: // s($X) <- floor(s($Y) / s(Z) such that (Z != 0)
                // and s(rR) <- s($Y) mod s(Z)
        divi();
       break;
      case ADDU: // u($X) <- (u($Y) + u($Z)) mod 2^64
        addu();
        break;
      case ADDUI: // u($X) <- (u($Y) + u(Z)) mod 2^64
        addui();
        break;
      case SUBU: // u($X) <- (u($Y) - u($Z)) mod 2^64
        subu();
        break;
      case SUBUI: // u($X) <- (u($Y) - u(Z)) mod 2^64
        subui();
        break;
      case MULU: // u(rH $X) <- u($Y) x u($Z)
        mulu();
       break;

      case MULUI: // u($X) <- (u($Y) - u(Z)) mod 2^64
        mului();
       break;

      case DIVU: // u($X) <- floor(u(rD $Y) / u($Z))
                 // u(rR) <- u(rD $Y) mod u($Z), if u($Z) > u(rD);
                 //     otherwise $X <- rD, rR <- $Y

        break;

      case DIVUI: // u($X) <- floor(u(rD $Y) / u(Z))
                  // u(rR) <- u(rD $Y) mod u(Z), if u(Z) > u(rD);
                  //    otherwise $X <- rD, rR <- $Y
       break;

      case i2ADDU: // u($X) <- (u($Y) x 2 + u($Z)) mod 2^4
        i2addu();
        break;
      case i2ADDUI: // u($X) <- (u($Y) x 2 + u(Z)) mod 2^4
        i2addui();
        break;
      case i4ADDU: // u($X) <- (u($Y) x 4 + u($Z)) mod 2^64
        i4addu();
        break;
      case i4ADDUI: // u($X) <- (u($Y) x 4 + u(Z)) mod 2^64
        i4addui();
        break;
      case i8ADDU: // u($X) <- (u($Y) x 8 + u($Z)) mod 2^64
        i8addu();
        break;
      case i8ADDUI: // u($X) <- (u($Y) x 8 + u(Z)) mod 2^64
        i8addui();
        break;
      case i16ADDU: // u($X) <- (u($Y) x 16 + u($Z)) mod 2^64
        i16addu();
        break;
      case i16ADDUI: // u($X) <- (u($Y) x 16 + u(Z)) mod 2^64
        i16addui();
        break;
      case NEG: // s($X) <- Y - s($Z)
        neg();
        break;
      case NEGI: // s($X) <- Y - s(Z)
        negi();
        break;
      case NEGU: // u($X) <- (Y - u($Z)) mod 2^64
        negu();
        break;
      case NEGUI: // u($X) <- (Y - u(Z)) mod 2^64
        negui();
        break;
      case SL: // s($X) <- s($Y) x 2^u($Z)
        sl();
        break;
      case SLI: // s($X) <- s($Y) x 2^u(Z)
        sli();
        break;
      case SLU: // u($X) <- (u($Y) x 2^u($Z)) mod 2^64
        slu();
        break;
      case SLUI: // u($X) <- (u($Y) x 2^u(Z)) mod 2^64
        slui();
        break;
      case SR: // s($X) <- floor( s($Y) / 2^u($Z) )
        sr();
        break;
      case SRI: // s($X) <- floor( s($Y) / 2^u(Z) )
        sri();
        break;
      case SRU: // u($X) <- floor( u($Y) / 2^u($Z) )
        sru();
        break;
      case SRUI: // u($X) <- floor( u($Y) / 2^u(Z) )
        srui();
        break;
      case CMP: // s($X) <- [s($Y) > s($Z)] - [s($Y) < s($Z)]
        cmp();
        break;
      case CMPI: // s($X) <- [s($Y) > s(Z)] - [s($Y) < s(Z)]
        cmpi();
        break;
      case CMPU: // s($X) <- [u($Y) > u($Z)] - [u($Y) < u($Z)]
        cmpu();
        break;
      case CMPUI: // s($X) <- [u($Y) > u(Z)] - [u($Y) < u(Z)]
        cmpui();
        break;
      case CSN: // if s($Y) < 0, set $X <- $Z
        csn();
        break;
      case CSNI: // if s($Y) < 0, set $X <- Z
        csni();
        break;
      case CSZ: // if $Y = 0, set $X <- $Z
        csz();
        break;
      case CSZI: // if $Y = 0, set $X <- Z
        cszi();
        break;
      case CSP: // if s($Y) > 0, set $X <- $Z
        csp();
        break;
      case CSPI: // if s($Y) > 0, set $X <- Z
        cspi();
        break;
      case CSOD: // if s($Y) mod 2 == 1, set $X <- $Z
        csod();
        break;
      case CSODI: // if s($Y) mod 2 == 1, set $X <- Z
        csodi();
        break;
      case CSNN: // if s($Y) >= 0, set $X <- $Z
        csnn();
        break;
      case CSNNI: // if s($Y) >= 0, set $X <- Z
        csnni();
        break;
      case CSNZ: // if $Y != 0, set $X <- $Z
        csnz();
        break;
      case CSNZI: // if $Y != 0, set $X <- Z
        csnzi();
        break;
      case CSNP: // if s($Y) <= 0, set $X <- $Z
        csnp();
        break;
      case CSNPI: // if s($Y) <= 0, set $X <- Z
        csnpi();
        break;
      case CSEV: // if s($Y) mod 2 == 0, set $X <- $Z
        csev();
        break;
      case CSEVI: // if s($Y) mod 2 == 0, set $X <- Z
        csevi();
        break;
      case ZSN: // $X <- $Z[s($Y) < 0]
        zsn();
        break;
      case ZSNI: // $X <- Z[s($Y) < 0]
        zsni();
        break;
      case ZSZ: // $X <- $Z[$Y = 0]
        zsz();
        break;
      case ZSZI: // $X <- Z[$Y = 0]
        zszi();
        break;
      case ZSP: // $X <- $Z[s($Y) > 0]
        zsp();
        break;
      case ZSPI: // $X <- Z[s($Y) > 0]
        zspi();
        break;
      case ZSOD: // $X <- $Z[s($Y) mod 2 == 1]
        zsod();
        break;
      case ZSODI: // $X <- Z[s($Y) mod 2 == 1]
        zsodi();
        break;
      case ZSNN: // $X <- $Z[s($Y) >= 0]
        zsnn();
        break;
      case ZSNNI: // $X <- Z[s($Y) >= 0]
        zsnni();
        break;
      case ZSNZ: // $X <- $Z[s($Y) != 0]
        zsnz();
        break;
      case ZSNZI: // $X <- Z[s($Y) != 0]
        zsnzi();
        break;
      case ZSNP: // $X <- $Z[s($Y) <= 0]
        zsnp();
        break;
      case ZSNPI: // $X <- Z[s($Y) <= 0]
        zsnpi();
        break;
      case ZSEV: // $X <- $Zs($Y) mod 2 == 0]
        zsev();
        break;
      case ZSEVI: // $X <- Zs($Y) mod 2 == 0]
        zsevi();
        break;
      case AND: // v($X) <- v($Y) & v($Z)
        opcode_AND();
        break;
      case ANDI: // v($X) <- v($Y) & v(Z)
        andi();
        break;
      case OR: // v($X) <- v($Y) v v($Z)
        opcode_OR();
        break;
      case ORI: // v($X) <- v($Y) v v(Z)
        ori();
        break;
      case XOR: // v($X) <-  v($Y) xor v($Z)
        opcode_xor();
        break;
      case XORI: // v($X) <-  v($Y) xor v(Z)
        xori();
        break;
      case ANDN: // v($X) <- v($Y) v ~v($Z)
        andn();
        break;
      case ANDNI: // v($X) <- v($Y) v ~v(Z)
        andni();
        break;
      case ORN: // v($X) <- v($Y) v ~v($Z)
        orn();
        break;
      case ORNI: // v($X) <- v($Y) v ~v(Z)
        orni();
        break;
      case NAND: // ~v($X) <- v($Y) & v($Z)
        nand();
        break;
      case NANDI: // ~v($X) <- v($Y) & v(Z)
        nandi();
        break;
      case NOR: // ~v($X) <- v($Y) v v($Z)
        nor();
        break;
      case NORI: // ~v($X) <- v($Y) v v(Z)
        nori();
        break;
      case NXOR: // ~v($X) <- v($Y) xor v($Z)
        nxor();
        break;
      case NXORI: // ~v($X) <- v($Y) xor v(Z)
        nxori();
        break;
      case MUX: // v($X) <- (v($Y) & v(rM)) | (v($Z) & ~v(rM))
        mux();
        break;
      case MUXI: // v($X) <- (v($Y) & v(rM)) | (v(Z) & ~v(rM))
        muxi();
        break;
      case SADD: // s($X) <- s(sum(v($Y) & ~v($Z)))
        sadd();
       break;

      case SADDI: // s($X) <- s(sum(v($Y) & ~v(Z)))
        saddi();
       break;

      case BDIF: // b($X) <- b($Y) .- b($Z)
        bdif();
       break;

      case BDIFI: // b($X) <- b($Y) .- b(Z)
        bdifi();
       break;

      case WDIF: // w($X) <- w($Y) - w($Z)
        wdif();
       break;

      case WDIFI: // w($X) <- w($Y) - w(Z)
        wdifi();
       break;

      case TDIF: // t($X) <- t($Y) - w($Z)
        tdif();
       break;

      case TDIFI: // t($X) <- t($Y) - w(Z)
        tdifi();
       break;

      case ODIF: // u($X) <- u($Y) - u($Z)
        odif();
       break;

      case ODIFI: // u($X) <- u($Y) - u(Z)
        odifi();
       break;

      case MOR: // m($X) <- m($Z) vx m($Y)
        mor();
        break;

      case MORI: // m($X) <- m(Z) vx m($Y)
        mori();
        break;
 
      case MXOR: // m($X) <- m($Z) xor x m($Y)
        mxor();
       break;

      case MXORI: // m($X) <- m(Z) xor x m($Y)
        mxori();
       break;

      case FADD: // f($X) <- f($Y) + f($Z)
        fadd();
        break;

      case FSUB: // f($X) <- f($Y) - f($Z)
        fsub();
       break;

      case FMUL: // f($X) <- f($Y) * f($Z)
        fmul();
        break;

      case FDIV: // f($X) <- f($Y) / f($Z)
        fdiv();
        break;

      case FREM: // f($X) <- f($Y) rem f($Z)
        frem();
       break;

      case FSQRT: // f($X) <-  f($Z)^(1/2)
        fsqrt();
       break;

      case FINT: // f($X) <- int f($Z)
        fint();
        break;

      case FCMP: // s($X) <- [f($Y) > f($Z)] - [f($Y) < f($Z)]
        fcmp();
        break;

      case FEQL: // s($X) <- [f($Y) == f($Z)]
        feql();
        break;

      case FUN: // s($X) <- [f($Y) || f($Z)]
        fun();
        break;

      case FCMPE: // s($X) <- [f($Y) } f($Z) (f(rE))] - [f($Y) { f($Z) (f(rE))]
        fcmpe();
        break;

      case FEQLE: // s($X) <- [f($Y) ~= f($Z) (f(rE))]
        feqle();
        break;

      case FUNE: // s($X) <- [f($Y) || f($Z) (f(rE))]
        fune();
        break;

      case FIX: // s($X) <- int f($Z)
        fix();
        break;

      case FIXU: // u($X) <- (int f($Z)) mod 2^64
        fixu();
        break;

      case FLOT: // f($X) <- s($Z)
        flot();
        break;

      case FLOTU: // f($x) <- u($Z)
        flotu();
        break;

      case FLOTUI: // f($X) <- u(Z)
        flotui();
        break;

      case SFLOT: // f($X) <- f(T) <- s($Z)
        sflot();
        break;

      case SFLOTI: // f($X) <- f(T) <- s(Z)
        sfloti();
        break;

      case SFLOTU: // f($X) <- f(T) <- u($Z)
        sflotu();
        break;

      case SFLOTUI: // f($X) <- f(T) <- u(Z)
        sflotui();
        break;

      case LDSF: // f($X) <- f(M_4[A])
        ldsf();
        break;

      case LDSFI: // f($X) <- f(M_4[A])
        ldsfi();
        break;
 
      case STSF: // f(M_4[A]) <- f($X)
        stsf();
        break;

      case STSFI: // f(M_4[A]) <- f($X)
        stsfi();
        break;

      case FLOTI: // f($X) <- s(Z)
        floti();
        break;

      case SETH: // u($X) <- YZ x2^48
        seth();
        break;

      case SETMH: // u($X) <- YZ x2^32
        setmh();
        break;

      case SETML: // u($Z) <- YZ
        setml();
        break;

      case SETL: // u($X) <- YZ x2^16
        setl();
        break;

      case INCH:  // u($X) <- ( u($X) + YZ x2^48) mod 2^64
        inch();
        break;

      case INCMH: // u($X) <- ( u($X) + YZ x2^32) mod 2^64
        incmh();
        break;

      case INCML: // u($X) <- ( u($X) + YZ x2^16) mod 2^64
        incml();
        break;
        
      case INCL: // u($X) <- ( u($X) + YZ) mod 2^64
        incl();
        break;

      case ORH: // v($X) <- v($X) v v(YZ << 48)
        orh();
        break;

      case ORMH: // v($X) <- v($X) v v(YZ << 32)
        ormh();
        break;

      case ORML: // v($X) <- v($X) v v(YZ << 16)
        orml();
        break;

      case ORL: // v($X) <- v($X) v v(YZ)
        orl();
        break;

      case ANDNH: // v($X) <- v($X) ^ ~v(YZ << 48)
        andnh();
        break;

      case ANDNMH: // v($X) <- v($X) ^ ~v(YZ<<32)
        andnmh();
        break;

      case ANDNML: // v($X) <- v($X) ^ ~v(YZ<<16)
        andnml();
        break;

      case ANDNL: // v($X) <- v($X) ^ ~v(YZ)
        andnl();
        break;
      
      case GO: // u($X) <- @+4, then @<-A
        go();
        break;

      case BN: // if s($X) < 0, set @ <- RA
        bn();
        break;

      case BZ: // if $X = 0, set @ <- RA
        bz();
        break;

      case BP: // if s($X) = 0, set @ <- RA
        bp();
        break;

      case BOD: // if s($X) mod 2 == 1, set @ <- RA
        bod();
        break;

      case BNN: // if s($X) >= 0, set @ <- RA
        bnn();
        break;

      case BNZ: // if s($X) != 0, set @ <- RA
        bnz();
        break;

      case BNP: // if s($X) <= 0, set @ <- RA
        bnp();
        break;

      case BEV: // if s($X) mod 2 == 0, set @ <- RA
        bev();
        break;

      case PBN: // if s($X) < 0, set @ <- RA
        pbn();
        break;

      case PBZ: // if $X = 0, set @ <- RA
        pbz();
        break;

      case PBP: // if s($X) = 0, set @ <- RA
        pbp();
        break;

      case PBOD: // if s($X) mod 2 == 1, set @ <- RA
        pbod();
        break;

      case PBNN: // if s($X) >= 0, set @ <- RA
        pbnn();
        break;

      case PBNZ: // if s($X) != 0, set @ <- RA
        pbnz();
        break;

      case PBNP: // if s($X) <= 0, set @ <- RA
        pbnp();
        break;

      case PBEV: // if s($X) mod 2 == 0, set @ <- RA
        pbev();
        break;

      case JMPB: 
        jmpb();
        break;

      case BNB: // if s($X) < 0, set @ <- -RA
        bnb();
        break;

      case BZB: // if $X = 0, set @ <- -RA
        bzb();
        break;

      case BPB: // if s($X) = 0, set @ <- -RA
        bpb();
        break;

      case BODB: // if s($X) mod 2 == 1, set @ <- -RA
        bodb();
        break;

      case BNNB: // if s($X) >= 0, set @ <- -RA
        bnnb();
        break;

      case BNZB: // if s($X) != 0, set @ <- -RA
        bnzb();
        break;

      case BNPB: // if s($X) <= 0, set @ <- -RA
        bnpb();
        break;

      case BEVB: // if s($X) mod 2 == 0, set @ <- -RA
        bevb();
        break;

      case PBNB: // if s($X) < 0, set @ <- -RA
        pbnb();
        break;

      case PBZB: // if $X = 0, set @ <- -RA
        pbzb();
        break;

      case PBPB: // if s($X) = 0, set @ <- -RA
        pbpb();
        break;

      case PBODB: // if s($X) mod 2 == 1, set @ <- -RA
        pbodb();
        break;

      case PBNNB: // if s($X) >= 0, set @ <- -RA
        pbnnb();
        break;

      case PBNZB: // if s($X) != 0, set @ <- -RA
        pbnzb();
        break;

      case PBNPB: // if s($X) <= 0, set @ <- -RA
        pbnpb();
        break;

      case PBEVB: // if s($X) mod 2 == 0, set @ <- -RA
        pbevb();
        break;

      case PUSHJ: // push(X) and set rJ<-@+4, then set @<-RA
        pushj();
        break;

      case PUSHJB: // push(X) and set rJ<-@+4, then set @<-RA
        pushjb();
        break;

      case PUSHGO: // push(X) and set rJ<-@+4, then set @<-A
        pushgo();
        break;

      case POP: // pop(X), then @<-rJ+4*YZ
        pop();
        break;

      case SAVE: // u($X) <- context
        save();
        break;

      case UNSAVE: // context<- u($Z)
        unsave();
        break;

      case LDUNC: // s($X) <- s(M_8[A])
        ldunc();
        break;

      case STUNC: // s(M_8[A]) <- s($X)
        stunc();
        break;

      case PRELD: 
        preld();
        break;

      case PREST:
        prest();
        break;

      case PREGO:
        prego();
        break;

      case SYNCID:
        syncid();
        break;

      case SYNCD:
        syncd();
        break;

      case SYNC:
        sync();
        break;

      case CSWAP: // if u(M_8[A] == u(rP), set u(M_8[A]) <- u($X) and
                  // u($X) <- 1. Otherwise set u(rP) <- u(M_8[A]) and
                  // u($X) <- 0
       cswap();
       break;

      case LDVTS: 
        ldvts();
        break;

      case CSWAPI: 
        cswapi();
        break;

      case LDUNCI:
        ldunci();
        break;

      case LDVTSI:
        ldvtsi();
        break;

      case PRELDI:
        preldi();
        break;

      case PREGOI:
        pregoi();
        break;

      case GOI:
        goi();
        break;

      case STUNCI: // s(M_8[A] <- s($X)
        stunci();
        break;

      case SYNCDI:
        syncdi();
        break;

      case PRESTI:
        presti();
        break;

      case SYNCIDI:
        syncidi();
        break;
      case PUSHGOI:
        pushgoi();
        break;

      case TRIP:
        trip();
        break;
      case TRAP:
        // MMIX simulators seem to implement 10 traps for basic i/o
        // the idea is that the NNIX kernel would implement all the trap
        // functions in a proper system.
        trap();
        break;

      case RESUME:
        resume();
        break;

      case GET: // u($X) <- u(g[Z]), where 0 <= Z < 32
        get();
        break;

      case PUT: // u(g[X]) <- u($Z), where 0 <= X < 32
        put();
        break;

      case PUTI: // u(g[X]) <- u(Z), where 0 <= X < 32
        puti();
        break;

      case GETA: // u($X) <- RA
        //R(x, ra);
        geta();
        break;

      case GETAB: // u($X) <- RA
        getab();
        break;

      case SWYM:
        swym();
        break;

      default:
          break;
  }

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
