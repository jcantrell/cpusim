#include <iostream>
#include <string>
#include "../../../model/cpu/cpu.h"
class TestCPU : public cpu
{
  public:
  TestCPU() : cpu(8, Address(65536), 32) {}
	bool testmemdump()
  {
    UnsignedMorsel a(0x5f);
    Address b(0x66);
    cpu mycpu(8, Address(65536), 32);
    mycpu.load(b, a);
    std::ostringstream oss;
    std::ostringstream exp;
    mycpu.memdump(oss);
    exp << "60: 0000 0000 0000 5f00 0000 0000 0000 0000" << endl;
    bool result = (oss.str() == exp.str());
    return result;
  }
	bool testload()
  {
    bool result = true;
      
    UnsignedMorsel a(95);
    Address b(102);
    cpu mycpu(8, Address(65536), 32);
    mycpu.load(b, a);
    UnsignedMorsel c;
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
      
    UnsignedMorsel a(95);
    Address b(102);
    cpu mycpu(8, Address(65536), 32);
    mycpu.load(b, a);
    UnsignedMorsel c;
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
    Address tip = mycpu.getip();

    result = result && (tip==Address(22));
    if (!(tip  == Address(22)))
    {
      cout << endl << "Expected: " << Address(22)
                   << " Actual: " << tip << endl;
    }
    return result;
  }
	bool testsetip()
  {
    bool result = true;

    cpu mycpu(8, Address(65536), 32);
    mycpu.setip(Address(22));
    Address tip = mycpu.getip();

    result = result && (tip==Address(22));
    if (!(tip  == Address(22)))
    {
      cout << endl << "Expected: " << Address(22)
                   << " Actual: " << tip << endl;
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
      
    UnsignedMorsel a(95);
    Address b(5);
    cpu mycpu(8, Address(65536), 32);
    mycpu.regs(b, a);
    UnsignedMorsel c;
    c = mycpu.regs(b);
    result = result && (a==c);
    if (!(a == c))
    {
      cout << endl << "Expected: " << a
                   << " Actual: " << c << endl;
    }
    return result;
  }
  bool testregsAddressUnsignedMorsel()
  {
    cpu mycpu(8, Address(65536), 32);

    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    TestCase tests[] = {
       {mycpu.regs(Address(67),UnsignedMorsel(4)),UnsignedMorsel(4)}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in == t.out);
      if (!(t.in == t.out))
      {
        cout << "failed" << endl 
             << "Expected: " << t.out 
             << " Actual: " << t.in << endl;
      }
    }
    return result;
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
  ,{"testregsAddressUnsignedMorsel",&TestCPU::testregsAddressUnsignedMorsel}
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
