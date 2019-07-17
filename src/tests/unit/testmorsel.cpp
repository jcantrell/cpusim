#include <iostream>
#include <string>
#include "../../model/cpu/morsel.h"
class TestMorsel : public Morsel
{
	public:
  TestMorsel() : Morsel() {}
  TestMorsel(dynamic_bitset<> in) : Morsel(in) {}
  bool testCount()
  {
    dynamic_bitset<> tbs(64,0ul);
    return tbs.count() == count();
  }
  bool TestMorselAddMorsel()
  {
    bool result;
    Morsel a(7);
    Morsel b(982);
    Morsel c(989);
    Morsel d;
    d = a + b;
    result  = (c == d);

    Morsel e(82);
    Morsel f(83);
    Morsel g(165);
    result = result && ( (e+f) == g );
    return result;
  }
  bool testMorselAddInt()
  {
    bool result;
    Morsel a(7);
    int b = 982;
    Morsel c(989);
    Morsel d;
    d = a+b;
    result = (c==d);
    return result;
  }
  bool testMorselSubMorsel()
  {
    bool result;
    Morsel a(989);
    Morsel b(7);
    Morsel c(982);
    Morsel d;
    d = a - b;
    result = (c==d);
    return result;
  }
  bool testIntSubMorsel()
  {
    bool result;
    int a = 989;
    Morsel b(7);
    Morsel c(982);
    Morsel d;
    d = a - b;
    result = (c==d);
    return result;
  }
  bool testMorselDecMorsel()
  {
    Morsel a(42);
    Morsel b(7);
    a -= b;
    Morsel c(35);
    bool result = (a == c);
    return result;
  }
  bool testMorselInc()
  {
    Morsel a(94);
    a++;
    Morsel b(95);
    bool result = (a==b);
    return result;
  }
  bool testMorselAssignInt()
  {
    Morsel a(95);
    Morsel b;
    b = 95;
    return (a==b);
  }
  bool testMorselAssignMorsel()
  {
    Morsel a(95);
    Morsel b;
    b = a;
    return (a==b);
  }
  bool testMorselAsString()
  {
    Morsel a(95);
    //string str = "1011111";
    string str = "5f";
    return (str == a.asString());
  }
  bool testStreamMorsel()
  {
    Morsel a(95);
    std::stringstream out;
    out << a;
    Morsel b(42);
    cout << " testStreamMorsel: a: " << a << endl;
    cout << " testStreamMorsel: b: " << b << endl;
    return (out.str() == "5f");
  }
  bool testMorselLessThanMorsel()
  {
    Morsel a(95);
    Morsel b(100);
    return a < b;
  }
  bool testMorselLessThanInt()
  {
    Morsel a(95);
    unsigned b = 100;
    return a < b;
  }
  bool testMorselLessOrEqualMorsel()
  {
    Morsel a(95);
    Morsel b(100);
    return a <= b;
  }
  bool testMorselLessOrEqualInt()
  {
    Morsel a(95);
    return a <= 100;
  }
  bool testIntLessOrEqualMorsel()
  {
    Morsel b(100);
    return 95 <= b;
  }
  bool testMorselGreaterThanInt()
  {
    Morsel a(95);
    return a > 90;
  }
  bool testIntGreaterThanMorsel()
  {
    Morsel a(95);
    bool result = (100 > a);
    return result;
  }
  bool testMorselGreaterThanMorsel()
  {
    Morsel a(7);
    Morsel b(21);
    return b > a;
  }
  bool testMorselGreaterOrEqualMorsel()
  {
    Morsel a(95);
    Morsel b(100);
    return b >= a;
  }
  bool testMorselDivMorsel()
  {
    Morsel a(21);
    Morsel b(7);
    Morsel c;
    c = a / b;
    Morsel d(3);
    return (c==d);
  }
  bool testMorselModMorsel()
  {
    Morsel a(25);
    Morsel b(7);
    Morsel c;
    Morsel d(4);
    c = a % b;
    return (c==d);
  }
  bool testMorselSize()
  {
    Morsel a(8);
    return (a.size() == 4);
  }
  bool testMorselEqualMorsel() 
  {
    Morsel a(27);
    Morsel b(27);
    return (a==b);
  }
  bool testMorselEqualInt()
  {
    Morsel a(27);
    int b = 27;
    return (a==b);
  }
  bool testMorselAsInt()
  {
    unsigned int t = 25;
    Morsel a(t);
    unsigned int b;
    b = a.asInt();
    return (t==b);
  }
  bool testMorselBitwiseAndMorsel()
  {
    Morsel a(0xFF00);
    Morsel b(0x0F0F);
    Morsel c;
    c = a & b;
    Morsel d(0x0F00);
    return (c==d);
  }
  bool testMorselBitwiseAndInt()
  {
    Morsel a(0xFF00);
    unsigned b = 0x0F0F;
    Morsel c;
    c = a & b;
    Morsel d(0x0F00);
    return (c==d);
  }
  bool testMorselBitwiseOrMorsel()
  {
    Morsel a(0xFF00);
    Morsel b(0x0F0F);
    Morsel c;
    c = a | b;
    Morsel d(0xFF0F);
    return (c==d);
  }
  bool testMorselBitwiseOrMorselAssign()
  {
    Morsel a(0xFF00);
    Morsel b(0x0F0F);
    a |= b;
    Morsel d(0xFF0F);
    return (a==d);
  }
  bool testMorselBitwiseInverse()
  {
    Morsel a(0xFF00);
    Morsel b;
    b = ~a;
    Morsel c(0x00FF);
    return (c==b);
  }
  bool testMorselBitwiseXorMorsel()
  {
    Morsel a(0xFF00);
    Morsel b(0x0F0F);
    Morsel c;
    c = a ^ b;
    Morsel d(0xF00F);
    return (c==d);
  }
  bool testMorselLeftshiftMorsel()
  {
    Morsel a(0xFF00);
    Morsel b(4);
    Morsel c;
    c = a << b;
    Morsel d(0xF000);
    return (c==d);
  }
  bool testMorselLeftShiftInt()
  {
    Morsel a(0xFF00);
    int b = (4);
    Morsel c;
    c = a << b;
    Morsel d(0xF000);
    return (c==d);
  }
  bool testMorselRightShiftInt()
  {
    Morsel a(0xFF00);
    int b = (4);
    Morsel c;
    c = a >> b;
    Morsel d(0x0FF0);
    return (c==d);
  }
  bool testMorselRightShiftMorsel()
  {
    Morsel a(0xFF00);
    Morsel b(4);
    Morsel c;
    c = a >> b;
    Morsel d(0x0FF0);
    return (c==d);
  }
  bool testMorselMultiplyMorsel()
  {
    Morsel a(3);
    Morsel b(7);
    Morsel c(21);
    Morsel d;
    d = a * b;
    return (c==d);
  }
  bool testMorselIntMultiplyMorsel()
  {
    unsigned int a = 3;
    Morsel b(7);
    Morsel c(21);
    Morsel d;
    d = a * b;
    return (c==d);
  }
  bool testMorselLeftShiftAssignInt()
  {
    Morsel a(0xFF0F);
    unsigned int b = 4;
    Morsel c(0xF0F0);
    a <<= b;
    return (c==a);
  }
  bool testMorselNEMorsel()
  {
    Morsel a(27);
    Morsel b(31);
    return a != b;
  }
  bool testMorselNEInt()
  {
    Morsel a(27);
    unsigned int b = 31;
    return a != b;
  }
  bool testMorselAsFloat()
  {
    Morsel a(0x4048f5c3);
    float pi = 3.14;
    return (a.asFloat()==pi);
  }
  bool testMorselAsChar()
  {
    Morsel a(0xDEAD);
    unsigned char b;
    b = a.asChar();
    unsigned char c = 0xAD;
    return (b == c);
  }

  void runAllTests()
  {
    TestMorsel tm;
    struct NameResultPair {
      string funcName;
      bool (TestMorsel::*funcPtr)();
    };
    NameResultPair tests[] = {
       {"testCount", TestMorsel::testCount}
      ,{"TestMorselAddMorsel", TestMorsel::TestMorselAddMorsel}
      ,{"testMorselAddInt", TestMorsel::testMorselAddInt}
      ,{"testMorselSubMorsel", TestMorsel::testMorselSubMorsel}
      ,{"testIntSubMorsel", TestMorsel::testIntSubMorsel}
      ,{"testMorselDecMorsel", TestMorsel::testMorselDecMorsel}
      ,{"testMorselInc", TestMorsel::testMorselInc}
      ,{"testMorselAssignInt", TestMorsel::testMorselAssignInt}
      ,{"testMorselAsString", TestMorsel::testMorselAsString}
      ,{"testStreamMorsel", TestMorsel::testStreamMorsel}
      ,{"testMorselLessThanMorsel", TestMorsel::testMorselLessThanMorsel}
      ,{"testMorselLessThanInt", TestMorsel::testMorselLessThanInt}
      ,{"testMorselLessOrEqualMorsel", TestMorsel::testMorselLessOrEqualMorsel}
      ,{"testMorselLessOrEqualInt", TestMorsel::testMorselLessOrEqualInt}
      ,{"testIntLessOrEqualMorsel", TestMorsel::testIntLessOrEqualMorsel}
      ,{"testMorselGreaterThanInt", TestMorsel::testMorselGreaterThanInt}
      ,{"testIntGreaterThanMorsel", TestMorsel::testIntGreaterThanMorsel}
      ,{"testMorselGreaterThanMorsel", TestMorsel::testMorselGreaterThanMorsel}
      ,{"testMorselGreaterOrEqualMorsel", TestMorsel::testMorselGreaterOrEqualMorsel}
      ,{"testMorselDivMorsel", TestMorsel::testMorselDivMorsel}
      ,{"testMorselModMorsel", TestMorsel::testMorselModMorsel}
      ,{"testMorselSize", TestMorsel::testMorselSize}
      ,{"testMorselEqualMorsel", TestMorsel::testMorselEqualMorsel}
      ,{"testMorselEqualInt", TestMorsel::testMorselEqualInt}
      ,{"testMorselAsInt", TestMorsel::testMorselAsInt}
      ,{"testMorselBitwiseAndMorsel", TestMorsel::testMorselBitwiseAndMorsel}
      ,{"testMorselBitwiseAndInt", TestMorsel::testMorselBitwiseAndInt}
      ,{"testMorselBitwiseOrMorsel", TestMorsel::testMorselBitwiseOrMorsel}
      ,{"testMorselBitwiseOrMorselAssign", TestMorsel::testMorselBitwiseOrMorselAssign}
      ,{"testMorselBitwiseInverse", TestMorsel::testMorselBitwiseInverse}
      ,{"testMorselBitwiseXorMorsel", TestMorsel::testMorselBitwiseXorMorsel}
      ,{"testMorselLeftshiftMorsel", TestMorsel::testMorselLeftshiftMorsel}
      ,{"testMorselLeftShiftInt", TestMorsel::testMorselLeftShiftInt}
      ,{"testMorselRightShiftInt", TestMorsel::testMorselRightShiftInt}
      ,{"testMorselRightShiftMorsel", TestMorsel::testMorselRightShiftMorsel}
      ,{"testMorselMultiplyMorsel", TestMorsel::testMorselMultiplyMorsel}
      ,{"testMorselIntMultiplyMorsel", TestMorsel::testMorselIntMultiplyMorsel}
      ,{"testMorselLeftShiftAssignInt", TestMorsel::testMorselLeftShiftAssignInt}
      ,{"testMorselNEMorsel", TestMorsel::testMorselNEMorsel}
      ,{"testMorselNEInt", TestMorsel::testMorselNEInt}
      ,{"testMorselAsFloat", TestMorsel::testMorselAsFloat}
      ,{"testMorselAsChar", testMorselAsChar}
  };

    for (NameResultPair &t : tests)
    {
	    std::cout << "Test " << t.funcName << " " 
        << ( (tm.*(t.funcPtr))() ? "passed" : "failed") << endl;
    }
  }
};

int main()
{
	TestMorsel tm;
  tm.runAllTests();
	return 0;
}
