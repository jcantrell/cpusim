#include "subleq.h"

subleq::subleq(int byte_in, int address_in) : cpu(byte_in, address_in)
{
}

void subleq::step(int inst)
{
  int a = view(getip()+1);
  int b = view(getip()+2);
  int c = view(getip()+3);
  int target = viewip()+1;

  switch (inst)
  {
      case 0xF0: // JMP
        {
        unsigned long target = ( (a<<16) & (b<<8) & (c<<0) );
        printf("jumping to target %d\n",target);
        }
        break;

      case 0x80: // LDB
        {
        unsigned int width = 1;
        unsigned long base = (regvalue(b) + regvalue(c)) % width ;
        // 64-bit value
        long long value = view(base);
        // sign-extend
        value = ((value << (64-width*8)) >> (64-width*8));

        regstore(a,value);
        }
        break;

      case 0x82: // LDBU
        {
          unsigned int width = 1;
          unsigned long base = (regvalue(b) + regvalue(c)) % width;
          long long value = view(base) ;

          regstore(a, value);
        }
        break;

      case 0x84: // LDW
        {
          unsigned int width = 2;
          unsigned long base = (regvalue(b) + regvalue(c)) % width;
          long long value = view(base) ;
          value = ((value << (64-width*8)) >> (64-width*8));
          regstore(a, value );
        }
        break;

      case 0x86: // LDWU
        {
          unsigned int width = 2;
          unsigned long base = (regvalue(b) + regvalue(c)) % width;
          long long value = view(base) ;

          regstore(a, value);
        }
        break;

      case 0x88: // LDT
        {
          unsigned int width = 4;
          unsigned long base = (regvalue(b) + regvalue(c)) % width;
          long long value = view(base) ;
          value = ((value << (64-width*8)) >> (64-width*8));
          regstore(a, value );
        }
        break;

      case 0x8A: // LDTU
        {
          unsigned int width = 4;
          unsigned long base = (regvalue(b) + regvalue(c)) % width;
          long long value = view(base) ;

          regstore(a, value);
        }
        break;

      case 0x8C: // LDO
        {
          unsigned int width = 8;
          unsigned long base = (regvalue(b) + regvalue(c)) % width;
          long long value = view(base) ;
          value = ((value << (64-width*8)) >> (64-width*8));
          regstore(a, value );
        }
        break;

      case 0x8E: // LDOU
        {
          unsigned int width = 8;
          unsigned long base = (regvalue(b) + regvalue(c)) % width;
          long long value = view(base) ;

          regstore(a, value);
        }
        break;

      case 0x92: // LDHT
        {
          unsigned int width = 4;
          unsigned long base = (regvalue(b) + regvalue(c)) % width;
          long long value = view(base);
          value = (value << (64 - width*8));
          regstore(a, value);
        }
        break;

      case 0x22: // ADDU
        {
                unsigned long base = (regvalue(b) + regvalue(c)) % width;
                regstore(a, base);
        }

      case 0x: // STB
        {
                unsigned long base = (regvalue(b) + regvalue(c)) % width;
                unsigned long value = view(base);
                value = (value & 0xff)
                memstore(base, value);

      default:
          break;
  }

  setip( target );
}
