#ifndef MMIX_H
#define MMIX_H

#include "model/cpu.h"

class mmix : public cpu {
  private:
    enum inst {
      JMP = 0xF0,
      LDB = 0x80,
      LDBU = 0x82,
      LDW = 0x84,
      LDWU = 0x86,
      LDT = 0x88,
      LDTU = 0x8A,
      LDO = 0x8C,
      LDOU = 0x8E,
      LDHT = 0x92,
      ADDU = 0x22,
      STB = 0xA8,
      STW = 0xA4,
      STT = 0xA0,
      STO = 0xAC,
      STBU = 0xA2,
      STWU = 0xA6,
      STTU = 0xAA,
      STOU = 0xAE,
      STHT = 0xB2,
      STCO = 0xB4,
      ADD = 0x20,
      SUB = 0x24,
      MUL = 0x10,
      DIV = 0x14,
      SUBU = 0x27,
      MULU = 0x1A,
      DIVU = 0x1E,
      i2ADDU = 0x28,
      i4ADDU = 0x2A,
      i8ADDU = 0x2C,
      i16ADDU = 0x2E,
      NEG = 0x34,
      NEGU = 0x36,
      SL = 0x38,
      SLU = 0x3A,
      SR = 0x3C,
      SRU = 0x3E,
      CMP = 0x30,
      CMPU = 0x32,
      CSN = 0x60,
      CSZ = 0x62,
      CSP = 0x64,
      CSOD = 0x66,
      CSNN = 0x68,
      CSNZ = 0x6A,
      CSNP = 0x6C,
      CSEV = 0x6E,
      ZSN = 0x70,
      ZSZ = 0x72,
      ZSP = 0x74,
      ZSOD = 0x76,
      ZSNN = 0x78,
      ZSNZ = 0x7A,
      ZSNP = 0x7C,
      ZSEV = 0x7E,
      AND = 0xC8,
      OR = 0xC0,
      XOR = 0xC6,
      ANDN = 0xCA,
      ORN = 0xC2,
      NAND = 0xCC,
      NOR = 0xC4,
      NXOR = 0xCE,
      MUX = 0xD8,
      SADD = 0xDA,
      BDIF,
      WDIF,
      TDIF,
      ODIF,
      MOR,
      MXOR,
      FADD,
      FSUB,
      FMUL,
      FDIV,
      FREM,
      FSQRT,
      FINT,
      FCMP,
      FEQL,
      FUN,
      FCMPE
    };
    enum special_registers {
      rA = 0x00,
      rB = 0x01,
      rC = 0x02,
      rD = 0x03,
      rE = 0x04,
      rF = 0x05,
      rG = 0x06,
      rH = 0x07,
      rI = 0x08,
      rJ = 0x09,
      rK = 0x0A,
      rL = 0x0B,
      rM = 0x0C,
      rN = 0x0D,
      rO = 0x0E,
      rP = 0x0F,
      rQ = 0x10,
      rR = 0x11,
      rS = 0x12,
      rT = 0x13,
      rU = 0x14,
      rV = 0x15,
      rW = 0x16,
      rX = 0x17,
      rY = 0x18,
      rZ = 0x19,
      rBB = 0x1A,
      rTT = 0x1B,
      rWW = 0x1C,
      rXX = 0x1D,
      rYY = 0x1E,
      rZZ = 0x1F,
    };
	public:
		mmix(int byte_size, int address_size);
		void step(int inst);

    void wideMult(unsigned long long int a, 
                  unsigned long long int b,
                  unsigned long long int *carry, 
                  unsigned long long int *result
                 );
    void wideDiv(unsigned long long int numerator_hi,
                 unsigned long long int numerator_lo,
                 unsigned long long int divisor,
                 unsigned long long int *quotient,
                 unsigned long long int *remainder
                );
    // Utility functions - see fascicle
    // Example: M_8(59) will give the octabyte at byte address 59,
    // by first rounding down to the nearest octabyte address
    // 63 / 8 = 56
    // So it concatenates the 8 bytes starting at byte address 56
    unsigned long long int M(unsigned int size, unsigned long long address);
    // M(size, address, value) calculates the address the same way, and
    // stores the given value there
    unsigned long long int M(
      unsigned int size, 
      unsigned long long address,
      unsigned long long value
    );

    // Load/store registers
    unsigned long long int R(unsigned int reg);
    unsigned long long int R(unsigned int reg, unsigned long long int value);
    // Load/store special registers
    unsigned long long int g(unsigned int reg);
    unsigned long long int g(unsigned int reg, unsigned long long int value);
};

#endif
