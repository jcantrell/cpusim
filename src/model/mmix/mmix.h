#ifndef MMIX_H
#define MMIX_H

#include <string>
#include <iostream>
#include <fstream>
#include "model/cpu/cpu.h"

class mmix : public cpu {
  private:
    unsigned int stepsize = 4;
    unsigned char L;
    unsigned char G;
    Address target;
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
		void step(Morsel inst);
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
void ldtu();
void ldtui();
void ldo();
void ldoi();
void ldou();
void ldoui();
void ldht();
void ldhti();
void stb();
void stbi();
void stw();
void stwi();
void stt();
void stti();
void sto();
void stoi();
void stbu();
void stbui();
void stwu();
void stwui();
void sttu();
void sttui();
void stou();
void stoui();
void stht();
void sthti();
void stco();
void stcoi();
void add();
void addi();
void sub();
void subi();
void mul();
void muli();
void div();
void divi();
void addu();
void addui();
void subu();
void subui();
void mulu();
void mului();
void divu();
void divui();
void i2addu();
void i2addui();
void i4addu();
void i4addui();
void i8addu();
void i8addui();
void i16addu();
void i16addui();
void neg();
void negi();
void negu();
void negui();
void sl();
void sli();
void slu();
void slui();
void sr();
void sri();
void sru();
void srui();
void cmp();
void cmpi();
void cmpu();
void cmpui();
void csn();
void csni();
void csz();
void cszi();
void csp();
void cspi();
void csod();
void csodi();
void csnn();
void csnni();
void csnz();
void csnzi();
void csnp();
void csnpi();
void csev();
void csevi();
void zsn();
void zsni();
void zsz();
void zszi();
void zsp();
void zspi();
void zsod();
void zsodi();
void zsnn();
void zsnni();
void zsnz();
void zsnzi();
void zsnp();
void zsnpi();
void zsev();
void zsevi();
void opcode_AND();
void andi();
void opcode_OR();
void ori();
void opcode_xor();
void xori();
void andn();
void andni();
void orn();
void orni();
void nand();
void nandi();
void nor();
void nori();
void nxor();
void nxori();
void mux();
void muxi();
void sadd();
void saddi();
void bdif();
void bdifi();
void wdif();
void wdifi();
void tdif();
void tdifi();
void odif();
void odifi();
void mor();
void mori();
void mxor();
void mxori();
void fadd();
void fsub();
void fmul();
void fdiv();
void frem();
void fsqrt();
void fint();
void fcmp();
void feql();
void fun();
void fcmpe();
void feqle();
void fune();
void fix();
void fixu();
void flot();
void flotu();
void flotui();
void sflot();
void sfloti();
void sflotu();
void sflotui();
void ldsf();
void ldsfi();
void stsf();
void stsfi();
void floti();
void seth();
void setmh();
void setml();
void setl();
void inch();
void incmh();
void incml();
void incl();
void orh();
void ormh();
void orml();
void orl();
void andnh();
void andnmh();
void andnml();
void andnl();
void go();
void bn();
void bz();
void bp();
void bod();
void bnn();
void bnz();
void bnp();
void bev();
void pbn();
void pbz();
void pbp();
void pbod();
void pbnn();
void pbnz();
void pbnp();
void pbev();
void jmpb();
void bnb();
void bzb();
void bpb();
void bodb();
void bnnb();
void bnzb();
void bnpb();
void bevb();
void pbnb();
void pbzb();
void pbpb();
void pbodb();
void pbnnb();
void pbnzb();
void pbnpb();
void pbevb();
void pushj();
void pushjb();
void pushgo();
void pop();
void save();
void unsave();
void ldunc();
void stunc();
void ldunci();
void cswap();
void cswapi();
void preld();
void preldi();
void prego();
void pregoi();
void goi();
void stunci();
void sync();
void syncd();
void syncdi();
void prest();
void presti();
void syncid();
void syncidi();
void pushgoi();
void trip();
void trap();
void resume();
void get();
void put();
void puti();
void geta();
void getab();
void jmp();
void ldb();
void ldbi();
void ldbu();
void ldbui();
void ldw();
void ldwi();
void ldwu();
void ldwui();
void ldt();
void ldti();
void swym();
void ldvts();
void ldvtsi();
void ofif();
};

#endif
