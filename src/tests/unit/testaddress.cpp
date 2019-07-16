#include <iostream>
#include <string>
#include "../../model/cpu/Address.h"
class TestAddress : public Address
{
	public:
  TestMorsel() : Morsel() {}
  TestMorsel(dynamic_bitset<> in) : Morsel(in) {}
/*
* Address();
* Address(const Address &other);
* Address(Morsel in);
*/
  assignInt();
  asInt();
  asString();
  streamAddress();
  AddressMultiplyAddress();
  IntMultiplyAddress();
  AddressMultiplyInt();
  AddressShiftInt();
  bool AddressLTAddress();
  bool AddressLTInt<();
  bool AddressGTInt();
  bool AddressGEInt();
  friend bool operator>(int other, Address rhs);
  Address operator+(int in);
  Address operator+(Address other);
  Address& operator+=(int in);
  Address operator-(Address other);
  Address operator-(uint64_t other);
  Address& operator++(int) ;
  Address operator%(int in);
  bool operator==(const Address& other) const;
  bool operator==(int other);
  bool operator<=(Morsel in);
  size_t hashVal();
  AddressAndAddress operator&(Address& other);
  AddressAndInt operator&(uint64_t other);
  AddressOrAddress operator|(Address& other);
  asMorsel();
  AddressDivAddress(Address& other);
  bool AddressDivInt(int rhs);

  void runAllTests()
  {
    TestMorsel tm;
    struct NameResultPair {
      string funcName;
      bool (TestMorsel::*funcPtr)();
    };

    NameResultPair tests[] = {
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
	TestMorsel tm;
  tm.runAllTests();
	return 0;
}
