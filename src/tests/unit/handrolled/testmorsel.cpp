#include <iostream>
#include <string>
#include "../../../model/cpu/morsel.h"
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
    bool result = true;
    struct TestCase {
      Morsel in;
      string out;
    };

    TestCase tests[] = {
       {Morsel(95), "5f"}
      ,{Morsel(0x2a), "2a"}
      ,{Morsel(0xf000), "f000"}
      ,{Morsel(0x28), "28"}
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
  bool testStreamMorsel()
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
      Morsel tMorsel(t.in);
      out << tMorsel;
      result = result && (out.str() == t.str);
    }

    return result;
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
    return a < static_cast<int>(b);
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
    bool result = true;
    struct TestCase {
      Morsel in;
      Morsel out;
    };

    TestCase tests[] = {
       {Morsel(23)/Morsel(7),Morsel(3)}
      //,{Morsel(27)/Morsel(4),Morsel(6)}
      //{Morsel(21)/Morsel(7),Morsel(3)}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in == t.out);
      if (!(t.in == t.out))
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in << endl;
      }
    }
    return result;
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
    c = a & static_cast<int>(b);
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
    bool result = true;
    struct TestCase {
      Morsel in;
      string out;
    };

    TestCase tests[] = {
       {Morsel(0xa5) << 3, "28"}
      ,{Morsel(0xef) << 4, "f0"}
      ,{Morsel(0xFF00) << 4, "f000"}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in.asString() == t.out);
      if (t.in.asString() != t.out)
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in.asString() << endl;
      }
    }
    return result;
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
    bool result = true;
    struct TestCase {
      Morsel in;
      unsigned char out;
    };

    TestCase tests[] = {
       {Morsel(0xDEAD), static_cast<unsigned char>(0xAD)}
      ,{Morsel(0x2a), static_cast<unsigned char>(0x2a)}
      ,{Morsel(0xf000), static_cast<unsigned char>(0xf000)}
      ,{Morsel(0x28), static_cast<unsigned char>(0x28)}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in.asChar() == t.out);
      if (t.in.asChar() != t.out)
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in.asString() << endl;
      }
    }
    return result;
  }

  void runAllTests()
  {
    TestMorsel tm;
    struct NameResultPair {
      bool runTest;
      string funcName;
      bool (TestMorsel::*funcPtr)();
    };
    NameResultPair tests[] = {
       {true, "testCount", &TestMorsel::testCount}
      ,{true, "TestMorselAddMorsel", &TestMorsel::TestMorselAddMorsel}
      ,{true, "testMorselAddInt", &TestMorsel::testMorselAddInt}
      ,{true, "testMorselSubMorsel", &TestMorsel::testMorselSubMorsel}
      ,{true, "testIntSubMorsel", &TestMorsel::testIntSubMorsel}
      ,{true, "testMorselDecMorsel", &TestMorsel::testMorselDecMorsel}
      ,{true, "testMorselInc", &TestMorsel::testMorselInc}
      ,{true, "testMorselAssignInt", &TestMorsel::testMorselAssignInt}
      ,{true, "testMorselAsString", &TestMorsel::testMorselAsString}
      ,{true, "testStreamMorsel", &TestMorsel::testStreamMorsel}
      ,{true, "testMorselLessThanMorsel", &TestMorsel::testMorselLessThanMorsel}
      ,{true, "testMorselLessThanInt", &TestMorsel::testMorselLessThanInt}
      ,{true, "testMorselLessOrEqualMorsel", &TestMorsel::testMorselLessOrEqualMorsel}
      ,{true, "testMorselLessOrEqualInt", &TestMorsel::testMorselLessOrEqualInt}
      ,{true, "testIntLessOrEqualMorsel", &TestMorsel::testIntLessOrEqualMorsel}
      ,{true, "testMorselGreaterThanInt", &TestMorsel::testMorselGreaterThanInt}
      ,{true, "testIntGreaterThanMorsel", &TestMorsel::testIntGreaterThanMorsel}
      ,{true, "testMorselGreaterThanMorsel", &TestMorsel::testMorselGreaterThanMorsel}
      ,{true, "testMorselGreaterOrEqualMorsel", &TestMorsel::testMorselGreaterOrEqualMorsel}
      ,{true, "testMorselDivMorsel", &TestMorsel::testMorselDivMorsel}
      ,{true, "testMorselModMorsel", &TestMorsel::testMorselModMorsel}
      ,{true, "testMorselSize", &TestMorsel::testMorselSize}
      ,{true, "testMorselEqualMorsel", &TestMorsel::testMorselEqualMorsel}
      ,{true, "testMorselEqualInt", &TestMorsel::testMorselEqualInt}
      ,{true, "testMorselAsInt", &TestMorsel::testMorselAsInt}
      ,{true, "testMorselBitwiseAndMorsel", &TestMorsel::testMorselBitwiseAndMorsel}
      ,{true, "testMorselBitwiseAndInt", &TestMorsel::testMorselBitwiseAndInt}
      ,{true, "testMorselBitwiseOrMorsel", &TestMorsel::testMorselBitwiseOrMorsel}
      ,{true, "testMorselBitwiseOrMorselAssign", &TestMorsel::testMorselBitwiseOrMorselAssign}
      ,{true, "testMorselBitwiseInverse", &TestMorsel::testMorselBitwiseInverse}
      ,{true, "testMorselBitwiseXorMorsel", &TestMorsel::testMorselBitwiseXorMorsel}
      ,{true, "testMorselLeftshiftMorsel", &TestMorsel::testMorselLeftshiftMorsel}
      ,{true, "testMorselLeftShiftInt", &TestMorsel::testMorselLeftShiftInt}
      ,{true, "testMorselRightShiftInt", &TestMorsel::testMorselRightShiftInt}
      ,{true, "testMorselRightShiftMorsel", &TestMorsel::testMorselRightShiftMorsel}
      ,{true, "testMorselMultiplyMorsel", &TestMorsel::testMorselMultiplyMorsel}
      ,{true, "testMorselIntMultiplyMorsel", &TestMorsel::testMorselIntMultiplyMorsel}
      ,{true, "testMorselLeftShiftAssignInt", &TestMorsel::testMorselLeftShiftAssignInt}
      ,{true, "testMorselNEMorsel", &TestMorsel::testMorselNEMorsel}
      ,{true, "testMorselNEInt", &TestMorsel::testMorselNEInt}
      ,{true, "testMorselAsFloat", &TestMorsel::testMorselAsFloat}
      ,{true, "testMorselAsChar", &TestMorsel::testMorselAsChar}
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
	TestMorsel tm;
  tm.runAllTests();
	return 0;
}
