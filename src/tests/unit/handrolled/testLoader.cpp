#include "model/cpu/UnsignedMorsel.h"
#include "model/cpu/cpu.h"
#include "model/mmix/mmix.h"
#include <fstream>
#include "model/loader/Loader.h"
#include "model/cpu/UnsignedMorsel.h"
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
  bool TestLoadObject();
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

    UnsignedMorsel t;
    vector<TestCase> tests = {
       {q1, false}
      ,{q2, true}
    };

    return runCases(tests);
}

bool TestLoader::TestLoc() {
  struct TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };

  vector<TestCase> tests = {
     {UnsignedMorsel(0), UnsignedMorsel(UnsignedMorsel(0x0200000000000100))}
    ,{UnsignedMorsel(0), UnsignedMorsel(UnsignedMorsel(0xDEADBEEFCAFEBABE))}
  };

  in = std::ifstream("tests/functional/testLoader/testLoc1", std::ifstream::binary);
  if (!in) { printf("No such file!\n"); return false; }
  loc(2, 1);
  tests[0].in = lambda;

  in = std::ifstream("tests/functional/testLoader/testLoc2", std::ifstream::binary);
  if (!in) { printf("No such file!\n"); return false; }
  loc(0x15, 2);
  tests[1].in = lambda;

  return runCases(tests);
}

bool TestLoader::TestSpec() {
  struct TestCase {
    string in;
    string out;
  };

  vector<TestCase> tests = {
     {"", "test"}
  };

  mmix mycpu(8, UnsignedMorsel(65536));
  in = std::ifstream("tests/functional/testLoader/testSpec1", 
                     std::ifstream::binary);
  if (!in) { printf("No such file!\n"); return false; }
  spec(mycpu);
  tests[0].in = fn;

  return runCases(tests);
}

bool TestLoader::TestSkip() {
  struct  TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };

  skip(2, 55);
  vector<TestCase> tests = {
    {lambda, UnsignedMorsel(0x0000000000000237)}
  };

  return runCases(tests);
}

bool TestLoader::TestLine() {
  struct TestCase {
    bool in;
    bool out;
  };
  line(static_cast<unsigned char>(0xBE),static_cast<unsigned char>(0xEF));
  vector<TestCase> tests = {
    {linenum==static_cast<unsigned>(0xBEEF),true}
  };
  return runCases(tests);
}

bool TestLoader::TestFile() {
  struct TestCase {
    bool in;
    bool out;
  };
  
  in = std::ifstream("tests/functional/testLoader/testFile1", std::ifstream::binary);
  file(2,4);
  vector<TestCase> tests = {
    {filenum==2 && fn=="somefilenamefour", true}
  };
  return runCases(tests);
}

bool TestLoader::TestPost() {
  struct TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };
  in = std::ifstream("tests/functional/testLoader/testPost1", std::ifstream::binary);
  mmix mycpu(8, UnsignedMorsel(65536));
  post(mycpu, 253);

  vector<TestCase> tests = {
    {mycpu.regs(UnsignedMorsel(253)), UnsignedMorsel(0xDEADBEEFCAFEBABEul).resize(64)},
    {mycpu.regs(UnsignedMorsel(254)), UnsignedMorsel(0x0200000000000000ul).resize(64)},
    {mycpu.regs(UnsignedMorsel(255)), UnsignedMorsel(0x0000010000000000ul).resize(64)}
  };
  return runCases(tests);
}

bool TestLoader::TestFixr() {
  struct TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };
  mmix mycpu(8, UnsignedMorsel(65536));
  UnsignedMorsel P = UnsignedMorsel(0xFFFFFFFFFFFC854C);
  UnsignedMorsel old = mycpu.view(P).pb(24) 
                      | mycpu.view(P+1).pb(16)
                      | mycpu.view(P+2).pb(8)
                      | mycpu.view(P+3) ;
  fixr(mycpu, 0xDE, 0xAD);
  UnsignedMorsel n = mycpu.view(P).pb(24) 
                      | mycpu.view(P+1).pb(16)
                      | mycpu.view(P+2).pb(8)
                      | mycpu.view(P+3) ;
  vector<TestCase> tests = {
    {((old&UnsignedMorsel(0xFFFF0000))==(n&UnsignedMorsel(0xFFFF0000))) && 
      ((n&UnsignedMorsel(0x0000FFFF)) == UnsignedMorsel(0xDEAD)), true}
  };
  return runCases(tests);
}

bool TestLoader::TestFixrx() {
  struct TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };
  in = std::ifstream("tests/functional/testLoader/testFixrx1", std::ifstream::binary);
  mmix mycpu(8, UnsignedMorsel(65536));
  fixrx(mycpu, 0, 16u);
  vector<TestCase> tests = {
    {mycpu.view(UnsignedMorsel(0xFEDADAE0u)), UnsignedMorsel(0x4Bu)}
  };
  return runCases(tests);
}

bool TestLoader::TestStab() {
  struct TestCase {
    bool in;
    bool out;
  };
  stab(0,0);
  vector<TestCase> tests = {
    {in_stab, true}
  };
  return runCases(tests);
}

bool TestLoader::TestEnd() {
  struct TestCase {
    bool in;
    bool out;
  };
  end(); // symbol table is 4 tetrabytes long; check that it is preceded by
            // the lop_stab symbol
  vector<TestCase> tests = {
    {in_stab==false, true}
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
  mmix mycpu(8, UnsignedMorsel(65536));
  pre(1);
  vector<TestCase> tests = {
    {timestamp, UnsignedMorsel(0x7fffu)}
  };
  return runCases(tests);
}

bool TestLoader::TestFixo()
{
  struct TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };
  mmix mycpu(8,UnsignedMorsel(65536));
  setfile("tests/functional/testLoader/testFixo1");
  fixo(mycpu,0xDE,2);
  vector<TestCase> tests = {
    {mycpu.view(UnsignedMorsel(0xDEADBEEFul))==lambda, true}
  };
  return runCases(tests);
}

bool TestLoader::TestLoadObject()
{
  struct TestCase {
    bool in;
    bool out;
  };
  mmix mycpu(8,UnsignedMorsel(65536));
  loadobject(mycpu, "tests/functional/mmix/test.mmo");
  vector<TestCase> tests = {
    {false, true}
  };

  struct memPair {
    UnsignedMorsel a;
    UnsignedMorsel v;
  };
  vector<memPair> m = {
     {UnsignedMorsel(0x2000000000000003), UnsignedMorsel(0x01)}
    ,{UnsignedMorsel(0x2000000000000004), UnsignedMorsel(0x23)}
    ,{UnsignedMorsel(0x2000000000000005), UnsignedMorsel(0x45)}
    ,{UnsignedMorsel(0x2000000000000006), UnsignedMorsel(0xa7)}
    ,{UnsignedMorsel(0x2000000000000007), UnsignedMorsel(0x68)}
    ,{UnsignedMorsel(0x2000000000000008), UnsignedMorsel(0x61)}
    ,{UnsignedMorsel(0x2000000000000009), UnsignedMorsel(0x62)}
    ,{UnsignedMorsel(0x200000000000000c), UnsignedMorsel(0x98)}
    ,{UnsignedMorsel(0x000000012345678e), UnsignedMorsel(0x0f)}
    ,{UnsignedMorsel(0x000000012345678f), UnsignedMorsel(0xf7)}
    ,{UnsignedMorsel(0x000000012345a768), UnsignedMorsel(0xf0)}
    ,{UnsignedMorsel(0x000000012345a76b), UnsignedMorsel(0x0a)}
    ,{UnsignedMorsel(0x000000012345a790), UnsignedMorsel(0x81)}
    ,{UnsignedMorsel(0x000000012345a791), UnsignedMorsel(0x03)}
    ,{UnsignedMorsel(0x000000012345a792), UnsignedMorsel(0xFE)}
    ,{UnsignedMorsel(0x000000012345a793), UnsignedMorsel(0x01)}
    ,{UnsignedMorsel(0x000000012345a794), UnsignedMorsel(0x01)}
    ,{UnsignedMorsel(0x000000012345a795), UnsignedMorsel(0x00)}
    ,{UnsignedMorsel(0x000000012345a796), UnsignedMorsel(0xFF)}
    ,{UnsignedMorsel(0x000000012345a797), UnsignedMorsel(0xF5)}
  };
  bool flag=true;
  for (auto i: m) flag = flag && (mycpu.view(i.a)==i.v);
  tests[0].in = (flag && (mycpu.regs(254)==UnsignedMorsel(0x2000000000000008))
                      && (mycpu.regs(255)==UnsignedMorsel(0x000000012345678c).resize(64)));
  return runCases(tests);
}

void TestLoader::runAllTests()
{
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
      ,{TestLoader(), "TestFixo",  &TestLoader::TestFixo}
      ,{TestLoader(), "TestFixr",  &TestLoader::TestFixr}
      ,{TestLoader(), "TestFixrx", &TestLoader::TestFixrx}
      ,{TestLoader(), "TestSpec",  &TestLoader::TestSpec}
      ,{TestLoader(), "TestStab",  &TestLoader::TestStab}
      ,{TestLoader(), "TestEnd",   &TestLoader::TestEnd}
      ,{TestLoader(), "TestLoadObject", &TestLoader::TestLoadObject}
    };

    for (NameResultPair &t : tests)
    {
	    std::cout << "Test " 
        << ( (t.tm.*(t.funcPtr))() ? "passed" : "failed") 
        << " " 
        << t.funcName 
        << endl;
    }
  }

int main()
{
	TestLoader tm;
  tm.runAllTests();
	return 0;
}
