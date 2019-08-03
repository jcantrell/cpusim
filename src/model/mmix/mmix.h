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
    boost::unordered_map<int, int> special_registers;
    boost::unordered_map<Address, Morsel> globals;
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
		mmix(int byte_size, Address address_size);
		void step(int inst);
    void loadfile(string &filename);

    void wideMult(Morsel a, 
                  Morsel b,
                  Morsel *carry, 
                  Morsel *result
                 );
    void wideDiv(Morsel numerator_hi,
                 Morsel numerator_lo,
                 Morsel divisor,
                 Morsel *quotient,
                 Morsel *remainder
                );

    void push(Morsel reg);
    void pop(Morsel reg);
    // Utility functions - see fascicle
    // Example: M_8(59) will give the octabyte at byte address 59,
    // by first rounding down to the nearest octabyte address
    // 63 / 8 = 56
    // So it concatenates the 8 bytes starting at byte address 56
    Morsel M(unsigned int size, Address address);
    // M(size, address, value) calculates the address the same way, and
    // stores the given value there
    Morsel M(
      unsigned int size, 
//      unsigned long long address,
      Address address,
      //unsigned long long value
      Morsel value
    );

    // Load/store registers
    Morsel R(Address reg);
    Morsel R(Address reg, Morsel value);
    // Load/store special registers
    Morsel g(Address reg);
    Morsel g(Address reg, Morsel value);

    // Is one double in the neighborhood of another?
    bool N(double, double, unsigned int, float);

    // load a .mmo file
    void loadobject(string filename);


    //Instruction set
    void jmp();
};

#endif
