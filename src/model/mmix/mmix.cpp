#include "mmix.h"
#include <bitset>
#include <math.h>
#include <iomanip>

mmix::mmix(int byte_size, int address_size) : cpu(byte_size, address_size,32)
{
  register_stack_top = 0;
}

bool mmix::N(double x, double f, unsigned int e, float epsilon)
{
  double d = fabs(x-f);
  return (d <= ((1<<(e-1022))*epsilon));
}

void mmix::wideMult(unsigned long long int a, unsigned long long int b,
    unsigned long long int *carry, unsigned long long int *result)
{
  uint64_t mask = 0x00000000FFFFFFFF;

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

  uint64_t ac = (a>>32) * (b>>32);
  uint64_t ad = (a&mask) * (b>>32);
  uint64_t bc = (a>>32) * (b&mask);
  uint64_t bd = (a&mask) * (b&mask);
  uint32_t o1 = bd & mask; // o1 = bd % 2^32
  uint64_t t1 = (bd>>32) + (bc&mask) + (ad&mask);
  uint32_t o2 = t1 & mask; // o2 = t1 % 2^32
  uint64_t t2 = (t1>>32) + (bc>>32) + (ad>>32) + (ac&mask);
  uint32_t o3 = t2 & mask;
  uint32_t o4 = (ac>>32) + (t2>>32);

  *carry  = ((((uint64_t) o4)<<32) | o3);
  *result = ((((uint64_t) o2)<<32) | o1);
}

void mmix::wideDiv(unsigned long long int numerator_hi,
                   unsigned long long int numerator_lo,
                   unsigned long long int divisor,
                   unsigned long long int *quotient,
                   unsigned long long int *remainder
                  )
{
  uint64_t a_hi = numerator_hi;
  uint64_t a_lo = numerator_lo;
  uint64_t b = divisor;

// quotient
  uint64_t q = a_lo << 1;

  // remainder
  uint64_t rem = a_hi;

  uint64_t carry = a_lo >> 63;
  uint64_t temp_carry = 0;
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
  char op,x,y,z;

  uint64_t lambda = 0;
  uint64_t address;
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
    
/*
    op =(tetra.num & 0xFF000000)>>16;
    x = (tetra.num & 0x00FF0000)>>16;
    y = (tetra.num & 0x0000FF00)>>8;
    z = (tetra.num & 0x000000FF)>>0;
*/
    op = tetra.ar[0];
    x = tetra.ar[1];
    y = tetra.ar[2];
    z = tetra.ar[3];

/*
    cout << "mmo_escape: " 
         << std::hex << setfill('0') << std::setw(2)
         << (int)(unsigned char)mmo_escape 
         << " and op: " 
         << std::hex << setfill('0') << std::setw(2)
         << (int)(unsigned char)tetra.ar[0] 
         << " and equivalencr: " 
          << ((char)mmo_escape == tetra.ar[0])
         << " and quoted_flag: " << quoted_flag
         << " and not quoted_flag: " << !quoted_flag
         << " and boolean: " 
          << (((char)mmo_escape == tetra.ar[0]) && !quoted_flag) 
          << "\n";
*/

	  if (((char)mmo_escape == tetra.ar[0]) && !quoted_flag)
	  {
//      cout << "switching on: "
//           << std::hex << setfill('0') << std::setw(2)
//          << (int)(unsigned char)tetra.ar[1] << "\n";
      switch (tetra.ar[1])
      {
        case lop_quote:
          quoted_flag = true;
          break;

        case lop_loc:
          {
          cout << "lop_loc called\n";
          uint64_t address = (uint64_t)y<<56;
          uint64_t offset = 0;
          union tetra_union addr1;
          union tetra_union addr2;
          in.read(addr1.ar,4);
          //in >> tetra.num;
          //offset = tetra1.num; // reverse this (little-endian)
          offset = (
              (addr1.ar[0]<<24) 
            | (addr1.ar[1]<<16) 
            | (addr1.ar[2]<<8)
            | (addr1.ar[3])
/*
              (((uint64_t) (unsigned char)(addr1.ar[0]))<<24) 
            & (((uint64_t) (unsigned char)(addr1.ar[1]))<<16) 
            & (((uint64_t) (unsigned char)(addr1.ar[2]))<<8)
            & (((uint64_t) (unsigned char)(addr1.ar[3])))
*/
          );
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
/*
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)(addr1.ar[0]<<24) 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)(addr1.ar[1]<<16)
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)(addr1.ar[2]<<8) 
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)(addr1.ar[3]) 
*/
          << endl;
          cout << "end first tetra" << endl;
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
            cout << "loaded second tetra of address: "
                 << std::hex << setfill('0') << std::setw(8)
                 << offset << endl;
          }
          lambda = address | offset;
          cout << "address: " 
                << std::hex << setfill('0') << std::setw(8)
                << address << endl
                << "offset: " 
                << std::hex << setfill('0') << std::setw(8)
                << offset << endl
                << "lambda: " 
                << std::hex << setfill('0') << std::setw(8)
                << lambda << endl;
          }
          break;

        case lop_skip: 
          lambda += ((y<<8) & z);
          break;

        case lop_fixo:
          {
          uint64_t address = 0;
          for (;z!=0;z--)
          {
            in >> tetra.num;
            address = address&((uint64_t)(tetra.num)<<32);
          }
          address += (((uint64_t)y)<<56);
          M(8, address, lambda);
          }
          break;

        case lop_fixr:
        {
          uint64_t delta = ((uint64_t)y<<32)&z;
          address = lambda - 4*delta;
          cout << "case lop_fixr\n";
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
          uint64_t P = lambda - 4*delta.num;
          M(8, P, lambda);
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
          cout << "lop_post called\n";
          special_registers[rG] = z;
          for (int i=0;i<=(256-G)*2;i+=2)
          {
            union tetra_union t1;
            union tetra_union t2;
            //in >> t1.num;
            in.read(t1.ar,4);
            //in >> t2.num;
            in.read(t2.ar,4);
            R(z+i, (unsigned long long)t1.num<<32 & (unsigned long long)t2.num);
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
      cout    << "saving "
              << std::hex << setfill('0') << std::setw(4)
              << tetra.num
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
              << " to " << lambda << "\n";
      M(4, lambda, tetra.num);
      unsigned long long v = M(4, lambda);
      cout << "value at " 
              << std::hex << setfill('0') << std::setw(2)
              << lambda
              << " is "
              << std::hex << setfill('0') << std::setw(2)
              << v << "\n";
      lambda = (lambda/4)*4+4;
	  }
  
  }
	
  in.close();
  return;
}

// Get value of memory at given address.
// Use size to determine whether to return byte, wyde, tetra, or octa
unsigned long long int
mmix::M(unsigned int size,
        unsigned long long address)
{
  const unsigned int octasize = 8;
  unsigned char octabyte[octasize];
  unsigned long long int base = address % size;

  // Load data into byte array
  for (int i=0;i<octasize;i++)
    octabyte[i] = view(base + i);

  // Copy data in byte array to 64-bit int
  unsigned long long int value = 0;
  for (int i=0;i<octasize;i++)
  {
    value & octabyte[octasize-(i+1)];
    value <<= 8;
  }
  value = value & (0xFF << (size - 1) );
  //printf("M1: At %llu, Value is: %llu\n", address, value);
  return value;
}

unsigned long long int
mmix::M( unsigned int size,
         unsigned long long address,
         unsigned long long int value)
{
  const unsigned int octasize = 8;
  unsigned char octabyte[octasize];
  unsigned long long int base = address - (address % size);
  cout << "base is "
            << std::hex << setfill('0') << std::setw(2)
            << (int) (unsigned char)(base)
        << " size is "
            << std::hex << setfill('0') << std::setw(2)
            << (unsigned int)(size)
        << " address is "
            << std::hex << setfill('0') << std::setw(16)
            << (unsigned long long)(address)
        << " value is "
            << std::hex << setfill('0') << std::setw(8)
            << (int) (unsigned char)(value);

  // Load data into byte array
  for (int i=0;i<size;i++)
    octabyte[i] = view(base + i);

  // Copy data in byte array to 64-bit int
  unsigned long long int retvalue = 0;
  for (int i=0;i<size;i++)
  {
    retvalue & octabyte[octasize-(i+1)];
    retvalue <<= 8;
  }
  retvalue = retvalue & (0xFF << (size - 1) );
  //printf("M2: Value is: %llu\n", retvalue);

  // Load new value into byte array
  char *value_array = static_cast<char*>(static_cast<void*>(&value));
  for (int i=0;i<size;i++)
    octabyte[i] = value_array[i];

  // Copy data in byte array to memory
  for (int i=0;i<size;i++)
  {
    cout << "M: calling load with "
              << std::hex << setfill('0') << std::setw(2)
              << (base+i)
        << " and "
              << std::hex << setfill('0') << std::setw(2)
              << (int) (unsigned char)octabyte[i] ;
    load(base+i,octabyte[i]);
  }

  return 0;
}

unsigned long long int mmix::R(unsigned int reg)
{
  //return registers[reg];
  return regs(reg);
}

unsigned long long int mmix::R(unsigned int reg, unsigned long long value)
{
  unsigned long long int ret = regs(reg); // registers[reg];
  //registers[reg] = value;
  regs(reg, value);
  return ret;
}

unsigned long long int mmix::g(unsigned int reg, unsigned long long value)
{
  unsigned long long int ret = globals[reg];
  globals[reg] = value;
  return ret;
}

unsigned long long int mmix::g(unsigned int reg)
{
  return globals[reg];
}

void mmix::push(unsigned char reg)
{
  for (int i=0;i<=reg;i++)
  {
    M(register_stack_top, R(i));
    register_stack_top++;
  }
}

void mmix::pop(unsigned char reg)
{
  for (int i=0;i<=reg;i++)
  {
    R(i, M(8, register_stack_top-1));
    register_stack_top--;
  }
}

void mmix::step(int inst)
{
  unsigned int stepsize = 4;
  // Pre-fetch register numbers, and their values, for convenience
  unsigned char x = M(1, getip()+1); 
  unsigned char y = M(1, getip()+2); 
  unsigned char z = M(1, getip()+3); 
  unsigned long long int uxi = x;
  unsigned long long int uyi = y;
  unsigned long long int uzi = z;
  unsigned long long int target = getip()+stepsize;

  unsigned long long int ux = R(x); // unsigned values at the given addresses
  unsigned long long int uy = R(y);
  unsigned long long int uz = R(z);
  long long int sx = R(x); // signed values
  long long int sy = R(y);
  long long int sz = R(z);
  double fx = R(x); // signed double values
  double fy = R(y);
  double fz = R(z);
  double frE = R(rE);

  // A <- (u($Y) + u($Z)) mod 2^64
  unsigned long long int a = ( (uy + uz ) & 0xFFFFFFFFFFFFFFFF );
  long long int ra = 4*( (uy<<8) & uz);

  switch (inst)
  {
      case JMP: // JMP
        target = ( (x<<16) & (y<<8) & (z<<0) );
        break;

      case LDB: // LDB: s($X) <- s(M_1[A])
        R( x, ((long long int) M(1, a)) );
        break;

      case LDBI: // s($X) <- s(M_1[A])
        R( x, ((long long int) M(1, (uy + z))) );
        break;

      case LDBU: // LDBU: u($X) <- u(M_1[A])
        R( x, M(1, a) );
        break;

      case LDBUI: // u($X) <- u(M_1[A])
        R( x, M(1, (uy+z) ) );
        break;

      case LDW: // s($X) <- s(M_2[A])
        R( x, ((long long int) M(2, a)) );
        break;

      case LDWI: // s($X) <- s(M_2[A])
        R( x, ((long long int) M(2, uy+z)) );
        break;

      case LDWU: // LDWU: u($X) <- u(M_2[A])
        R( x, M(2, a) );
        break;
  
      case LDWUI: // u($X) <- u(M_2[A])
        R( x, M(2, uy+z) );
        break;

      case LDT: // LDT: s($X) <- s(M_4[A])
        R( x, ((long long int) M(4, a)) );
        break;

      case LDTI: // s($X) <- s(M_4[A])
        R( x, ((long long int) M(4, uy+z)) );
        break;

      case LDTU: // u($X) <- u(M_4[A])
        R( x, M(4, a) );
        break;

      case LDTUI: // u($X) <- u(M_4[A])
        R( x, M(4, uy+z) );
        break;

      case LDO: // s($X) <- s(M_8[A])
        R( x, ((long long int) M(8, a)) );
        break;

      case LDOI: // s($X) <- s(M_8[A])
        R( x, ((long long int) M(8, uy+z)) );
        break;

      case LDOU: // u($X) <- u(M_8[A])
        R( x, M(8, a) );
        break;

      case LDOUI: // u($X) <- u(M_8[A])
        R( x, M(8, uy+z) );
        break;

      case LDHT: // u($X) <- u(M_4[A]) x 2^32
        R( x, (M(4, a) << 32));
        break;

      case LDHTI: // u($X) <- u(M_4[A]) x 2^32
        R( x, (M(4, uy+z) << 32));
        break;

      case STB: // s(M_1[A]) <- s($X)
        M(1, a, (long long int) R(x));
        break;

      case STBI: // s(M_1[A]) <- s($X)
        M(1, uy+z, (long long int) R(x));
        break;

      case STW: // s(M_2[A]) <- s($X)
        M(2, a, (long long int) R(x));
        break;

      case STWI: // s(M_2[A]) <- s($X)
        M(2, uy+z, (long long int) R(x));
        break;
      
      case STT: // s(M_4[A] <- s($X)
        M(4, a, (long long int) R(x));
        break;
      case STTI: // s(M_4[A] <- s($X)
        M(4, uy+z, (long long int) R(x));
        break;
      case STO: // s(M_8[A] <- s($X)
        M(8, a, (long long int) R(x));
        break;
      case STOI: // s(M_8[A] <- s($X)
        M(8, uy+z, (long long int) R(x));
        break;

      case STBU: // u(M_1[A]) <- u($X) mod 2^8
        M(1, a, R(x));
        break;

      case STBUI: // u(M_1[A]) <- u($X) mod 2^8
        M(1, uy+z, R(x));
        break;
      case STWU: // u(M_2[A]) <- u($X) mod 2^16
        M(2, a, R(x));
        break;
      case STWUI: // u(M_2[A]) <- u($X) mod 2^16
        M(2, uy+z, R(x));
        break;
      case STTU: // u(M_4[A]) <- u($X) mod 2^32
        M(4, a, R(x));
        break;
      case STTUI: // u(M_4[A]) <- u($X) mod 2^32
        M(4, uy+z, R(x));
        break;
      case STOU: // u(M_8[A]) <- u($X)
        M(8, a, R(x));
        break;
      case STOUI: // u(M_8[A]) <- u($X)
        M(8, uy+z, R(x));
        break;
      case STHT: // u(M_4[A]) <- floor( u($X) / 2^32 )
        M(4, a, ((R(x) & 0xFFFFFFFF00000000) >> 16));
        break;
      case STHTI: // u(M_4[A]) <- floor( u($X) / 2^32 )
        M(4, uy+z, ((R(x) & 0xFFFFFFFF00000000) >> 16));
        break;
      case STCO: // u(M_8[A]) <- X
        M(8, a, x);
        break;
      case STCOI: // u(M_8[A]) <- X
        M(8, uy+z, x);
        break;
      case ADD: // s($X) <- s($Y) + s($Z)
        R(x, ((long long int) R(y)) + ((signed long long) R(z)));
        break;
      case ADDI: // s($X) <- s($Y) + s(Z)
        R(x, ((long long int) R(y)) + ((signed long long) z));
        break;
      case SUB: // s($X) <- s($Y) - s($Z)
        R(x, ((long long int) R(y)) - ((signed long long) R(z)));
        break;
      case SUBI: // s($X) <- s($Y) - s(Z)
        R(x, ((long long int) R(y)) - ((signed long long) z));
        break;
      case MUL: // s($X) <- s($Y) x s($Z)
        R(x, ((long long int) R(y)) * ((signed long long) R(z)));
        break;

      case MULI: // s($X) <- s($Y) x s(Z)
        R(x, ((long long int) R(y)) * ((signed long long) z));
        break;

      case DIV: // s($X) <- floor(s($Y) / s($Z)) such that ($Z != 0)
                // and s(rR) <- s($Y) mod s($Z)
        R(x, (R(z) == 0) ? 0 : 
              (((long long int) R(y)) / ((signed long long) R(z))));
        g(rR, (R(z) == 0) ? R(y) :
              (((long long int) R(y)) % ((signed long long) R(z))));
        break;

      case DIVI: // s($X) <- floor(s($Y) / s(Z) such that (Z != 0)
                // and s(rR) <- s($Y) mod s(Z)
        R(x, (z == 0) ? 0 :
              (((long long int) R(y)) / ((signed long long) z)));
        g(rR, (z == 0) ? R(y) :
              (((long long int) R(y)) % ((signed long long) z)));
        break;
      case ADDU: // u($X) <- (u($Y) + u($Z)) mod 2^64
        R(x, a);
        break;
      case ADDUI: // u($X) <- (u($Y) + u(Z)) mod 2^64
        R(x, (ux + z) & 0xFFFFFFFFFFFFFFFF);
        break;
      case SUBU: // u($X) <- (u($Y) - u($Z)) mod 2^64
        R(x, (R(y) - R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case SUBUI: // u($X) <- (u($Y) - u(Z)) mod 2^64
        R(x, (R(y) - z) & 0xFFFFFFFFFFFFFFFF);
        break;
      case MULU: // u(rH $X) <- u($Y) x u($Z)
        {
        unsigned long long carry;
        unsigned long long result;
        wideMult(R(y), R(z), &carry, &result);
        g(rH, carry);
        R(x, result);
        }
        break;

      case MULUI: // u($X) <- (u($Y) - u(Z)) mod 2^64
        {
        unsigned long long carry;
        unsigned long long result;
        wideMult(R(y), z, &carry, &result);
        g(rH, carry);
        R(x, result);
        }
        break;

      case DIVU: // u($X) <- floor(u(rD $Y) / u($Z))
                 // u(rR) <- u(rD $Y) mod u($Z), if u($Z) > u(rD);
                 //     otherwise $X <- rD, rR <- $Y
        {
        unsigned long long numerator_hi;
        unsigned long long numerator_lo;
        unsigned long long divisor;
        unsigned long long quotient;
        unsigned long long remainder;

        numerator_hi = g(rD);
        numerator_lo = R(y);
        divisor = R(z);

        if (divisor > numerator_hi)
        {
          wideDiv(numerator_hi, numerator_lo, divisor, &quotient, &remainder);
          R(x, quotient);
          g(rR, remainder);
        } else {
          R(x, numerator_hi);
          g(rR, numerator_lo );
        }
  
        }
        break;

      case DIVUI: // u($X) <- floor(u(rD $Y) / u(Z))
                  // u(rR) <- u(rD $Y) mod u(Z), if u(Z) > u(rD);
                  //    otherwise $X <- rD, rR <- $Y
        {
        unsigned long long numerator_hi;
        unsigned long long numerator_lo;
        unsigned long long divisor;
        unsigned long long quotient;
        unsigned long long remainder;

        numerator_hi = g(rD);
        numerator_lo = R(y);
        divisor = z;

        if (divisor > numerator_hi)
        {
          wideDiv(numerator_hi, numerator_lo, divisor, &quotient, &remainder);
          R(x, quotient);
          g(rR, remainder);
        } else {
          R(x, numerator_hi);
          g(rR, numerator_lo );
        }
  
        }
        break;

      case i2ADDU: // u($X) <- (u($Y) x 2 + u($Z)) mod 2^4
        R(x, (R(y)*2 + R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i2ADDUI: // u($X) <- (u($Y) x 2 + u(Z)) mod 2^4
        R(x, (R(y)*2 + z) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i4ADDU: // u($X) <- (u($Y) x 4 + u($Z)) mod 2^64
        R(x, (R(y)*4 + R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i4ADDUI: // u($X) <- (u($Y) x 4 + u(Z)) mod 2^64
        R(x, (R(y)*4 + z) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i8ADDU: // u($X) <- (u($Y) x 8 + u($Z)) mod 2^64
        R(x, (R(y)*8 + R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i8ADDUI: // u($X) <- (u($Y) x 8 + u(Z)) mod 2^64
        R(x, (R(y)*8 + z) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i16ADDU: // u($X) <- (u($Y) x 16 + u($Z)) mod 2^64
        R(x, (R(y)*16 + R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i16ADDUI: // u($X) <- (u($Y) x 16 + u(Z)) mod 2^64
        R(x, (R(y)*16 + z) & 0xFFFFFFFFFFFFFFFF);
        break;
      case NEG: // s($X) <- Y - s($Z)
        R(x, y - ( (unsigned long long) R(z)) );
        break;
      case NEGI: // s($X) <- Y - s(Z)
        R(x, y - ( (unsigned long long) z) );
        break;
      case NEGU: // u($X) <- (Y - u($Z)) mod 2^64
        R(x, y - R(z));
        break;
      case NEGUI: // u($X) <- (Y - u(Z)) mod 2^64
        R(x, y - z);
        break;
      case SL: // s($X) <- s($Y) x 2^u($Z)
        R(x, R(y) << R(z));
        break;
      case SLI: // s($X) <- s($Y) x 2^u(Z)
        R(x, R(y) << z);
        break;
      case SLU: // u($X) <- (u($Y) x 2^u($Z)) mod 2^64
        R(x, R(y) << R(z));
        break;
      case SLUI: // u($X) <- (u($Y) x 2^u(Z)) mod 2^64
        R(x, R(y) << z);
        break;
      case SR: // s($X) <- floor( s($Y) / 2^u($Z) )
        R(x, R(y) >> R(z) );
        break;
      case SRI: // s($X) <- floor( s($Y) / 2^u(Z) )
        R(x, R(y) >> z );
        break;
      case SRU: // u($X) <- floor( u($Y) / 2^u($Z) )
        R(x, R(y) >> R(z) );
        break;
      case SRUI: // u($X) <- floor( u($Y) / 2^u(Z) )
        R(x, R(y) >> z );
        break;
      case CMP: // s($X) <- [s($Y) > s($Z)] - [s($Y) < s($Z)]
        R(x, ((R(y) > R(z)) ? 1 : (R(y) < R(z) ? -1 : 0)) );
        break;
      case CMPI: // s($X) <- [s($Y) > s(Z)] - [s($Y) < s(Z)]
        R(x, ((R(y) > z) ? 1 : (R(y) < z ? -1 : 0)) );
        break;
      case CMPU: // s($X) <- [u($Y) > u($Z)] - [u($Y) < u($Z)]
        R(x, ((R(y) > R(z)) ? 1 : (R(y) < R(z) ? -1 : 0)) );
        break;
      case CMPUI: // s($X) <- [u($Y) > u(Z)] - [u($Y) < u(Z)]
        R(x, ((R(y) > z) ? 1 : (R(y) < z ? -1 : 0)) );
        break;
      case CSN: // if s($Y) < 0, set $X <- $Z
        R(x, ((R(y) < 0) ? R(z) : R(x)) );
        break;
      case CSNI: // if s($Y) < 0, set $X <- Z
        R(x, ((R(y) < 0) ? z : R(x)) );
        break;
      case CSZ: // if $Y = 0, set $X <- $Z
        R(x, ((R(y) == 0) ? R(z) : R(x)) );
        break;
      case CSZI: // if $Y = 0, set $X <- Z
        R(x, ((R(y) == 0) ? z : R(x)) );
        break;
      case CSP: // if s($Y) > 0, set $X <- $Z
        R(x, ((R(y) > 0) ? R(z) : R(x)) );
        break;
      case CSPI: // if s($Y) > 0, set $X <- Z
        R(x, ((R(y) > 0) ? z : R(x)) );
        break;
      case CSOD: // if s($Y) mod 2 == 1, set $X <- $Z
        R(x, ( (R(y) & 0x01) ? R(z) : R(x)) );
        break;
      case CSODI: // if s($Y) mod 2 == 1, set $X <- Z
        R(x, ( (R(y) & 0x01) ? z : R(x)) );
        break;
      case CSNN: // if s($Y) >= 0, set $X <- $Z
        R(x, ((R(y) >= 0) ? R(z) : R(x)) );
        break;
      case CSNNI: // if s($Y) >= 0, set $X <- Z
        R(x, ((R(y) >= 0) ? z : R(x)) );
        break;
      case CSNZ: // if $Y != 0, set $X <- $Z
        R(x, ((R(y) != 0) ? R(z) : R(x)) );
        break;
      case CSNZI: // if $Y != 0, set $X <- Z
        R(x, ((R(y) != 0) ? z : R(x)) );
        break;
      case CSNP: // if s($Y) <= 0, set $X <- $Z
        R(x, ((R(y) <= 0) ? R(z) : R(x)) );
        break;
      case CSNPI: // if s($Y) <= 0, set $X <- Z
        R(x, ((R(y) <= 0) ? z : R(x)) );
        break;
      case CSEV: // if s($Y) mod 2 == 0, set $X <- $Z
        R(x, ( (!(R(y) & 0x01)) ? R(z) : R(x)) );
        break;
      case CSEVI: // if s($Y) mod 2 == 0, set $X <- Z
        R(x, ( (!(R(y) & 0x01)) ? z : R(x)) );
        break;
      case ZSN: // $X <- $Z[s($Y) < 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSNI: // $X <- Z[s($Y) < 0]
        R(x, ((R(y) < 0) ? z : 0) );
        break;
      case ZSZ: // $X <- $Z[$Y = 0]
        R(x, ((R(y) == 0) ? R(z) : 0) );
        break;
      case ZSZI: // $X <- Z[$Y = 0]
        R(x, ((R(y) == 0) ? z : 0) );
        break;
      case ZSP: // $X <- $Z[s($Y) > 0]
        R(x, ((R(y) > 0) ? R(z) : 0) );
        break;
      case ZSPI: // $X <- Z[s($Y) > 0]
        R(x, ((R(y) > 0) ? z : 0) );
        break;
      case ZSOD: // $X <- $Z[s($Y) mod 2 == 1]
        R(x, ((R(y) & 0x01 ) ? R(z) : 0) );
        break;
      case ZSODI: // $X <- Z[s($Y) mod 2 == 1]
        R(x, ((R(y) & 0x01 ) ? z : 0) );
        break;
      case ZSNN: // $X <- $Z[s($Y) >= 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSNNI: // $X <- Z[s($Y) >= 0]
        R(x, ((R(y) < 0) ? z : 0) );
        break;
      case ZSNZ: // $X <- $Z[s($Y) != 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSNZI: // $X <- Z[s($Y) != 0]
        R(x, ((R(y) < 0) ? z : 0) );
        break;
      case ZSNP: // $X <- $Z[s($Y) <= 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSNPI: // $X <- Z[s($Y) <= 0]
        R(x, ((R(y) < 0) ? z : 0) );
        break;
      case ZSEV: // $X <- $Zs($Y) mod 2 == 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSEVI: // $X <- Zs($Y) mod 2 == 0]
        R(x, ((R(y) < 0) ? z : 0) );
        break;
      case AND: // v($X) <- v($Y) & v($Z)
        R(x, R(y) & R(z));
        break;
      case ANDI: // v($X) <- v($Y) & v(Z)
        R(x, R(y) & z);
        break;
      case OR: // v($X) <- v($Y) v v($Z)
        R(x, R(y) | R(z));
        break;
      case ORI: // v($X) <- v($Y) v v(Z)
        R(x, R(y) | z);
        break;
      case XOR: // v($X) <-  v($Y) xor v($Z)
        R(x, R(y) ^ R(z));
        break;
      case XORI: // v($X) <-  v($Y) xor v(Z)
        R(x, R(y) ^ z);
        break;
      case ANDN: // v($X) <- v($Y) v ~v($Z)
        R(x, R(y) & ~R(z));
        break;
      case ANDNI: // v($X) <- v($Y) v ~v(Z)
        R(x, R(y) & ~z);
        break;
      case ORN: // v($X) <- v($Y) v ~v($Z)
        R(x, R(y) | ~R(z));
        break;
      case ORNI: // v($X) <- v($Y) v ~v(Z)
        R(x, R(y) | ~z);
        break;
      case NAND: // ~v($X) <- v($Y) & v($Z)
        R(x, ~( R(y) & R(z)) );
        break;
      case NANDI: // ~v($X) <- v($Y) & v(Z)
        R(x, ~( R(y) & z) );
        break;
      case NOR: // ~v($X) <- v($Y) v v($Z)
        R(x, ~( R(y) | R(z)) );
        break;
      case NORI: // ~v($X) <- v($Y) v v(Z)
        R(x, ~( R(y) | z) );
        break;
      case NXOR: // ~v($X) <- v($Y) xor v($Z)
        R(x, ~( R(y) ^ R(z)) );
        break;
      case NXORI: // ~v($X) <- v($Y) xor v(Z)
        R(x, ~( R(y) ^ z) );
        break;
      case MUX: // v($X) <- (v($Y) & v(rM)) | (v($Z) & ~v(rM))
        R(x, (R(y) & g(rM)) | (R(z) & ~g(rM)) );
        break;
      case MUXI: // v($X) <- (v($Y) & v(rM)) | (v(Z) & ~v(rM))
        R(x, (R(y) & g(rM)) | (z & ~g(rM)) );
        break;
      case SADD: // s($X) <- s(sum(v($Y) & ~v($Z)))
        {
        std::bitset<64> temp( R(y) & ~R(z) );
        R(x, temp.count() );
        //R(x, (new std::bitset<64>( R(y) & ~R(z) ))->count() );
        }
        break;

      case SADDI: // s($X) <- s(sum(v($Y) & ~v(Z)))
        {
        std::bitset<64> temp( R(y) & ~z );
        R(x, temp.count() );
        //R(x, (new std::bitset<64>( R(y) & ~R(z) ))->count() );
        }
        break;

      case BDIF: // b($X) <- b($Y) .- b($Z)
        {
        unsigned long long int b0 = ((R(y)>> 0)&0xFF) - ((R(z)>> 0)&0xFF);
        unsigned long long int b1 = ((R(y)>> 8)&0xFF) - ((R(z)>> 8)&0xFF);
        unsigned long long int b2 = ((R(y)>>16)&0xFF) - ((R(z)>>16)&0xFF);
        unsigned long long int b3 = ((R(y)>>24)&0xFF) - ((R(z)>>24)&0xFF);
        unsigned long long int b4 = ((R(y)>>32)&0xFF) - ((R(z)>>32)&0xFF);
        unsigned long long int b5 = ((R(y)>>40)&0xFF) - ((R(z)>>40)&0xFF);
        unsigned long long int b6 = ((R(y)>>48)&0xFF) - ((R(z)>>48)&0xFF);
        unsigned long long int b7 = ((R(y)>>56)&0xFF) - ((R(z)>>56)&0xFF);
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
        break;

      case BDIFI: // b($X) <- b($Y) .- b(Z)
        {
        unsigned long long int b0 = ((R(y)>> 0)&0xFF) - ((uzi>> 0)&0xFF);
        unsigned long long int b1 = ((R(y)>> 8)&0xFF) - ((uzi>> 8)&0xFF);
        unsigned long long int b2 = ((R(y)>>16)&0xFF) - ((uzi>>16)&0xFF);
        unsigned long long int b3 = ((R(y)>>24)&0xFF) - ((uzi>>24)&0xFF);
        unsigned long long int b4 = ((R(y)>>32)&0xFF) - ((uzi>>32)&0xFF);
        unsigned long long int b5 = ((R(y)>>40)&0xFF) - ((uzi>>40)&0xFF);
        unsigned long long int b6 = ((R(y)>>48)&0xFF) - ((uzi>>48)&0xFF);
        unsigned long long int b7 = ((R(y)>>56)&0xFF) - ((uzi>>56)&0xFF);
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
        break;

      case WDIF: // w($X) <- w($Y) - w($Z)
        {
        unsigned long long int w0 = ((R(y)>> 0)&0xFFFF) - ((R(z)>> 0)&0xFFFF);
        unsigned long long int w1 = ((R(y)>>16)&0xFFFF) - ((R(z)>>16)&0xFFFF);
        unsigned long long int w2 = ((R(y)>>32)&0xFFFF) - ((R(z)>>32)&0xFFFF);
        unsigned long long int w3 = ((R(y)>>48)&0xFFFF) - ((R(z)>>48)&0xFFFF);
        w0 = (w0 < 0) ? 0 : w0;
        w1 = (w0 < 0) ? 0 : w1;
        w2 = (w0 < 0) ? 0 : w2;
        w3 = (w0 < 0) ? 0 : w3;
        R(x, (w3<<48)&(w2<<32)&(w1<<16)&(w0<<0) );
        }
        break;

      case WDIFI: // w($X) <- w($Y) - w(Z)
        {
        unsigned long long int w0 = ((R(y)>> 0)&0xFFFF) - ((uzi>> 0)&0xFFFF);
        unsigned long long int w1 = ((R(y)>>16)&0xFFFF) - ((uzi>>16)&0xFFFF);
        unsigned long long int w2 = ((R(y)>>32)&0xFFFF) - ((uzi>>32)&0xFFFF);
        unsigned long long int w3 = ((R(y)>>48)&0xFFFF) - ((uzi>>48)&0xFFFF);
        w0 = (w0 < 0) ? 0 : w0;
        w1 = (w0 < 0) ? 0 : w1;
        w2 = (w0 < 0) ? 0 : w2;
        w3 = (w0 < 0) ? 0 : w3;
        R(x, (w3<<48)&(w2<<32)&(w1<<16)&(w0<<0) );
        }
        break;

      case TDIF: // t($X) <- t($Y) - w($Z)
        {
        unsigned long long int t0 = ((R(y)>> 0)&0xFFFFFFFF) - ((R(z)>> 0)&0xFFFFFFFF);
        unsigned long long int t1 = ((R(y)>>32)&0xFFFFFFFF) - ((R(z)>>32)&0xFFFFFFFF);
        t0 = (t0 < 0) ? 0 : t0;
        t1 = (t1 < 0) ? 0 : t1;
        R(x, (t1<<32)&(t0<<0) );
        }
        break;

      case TDIFI: // t($X) <- t($Y) - w(Z)
        {
        unsigned long long int t0 = ((R(y)>> 0)&0xFFFFFFFF) - ((uzi>> 0)&0xFFFFFFFF);
        unsigned long long int t1 = ((R(y)>>32)&0xFFFFFFFF) - ((uzi>>32)&0xFFFFFFFF);
        t0 = (t0 < 0) ? 0 : t0;
        t1 = (t1 < 0) ? 0 : t1;
        R(x, (t1<<32)&(t0<<0) );
        }
        break;

      case ODIF: // u($X) <- u($Y) - u($Z)
        {
        uint64_t u0 = (R(y)) - R(z);
        u0 = (u0 > R(y)) ? 0 : u0;
        }
        break;

      case ODIFI: // u($X) <- u($Y) - u(Z)
        {
        uint64_t u0 = (R(y)) - z;
        u0 = (u0 > R(y)) ? 0 : u0;
        }
        break;

      case MOR: // m($X) <- m($Z) vx m($Y)
        {
        unsigned int r;
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
        break;

      case MORI: // m($X) <- m(Z) vx m($Y)
        {
        unsigned int r;
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
        break;
 
      case MXOR: // m($X) <- m($Z) xor x m($Y)
        {
        unsigned int r;
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
        break;

      case MXORI: // m($X) <- m(Z) xor x m($Y)
        {
        unsigned int r;
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
        break;

      case FADD: // f($X) <- f($Y) + f($Z)
        {
        double t = ( *(double*)&uy + *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
        break;

      case FSUB: // f($X) <- f($Y) - f($Z)
        {
        double t = ( *(double*)&uy - *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
        break;

      case FMUL: // f($X) <- f($Y) * f($Z)
        {
        double t = ( *(double*)&uy * *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
        break;

      case FDIV: // f($X) <- f($Y) / f($Z)
        {
        double t = ( *(double*)&uy / *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
        break;

      case FREM: // f($X) <- f($Y) rem f($Z)
        {
        double t = fmod( *(double*)&uy , *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
        break;

      case FSQRT: // f($X) <-  f($Z)^(1/2)
        {
        double t = sqrt( *(double*)&uz );
        R(x, *(unsigned long long int *)&t );
        }
        break;

      case FINT: // f($X) <- int f($Z)
        R(x, (unsigned long long int)( *(double*)&uz ));
        break;

      case FCMP: // s($X) <- [f($Y) > f($Z)] - [f($Y) < f($Z)]
        R(x, ((*(double*)&uy > *(double*)&uz) ? 1 : 0) 
           - ((*(double*)&uy < *(double*)&uz) ? 1 : 0) );
        break;

      case FEQL: // s($X) <- [f($Y) == f($Z)]
        R(x, *(double*)&uy == *(double*)&uz );
        break;

      case FUN: // s($X) <- [f($Y) || f($Z)]
        R(x, fy != fy || fz != fz );
        break;

      case FCMPE: // s($X) <- [f($Y) } f($Z) (f(rE))] - [f($Y) { f($Z) (f(rE))]
        {
        int e;
        frexp(fy, &e);
        R(x, (fy > fz && !N(fy, fz, e, frE)) -
             (fy < fz && !N(fy, fz, e, frE)));
        }
        break;

      case FEQLE: // s($X) <- [f($Y) ~= f($Z) (f(rE))]
        {
        int e;
        frexp(fy, &e);
        R(x, N(fy, fz, e, frE));
        }
        break;

      case FUNE: // s($X) <- [f($Y) || f($Z) (f(rE))]
        R(x, fy != fy || fz != fz || frE != frE);
        break;

      case FIX: // s($X) <- int f($Z)
        R(x, (long long int) fz);
        break;

      case FIXU: // u($X) <- (int f($Z)) mod 2^64
        R(x, (unsigned long long int) fz);
        break;

      case FLOT: // f($X) <- s($Z)
        R(x, (double) sz);
        break;

      case FLOTU: // f($x) <- u($Z)
        R(x, (double) uz);
        break;

      case FLOTUI: // f($X) <- u(Z)
        R(x, (double) z);
        break;

      case SFLOT: // f($X) <- f(T) <- s($Z)
        R(x, (float) fz);
        break;

      case SFLOTI: // f($X) <- f(T) <- s(Z)
        R(x, (float) z);
        break;

      case SFLOTU: // f($X) <- f(T) <- u($Z)
        R(x, (float) fz);
        break;

      case SFLOTUI: // f($X) <- f(T) <- u(Z)
        R(x, (float) z);
        break;

      case LDSF: // f($X) <- f(M_4[A])
        R(x, M(4, a));
        break;

      case LDSFI: // f($X) <- f(M_4[A])
        R(x, M(4, uy+z));
        break;
 
      case STSF: // f(M_4[A]) <- f($X)
        M(4, a, fx);
        break;

      case STSFI: // f(M_4[A]) <- f($X)
        M(4, uy+z, fx);
        break;

      case FLOTI: // f($X) <- s(Z)
        R(x, (double) z);
        break;

      case SETH: // u($X) <- YZ x2^48
        R(x, ((uyi<<8) & uzi) << 48);
        break;

      case SETMH: // u($X) <- YZ x2^32
        R(x, ((uyi<<8) & uzi) << 32);
        break;

      case SETML: // u($Z) <- YZ
        R(x, ((uyi<<8) & uzi));
        break;

      case SETL: // u($X) <- YZ x2^16
        R(x, ((uyi<<8) & uzi) << 16);
        break;

      case INCH:  // u($X) <- ( u($X) + YZ x2^48) mod 2^64
        R(x, R(x) + (((uyi<<8) & uzi) << 48) );
        break;

      case INCMH: // u($X) <- ( u($X) + YZ x2^32) mod 2^64
        R(x, R(x) + (((uyi<<8) & uzi) << 32) );
        break;

      case INCML: // u($X) <- ( u($X) + YZ x2^16) mod 2^64
        R(x, R(x) + (((uyi<<8) & uzi) << 16) );
        break;
        
      case INCL: // u($X) <- ( u($X) + YZ) mod 2^64
        R(x, ux + ((uyi<<8) & uzi) );
        break;

      case ORH: // v($X) <- v($X) v v(YZ << 48)
        R(x, ux | (((uyi<<8) & uzi)<<48) );
        break;

      case ORMH: // v($X) <- v($X) v v(YZ << 32)
        R(x, ux | (((uyi<<8) & uzi)<<32) );
        break;

      case ORML: // v($X) <- v($X) v v(YZ << 16)
        R(x, ux | (((uyi<<8) & uzi)<<16) );
        break;

      case ORL: // v($X) <- v($X) v v(YZ)
        R(x, ux | ((uyi<<8) & uzi) );
        break;

      case ANDNH: // v($X) <- v($X) ^ ~v(YZ << 48)
        R(x, ux & ~(((uyi<<8) & uzi)<<48) );
        break;

      case ANDNMH: // v($X) <- v($X) ^ ~v(YZ<<32)
        R(x, ux & ~(((uyi<<8) & uzi)<<32) );
        break;

      case ANDNML: // v($X) <- v($X) ^ ~v(YZ<<16)
        R(x, ux & ~(((uyi<<8) & uzi)<<16) );
        break;

      case ANDNL: // v($X) <- v($X) ^ ~v(YZ)
        R(x, ux & ~((uyi<<8) & uzi) );
        break;
      
      case GO: // u($X) <- @+4, then @<-A
        R(x, target);
        target = a;
        break;

      case BN: // if s($X) < 0, set @ <- RA
        target = ( ( sx < 0 ) ? ra : target );
        break;

      case BZ: // if $X = 0, set @ <- RA
        target = ( ( uz == 0 ) ? ra : target );
        break;

      case BP: // if s($X) = 0, set @ <- RA
        target = ( ( sz > 0 ) ? ra : target );
        break;

      case BOD: // if s($X) mod 2 == 1, set @ <- RA
        target = ( ( (uz & 1) == 1 ) ? ra : target );
        break;

      case BNN: // if s($X) >= 0, set @ <- RA
        target = ( ( sz >= 0 ) ? ra : target );
        break;

      case BNZ: // if s($X) != 0, set @ <- RA
        target = ( ( sz != 0 ) ? ra : target );
        break;

      case BNP: // if s($X) <= 0, set @ <- RA
        target = ( ( sz <= 0 ) ? ra : target );
        break;

      case BEV: // if s($X) mod 2 == 0, set @ <- RA
        target = ( ( (uz & 1) == 0 ) ? ra : target );
        break;

      case PBN: // if s($X) < 0, set @ <- RA
        target = ( ( sx < 0 ) ? ra : target );
        break;

      case PBZ: // if $X = 0, set @ <- RA
        target = ( ( uz == 0 ) ? ra : target );
        break;

      case PBP: // if s($X) = 0, set @ <- RA
        target = ( ( sz > 0 ) ? ra : target );
        break;

      case PBOD: // if s($X) mod 2 == 1, set @ <- RA
        target = ( ( (uz & 1) == 1 ) ? ra : target );
        break;

      case PBNN: // if s($X) >= 0, set @ <- RA
        target = ( ( sz >= 0 ) ? ra : target );
        break;

      case PBNZ: // if s($X) != 0, set @ <- RA
        target = ( ( sz != 0 ) ? ra : target );
        break;

      case PBNP: // if s($X) <= 0, set @ <- RA
        target = ( ( sz <= 0 ) ? ra : target );
        break;

      case PBEV: // if s($X) mod 2 == 0, set @ <- RA
        target = ( ( (uz & 1) == 0 ) ? ra : target );
        break;

      case JMPB: 
        target = -1*ra;
        break;

      case BNB: // if s($X) < 0, set @ <- -RA
        target = ( ( sx < 0 ) ? -1*ra : target );
        break;

      case BZB: // if $X = 0, set @ <- -RA
        target = ( ( uz == 0 ) ? -1*ra : target );
        break;

      case BPB: // if s($X) = 0, set @ <- -RA
        target = ( ( sz > 0 ) ? -1*ra : target );
        break;

      case BODB: // if s($X) mod 2 == 1, set @ <- -RA
        target = ( ( (uz & 1) == 1 ) ? -1*ra : target );
        break;

      case BNNB: // if s($X) >= 0, set @ <- -RA
        target = ( ( sz >= 0 ) ? -1*ra : target );
        break;

      case BNZB: // if s($X) != 0, set @ <- -RA
        target = ( ( sz != 0 ) ? -1*ra : target );
        break;

      case BNPB: // if s($X) <= 0, set @ <- -RA
        target = ( ( sz <= 0 ) ? -1*ra : target );
        break;

      case BEVB: // if s($X) mod 2 == 0, set @ <- -RA
        target = ( ( (uz & 1) == 0 ) ? -1*ra : target );
        break;

      case PBNB: // if s($X) < 0, set @ <- -RA
        target = ( ( sx < 0 ) ? -1*ra : target );
        break;

      case PBZB: // if $X = 0, set @ <- -RA
        target = ( ( uz == 0 ) ? -1*ra : target );
        break;

      case PBPB: // if s($X) = 0, set @ <- -RA
        target = ( ( sz > 0 ) ? -1*ra : target );
        break;

      case PBODB: // if s($X) mod 2 == 1, set @ <- -RA
        target = ( ( (uz & 1) == 1 ) ? -1*ra : target );
        break;

      case PBNNB: // if s($X) >= 0, set @ <- -RA
        target = ( ( sz >= 0 ) ? -1*ra : target );
        break;

      case PBNZB: // if s($X) != 0, set @ <- -RA
        target = ( ( sz != 0 ) ? -1*ra : target );
        break;

      case PBNPB: // if s($X) <= 0, set @ <- -RA
        target = ( ( sz <= 0 ) ? -1*ra : target );
        break;

      case PBEVB: // if s($X) mod 2 == 0, set @ <- -RA
        target = ( ( (uz & 1) == 0 ) ? -1*ra : target );
        break;

      case PUSHJ: // push(X) and set rJ<-@+4, then set @<-RA
        push(x);
        R(rJ, target+4);
        target = ra;
        break;

      case PUSHJB: // push(X) and set rJ<-@+4, then set @<-RA
        push(x);
        R(rJ, -1*(target+4));
        target = ra;
        break;

      case PUSHGO: // push(X) and set rJ<-@+4, then set @<-A
        push(x);
        R(rJ, target+4);
        target = a;
        break;

      case POP: // pop(X), then @<-rJ+4*YZ
        pop(x);
        target = R(rJ)+4*((y<<8) & z);
        break;

      case SAVE: // u($X) <- context
        push(255);
        R(x, register_stack_top);
        break;

      case UNSAVE: // context<- u($Z)
        pop(255);
        R(x, register_stack_top);
        break;

      case LDUNC: // s($X) <- s(M_8[A])
        R( x, ((long long int) M(8, a)) );
        break;

      case STUNC: // s(M_8[A]) <- s($X)
        M(8, a, (long long int) R(x));
        break;

      case PRELD: 
        break;

      case PREST:
        break;

      case PREGO:
        break;

      case SYNCID:
        break;

      case SYNCD:
        break;

      case SYNC:
        break;

      case CSWAP: // if u(M_8[A] == u(rP), set u(M_8[A]) <- u($X) and
                  // u($X) <- 1. Otherwise set u(rP) <- u(M_8[A]) and
                  // u($X) <- 0
        if (M(8, a) == R(rP)) {
          M(8,a,R(x));
          R(x,1);
        } else {
          R(rP, M(8,a));
          R(x, 0);
        }
        break;

      case LDVTS: 
      case CSWAPI: 
        break;

      case LDUNCI:
        R( x, ((long long int) M(8, a)) );
        break;

      case LDVTSI:
      case PRELDI:
      case PREGOI:
      case GOI:
        break;

      case STUNCI: // s(M_8[A] <- s($X)
        M(8, uy+z, (long long int) R(x));
        break;

      case SYNCDI:
      case PRESTI:
      case SYNCIDI:
      case PUSHGOI:
        break;

      case TRIP:
      case TRAP:
        // MMIX simulators seem to implement 10 traps for basic i/o
        // the idea is that the NNIX kernel would implement all the trap
        // functions in a proper system.

        switch (y) {
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
            for (int i=0; M(1,R(255) + i)  != '\0'; i++)
            {
              printf( "%c", (char)M(1,R(255) + i) );
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
        break;

      case RESUME:
        break;

      case GET: // u($X) <- u(g[Z]), where 0 <= Z < 32
        R(x, g(z));
        break;

      case PUT: // u(g[X]) <- u($Z), where 0 <= X < 32
        g(x, R(z));
        break;

      case PUTI: // u(g[X]) <- u(Z), where 0 <= X < 32
        g(x, z);
        break;

      case GETA: // u($X) <- RA
        R(x, ra);
        break;

      case GETAB: // u($X) <- RA
        R(x, -1*ra);
        break;

      case SWYM:
        break;

      default:
          break;
  }

  setip( target );
}
