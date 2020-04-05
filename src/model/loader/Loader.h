class Loader {
  protected:  
    enum loader_opcodes {
#include "model/mmix/loader_opcodes.h"
    };
  union tetra_union {
    uint32_t num;
    char ar[4];
  };
    bool quoted_flag;
    bool in_stab;
    bool spec_flag;
    unsigned filenum;
    unsigned linenum;
    unsigned long long timestamp;
    UnsignedMorsel lambda;
    string fn;
    std::ifstream in;
  public:
    Loader() : quoted_flag(false), in_stab(false), lambda(UnsignedMorsel(0)) {};
    void loadobject(cpu& mycpu, string filename);
    void step(cpu& mycpu, unsigned char w, unsigned char x, unsigned char y, unsigned char z);
    void setfile(string filename);

    void quote(unsigned char y, unsigned char z);
    void loc(unsigned char y, unsigned char z);
    void skip(unsigned char y, unsigned char z);
    void fixo(cpu& c, unsigned char y, unsigned char z);
    void fixr(cpu& c, unsigned char y, unsigned char z);
    void fixrx(cpu& c, unsigned char y, unsigned char z);
    void file(unsigned char y, unsigned char z);
    void line(unsigned char y, unsigned char z);
    void spec(cpu& c);
    void pre(unsigned char z);
    void post(cpu& mycpu, unsigned char z);
    void stab(unsigned char y, unsigned char z);
    void end();
};
