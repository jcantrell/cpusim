#include <iostream>
#include <string>
#include "../../../model/cpu/SignedMorsel.h"
class TestSignedMorsel : public SignedMorsel
{
	public:
  TestSignedMorsel() : SignedMorsel() {}
  TestSignedMorsel(dynamic_bitset<> in) : SignedMorsel(in) {}
  bool testCount()
  {
    dynamic_bitset<> tbs(64,0ul);
    return tbs.count() == count();
  }
  bool TestSignedMorselAddSignedMorsel()
  {
    bool result;
    SignedMorsel a(7);
    SignedMorsel b(982);
    SignedMorsel c(989);
    SignedMorsel d;
    d = a + b;
    result  = (c == d);

    SignedMorsel e(82);
    SignedMorsel f(83);
    SignedMorsel g(165);
    result = result && ( (e+f) == g );
    return result;
  }
  bool testSignedMorselAddInt()
  {
    bool result;
    SignedMorsel a(7);
    int b = 982;
    SignedMorsel c(989);
    SignedMorsel d;
    d = a+b;
    result = (c==d);
    return result;
  }
  bool testSignedMorselSubSignedMorsel()
  {
    bool result;
    SignedMorsel a(989);
    SignedMorsel b(7);
    SignedMorsel c(982);
    SignedMorsel d;
    d = a - b;
    result = (c==d);
    return result;
  }
  bool testIntSubSignedMorsel()
  {
    bool result;
    int a = 989;
    SignedMorsel b(7);
    SignedMorsel c(982);
    SignedMorsel d;
    d = a - b;
    result = (c==d);
    return result;
  }
  bool testSignedMorselDecSignedMorsel()
  {
    SignedMorsel a(42);
    SignedMorsel b(7);
    a -= b;
    SignedMorsel c(35);
    bool result = (a == c);
    return result;
  }
  bool testSignedMorselInc()
  {
    SignedMorsel a(94);
    a++;
    SignedMorsel b(95);
    bool result = (a==b);
    return result;
  }
  bool testSignedMorselAssignInt()
  {
    SignedMorsel a(95);
    SignedMorsel b;
    b = 95;
    return (a==b);
  }
  bool testSignedMorselAssignSignedMorsel()
  {
    SignedMorsel a(95);
    SignedMorsel b;
    b = a;
    return (a==b);
  }
  bool testSignedMorselAsString()
  {
    bool result = true;
    struct TestCase {
      SignedMorsel in;
      string out;
    };

    TestCase tests[] = {
       {SignedMorsel(95), "5f"}
      ,{SignedMorsel(0x2a), "2a"}
      ,{SignedMorsel(0xf000), "f000"}
      ,{SignedMorsel(0x28), "28"}
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
  bool testStreamSignedMorsel()
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
      SignedMorsel tSignedMorsel(t.in);
      out << tSignedMorsel;
      result = result && (out.str() == t.str);
    }

    return result;
  }
  bool testSignedMorselLessThanSignedMorsel()
  {
    SignedMorsel a(95);
    SignedMorsel b(100);
    return a < b;
  }
  bool testSignedMorselLessThanInt()
  {
    SignedMorsel a(95);
    int b = 100;
    return a < b;
  }
  bool testSignedMorselLessOrEqualSignedMorsel()
  {
    SignedMorsel a(95);
    SignedMorsel b(100);
    return a <= b;
  }
  bool testSignedMorselLessOrEqualInt()
  {
    SignedMorsel a(95);
    return a <= 100;
  }
  bool testIntLessOrEqualSignedMorsel()
  {
    SignedMorsel b(100);
    return 95 <= b;
  }
  bool testSignedMorselGreaterThanInt()
  {
    SignedMorsel a(95);
    return a > 90;
  }
  bool testIntGreaterThanSignedMorsel()
  {
    SignedMorsel a(95);
    bool result = (100 > a);
    return result;
  }
  bool testSignedMorselGreaterThanSignedMorsel()
  {
    SignedMorsel a(7);
    SignedMorsel b(21);
    return b > a;
  }
  bool testSignedMorselGreaterOrEqualSignedMorsel()
  {
    SignedMorsel a(95);
    SignedMorsel b(100);
    return b >= a;
  }
  bool testSignedMorselDivSignedMorsel()
  {
    bool result = true;
    struct TestCase {
      SignedMorsel in;
      SignedMorsel out;
    };

    TestCase tests[] = {
       {SignedMorsel(23)/SignedMorsel(7),SignedMorsel(3)}
      //,{SignedMorsel(27)/SignedMorsel(4),SignedMorsel(6)}
      //{SignedMorsel(21)/SignedMorsel(7),SignedMorsel(3)}
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
  bool testSignedMorselModSignedMorsel()
  {
    SignedMorsel a(25);
    SignedMorsel b(7);
    SignedMorsel c;
    SignedMorsel d(4);
    c = a % b;
    return (c==d);
  }
  bool testSignedMorselSize()
  {
    SignedMorsel a(8);
    return (a.size() == 4);
  }
  bool testSignedMorselEqualSignedMorsel() 
  {
    SignedMorsel a(27);
    SignedMorsel b(27);
    return (a==b);
  }
  bool testSignedMorselEqualInt()
  {
    SignedMorsel a(27);
    int b = 27;
    return (a==b);
  }
  bool testSignedMorselAsInt()
  {
    unsigned int t = 25;
    SignedMorsel a(t);
    unsigned int b;
    b = a.asInt();
    return (t==b);
  }
  bool testSignedMorselBitwiseAndSignedMorsel()
  {
    SignedMorsel a(0xFF00);
    SignedMorsel b(0x0F0F);
    SignedMorsel c;
    c = a & b;
    SignedMorsel d(0x0F00);
    return (c==d);
  }
  bool testSignedMorselBitwiseAndInt()
  {
    SignedMorsel a(0xFF00);
    int b = 0x0F0F;
    SignedMorsel c;
    c = a & b;
    SignedMorsel d(0x0F00);
    return (c==d);
  }
  bool testSignedMorselBitwiseOrSignedMorsel()
  {
    SignedMorsel a(0xFF00);
    SignedMorsel b(0x0F0F);
    SignedMorsel c;
    c = a | b;
    SignedMorsel d(0xFF0F);
    return (c==d);
  }
  bool testSignedMorselBitwiseOrSignedMorselAssign()
  {
    SignedMorsel a(0xFF00);
    SignedMorsel b(0x0F0F);
    a |= b;
    SignedMorsel d(0xFF0F);
    return (a==d);
  }
  bool testSignedMorselBitwiseInverse()
  {
    SignedMorsel a(0xFF00);
    SignedMorsel b;
    b = ~a;
    SignedMorsel c(0x00FF);
    return (c==b);
  }
  bool testSignedMorselBitwiseXorSignedMorsel()
  {
    SignedMorsel a(0xFF00);
    SignedMorsel b(0x0F0F);
    SignedMorsel c;
    c = a ^ b;
    SignedMorsel d(0xF00F);
    return (c==d);
  }
  bool testSignedMorselLeftshiftSignedMorsel()
  {
    bool result = true;
    struct TestCase {
      SignedMorsel in;
      string out;
    };

    TestCase tests[] = {
       {SignedMorsel(0xa5) << 3, "28"}
      ,{SignedMorsel(0xef) << 4, "f0"}
      ,{SignedMorsel(0xFF00) << 4, "f000"}
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
  bool testSignedMorselLeftShiftInt()
  {
    SignedMorsel a(0xFF00);
    int b = (4);
    SignedMorsel c;
    c = a << b;
    SignedMorsel d(0xF000);
    return (c==d);
  }
  bool testSignedMorselRightShiftInt()
  {
    SignedMorsel a(0xFF00);
    int b = (4);
    SignedMorsel c;
    c = a >> b;
    SignedMorsel d(0x0FF0);
    return (c==d);
  }
  bool testSignedMorselRightShiftSignedMorsel()
  {
    SignedMorsel a(0xFF00);
    SignedMorsel b(4);
    SignedMorsel c;
    c = a >> b;
    SignedMorsel d(0x0FF0);
    return (c==d);
  }
  bool testSignedMorselMultiplySignedMorsel()
  {
    SignedMorsel a(3);
    SignedMorsel b(7);
    SignedMorsel c(21);
    SignedMorsel d;
    d = a * b;
    return (c==d);
  }
  bool testSignedMorselIntMultiplySignedMorsel()
  {
    int a = 3;
    SignedMorsel b(7);
    SignedMorsel c(21);
    SignedMorsel d;
    d = a*b;
    return (c==d);
  }
  bool testSignedMorselLeftShiftAssignInt()
  {
    SignedMorsel a(0xFF0F);
    unsigned int b = 4;
    SignedMorsel c(0xF0F0);
    a <<= b;
    return (c==a);
  }
  bool testSignedMorselNESignedMorsel()
  {
    SignedMorsel a(27);
    SignedMorsel b(31);
    return a != b;
  }
  bool testSignedMorselNEInt()
  {
    SignedMorsel a(27);
    unsigned int b = 31;
    return a != b;
  }
  bool testSignedMorselAsFloat()
  {
    SignedMorsel a(0x4048f5c3);
    float pi = 3.14;
    return (a.asFloat()==pi);
  }
  bool testSignedMorselAsChar()
  {
    bool result = true;
    struct TestCase {
      SignedMorsel in;
      unsigned char out;
    };

    TestCase tests[] = {
       {SignedMorsel(0xDEAD), static_cast<unsigned char>(0xAD)}
      ,{SignedMorsel(0x2a), static_cast<unsigned char>(0x2a)}
      ,{SignedMorsel(0xf000), static_cast<unsigned char>(0xf000)}
      ,{SignedMorsel(0x28), static_cast<unsigned char>(0x28)}
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
    TestSignedMorsel tm;
    struct NameResultPair {
      bool runTest;
      string funcName;
      bool (TestSignedMorsel::*funcPtr)();
    };
    NameResultPair tests[] = {
       {true, "testCount", &TestSignedMorsel::testCount}
      ,{true, "TestSignedMorselAddSignedMorsel", &TestSignedMorsel::TestSignedMorselAddSignedMorsel}
      ,{true, "testSignedMorselAddInt", &TestSignedMorsel::testSignedMorselAddInt}
      ,{true, "testSignedMorselSubSignedMorsel", &TestSignedMorsel::testSignedMorselSubSignedMorsel}
      ,{true, "testIntSubSignedMorsel", &TestSignedMorsel::testIntSubSignedMorsel}
      ,{true, "testSignedMorselDecSignedMorsel", &TestSignedMorsel::testSignedMorselDecSignedMorsel}
      ,{true, "testSignedMorselInc", &TestSignedMorsel::testSignedMorselInc}
      ,{true, "testSignedMorselAssignInt", &TestSignedMorsel::testSignedMorselAssignInt}
      ,{true, "testSignedMorselAsString", &TestSignedMorsel::testSignedMorselAsString}
      ,{true, "testStreamSignedMorsel", &TestSignedMorsel::testStreamSignedMorsel}
      ,{true, "testSignedMorselLessThanSignedMorsel", &TestSignedMorsel::testSignedMorselLessThanSignedMorsel}
      ,{true, "testSignedMorselLessThanInt", &TestSignedMorsel::testSignedMorselLessThanInt}
      ,{true, "testSignedMorselLessOrEqualSignedMorsel", &TestSignedMorsel::testSignedMorselLessOrEqualSignedMorsel}
      ,{true, "testSignedMorselLessOrEqualInt", &TestSignedMorsel::testSignedMorselLessOrEqualInt}
      ,{true, "testIntLessOrEqualSignedMorsel", &TestSignedMorsel::testIntLessOrEqualSignedMorsel}
      ,{true, "testSignedMorselGreaterThanInt", &TestSignedMorsel::testSignedMorselGreaterThanInt}
      ,{true, "testIntGreaterThanSignedMorsel", &TestSignedMorsel::testIntGreaterThanSignedMorsel}
      ,{true, "testSignedMorselGreaterThanSignedMorsel", &TestSignedMorsel::testSignedMorselGreaterThanSignedMorsel}
      ,{true, "testSignedMorselGreaterOrEqualSignedMorsel", &TestSignedMorsel::testSignedMorselGreaterOrEqualSignedMorsel}
      ,{true, "testSignedMorselDivSignedMorsel", &TestSignedMorsel::testSignedMorselDivSignedMorsel}
      ,{true, "testSignedMorselModSignedMorsel", &TestSignedMorsel::testSignedMorselModSignedMorsel}
      ,{true, "testSignedMorselSize", &TestSignedMorsel::testSignedMorselSize}
      ,{true, "testSignedMorselEqualSignedMorsel", &TestSignedMorsel::testSignedMorselEqualSignedMorsel}
      ,{true, "testSignedMorselEqualInt", &TestSignedMorsel::testSignedMorselEqualInt}
      ,{true, "testSignedMorselAsInt", &TestSignedMorsel::testSignedMorselAsInt}
      ,{true, "testSignedMorselBitwiseAndSignedMorsel", &TestSignedMorsel::testSignedMorselBitwiseAndSignedMorsel}
      ,{true, "testSignedMorselBitwiseAndInt", &TestSignedMorsel::testSignedMorselBitwiseAndInt}
      ,{true, "testSignedMorselBitwiseOrSignedMorsel", &TestSignedMorsel::testSignedMorselBitwiseOrSignedMorsel}
      ,{true, "testSignedMorselBitwiseOrSignedMorselAssign", &TestSignedMorsel::testSignedMorselBitwiseOrSignedMorselAssign}
      ,{true, "testSignedMorselBitwiseInverse", &TestSignedMorsel::testSignedMorselBitwiseInverse}
      ,{true, "testSignedMorselBitwiseXorSignedMorsel", &TestSignedMorsel::testSignedMorselBitwiseXorSignedMorsel}
      ,{true, "testSignedMorselLeftshiftSignedMorsel", &TestSignedMorsel::testSignedMorselLeftshiftSignedMorsel}
      ,{true, "testSignedMorselLeftShiftInt", &TestSignedMorsel::testSignedMorselLeftShiftInt}
      ,{true, "testSignedMorselRightShiftInt", &TestSignedMorsel::testSignedMorselRightShiftInt}
      ,{true, "testSignedMorselRightShiftSignedMorsel", &TestSignedMorsel::testSignedMorselRightShiftSignedMorsel}
      ,{true, "testSignedMorselMultiplySignedMorsel", &TestSignedMorsel::testSignedMorselMultiplySignedMorsel}
      ,{true, "testSignedMorselIntMultiplySignedMorsel", &TestSignedMorsel::testSignedMorselIntMultiplySignedMorsel}
      ,{true, "testSignedMorselLeftShiftAssignInt", &TestSignedMorsel::testSignedMorselLeftShiftAssignInt}
      ,{true, "testSignedMorselNESignedMorsel", &TestSignedMorsel::testSignedMorselNESignedMorsel}
      ,{true, "testSignedMorselNEInt", &TestSignedMorsel::testSignedMorselNEInt}
      ,{true, "testSignedMorselAsFloat", &TestSignedMorsel::testSignedMorselAsFloat}
      ,{true, "testSignedMorselAsChar", &TestSignedMorsel::testSignedMorselAsChar}
    };

    for (NameResultPair &t : tests)
    {
      if (t.runTest)
	      std::cout << "Test " << t.funcName << " " 
          << ( (tm.*(t.funcPtr))() ? "passed" : "failed") << endl;
    }
  }
};

int main()
{
	TestSignedMorsel tm;
  tm.runAllTests();
	return 0;
}
