#ifndef MMIX_H
#define MMIX_H

#include <string>
#include <iostream>
#include <fstream>
#include "model/cpu/cpu.h"

class mmix : public cpu {
  private:
    unsigned char L;
    unsigned char G;
    unsigned long long int register_stack_top;
    std::unordered_map<int, int> special_registers;
    std::unordered_map<int, int> globals;
    enum inst {
#include "model/mmix/opcodes_mmix.h"
    };
    enum special_registers {
#include "model/mmix/special_registers_mmix.h"
    };
    enum loader_opcodes {
#include "model/mmix/loader_opcodes.h"
    };
    enum sys_calls {
      Halt, Fopen, Fclose, Fread, Fgets, Fgetws, Fwrite, Fputs,
      Fputws, Fseek, Ftell
    };
	public:
		mmix(int byte_size, int address_size);
		void step(int inst);
    void loadfile(string &filename);

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

    void push(unsigned char reg);
    void pop(unsigned char reg);
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

    // Is one double in the neighborhood of another?
    bool N(double, double, unsigned int, float);
};

#endif
