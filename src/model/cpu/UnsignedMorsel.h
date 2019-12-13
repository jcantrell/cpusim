#ifndef UNSIGNEDMORSEL_H
#define UNSIGNEDMORSEL_H
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include <iostream>

#include "SignedMorsel.h"
class Address;
using namespace std;
using namespace boost;
class UnsignedMorsel 
{
  private:
    dynamic_bitset<> bs;
  public:
  UnsignedMorsel() : bs(64,0ul) {}
  size_t count();
  UnsignedMorsel(dynamic_bitset<> in);
  UnsignedMorsel(unsigned long long int in);
/*
  UnsignedMorsel(SignedMorsel in)
  {
*/
/*
    UnsignedMorsel other(in);
    SignedMorsel res(0);
    res.resize(other.size());
    
    while (other != 0)
    {
      res = res + 1;
      other = other - 1;
    }
    *this = res;
*/
/*
    UnsignedMorsel other(in);
    SignedMorsel res(0);
    res.resize(other.size());
    
    while (other != 0)
    {
      cout << "STEP 4 before: " << res << endl;
      res = res << SignedMorsel(1);
      cout << "STEP 4 after: " << res << endl;
      UnsignedMorsel t;
      cout << "STEP 4.1: " << other << endl;
      t = (other&1);
      cout << "STEP 4.2: " << t << endl;
      res = res | t;
      cout << "STEP 4.3: " << res << endl;
      cout << "STEP 5 other: " << other << endl;
      other = other >> 1;
      cout << "STEP 5 after: " << other << endl;
      cout << "STEP 5 comp: " << (other != 0) << endl;
    }
  cout << "CAST 6 FIXRX" << endl;
    *this = res;
  cout << "CAST 7 FIXRX" << endl;
  }
*/
  UnsignedMorsel operator+(const UnsignedMorsel& other) ;
  UnsignedMorsel operator+(unsigned int rhs);
  UnsignedMorsel operator-(const UnsignedMorsel& other);
  friend UnsignedMorsel operator-(unsigned int lhsInt, const UnsignedMorsel& other);
  UnsignedMorsel operator-=(const UnsignedMorsel& other);
  UnsignedMorsel& operator++(int);
  UnsignedMorsel& operator=(unsigned long long int in);
  UnsignedMorsel& operator=(const UnsignedMorsel& other);
  string asString() const ;
  friend std::ostream& operator<<( std::ostream& stream, const UnsignedMorsel& addr ) ;
  bool operator<(const UnsignedMorsel other) const;
  bool operator<(unsigned int other);
  bool operator<=(const UnsignedMorsel& other);
  bool operator<=(unsigned int other);
  friend bool operator<=(unsigned int lhs, const UnsignedMorsel& rhs);
  bool operator>(unsigned int other);
  friend bool operator>(unsigned int lhs, UnsignedMorsel rhs);
  bool operator>(UnsignedMorsel other);
  bool operator>=(const UnsignedMorsel& other);
  UnsignedMorsel operator/(const UnsignedMorsel& other);
  UnsignedMorsel operator%(const UnsignedMorsel& other);
  unsigned int size() const;
  bool operator==(UnsignedMorsel other) const ;
  bool operator==(unsigned int other) const;
  unsigned int asInt() const;
  UnsignedMorsel operator&(const UnsignedMorsel& other);
  UnsignedMorsel operator&(unsigned int otherInt);
  UnsignedMorsel operator|(const UnsignedMorsel& other);
  UnsignedMorsel operator^(const unsigned int& otherInt);
  UnsignedMorsel operator^(const UnsignedMorsel& other);
  UnsignedMorsel operator|=(const UnsignedMorsel& other);
  UnsignedMorsel operator~();
  UnsignedMorsel operator^(const UnsignedMorsel& other) const;
  UnsignedMorsel operator<<(const UnsignedMorsel& other);
  UnsignedMorsel operator<<(unsigned int other);
  UnsignedMorsel pb(unsigned int other);
  UnsignedMorsel operator>>(unsigned int other);
  UnsignedMorsel operator>>(const UnsignedMorsel& other);
  UnsignedMorsel operator*(const UnsignedMorsel& other);
  friend UnsignedMorsel operator*(unsigned int lhs, const UnsignedMorsel& other);
  UnsignedMorsel& operator<<=(uint64_t in);
  bool operator!=(const UnsignedMorsel& other);
  bool operator!=(unsigned long long int in);
  float asFloat() const;
  unsigned char asChar();
  UnsignedMorsel& resize(unsigned int newsize);
};
#endif
