#ifndef SUBLEQ_H
#define SUBLEQ_H

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

    void wideMult(unsigned long long int a, unsigned long long int b,
        unsigned long long int *carry, unsigned long long int *result);
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
    unsigned long long int SR(unsigned int reg);
    unsigned long long int SR(unsigned int reg, unsigned long long int value);
};

#endif
