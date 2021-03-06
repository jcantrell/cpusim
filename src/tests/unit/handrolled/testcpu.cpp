#include <iostream>
#include <string>
#include "../../../model/cpu/cpu.h"
class TestCPU : public cpu
{
  private:
    template <typename T>
    bool runCases(vector<T> tests) {
      bool result = true;
      for (auto &t : tests)
      {
        result = result && (t.in == t.out);
        if (!(t.in == t.out)) {
          cout << endl << "Expected: " << t.out << " Actual: " << t.in << endl;
        }
      }
      return result;
    }
  public:
  TestCPU() : cpu(8, UnsignedMorsel(65536), 32) {}
	bool testmemdump()
  {
    struct TestCase {
      string in;
      string out;
    };

    UnsignedMorsel a(0x5f);
    UnsignedMorsel b(0x66);
    cpu mycpu(8, UnsignedMorsel(65536), 32);
    mycpu.load(b, a);
    std::ostringstream oss;
    std::ostringstream exp;
    mycpu.memdump(oss);
    exp << "0060: 0000 0000 0000 5f00 0000 0000 0000 0000" << endl;

    vector<TestCase> tests = {
      {oss.str(), exp.str()}
    };
    return runCases(tests);
  }
	bool testload()
  {
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    bool result = true;
      
    UnsignedMorsel a(95);
    UnsignedMorsel b(102);
    cpu mycpu(8, UnsignedMorsel(65536), 32);
    mycpu.load(b, a);
    UnsignedMorsel c;
    c = mycpu.view(b);

    vector<TestCase> tests = {
      {a,c}
    };
    return runCases(tests);
  }
	bool testview()
  {
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };
      
    UnsignedMorsel a(95);
    UnsignedMorsel b(102);
    cpu mycpu(8, UnsignedMorsel(65536), 32);
    mycpu.load(b, a);
    UnsignedMorsel c;
    c = mycpu.view(b);

    vector<TestCase> tests = {
      {a,c}
    };
    return runCases(tests);
  }
	bool testgetip()
  {
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    cpu mycpu(8, UnsignedMorsel(65536), 32);
    mycpu.setip(UnsignedMorsel(22));
    UnsignedMorsel tip = mycpu.getip();

    vector<TestCase> tests = {
      {tip,UnsignedMorsel(22)}
    };
    return runCases(tests);
  }
	bool testsetip()
  {
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    cpu mycpu(8, UnsignedMorsel(65536), 32);
    mycpu.setip(UnsignedMorsel(22));
    UnsignedMorsel tip = mycpu.getip();

    vector<TestCase> tests = {
      {tip,UnsignedMorsel(22)}
    };
    return runCases(tests);
  }
	bool testtoString()
  {
    struct TestCase {
      string in;
      string out;
    };

    cpu mycpu(8, UnsignedMorsel(65536), 32);
    string actual 
            = "Byte width: 8\nUnsignedMorsel size: 010000\nIP: 00\nFlags: 42\n";
    actual += "-----------------------------------------------\n";
    actual += "| OF | DF | INF | TF | SF | ZF | AF | PF | CF |\n";
    actual += "-----------------------------------------------\n";
    actual += "|  0 |  0 |   0 |  1 |  0 |  1 |  0 |  1 |  0 |\n";
    actual += "-----------------------------------------------\n";

    vector<TestCase> tests = {
      {mycpu.toString(),actual}
    };
    return runCases(tests);
  }
  bool testregs()
  {
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    cpu mycpu(8, UnsignedMorsel(65536), 32);
    UnsignedMorsel a(95);
    UnsignedMorsel b(5);
    mycpu.regs(b, a);

    vector<TestCase> tests = {
      {mycpu.regs(b), UnsignedMorsel(95)}
    };

    return runCases(tests);
  }
  bool testregsUnsignedMorselUnsignedMorsel()
  {
    cpu mycpu(8, UnsignedMorsel(65536), 32);

    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    vector<TestCase> tests = {
       {mycpu.regs(UnsignedMorsel(67),UnsignedMorsel(4)),UnsignedMorsel(4)}
    };

    return runCases(tests);
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
  ,{"testregsUnsignedMorselUnsignedMorsel",&TestCPU::testregsUnsignedMorselUnsignedMorsel}
  ,{"testloadimage",&TestCPU::testloadimage}
    };

    for (UnitTest &t : tests)
    {
	    std::cout << "Test " 
        << ( (tm.*(t.funcPtr))() ? "passed" : "failed") 
        << " " 
        << t.funcName 
        << endl;
    }
 
  }
};

int main()
{
	TestCPU tm;
  tm.runAllTests();
	return 0;
}
