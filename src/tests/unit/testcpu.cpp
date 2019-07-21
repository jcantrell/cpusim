#include <iostream>
#include <string>
#include "../../model/cpu/cpu.h"
class TestCPU : public cpu
{
  public:
  TestCPU() : cpu(8, Address(65536), 32) {}
	bool testmemdump()
  {
    Morsel a(95);
    Address b(102);
    cpu mycpu(8, Address(65536), 32);
    mycpu.load(b, a);
    mycpu.memdump();
    return false;
  }
	bool testload()
  {
    bool result = true;
      
    Morsel a(95);
    Address b(102);
    cpu mycpu(8, Address(65536), 32);
    mycpu.load(b, a);
    Morsel c;
    c = mycpu.view(b);
    result = result && (a==c);
    if (!(a == c))
    {
      cout << endl << "Expected: " << a
                   << " Actual: " << c << endl;
    }
    return result;
  }
	bool testview()
  {
    bool result = true;
      
    Morsel a(95);
    Address b(102);
    cpu mycpu(8, Address(65536), 32);
    mycpu.load(b, a);
    Morsel c;
    c = mycpu.view(b);
    result = result && (a==c);
    if (!(a == c))
    {
      cout << endl << "Expected: " << a
                   << " Actual: " << c << endl;
    }
    return result;
  }
	bool testgetip()
  {
    bool result = true;

    cpu mycpu(8, Address(65536), 32);
    mycpu.setip(Address(22));
    Address ip = mycpu.getip();

    result = result && (ip==Address(22));
    if (!(ip  == Address(22)))
    {
      cout << endl << "Expected: " << Address(22)
                   << " Actual: " << ip << endl;
    }
    return result;
  }
	bool testsetip()
  {
    bool result = true;

    cpu mycpu(8, Address(65536), 32);
    mycpu.setip(Address(22));
    Address ip = mycpu.getip();

    result = result && (ip==Address(22));
    if (!(ip  == Address(22)))
    {
      cout << endl << "Expected: " << Address(22)
                   << " Actual: " << ip << endl;
    }
    return result;
  }
	bool testtoString()
  {
    bool result = true;
    cpu mycpu(8, Address(65536), 32);
    string actual = "Byte width: 8\nAddress size: 010000\nIP: 00\nFlags: 42\n";
    actual += "-----------------------------------------------\n";
    actual += "| OF | DF | INF | TF | SF | ZF | AF | PF | CF |\n";
    actual += "-----------------------------------------------\n";
    actual += "|  0 |  0 |   0 |  1 |  0 |  1 |  0 |  1 |  0 |\n";
    actual += "-----------------------------------------------\n";
    result = result && (mycpu.toString() == actual);
    if (!( mycpu.toString() == actual))
    {
      cout << endl << "Expected: " << mycpu.toString()
                   << " Actual: " << actual << endl;
    }
    return result;
  }
  bool testregs()
  {
    bool result = true;
      
    Morsel a(95);
    Address b(5);
    cpu mycpu(8, Address(65536), 32);
    mycpu.regs(b, a);
    Morsel c;
    c = mycpu.regs(b);
    result = result && (a==c);
    if (!(a == c))
    {
      cout << endl << "Expected: " << a
                   << " Actual: " << c << endl;
    }
    return result;
  }
  bool testregsAddressMorsel()
  {
    return false;
  }
  bool testloadimage()
  {
    return false;
  }

  void runAllTests()
  {
    TestCPU tm;
    struct UnitTest {
      string funcName;
      bool (TestCPU::*funcPtr)();
    };

    UnitTest tests[] = {
      {"testmemdump", &TestCPU::testmemdump}
	,{"testload",&TestCPU::testload}
	,{"testview",&TestCPU::testview}
	,{"testgetip",&TestCPU::testgetip}
	,{"testsetip",&TestCPU::testsetip}
	,{"testtoString",&TestCPU::testtoString}
  ,{"testregs",&TestCPU::testregs}
  ,{"testregsAddressMorsel",&TestCPU::testregsAddressMorsel}
  ,{"testloadimage",&TestCPU::testloadimage}
    };

    for (UnitTest &t : tests)
    {
	    std::cout << "Test " << t.funcName << " " 
        << ( (tm.*(t.funcPtr))() ? "passed" : "failed") << endl;
    }
 
  }
};

int main()
{
	TestCPU tm;
  tm.runAllTests();
	return 0;
}
