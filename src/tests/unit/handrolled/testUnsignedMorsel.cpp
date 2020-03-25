#include <iostream>
#include <string>
#include "../../../model/cpu/UnsignedMorsel.h"
class TestUnsignedMorsel : public UnsignedMorsel
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
  TestUnsignedMorsel() : UnsignedMorsel() {}
  TestUnsignedMorsel(dynamic_bitset<> in) : UnsignedMorsel(in) {}
  bool testCount()
  {
    dynamic_bitset<> tbs(64,0ul);
    return tbs.count() == count();
  }
  bool TestUnsignedMorselAddUnsignedMorsel()
  {
    bool result;
    UnsignedMorsel a(7);
    UnsignedMorsel b(982);
    UnsignedMorsel c(989);
    UnsignedMorsel d;
    d = a + b;
    result  = (c == d);

    UnsignedMorsel e(82);
    UnsignedMorsel f(83);
    UnsignedMorsel g(165);
    result = result && ( (e+f) == g );
    return result;
  }
  bool testUnsignedMorselAddInt()
  {
    bool result;
    UnsignedMorsel a(7);
    unsigned int b = 982;
    UnsignedMorsel c(989);
    UnsignedMorsel d;
    d = a+b;
    result = (c==d);
    return result;
  }
  bool testUnsignedMorselSubUnsignedMorsel()
  {
    bool result;
    UnsignedMorsel a(989);
    UnsignedMorsel b(7);
    UnsignedMorsel c(982);
    UnsignedMorsel d;
    d = a - b;
    result = (c==d);
    return result;
  }
  bool testIntSubUnsignedMorsel()
  {
    bool result;
    unsigned int a = 989;
    UnsignedMorsel b(7);
    UnsignedMorsel c(982);
    UnsignedMorsel d;
    d = a - b;
    result = (c==d);
    return result;
  }
  bool testUnsignedMorselDecUnsignedMorsel()
  {
    UnsignedMorsel a(42);
    UnsignedMorsel b(7);
    a -= b;
    UnsignedMorsel c(35);
    bool result = (a == c);
    return result;
  }
  bool testUnsignedMorselInc()
  {
    UnsignedMorsel a(94);
    a++;
    UnsignedMorsel b(95);
    bool result = (a==b);
    return result;
  }
  bool testUnsignedMorselAssignInt()
  {
    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };
    UnsignedMorsel a(0xFFFFFFFFFFFFFFFF);
    a = 2;
   // TestCase tests[] = {
    vector<TestCase> tests = {
       {UnsignedMorsel(0),UnsignedMorsel(95)}
      ,{UnsignedMorsel(0),UnsignedMorsel(23)}
      ,{UnsignedMorsel(0),UnsignedMorsel(7)}
      ,{UnsignedMorsel(0),UnsignedMorsel(3)}
      ,{a,UnsignedMorsel(0x0000000000000002)}
      //,{UnsignedMorsel(27)/UnsignedMorsel(4),UnsignedMorsel(6)}
      //{UnsignedMorsel(21)/UnsignedMorsel(7),UnsignedMorsel(3)}
    };
    tests[0].in = 95;
    tests[1].in = 23;
    tests[2].in = 7;
    tests[3].in = 3;
    //tests[4].in = 95;

/*
    for (TestCase &t : tests)
    {
      result = result && (t.in == t.out);
      if (!(t.in == t.out))
      {
        cout << "Failed" << endl 
             << "Expected: " << t.out 
             << " Actual: " << t.in << endl;
      }
    }
    return result;
  vector<TestCase> tests = {
*/
    return runCases(tests);
  }
  bool testUnsignedMorselAssignUnsignedMorsel()
  {
    UnsignedMorsel a(95);
    UnsignedMorsel b;
    b = a;
    return (a==b);
  }
  bool testUnsignedMorselAsString()
  {
    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      string out;
    };

    TestCase tests[] = {
       {UnsignedMorsel(95), "5f"}
      ,{UnsignedMorsel(0x2a), "2a"}
      ,{UnsignedMorsel(0xf000), "f000"}
      ,{UnsignedMorsel(0x28), "28"}
      ,{UnsignedMorsel(23),"17"}
    };

  //cout << "begin asString testloop" << endl;
    for (TestCase &t : tests)
    {
      result = result && (t.in.asString() == t.out);
      if (t.in.asString() != t.out)
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in.asString() << endl;
      }
    }
  //cout << "end asString testloop" << endl;
    return result;
  }
  bool testStreamUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      unsigned int in;
      string str;
    };

    TestCase tests[] = {
       {95, "5f"}
      ,{42, "2a"}
    };

    for (TestCase &t : tests)
    {
      std::stringstream out;
      UnsignedMorsel tUnsignedMorsel(t.in);
      out << tUnsignedMorsel;
      result = result && (out.str() == t.str);
    }

    return result;
  }
  bool testUnsignedMorselLessThanUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       //{UnsignedMorsel(23)/UnsignedMorsel(7),UnsignedMorsel(3)}
       {UnsignedMorsel(4)<UnsignedMorsel(5),true}
      ,{UnsignedMorsel(5)<UnsignedMorsel(4),false}
      ,{UnsignedMorsel(1)<UnsignedMorsel(4),true}
      ,{UnsignedMorsel(5)<UnsignedMorsel(7),true}
      ,{UnsignedMorsel(27)<UnsignedMorsel(4),false}
      ,{UnsignedMorsel(21)<UnsignedMorsel(7),false}
      ,{UnsignedMorsel(95)<UnsignedMorsel(100),true}
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
  bool testUnsignedMorselLessThanInt()
  {
    UnsignedMorsel a(95);
    unsigned b = 100;
    return a < b;
  }
  bool testUnsignedMorselLessOrEqualUnsignedMorsel()
  {
    UnsignedMorsel a(95);
    UnsignedMorsel b(100);
    return a <= b;
  }
  bool testUnsignedMorselLessOrEqualInt()
  {
    UnsignedMorsel a(95);
    return a <= 100;
  }
  bool testIntLessOrEqualUnsignedMorsel()
  {
    UnsignedMorsel b(100);
    return 95 <= b;
  }
  bool testUnsignedMorselGreaterThanInt()
  {
    UnsignedMorsel a(95);
    return a > 90;
  }
  bool testIntGreaterThanUnsignedMorsel()
  {
    UnsignedMorsel a(95);
    bool result = (100 > a);
    return result;
  }
  bool testUnsignedMorselGreaterThanUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       //{UnsignedMorsel(23)/UnsignedMorsel(7),UnsignedMorsel(3)}
       //{UnsignedMorsel(1)>UnsignedMorsel(4),false}
      {UnsignedMorsel(4)>UnsignedMorsel(5),false}
/*
      ,{UnsignedMorsel(5)>UnsignedMorsel(7),false}
      ,{UnsignedMorsel(27)>UnsignedMorsel(4),true}
      ,{UnsignedMorsel(21)>UnsignedMorsel(7),true}
      ,{UnsignedMorsel(95)>UnsignedMorsel(100),false}
*/
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
  bool testUnsignedMorselGreaterOrEqualUnsignedMorsel()
  {
    UnsignedMorsel a(95);
    UnsignedMorsel b(100);
    return b >= a;
  }
  bool testUnsignedMorselDivUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    TestCase tests[] = {
       {UnsignedMorsel(5)/UnsignedMorsel(4),UnsignedMorsel(1)}
       //{UnsignedMorsel(23)/UnsignedMorsel(7),UnsignedMorsel(3)}
      //{UnsignedMorsel(1)/UnsignedMorsel(4),UnsignedMorsel(0)}
      //,{UnsignedMorsel(5)/UnsignedMorsel(7),UnsignedMorsel(0)}
      //,{UnsignedMorsel(27)/UnsignedMorsel(4),UnsignedMorsel(6)}
      //,{UnsignedMorsel(21)/UnsignedMorsel(7),UnsignedMorsel(3)}
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
  bool testUnsignedMorselModUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    TestCase tests[] = {
      {UnsignedMorsel(25)%UnsignedMorsel(7),UnsignedMorsel(4)}
      ,{UnsignedMorsel(5)%UnsignedMorsel(7),UnsignedMorsel(5)}
      ,{UnsignedMorsel(27)%UnsignedMorsel(4),UnsignedMorsel(3)}
      ,{UnsignedMorsel(21)%UnsignedMorsel(7),UnsignedMorsel(0)}
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
  bool testUnsignedMorselSize()
  {
    UnsignedMorsel a(8);
    return (a.size() == 4);
  }
  bool testUnsignedMorselEqualUnsignedMorsel() 
  {
    UnsignedMorsel a(27);
    UnsignedMorsel b(27);
    return (a==b);
  }
  bool testUnsignedMorselEqualInt()
  {
    UnsignedMorsel a(27);
    unsigned int b = 27;
    return (a==b);
  }
  bool testUnsignedMorselAsInt()
  {
    unsigned int t = 25;
    UnsignedMorsel a(t);
    unsigned int b;
    b = a.asInt();
    return (t==b);
  }
  bool testUnsignedMorselBitwiseAndUnsignedMorsel()
  {
    UnsignedMorsel a(0xFF00);
    UnsignedMorsel b(0x0F0F);
    UnsignedMorsel c;
    c = a & b;
    UnsignedMorsel d(0x0F00);
    return (c==d);
  }
  bool testUnsignedMorselBitwiseAndInt()
  {
    UnsignedMorsel a(0xFF00);
    unsigned b = 0x0F0F;
    UnsignedMorsel c;
    c = a & b;
    UnsignedMorsel d(0x0F00);
    return (c==d);
  }
  bool testUnsignedMorselBitwiseOrUnsignedMorsel()
  {
    UnsignedMorsel a(0xFF00);
    UnsignedMorsel b(0x0F0F);
    UnsignedMorsel c;
    c = a | b;
    UnsignedMorsel d(0xFF0F);
    return (c==d);
  }
  bool testUnsignedMorselBitwiseOrUnsignedMorselAssign()
  {
    UnsignedMorsel a(0xFF00);
    UnsignedMorsel b(0x0F0F);
    a |= b;
    UnsignedMorsel d(0xFF0F);
    return (a==d);
  }
  bool testUnsignedMorselBitwiseInverse()
  {
    UnsignedMorsel a(0xFF00);
    UnsignedMorsel b;
    b = ~a;
    UnsignedMorsel c(0x00FF);
    return (c==b);
  }
  bool testUnsignedMorselBitwiseXorUnsignedMorsel()
  {
    UnsignedMorsel a(0xFF00);
    UnsignedMorsel b(0x0F0F);
    UnsignedMorsel c;
    c = a ^ b;
    UnsignedMorsel d(0xF00F);
    return (c==d);
  }
  bool testUnsignedMorselLeftshiftUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      //UnsignedMorsel in;
      string in;
      string out;
    };

    vector<TestCase> tests = {
       {(UnsignedMorsel(0xa5) << 3).asString(), "28"}
      ,{(UnsignedMorsel(0xef) << 4).asString(), "f0"}
      ,{(UnsignedMorsel(0xFF00) << 4).asString(), "f000"}
      ,{UnsignedMorsel(0x0000000000000002).asString(), "0000000000000200"}
    };
    UnsignedMorsel t(0x0000000000000002);
    t.resize(64);
    t = t << 8;
    tests[3].in = t.asString();

    return runCases(tests);
  }
  bool testUnsignedMorselLeftShiftInt()
  {
  struct  TestCase {
    UnsignedMorsel in;
    UnsignedMorsel out;
  };

  UnsignedMorsel t;
  t = 2;
  t.resize(56);
  t = (t<<8);
  //t<<=8;
  vector<TestCase> tests = {
     {UnsignedMorsel(0xFF00)<<4, UnsignedMorsel(0xF000)}
    ,{UnsignedMorsel(2), UnsignedMorsel(0x0200)}
    ,{t, UnsignedMorsel(0x0000000000000200)}
  };

  tests[1].in.resize(64);
  tests[1].in = tests[1].in<<8;

  return runCases(tests);
  }
  bool testUnsignedMorselRightShiftInt()
  {
    UnsignedMorsel a(0xFF00);
    unsigned int b = (4);
    UnsignedMorsel c;
    c = a >> b;
    UnsignedMorsel d(0x0FF0);
    return (c==d);
  }
  bool testUnsignedMorselRightShiftUnsignedMorsel()
  {
    UnsignedMorsel a(0xFF00);
    UnsignedMorsel b(4);
    UnsignedMorsel c;
    c = a >> b;
    UnsignedMorsel d(0x0FF0);
    return (c==d);
  }
  bool testUnsignedMorselMultiplyUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    vector<TestCase> tests = {
        {UnsignedMorsel(2)*UnsignedMorsel(2),UnsignedMorsel(4)}
       ,{UnsignedMorsel(3)*UnsignedMorsel(7),UnsignedMorsel(21)}
       ,{UnsignedMorsel(6)*UnsignedMorsel(12),UnsignedMorsel(72)}
    };

    return runCases(tests);
  }
  bool testUnsignedMorselIntMultiplyUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      UnsignedMorsel out;
    };

    vector<TestCase> tests = {
        {2u*UnsignedMorsel(2),UnsignedMorsel(4)}
       ,{3u*UnsignedMorsel(7),UnsignedMorsel(21)}
       ,{6u*UnsignedMorsel(12),UnsignedMorsel(72)}
    };

    return runCases(tests);
  }
  bool testUnsignedMorselLeftShiftAssignInt()
  {
    UnsignedMorsel a(0xFF0F);
    unsigned int b = 4;
    UnsignedMorsel c(0xF0F0);
    a <<= b;
    return (c==a);
  }
  bool testUnsignedMorselNEUnsignedMorsel()
  {
    UnsignedMorsel a(27);
    UnsignedMorsel b(31);
    return a != b;
  }
  bool testUnsignedMorselNEInt()
  {
    UnsignedMorsel a(27);
    unsigned int b = 31;
    return a != b;
  }
  bool testUnsignedMorselAsFloat()
  {
    UnsignedMorsel a(0x4048f5c3);
    float pi = 3.14;
    return (a.asFloat()==pi);
  }
  bool testUnsignedMorselAsChar()
  {
    bool result = true;
    struct TestCase {
      UnsignedMorsel in;
      unsigned char out;
    };

    TestCase tests[] = {
       {UnsignedMorsel(0xDEAD), static_cast<unsigned char>(0xAD)}
      ,{UnsignedMorsel(0x2a),   static_cast<unsigned char>(0x2a)}
      ,{UnsignedMorsel(0xf000), static_cast<unsigned char>(0xf000)}
      ,{UnsignedMorsel(0x28),   static_cast<unsigned char>(0x28)}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in.asChar() == t.out);
      if (t.in.asChar() != t.out)
      {
        cout << "Failed" << endl 
             << "Expected: " << t.out 
             << " Actual: " << t.in.asString() << endl;
      }
    }
    return result;
  }

  void runAllTests()
  {
    TestUnsignedMorsel tm;
    struct NameResultPair {
      bool runTest;
      string funcName;
      bool (TestUnsignedMorsel::*funcPtr)();
    };
    NameResultPair tests[] = {
       {true, "testCount", &TestUnsignedMorsel::testCount}
      ,{true, "TestUnsignedMorselAddUnsignedMorsel", &TestUnsignedMorsel::TestUnsignedMorselAddUnsignedMorsel}
      ,{true, "testUnsignedMorselAddInt", &TestUnsignedMorsel::testUnsignedMorselAddInt}
      ,{true, "testUnsignedMorselSubUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselSubUnsignedMorsel}
      ,{true, "testIntSubUnsignedMorsel", &TestUnsignedMorsel::testIntSubUnsignedMorsel}
      ,{true, "testUnsignedMorselDecUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselDecUnsignedMorsel}
      ,{true, "testUnsignedMorselInc", &TestUnsignedMorsel::testUnsignedMorselInc}
      ,{true, "testUnsignedMorselAssignInt", &TestUnsignedMorsel::testUnsignedMorselAssignInt}
      ,{true, "testUnsignedMorselAsString", &TestUnsignedMorsel::testUnsignedMorselAsString}
      ,{true, "testStreamUnsignedMorsel", &TestUnsignedMorsel::testStreamUnsignedMorsel}
      ,{true, "testUnsignedMorselLessThanUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselLessThanUnsignedMorsel}
      ,{true, "testUnsignedMorselLessThanInt", &TestUnsignedMorsel::testUnsignedMorselLessThanInt}
      ,{true, "testUnsignedMorselLessOrEqualUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselLessOrEqualUnsignedMorsel}
      ,{true, "testUnsignedMorselLessOrEqualInt", &TestUnsignedMorsel::testUnsignedMorselLessOrEqualInt}
      ,{true, "testIntLessOrEqualUnsignedMorsel", &TestUnsignedMorsel::testIntLessOrEqualUnsignedMorsel}
      ,{true, "testUnsignedMorselGreaterThanInt", &TestUnsignedMorsel::testUnsignedMorselGreaterThanInt}
      ,{true, "testIntGreaterThanUnsignedMorsel", &TestUnsignedMorsel::testIntGreaterThanUnsignedMorsel}
      ,{true, "testUnsignedMorselGreaterThanUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselGreaterThanUnsignedMorsel}
      ,{true, "testUnsignedMorselGreaterOrEqualUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselGreaterOrEqualUnsignedMorsel}
      ,{true, "testUnsignedMorselDivUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselDivUnsignedMorsel}
      ,{true, "testUnsignedMorselModUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselModUnsignedMorsel}
      ,{true, "testUnsignedMorselSize", &TestUnsignedMorsel::testUnsignedMorselSize}
      ,{true, "testUnsignedMorselEqualUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselEqualUnsignedMorsel}
      ,{true, "testUnsignedMorselEqualInt", &TestUnsignedMorsel::testUnsignedMorselEqualInt}
      ,{true, "testUnsignedMorselAsInt", &TestUnsignedMorsel::testUnsignedMorselAsInt}
      ,{true, "testUnsignedMorselBitwiseAndUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselBitwiseAndUnsignedMorsel}
      ,{true, "testUnsignedMorselBitwiseAndInt", &TestUnsignedMorsel::testUnsignedMorselBitwiseAndInt}
      ,{true, "testUnsignedMorselBitwiseOrUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselBitwiseOrUnsignedMorsel}
      ,{true, "testUnsignedMorselBitwiseOrUnsignedMorselAssign", &TestUnsignedMorsel::testUnsignedMorselBitwiseOrUnsignedMorselAssign}
      ,{true, "testUnsignedMorselBitwiseInverse", &TestUnsignedMorsel::testUnsignedMorselBitwiseInverse}
      ,{true, "testUnsignedMorselBitwiseXorUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselBitwiseXorUnsignedMorsel}
      ,{true, "testUnsignedMorselLeftshiftUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselLeftshiftUnsignedMorsel}
      ,{true, "testUnsignedMorselLeftShiftInt", &TestUnsignedMorsel::testUnsignedMorselLeftShiftInt}
      ,{true, "testUnsignedMorselRightShiftInt", &TestUnsignedMorsel::testUnsignedMorselRightShiftInt}
      ,{true, "testUnsignedMorselRightShiftUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselRightShiftUnsignedMorsel}
      ,{true, "testUnsignedMorselMultiplyUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselMultiplyUnsignedMorsel}
      ,{true, "testUnsignedMorselIntMultiplyUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselIntMultiplyUnsignedMorsel}
      ,{true, "testUnsignedMorselLeftShiftAssignInt", &TestUnsignedMorsel::testUnsignedMorselLeftShiftAssignInt}
      ,{true, "testUnsignedMorselNEUnsignedMorsel", &TestUnsignedMorsel::testUnsignedMorselNEUnsignedMorsel}
      ,{true, "testUnsignedMorselNEInt", &TestUnsignedMorsel::testUnsignedMorselNEInt}
      ,{true, "testUnsignedMorselAsFloat", &TestUnsignedMorsel::testUnsignedMorselAsFloat}
      ,{true, "testUnsignedMorselAsChar", &TestUnsignedMorsel::testUnsignedMorselAsChar}
    };

    for (NameResultPair &t : tests)
    {
      if (t.runTest)
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
	TestUnsignedMorsel tm;
  tm.runAllTests();
	return 0;
}
