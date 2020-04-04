#ifndef SIGNEDMORSEL_H
#define SIGNEDMORSEL_H
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include <iostream>
#include "UnsignedMorsel.h"
using namespace std;
using namespace boost;
class UnsignedMorsel;
class SignedMorsel 
{
  private:
    dynamic_bitset<> bs;
  public:
  SignedMorsel() : bs(64,0ul) {}
  size_t count();
  SignedMorsel(dynamic_bitset<> in);
  SignedMorsel(unsigned long long int in);
  SignedMorsel(const UnsignedMorsel in);
  SignedMorsel operator+(const SignedMorsel& other) ;
  SignedMorsel operator+(int rhs);
  SignedMorsel operator-(const SignedMorsel& other);
  friend SignedMorsel operator-(int lhsInt, const SignedMorsel& other);
  SignedMorsel operator-=(const SignedMorsel& other);
  SignedMorsel& operator++(int);
  SignedMorsel& operator=(unsigned long long int in);
  SignedMorsel& operator=(const SignedMorsel& other);
  string asString() const ;
  friend std::ostream& operator<<( std::ostream& stream, const SignedMorsel& addr ) ;
  bool operator<(const SignedMorsel other) const;
  bool operator<(int other);
  bool operator<=(const SignedMorsel& other);
  bool operator<=(int other);
  friend bool operator<=(int lhs, const SignedMorsel& rhs);
  bool operator>(int other);
  friend bool operator>(int lhs, SignedMorsel rhs);
  bool operator>(SignedMorsel other);
  bool operator>=(const SignedMorsel& other);
  SignedMorsel operator/(const SignedMorsel& other);
  SignedMorsel operator%(const SignedMorsel& other);
  unsigned int size() const;
  bool operator==(SignedMorsel other) const ;
  bool operator==(int other) const ;
  unsigned int asInt() const;
  SignedMorsel operator&(const SignedMorsel& other);
  SignedMorsel operator&(int otherInt);
  SignedMorsel operator|(const SignedMorsel& other);
  SignedMorsel operator|=(const SignedMorsel& other);
  SignedMorsel operator~();
  SignedMorsel operator-();
  SignedMorsel operator^(const SignedMorsel& other) const;
  SignedMorsel operator<<(const SignedMorsel& other);
  SignedMorsel operator<<(int other);
  SignedMorsel pb(int other);
  SignedMorsel operator>>(int other);
  SignedMorsel operator>>(const SignedMorsel& other);
  SignedMorsel operator*(const SignedMorsel& other);
  friend SignedMorsel operator*(int lhs, const SignedMorsel& other);
  SignedMorsel& operator<<=(uint64_t in);
  bool operator!=(const SignedMorsel& other);
  bool operator!=(unsigned long long int in);
  float asFloat() const;
  unsigned char asChar();
  SignedMorsel& resize(unsigned int newsize);
};
#endif
