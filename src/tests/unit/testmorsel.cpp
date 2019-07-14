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
    std::cout << "a: " << a << " b: " << b << " c: " << c << " d: " << d << endl;
    return result;
  }

  void runAllTests()
  {
    TestMorsel tm;
    struct NameResultPair {
      string funcName;
      bool (TestMorsel::*funcPtr)();
      bool result;
    };
    NameResultPair tests[] = {
       {"testCount", TestMorsel::testCount, false}
      ,{"TestMorselAddMorsel", TestMorsel::TestMorselAddMorsel, false}
      ,{"testMorselAddInt", TestMorsel::testMorselAddInt, false}
      ,{"testMorselSubMorsel", TestMorsel::testMorselSubMorsel, false}
      ,{"testIntSubMorsel", TestMorsel::testIntSubMorsel, false}
    };

    for (NameResultPair &t : tests)
    {
      t.result = (tm.*(t.funcPtr))();
	    std::cout << "Test " << t.funcName << " " 
        << (t.result ? "passed" : "failed") << endl;
    }
  }
};

int main()
{
	TestMorsel tm;
  tm.runAllTests();
	return 0;
}
