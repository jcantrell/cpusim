#include <iostream>
#include <string>
#include "../../../model/cpu/Address.h"
class TestAddress : public Address
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
  TestAddress() : Address() {}
  TestAddress(dynamic_bitset<> in) : Address(SignedMorsel(in)) {}
  bool assignInt()
  { 
    struct TestCase {
      string in;
      string out;
    };

    Address t;
    vector<TestCase> tests = {
       {(t = 95).asString(), "5f"}
      ,{(t = 42).asString(), "2a"}
    };

    return runCases(tests);
  }
  bool asInt()
  {
    struct TestCase {
      int in;
      int out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(95)).asInt(), 0x5f}
      ,{Address(UnsignedMorsel(42)).asInt(), 0x2a}
    };

    return runCases(tests);
  }
  bool asString()
  {
    struct TestCase {
      string in;
      string out;
    };

    vector<TestCase> tests = {
      {Address(UnsignedMorsel(95)).asString(), "5f"}
    };
    return runCases(tests);
  }
/*
  bool ostreamAddress()
  {
    struct TestCase {
      string in;
      string out;
    };

    std::ostream z;
    z << Address(UnsignedMorsel(95));
    std::ostream s2;
    s2 << Address(UnsignedMorsel(42));

    std::stringstream s3; s3 << s1.rdbuf();
    std::stringstream s4; s4 << s1.rdbuf();

    vector<TestCase> tests = {
        {s3.str(), "5f"}
       ,{s4.str(), "2a"}
    };

    return runCases(tests);
  }
*/
  bool streamAddress()
  {
    struct TestCase {
      string in;
      string out;
    };

    std::stringstream s1;
    s1 << Address(UnsignedMorsel(95));
    std::stringstream s2;
    s2 << Address(UnsignedMorsel(42));

    vector<TestCase> tests = {
        {s1.str(), "5f"}
       ,{s2.str(), "2a"}
    };

    return runCases(tests);
  }
  bool AddressMultiplyAddress()
  {
    bool result = true;
    struct TestCase {
      string in;
      string out;
    };

    cout << "CALLING ADDRESS MULT" << endl;
    vector<TestCase> tests = {
       {(Address(UnsignedMorsel(7)) * Address(UnsignedMorsel(3))).asString(), "15"}
      ,{(Address(UnsignedMorsel(4)) * Address(UnsignedMorsel(13))).asString(), "34"}
    };

    cout << "END CALLING ADDRESS MULT" << endl;
    return runCases(tests);
  }
  bool IntMultiplyAddress()
  {
    struct TestCase {
      string in;
      string out;
    };

    vector<TestCase> tests = {
       {(7 * Address(UnsignedMorsel(3))).asString(), "15"}
      ,{(4 * Address(UnsignedMorsel(13))).asString(), "34"}
    };

    return runCases(tests);
  }
  bool AddressMultiplyInt()
  {
    struct TestCase {
      string in;
      string out;
    };

    vector<TestCase> tests = {
       {(Address(UnsignedMorsel(7)) * 3).asString(), "15"}
      ,{(Address(UnsignedMorsel(13)) * 4).asString(), "34"}
    };

    return runCases(tests);
  } 
  bool AddressLeftShiftInt()
  {
    struct TestCase {
      string in;
      string out;
    };

    vector<TestCase> tests = {
       {(Address(UnsignedMorsel(0xa5)) << 3).asString(), "28"}
      ,{(Address(UnsignedMorsel(0xef)) << 4).asString(), "f0"}
    };

    return runCases(tests);
  }
  bool AddressLTAddress()
  {
    struct TestCase {
      bool in;
      bool out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)) < Address(UnsignedMorsel(3)), false}
      ,{Address(UnsignedMorsel(27)) < Address(UnsignedMorsel(46)), true}
    };

    return runCases(tests);
  }
  bool AddressLTInt()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)) <3, false}
      ,{Address(UnsignedMorsel(27)) <46, true}
    };

    return runCases(tests);
  }
  bool AddressGTInt()
  {
    struct TestCase {
      bool in;
      bool out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)) >3, true}
      ,{Address(UnsignedMorsel(27)) >46, false}
    };

    return runCases(tests);
  }
  bool AddressLEInt()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)) <=3, false}
      ,{Address(UnsignedMorsel(27)) <=46, true}
      ,{Address(UnsignedMorsel(27)) <=27, true}
    };

    return runCases(tests);
  }
  bool IntGTAddress()
  {
    struct TestCase {
      bool in;
      bool out;
    };

    vector<TestCase> tests = {
       { 3 > Address(UnsignedMorsel(23)), false}
      ,{ 46 > Address(UnsignedMorsel(27)), true}
      ,{ 27 > Address(UnsignedMorsel(27)), false}
    };

    return runCases(tests);
  }
  bool AddressAddInt()
  {
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))+3,Address(UnsignedMorsel(26))}
      ,{Address(UnsignedMorsel(27))+4,Address(UnsignedMorsel(31))}
      ,{Address(UnsignedMorsel(27))+0,Address(UnsignedMorsel(27))}
    };

    return runCases(tests);
  }
  bool AddressAddAddress()
  {
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))+Address(UnsignedMorsel(3)),Address(UnsignedMorsel(26))}
      ,{Address(UnsignedMorsel(27))+Address(UnsignedMorsel(4)),Address(UnsignedMorsel(31))}
      ,{Address(UnsignedMorsel(27))+Address(UnsignedMorsel(0)),Address(UnsignedMorsel(27))}
    };

    return runCases(tests);
  }
  bool AddressAddAssignInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))+=3,Address(UnsignedMorsel(26))}
      ,{Address(UnsignedMorsel(27))+=4,Address(UnsignedMorsel(31))}
      ,{Address(UnsignedMorsel(27))+=0,Address(UnsignedMorsel(27))}
    };

    return runCases(tests);
  }
  bool AddressSubAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))-Address(UnsignedMorsel(3)),Address(UnsignedMorsel(20))}
      ,{Address(UnsignedMorsel(27))-Address(UnsignedMorsel(4)),Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27))-Address(UnsignedMorsel(0)),Address(UnsignedMorsel(27))}
    };

    return runCases(tests);
  }
  bool AddressSubInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))-3,Address(UnsignedMorsel(20))}
      ,{Address(UnsignedMorsel(27))-4,Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27))-0,Address(UnsignedMorsel(27))}
    };

    return runCases(tests);
  }
  bool AddressInc()
  {
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))++,Address(UnsignedMorsel(24))}
      ,{Address(UnsignedMorsel(27))++,Address(UnsignedMorsel(28))}
    };

    return runCases(tests);
  }
  bool AddressModulusInt()
  {
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))%7,Address(UnsignedMorsel(2))}
      ,{Address(UnsignedMorsel(27))%11,Address(UnsignedMorsel(5))}
    };

    return runCases(tests);
  }
  bool AddressEQAddress()
  {
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)),Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27)),Address(UnsignedMorsel(27))}
    };

    return runCases(tests);
  }
  bool AddressEQInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      int out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)),23}
      ,{Address(UnsignedMorsel(27)),27}
    };

    return runCases(tests);
  }
  bool AddressLEUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)) <= UnsignedMorsel(23),true}
      ,{Address(UnsignedMorsel(27)) <= UnsignedMorsel(27),true}
    };

    return runCases(tests);
  }
  bool AddressAndAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)) & Address(UnsignedMorsel(23)),Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27)) & Address(UnsignedMorsel(27)),Address(UnsignedMorsel(27))}
    };

    return runCases(tests);

  }
  bool AddressAndInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)) & 23, Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27)) & 27, Address(UnsignedMorsel(27))}
    };

    return runCases(tests);
  }
  bool AddressOrAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)) | Address(UnsignedMorsel(23)), Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27)) | Address(UnsignedMorsel(27)), Address(UnsignedMorsel(27))}
    };

    return runCases(tests);
  }
  bool asUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)).asUnsignedMorsel(), UnsignedMorsel(23)}
      ,{Address(UnsignedMorsel(27)).asUnsignedMorsel(), UnsignedMorsel(27)}
    };

    return runCases(tests);
  }
  bool AddressDivAddress()
  {
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))/Address(UnsignedMorsel(7)), Address(UnsignedMorsel(3))}
      ,{Address(UnsignedMorsel(27))/Address(UnsignedMorsel(4)), Address(UnsignedMorsel(6))}
    };

    return runCases(tests);
  }
  bool AddressDivInt()
  {
    struct TestCase {
      Address in;
      Address out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23))/7, Address(UnsignedMorsel(3))}
      ,{Address(UnsignedMorsel(27))/4, Address(UnsignedMorsel(6))}
    };

    return runCases(tests);
  }

  bool TestSize()
  {
    struct TestCase {
      unsigned in;
      unsigned out;
    };

    vector<TestCase> tests = {
       {Address(UnsignedMorsel(23)).size(), 5}
      ,{Address(UnsignedMorsel(27)).size(), 5}
    };

    return runCases(tests);
  }

  bool TestResize()
  {
    struct TestCase {
      unsigned in;
      unsigned out;
    };

    Address a(UnsignedMorsel(23)); a.resize(3);
    Address b(UnsignedMorsel(32)); b.resize(15);

    vector<TestCase> tests = {
       {a.size(), 3}
      ,{b.size(), 15}
    };

    return runCases(tests);
  }
  void runAllTests()
  {
    TestAddress tm;
    struct NameResultPair {
      string funcName;
      bool (TestAddress::*funcPtr)();
    };

    NameResultPair tests[] = {
   {"assignInt", &TestAddress::assignInt}
  ,{"asInt", &TestAddress::asInt}
  ,{"asString", &TestAddress::asString}
  ,{"streamAddress", &TestAddress::streamAddress}
  ,{"AddressMultiplyAddress", &TestAddress::AddressMultiplyAddress}
  ,{"IntMultiplyAddress", &TestAddress::IntMultiplyAddress}
  ,{"AddressMultiplyInt", &TestAddress::AddressMultiplyInt}
  ,{"AddressLeftShiftInt", &TestAddress::AddressLeftShiftInt}
  ,{"AddressLTAddress", &TestAddress::AddressLTAddress}
  ,{"AddressLTInt", &TestAddress::AddressLTInt}
  ,{"AddressGTInt", &TestAddress::AddressGTInt}
  ,{"AddressLEInt", &TestAddress::AddressLEInt}
  ,{"IntGTAddress", &TestAddress::IntGTAddress}
  ,{"AddressAddInt", &TestAddress::AddressAddInt}
  ,{"AddressAddAddress", &TestAddress::AddressAddAddress}
  ,{"AddressAddAssignInt", &TestAddress::AddressAddAssignInt}
  ,{"AddressSubAddress", &TestAddress::AddressSubAddress}
  ,{"AddressSubInt", &TestAddress::AddressSubInt}
  ,{"AddressInc", &TestAddress::AddressInc}
  ,{"AddressModulusInt", &TestAddress::AddressModulusInt}
  ,{"AddressEQAddress", &TestAddress::AddressEQAddress}
  ,{"AddressEQInt", &TestAddress::AddressEQInt}
  ,{"AddressLEUnsignedMorsel", &TestAddress::AddressLEUnsignedMorsel}
  ,{"AddressAndAddress", &TestAddress::AddressAndAddress}
  ,{"AddressAndInt", &TestAddress::AddressAndInt}
  ,{"AddressOrAddress", &TestAddress::AddressOrAddress}
  ,{"asUnsignedMorsel", &TestAddress::asUnsignedMorsel}
  ,{"AddressDivAddress", &TestAddress::AddressDivAddress}
  ,{"AddressDivInt", &TestAddress::AddressDivInt}
//  ,{"ostreamAddress", &TestAddress::ostreamAddress}
  ,{"AddressSize", &TestAddress::TestSize}
  ,{"TestResize", &TestAddress::TestResize}

    };
    cout << "TestAddress" << endl;
    for (NameResultPair &t : tests)
    {
	    std::cout << "Test " << t.funcName << " " 
        << ( (tm.*(t.funcPtr))() ? "passed" : "failed") << endl;
    }
  }
};

int main()
{
	TestAddress tm;
  tm.runAllTests();
	return 0;
}
