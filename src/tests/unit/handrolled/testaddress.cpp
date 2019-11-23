#include <iostream>
#include <string>
#include "../../../model/cpu/Address.h"
class TestAddress : public Address
{
	public:
  TestAddress() : Address() {}
  TestAddress(dynamic_bitset<> in) : Address(in) {}
/*
* Address();
* Address(const Address &other);
* Address(UnsignedMorsel in);
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
       {Address(UnsignedMorsel(95)), 0x5f}
      ,{Address(UnsignedMorsel(42)), 0x2a}
    };

    for (TestCase &t : tests)
    {
      result = result && (static_cast<unsigned int>(t.in.asInt()) == t.out);
    }
    return result;


  }
  bool asString()
  {
    UnsignedMorsel a(95);
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
       {Address(UnsignedMorsel(95)), "5f"}
      ,{Address(UnsignedMorsel(42)), "2a"}
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
       {Address(UnsignedMorsel(7)) * Address(UnsignedMorsel(3)), "15"}
      ,{Address(UnsignedMorsel(4)) * Address(UnsignedMorsel(13)), "34"}
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
       {7 * Address(UnsignedMorsel(3)), "15"}
      ,{4 * Address(UnsignedMorsel(13)), "34"}
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
       {Address(UnsignedMorsel(7)) * 3, "15"}
      ,{Address(UnsignedMorsel(13)) * 4, "34"}
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
       {Address(UnsignedMorsel(0xa5)) << 3, "28"}
      ,{Address(UnsignedMorsel(0xef)) << 4, "f0"}
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
       {Address(UnsignedMorsel(23)) < Address(UnsignedMorsel(3)), false}
      ,{Address(UnsignedMorsel(27)) < Address(UnsignedMorsel(46)), true}
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
       {Address(UnsignedMorsel(23)) <3, false}
      ,{Address(UnsignedMorsel(27)) <46, true}
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
       {Address(UnsignedMorsel(23)) >3, true}
      ,{Address(UnsignedMorsel(27)) >46, false}
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
       {Address(UnsignedMorsel(23)) <=3, false}
      ,{Address(UnsignedMorsel(27)) <=46, true}
      ,{Address(UnsignedMorsel(27)) <=27, true}
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
       { 3 > Address(UnsignedMorsel(23)), false}
      ,{ 46 > Address(UnsignedMorsel(27)), true}
      ,{ 27 > Address(UnsignedMorsel(27)), false}
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
       {Address(UnsignedMorsel(23))+3,Address(UnsignedMorsel(26))}
      ,{Address(UnsignedMorsel(27))+4,Address(UnsignedMorsel(31))}
      ,{Address(UnsignedMorsel(27))+0,Address(UnsignedMorsel(27))}
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
       {Address(UnsignedMorsel(23))+Address(UnsignedMorsel(3)),Address(UnsignedMorsel(26))}
      ,{Address(UnsignedMorsel(27))+Address(UnsignedMorsel(4)),Address(UnsignedMorsel(31))}
      ,{Address(UnsignedMorsel(27))+Address(UnsignedMorsel(0)),Address(UnsignedMorsel(27))}
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
       {Address(UnsignedMorsel(23))+=3,Address(UnsignedMorsel(26))}
      ,{Address(UnsignedMorsel(27))+=4,Address(UnsignedMorsel(31))}
      ,{Address(UnsignedMorsel(27))+=0,Address(UnsignedMorsel(27))}
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
       {Address(UnsignedMorsel(23))-Address(UnsignedMorsel(3)),Address(UnsignedMorsel(20))}
      ,{Address(UnsignedMorsel(27))-Address(UnsignedMorsel(4)),Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27))-Address(UnsignedMorsel(0)),Address(UnsignedMorsel(27))}
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
       {Address(UnsignedMorsel(23))-3,Address(UnsignedMorsel(20))}
      ,{Address(UnsignedMorsel(27))-4,Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27))-0,Address(UnsignedMorsel(27))}
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
       {Address(UnsignedMorsel(23))++,Address(UnsignedMorsel(24))}
      ,{Address(UnsignedMorsel(27))++,Address(UnsignedMorsel(28))}
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
       {Address(UnsignedMorsel(23))%7,Address(UnsignedMorsel(2))}
      ,{Address(UnsignedMorsel(27))%11,Address(UnsignedMorsel(5))}
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
       {Address(UnsignedMorsel(23)),Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27)),Address(UnsignedMorsel(27))}
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
       {Address(UnsignedMorsel(23)),23}
      ,{Address(UnsignedMorsel(27)),27}
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
  bool AddressLEUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      bool in;
      bool out;
    };

    TestCase tests[] = {
       {Address(UnsignedMorsel(23)) <= UnsignedMorsel(23),true}
      ,{Address(UnsignedMorsel(27)) <= UnsignedMorsel(27),true}
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
       {Address(UnsignedMorsel(23)) & Address(UnsignedMorsel(23)),Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27)) & Address(UnsignedMorsel(27)),Address(UnsignedMorsel(27))}
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
       {Address(UnsignedMorsel(23)) & 23, Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27)) & 27, Address(UnsignedMorsel(27))}
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
       {Address(UnsignedMorsel(23)) | Address(UnsignedMorsel(23)), Address(UnsignedMorsel(23))}
      ,{Address(UnsignedMorsel(27)) | Address(UnsignedMorsel(27)), Address(UnsignedMorsel(27))}
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
  bool asUnsignedMorsel()
  {
    bool result = true;
    struct TestCase {
      Address in;
      Address out;
    };

    TestCase tests[] = {
       {Address(UnsignedMorsel(23)).asUnsignedMorsel(), UnsignedMorsel(23)}
      ,{Address(UnsignedMorsel(27)).asUnsignedMorsel(), UnsignedMorsel(27)}
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
       {Address(UnsignedMorsel(23))/Address(UnsignedMorsel(7)), Address(UnsignedMorsel(3))}
      ,{Address(UnsignedMorsel(27))/Address(UnsignedMorsel(4)), Address(UnsignedMorsel(6))}
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
       {Address(UnsignedMorsel(23))/7, Address(UnsignedMorsel(3))}
      ,{Address(UnsignedMorsel(27))/4, Address(UnsignedMorsel(6))}
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
  ,{"AddressLEUnsignedMorsel", &TestAddress::AddressLEUnsignedMorsel}
  ,{"AddressAndAddress", &TestAddress::AddressAndAddress}
  ,{"AddressAndInt", &TestAddress::AddressAndInt}
  ,{"AddressOrAddress", &TestAddress::AddressOrAddress}
  ,{"asUnsignedMorsel", &TestAddress::asUnsignedMorsel}
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
