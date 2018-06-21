#include "mmix.h"
#include <bitset>

mmix::mmix(int byte_size, int address_size) : cpu(byte_size, address_size)
{
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

unsigned long long int mmix::M(unsigned int size, unsigned long long address)
{
  const unsigned int octasize = 8;
  unsigned char octabyte[octasize];
  unsigned long long int base = address % size;
  for (int i=0;i<octasize;i++)
    octabyte[i] = view(base + i);
  unsigned long long int value = 0;
  for (int i=0;i<octasize;i++)
  {
    value & octabyte[octasize-(i+1)];
    value <<= 8;
  }
  value = value & (0xFF << (size - 1) );
  printf("Value is: %llu\n", value);
  return value;
}

unsigned long long int mmix::M(unsigned int size, unsigned long long address,
  unsigned long long int value)
{
  return 0;
}

unsigned long long int mmix::R(unsigned int reg)
{
  return 0;
}

unsigned long long int mmix::R(unsigned int reg, unsigned long long value)
{
  return 0;
}

unsigned long long int mmix::g(unsigned int reg, unsigned long long value)
{
  return 0;
}

unsigned long long int mmix::g(unsigned int reg)
{
  return 0;
}

void mmix::step(int inst)
{
  unsigned long long int x = M(1, getip()+1); 
  unsigned long long int y = M(1, getip()+2); 
  unsigned long long int z = M(1, getip()+3); 
  unsigned long long int target = getip()+1;

  // A <- (u($Y) + u($Z)) mod 2^64
  unsigned long long int a = ( (y + z ) & 0xFFFFFFFFFFFFFFFF );

  switch (inst)
  {
      case JMP: // JMP
        target = ( (x<<16) & (y<<8) & (z<<0) );
        break;

      case LDB: // LDB: s($X) <- s(M_1[A])
        R( x, ((long long int) M(1, a)) );
        break;

      case LDBU: // LDBU: u($X) <- u(M_1[A])
        R( x, M(1, a) );
        break;

      case LDW: // s($X) <- s(M_2[A])
        R( x, ((long long int) M(2, a)) );
        break;

      case LDWU: // LDWU: u($X) <- u(M_2[A])
        R( x, M(2, a) );
        break;

      case LDT: // LDT: s($X) <- s(M_4[A])
        R( x, ((long long int) M(4, a)) );
        break;

      case LDTU: // u($X) <- u(M_4[A])
        R( x, M(4, a) );
        break;

      case LDO: // s($X) <- s(M_8[A])
        R( x, ((long long int) M(8, a)) );
        break;

      case LDOU: // u($X) <- u(M_8[A])
        R( x, M(8, a) );
        break;

      case LDHT: // u($X) <- u(M_4[A]) x 2^32
        R( x, (M(4, a) << 32));
        break;

      case STB: // s(M_1[A]) <- s($X)
        M(1, a, (long long int) R(x));
        break;

      case STW: // s(M_2[A]) <- s($X)
        M(2, a, (long long int) R(x));
        break;
      
      case STT: // s(M_4[A] <- s($X)
        M(4, a, (long long int) R(x));
        break;
      case STO: // s(M_8[A] <- s($X)
        M(8, a, (long long int) R(x));
        break;

      case STBU: // u(M_1[A]) <- u($X) mod 2^8
        M(1, a, R(x));
        break;
      case STWU: // u(M_2[A]) <- u($X) mod 2^16
        M(2, a, R(x));
        break;
      case STTU: // u(M_4[A]) <- u($X) mod 2^32
        M(4, a, R(x));
        break;
      case STOU: // u(M_8[A]) <- u($X)
        M(8, a, R(x));
        break;
      case STHT: // u(M_4[A]) <- floor( u($X) / 2^32 )
        M(4, a, ((R(x) & 0xFFFFFFFF00000000) >> 16));
        break;
      case STCO: // u(M_8[A]) <- X
        M(8, a, x);
        break;
      case ADD: // s($X) <- s($Y) + s($Z)
        R(x, ((long long int) R(y)) + ((signed long long) R(z)));
        break;
      case SUB: // s($X) <- s($Y) - s($Z)
        R(x, ((long long int) R(y)) - ((signed long long) R(z)));
        break;
      case MUL: // s($X) <- s($Y) x s($Z)
        R(x, ((long long int) R(y)) * ((signed long long) R(z)));
        break;
      case DIV: // s($X) <- floor(s($Y) / s($Z)) such that ($Z != 0)
                // and s(rR) <- s($Y) mod s($Z)
        R(x, (R(z) == 0) ? 0 : 
              (((long long int) R(y)) / ((signed long long) R(z))));
        g(rR, (R(z) == 0) ? R(y) :
              (((long long int) R(y)) % ((signed long long) R(z))));
        break;
      case ADDU: // u($X) <- (u($Y) + u($Z)) mod 2^64
        R(x, a);
        break;
      case SUBU: // u($X) <- (u($Y) - u($Z)) mod 2^64
        R(x, (R(y) - R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case MULU: // u(rH $X) <- u($Y) x u($Z)
        unsigned long long carry;
        unsigned long long result;
        wideMult(R(y), R(z), &carry, &result);
        g(rH, carry);
        R(x, result);
        break;
      case DIVU: // u($X) <- floor(u(rD $Y) / u($Z))
                 // u(rR) <- u(rD $Y) mod u($Z), if u($Z) > u(rD);
                 //     otherwise $X <- rD, rR <- $Y
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
  
        break;

      case i2ADDU: // u($X) <- (u($Y) x 2 + u($Z)) mod 2^4
        R(x, (R(y)*2 + R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i4ADDU: // u($X) <- (u($Y) x 4 + u($Z)) mod 2^64
        R(x, (R(y)*4 + R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i8ADDU: // u($X) <- (u($Y) x 8 + u($Z)) mod 2^64
        R(x, (R(y)*8 + R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case i16ADDU: // u($X) <- (u($Y) x 16 + u($Z)) mod 2^64
        R(x, (R(y)*16 + R(z)) & 0xFFFFFFFFFFFFFFFF);
        break;
      case NEG: // s($X) <- Y - s($Z)
        R(x, y - ( (unsigned long long) R(z)) );
        break;
      case NEGU: // u($X) <- (Y - u($Z)) mod 2^64
        R(x, y - R(z));
        break;
      case SL: // s($X) <- s($Y) x 2^u($Z)
        R(x, R(y) << R(z));
        break;
      case SLU: // u($X) <- (u($Y) x 2^u($Z)) mod 2^64
        R(x, R(y) << R(z));
        break;
      case SR: // s($X) <- floor( s($Y) / 2^u($Z) )
        R(x, R(y) >> R(z) );
        break;
      case SRU: // u($X) <- floor( u($Y) / 2^u($Z) )
        R(x, R(y) >> R(z) );
        break;
      case CMP: // s($X) <- [s($Y) > s($Z)] - [s($Y) < s($Z)]
        R(x, ((R(y) > R(z)) ? 1 : (R(y) < R(z) ? -1 : 0)) );
        break;
      case CMPU: // s($X) <- [u($Y) > u($Z)] - [u($Y) < u($Z)]
        R(x, ((R(y) > R(z)) ? 1 : (R(y) < R(z) ? -1 : 0)) );
        break;
      case CSN: // if s($Y) < 0, set $X <- $Z
        R(x, ((R(y) < 0) ? R(z) : R(x)) );
        break;
      case CSZ: // if $Y = 0, set $X <- $Z
        R(x, ((R(y) == 0) ? R(z) : R(x)) );
        break;
      case CSP: // if s($Y) > 0, set $X <- $Z
        R(x, ((R(y) > 0) ? R(z) : R(x)) );
        break;
      case CSOD: // if s($Y) mod 2 == 1, set $X <- $Z
        R(x, ( (R(y) & 0x01) ? R(z) : R(x)) );
        break;
      case CSNN: // if s($Y) >= 0, set $X <- $Z
        R(x, ((R(y) >= 0) ? R(z) : R(x)) );
        break;
      case CSNZ: // if $Y != 0, set $X <- $Z
        R(x, ((R(y) != 0) ? R(z) : R(x)) );
        break;
      case CSNP: // if s($Y) <= 0, set $X <- $Z
        R(x, ((R(y) <= 0) ? R(z) : R(x)) );
        break;
      case CSEV: // if s($Y) mod 2 == 0, set $X <- $Z
        R(x, ( (!(R(y) & 0x01)) ? R(z) : R(x)) );
        break;
      case ZSN: // $X <- $Z[s($Y) < 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSZ: // $X <- $Z[$Y = 0]
        R(x, ((R(y) == 0) ? R(z) : 0) );
        break;
      case ZSP: // $X <- $Z[s($Y) > 0]
        R(x, ((R(y) > 0) ? R(z) : 0) );
        break;
      case ZSOD: // $X <- $Z[s($Y) mod 2 == 1]
        R(x, ((R(y) & 0x01 ) ? R(z) : 0) );
        break;
      case ZSNN: // $X <- $Z[s($Y) >= 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSNZ: // $X <- $Z[s($Y) != 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSNP: // $X <- $Z[s($Y) <= 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case ZSEV: // $X <- $Zs($Y) mod 2 == 0]
        R(x, ((R(y) < 0) ? R(z) : 0) );
        break;
      case AND: // v($X) <- v($Y) & v($Z)
        R(x, R(y) & R(z));
        break;
      case OR: // v($X) <- v($Y) v v($Z)
        R(x, R(y) | R(z));
        break;
      case XOR: // v($X) <-  v($Y) xor v($Z)
        R(x, R(y) ^ R(z));
        break;
      case ANDN: // v($X) <- v($Y) v ~v($Z)
        R(x, R(y) & ~R(z));
        break;
      case ORN: // v($X) <- v($Y) v ~v($Z)
        R(x, R(y) | ~R(z));
        break;
      case NAND: // ~v($X) <- v($Y) & v($Z)
        R(x, ~( R(y) & R(z)) );
        break;
      case NOR: // ~v($X) <- v($Y) v v($Z)
        R(x, ~( R(y) | R(z)) );
        break;
      case NXOR: // ~v($X) <- v($Y) xor v($Z)
        R(x, ~( R(y) ^ R(z)) );
        break;
      case MUX: // v($X) <- (v($Y) & v(rM)) | (v($Z) & ~v(rM))
        R(x, (R(y) & g(rM)) | (R(z) & ~g(rM)) );
        break;
      case SADD: // s($X) <- s(sum(v($Y) & ~v($Z)))
        R(x, (new std::bitset<64>( R(y) & ~R(z) ))->count() );
        break;

      default:
          break;
  }

  setip( target );
}
