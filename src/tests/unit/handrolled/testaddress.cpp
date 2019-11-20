#include <iostream>
#include <string>
#include "../../../../model/cpu/Address.h"
class TestAddress : public Address
{
	public:
  TestAddress() : Address() {}
  TestAddress(dynamic_bitset<> in) : Address(in) {}
/*
* Address();
* Address(const Address &other);
* Address(Morsel in);
*/
  bool assignInt()
  { 
    bool result = true;
    struct TestCase {
      unsigned int in;
      string out;
    };

    TestCase tests[] = {
       {95, "5f"}
      ,{42, "2a"}
    };

    for (TestCase &t : tests)
    {
      Address ta;
      ta = t.in;
      result = result && (ta.asString() == t.out);
    }
    return result;
  }
  bool asInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      unsigned int out;
    };

    TestCase tests[] = {
       {Address(Morsel(95)), 0x5f}
      ,{Address(Morsel(42)), 0x2a}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in.asInt() == t.out);
    }
    return result;


  }
  bool asString()
  {
    Morsel a(95);
    Address b(a);
    bool result = (b.asString() == a.asString());
    return result;
  }
  bool streamAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      string out;
    };

    TestCase tests[] = {
       {Address(Morsel(95)), "5f"}
      ,{Address(Morsel(42)), "2a"}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in.asString() == t.out);
    }
    return result;
  }
  bool AddressMultiplyAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      string out;
    };

    TestCase tests[] = {
       {Address(Morsel(7)) * Address(Morsel(3)), "15"}
      ,{Address(Morsel(4)) * Address(Morsel(13)), "34"}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in.asString() == t.out);
    }
    return result;
  }
  bool IntMultiplyAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      string out;
    };

    TestCase tests[] = {
       {7 * Address(Morsel(3)), "15"}
      ,{4 * Address(Morsel(13)), "34"}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in.asString() == t.out);
    }
    return result;
  }
  bool AddressMultiplyInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      string out;
    };

    TestCase tests[] = {
       {Address(Morsel(7)) * 3, "15"}
      ,{Address(Morsel(13)) * 4, "34"}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in.asString() == t.out);
      if (t.in.asString() != t.out)
      {
        cout << "Expected: " << t.out << " Actual: " << t.in.asString() << endl;
      }
    }
    return result;
  } 
  bool AddressLeftShiftInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      string out;
    };

    TestCase tests[] = {
       {Address(Morsel(0xa5)) << 3, "28"}
      ,{Address(Morsel(0xef)) << 4, "f0"}
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
  bool AddressLTAddress()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)) < Address(Morsel(3)), false}
      ,{Address(Morsel(27)) < Address(Morsel(46)), true}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in == t.out);
      if (t.in != t.out)
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in << endl;
      }
    }
    return result;
  }
  bool AddressLTInt()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)) <3, false}
      ,{Address(Morsel(27)) <46, true}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in == t.out);
      if (t.in != t.out)
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in << endl;
      }
    }
    return result;
  }
  bool AddressGTInt()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)) >3, true}
      ,{Address(Morsel(27)) >46, false}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in == t.out);
      if (t.in != t.out)
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in << endl;
      }
    }
    return result;
 
  }
  bool AddressLEInt()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)) <=3, false}
      ,{Address(Morsel(27)) <=46, true}
      ,{Address(Morsel(27)) <=27, true}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in == t.out);
      if (t.in != t.out)
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in << endl;
      }
    }
    return result;
  }
  bool IntGTAddress()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       { 3 > Address(Morsel(23)), false}
      ,{ 46 > Address(Morsel(27)), true}
      ,{ 27 > Address(Morsel(27)), false}
    };

    for (TestCase &t : tests)
    {
      result = result && (t.in == t.out);
      if (t.in != t.out)
      {
        cout << endl << "Expected: " << t.out 
                     << " Actual: " << t.in << endl;
      }
    }
    return result;
  }
  bool AddressAddInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))+3,Address(Morsel(26))}
      ,{Address(Morsel(27))+4,Address(Morsel(31))}
      ,{Address(Morsel(27))+0,Address(Morsel(27))}
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
  bool AddressAddAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))+Address(Morsel(3)),Address(Morsel(26))}
      ,{Address(Morsel(27))+Address(Morsel(4)),Address(Morsel(31))}
      ,{Address(Morsel(27))+Address(Morsel(0)),Address(Morsel(27))}
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
  bool AddressAddAssignInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))+=3,Address(Morsel(26))}
      ,{Address(Morsel(27))+=4,Address(Morsel(31))}
      ,{Address(Morsel(27))+=0,Address(Morsel(27))}
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
  bool AddressSubAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))-Address(Morsel(3)),Address(Morsel(20))}
      ,{Address(Morsel(27))-Address(Morsel(4)),Address(Morsel(23))}
      ,{Address(Morsel(27))-Address(Morsel(0)),Address(Morsel(27))}
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
  bool AddressSubInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))-3,Address(Morsel(20))}
      ,{Address(Morsel(27))-4,Address(Morsel(23))}
      ,{Address(Morsel(27))-0,Address(Morsel(27))}
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
  bool AddressInc()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))++,Address(Morsel(24))}
      ,{Address(Morsel(27))++,Address(Morsel(28))}
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
  bool AddressModulusInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))%7,Address(Morsel(2))}
      ,{Address(Morsel(27))%11,Address(Morsel(5))}
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
  bool AddressEQAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)),Address(Morsel(23))}
      ,{Address(Morsel(27)),Address(Morsel(27))}
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
  bool AddressEQInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      int out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)),23}
      ,{Address(Morsel(27)),27}
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
  bool AddressLEMorsel()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)) <= Morsel(23),true}
      ,{Address(Morsel(27)) <= Morsel(27),true}
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
  bool AddressAndAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)) & Address(Morsel(23)),Address(Morsel(23))}
      ,{Address(Morsel(27)) & Address(Morsel(27)),Address(Morsel(27))}
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
  bool AddressAndInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)) & 23, Address(Morsel(23))}
      ,{Address(Morsel(27)) & 27, Address(Morsel(27))}
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
  bool AddressOrAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)) | Address(Morsel(23)), Address(Morsel(23))}
      ,{Address(Morsel(27)) | Address(Morsel(27)), Address(Morsel(27))}
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
  bool asMorsel()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23)).asMorsel(), Morsel(23)}
      ,{Address(Morsel(27)).asMorsel(), Morsel(27)}
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
  bool AddressDivAddress()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))/Address(Morsel(7)), Address(Morsel(3))}
      ,{Address(Morsel(27))/Address(Morsel(4)), Address(Morsel(6))}
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
  bool AddressDivInt()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(Morsel(23))/7, Address(Morsel(3))}
      ,{Address(Morsel(27))/4, Address(Morsel(6))}
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
  ,{"AddressLEMorsel", &TestAddress::AddressLEMorsel}
  ,{"AddressAndAddress", &TestAddress::AddressAndAddress}
  ,{"AddressAndInt", &TestAddress::AddressAndInt}
  ,{"AddressOrAddress", &TestAddress::AddressOrAddress}
  ,{"asMorsel", &TestAddress::asMorsel}
  ,{"AddressDivAddress", &TestAddress::AddressDivAddress}
  ,{"AddressDivInt", &TestAddress::AddressDivInt}

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
