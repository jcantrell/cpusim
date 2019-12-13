#include "model/cpu/Address.h"
#include "model/cpu/cpu.h"
#include "model/mmix/mmix.h"
#include <fstream>
#include "model/loader/Loader.h"
#include "model/cpu/UnsignedMorsel.h"
//#include <stdint.h>
//#include <vector>
class TestLoader : public Loader
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
  TestLoader() : Loader() {}
  void runAllTests();
  bool TestQuote();
  bool TestLoc();
  bool TestSkip();
  bool TestFixo();
  bool TestFixr();
  bool TestFixrx();
  bool TestFile();
  bool TestLine();
  bool TestSpec();
  bool TestPre();
  bool TestPost();
  bool TestStab();
  bool TestEnd();
};

bool TestLoader::TestQuote() {
    struct TestCase {
      bool in;
      bool out;
    };

    Loader newLoader;
    bool q1 = quoted_flag;
    quote(0,1);
    bool q2 = quoted_flag;

    Address t;
    vector<TestCase> tests = {
       {q1, false}
      ,{q2, true}
    };

    return runCases(tests);
}

bool TestLoader::TestLoc() {
  struct TestCase {
    Address in;
    Address out;
  };

  // read from file
  // save address

  vector<TestCase> tests = {
     {Address(0), Address(UnsignedMorsel(0x0200000000000100))}
    ,{Address(0), Address(UnsignedMorsel(0xDEADBEEFCAFEBABE))}
  };

  in = std::ifstream("tests/functional/t", std::ifstream::binary);
  if (!in) { printf("No such file!\n"); return false; }
  loc(2, 1);
  tests[0].in = lambda;

  in = std::ifstream("tests/functional/w", std::ifstream::binary);
  if (!in) { printf("No such file!\n"); return false; }
  loc(0x15, 2);
  tests[1].in = lambda;

  return runCases(tests);
}

bool TestLoader::TestSkip() {
  struct  TestCase {
    Address in;
    Address out;
  };

  skip(2, 55);
  vector<TestCase> tests = {
    {lambda, Address(0x0000000000000237)}
  };

  return runCases(tests);
}

bool TestLoader::TestLine() {
  struct TestCase {
    bool in;
    bool out;
  };
  vector<TestCase> tests = {
    {true, true}
  };
  return runCases(tests);
}

bool TestLoader::TestFile() {
  struct TestCase {
    bool in;
    bool out;
  };
  vector<TestCase> tests = {
    {true, true}
  };
  return runCases(tests);
}

bool TestLoader::TestPost() {
  struct TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };
  in = std::ifstream("tests/functional/u", std::ifstream::binary);
  mmix mycpu(8, Address(65536));
  post(mycpu, 246);

  vector<TestCase> tests = {
    {mycpu.regs(Address(255)), UnsignedMorsel(0x48494a4b4c4d4e4ful)}
  };
  return runCases(tests);
}

bool TestLoader::TestFixrx() {
  struct TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };
  in = std::ifstream("tests/functional/x", std::ifstream::binary);
  mmix mycpu(8, Address(65536));
  fixrx(mycpu, 0, 16u);
  vector<TestCase> tests = {
    {mycpu.view(Address(0x2Cu)), UnsignedMorsel(0x00000010u)}
  };
  return runCases(tests);
}

bool TestLoader::TestPre() {
  struct TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };
  Loader l;
  setfile("tests/functional/testLoader/testPre1");
  mmix mycpu(8, Address(65536));
  pre(1);
  vector<TestCase> tests = {
    {UnsignedMorsel(0x0u), UnsignedMorsel(0x0u)}
  };
  return runCases(tests);
}

void TestLoader::runAllTests()
{
    //TestLoader tm;
    struct NameResultPair {
      TestLoader tm;
      string funcName;
      bool (TestLoader::*funcPtr)();
    };

    NameResultPair tests[] = {
       {TestLoader(), "TestPre",   &TestLoader::TestPre}
      ,{TestLoader(), "TestQuote", &TestLoader::TestQuote}
      ,{TestLoader(), "TestLoc",   &TestLoader::TestLoc}
      ,{TestLoader(), "TestSkip",  &TestLoader::TestSkip}
      ,{TestLoader(), "TestLine",  &TestLoader::TestLine}
      ,{TestLoader(), "TestFile",  &TestLoader::TestFile}
      ,{TestLoader(), "TestPost",  &TestLoader::TestPost}
      ,{TestLoader(), "TestFixrx", &TestLoader::TestFixrx}
    };
    cout << "TestLoader" << endl;
    for (NameResultPair &t : tests)
    {
	    std::cout << "Test " << t.funcName << " " 
        << ( (t.tm.*(t.funcPtr))() ? "passed" : "failed") << endl;
    }
  }

int main()
{
	TestLoader tm;
  tm.runAllTests();
	return 0;
}
