class Loader {
  private:  
    enum loader_opcodes {
#include "model/mmix/loader_opcodes.h"
    };
  union tetra_union {
    uint32_t num;
    char ar[4];
  };
    bool quoted_flag;
    Address lambda;
    Address address;
    std::ifstream in;
  public:
    Loader() : quoted_flag(false) , lambda(0) {};
    void loadobject(cpu& mycpu, string filename);
    void quote();
    void loc(unsigned char y, unsigned char z);
    void skip(unsigned char y, unsigned char z);
    void fixo(cpu& c, unsigned char y, unsigned char z);
    void fixr(cpu& c, unsigned char y, unsigned char z);
    void fixrx(cpu& c, unsigned char z);
    void file(tetra_union& tetra, unsigned char z);
    void line();
    void spec();
    void pre(tetra_union& tetra, unsigned char z);
    void post(cpu& mycpu, unsigned char z);
    void stab();
    void end();
};
