#include <iostream>
#include <string>
#include "../../model/cpu/Address.h"
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
    return false;
  }
  bool asInt(){return false;}
  bool asString()
  {
    Morsel a(95);
    Address b(a);
    bool result = (b.asString() == a.asString());
    return result;
  }
  bool streamAddress(){
    stringstream out;
    Morsel m(42);
    Address a(m);
    out << a;
    cout << "a: " << a << " m: " << m << " out: " << out.str() << endl;
    return false;
  }
  bool AddressMultiplyAddress(){return false;}
  bool IntMultiplyAddress(){return false;}
  bool AddressMultiplyInt(){return false;}
  bool AddressShiftInt(){return false;}
  bool AddressLTAddress(){return false;}
  bool AddressLTInt(){return false;}
  bool AddressGTInt(){return false;}
  bool AddressGEInt(){return false;}
  bool IntGTAddress(){return false;}
  bool AddressAddInt(){return false;}
  bool AddressAddAddress(){return false;}
  bool AddressAddAssignInt(){return false;}
  bool AddressSubAddress(){return false;}
  bool AddressSubInt(){return false;}
  bool AddressInc() {return false;}
  bool AddressModulusInt(){return false;}
  bool AddressEQAddress(){return false;}
  bool AddressEQInt(){return false;}
  bool AddressLEMorsel(){return false;}
  bool hashVal(){return false;}
  bool AddressAndAddress(){return false;}
  bool AddressAndInt(){return false;}
  bool AddressOrAddress(){return false;}
  bool asMorsel(){return false;}
  bool AddressDivAddress(){return false;}
  bool AddressDivInt(){return false;}

  void runAllTests()
  {
    TestAddress tm;
    struct NameResultPair {
      string funcName;
      bool (TestAddress::*funcPtr)();
    };

    NameResultPair tests[] = {
  {"assignInt", TestAddress::assignInt}
  ,{"asInt", TestAddress::asInt}
  ,{"asString", TestAddress::asString}
  ,{"streamAddress", TestAddress::streamAddress}
  ,{"AddressMultiplyAddress", TestAddress::AddressMultiplyAddress}
  ,{"IntMultiplyAddress", TestAddress::IntMultiplyAddress}
  ,{"AddressMultiplyInt", TestAddress::AddressMultiplyInt}
  ,{"AddressShiftInt", TestAddress::AddressShiftInt}
  ,{"AddressLTAddress", TestAddress::AddressLTAddress}
  ,{"AddressLTInt", TestAddress::AddressLTInt}
  ,{"AddressGTInt", TestAddress::AddressGTInt}
  ,{"AddressGEInt", TestAddress::AddressGEInt}
  ,{"IntGTAddress", TestAddress::IntGTAddress}
  ,{"AddressAddInt", TestAddress::AddressAddInt}
  ,{"AddressAddAddress", TestAddress::AddressAddAddress}
  ,{"AddressAddAssignInt", TestAddress::AddressAddAssignInt}
  ,{"AddressSubAddress", TestAddress::AddressSubAddress}
  ,{"AddressSubInt", TestAddress::AddressSubInt}
  ,{"AddressInc", TestAddress::AddressInc}
  ,{"AddressModulusInt", TestAddress::AddressModulusInt}
  ,{"AddressEQAddress", TestAddress::AddressEQAddress}
  ,{"AddressEQInt", TestAddress::AddressEQInt}
  ,{"AddressLEMorsel", TestAddress::AddressLEMorsel}
  ,{"hashVal", TestAddress::hashVal}
  ,{"AddressAndAddress", TestAddress::AddressAndAddress}
  ,{"AddressAndInt", TestAddress::AddressAndInt}
  ,{"AddressOrAddress", TestAddress::AddressOrAddress}
  ,{"asMorsel", TestAddress::asMorsel}
  ,{"AddressDivAddress", TestAddress::AddressDivAddress}
  ,{"AddressDivInt", TestAddress::AddressDivInt}

    };
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
