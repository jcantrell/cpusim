#include <iostream>
#include "../../model/cpu/morsel.h"
class TestMorsel : public Morsel
{
  private:
    dynamic_bitset<> getBitset()
    {
      return bs;
    }
	public:
  TestMorsel() : Morsel() {}
  TestMorsel(dynamic_bitset<> in) : Morsel(in) {}
  bool testCount()
  {
    dynamic_bitset<> tbs(64,0ul);
    return tbs.count() == count();
  }
  bool testMorsel_bitset()
  {
    dynamic_bitset<> tbs(64,0ul);
    TestMorsel tm(tbs);
    return tbs == tm.getBitset();
  }

  void runAllTests()
  {
    TestMorsel tm;
	  std::cout << "Test " << (tm.testCount() ? "passed" : "failed") << endl;
	  std::cout << "Test " << (tm.testMorsel_bitset() ? "passed" : "failed") << endl;
  }
};

int main()
{
	TestMorsel tm;
  tm.runAllTests();
	return 0;
}
