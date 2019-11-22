#ifndef ADDRESS_H
#define ADDRESS_H
#include <string>
#include "./UnsignedMorsel.h"
#include <boost/functional/hash.hpp>
using namespace std;
class Address {
  private:
    UnsignedMorsel m;
  public:
  Address();
  Address(const Address &other);
  Address(UnsignedMorsel in);
  Address& operator=(unsigned int in);
  int asInt() const;
  string asString() const;
  friend std::ostream& operator<<(std::ostream& stream, const Address& addr);
  Address operator*(const Address& other);
  friend Address operator*(unsigned int lhs, const Address& rhs);
  Address operator*(int in);
  Address operator<<(unsigned int other);
  bool operator<(const Address other) const;
  bool operator<(unsigned int other);
  bool operator>(unsigned int other);
  bool operator<=(unsigned int other);
  friend bool operator>(unsigned int other, Address rhs);
  Address operator+(unsigned int in);
  Address operator+(Address other);
  Address& operator+=(int in);
  Address operator-(Address other);
  Address operator-(uint64_t other);
  Address& operator++(int) ;
  Address operator%(unsigned int in);
  bool operator==(const Address& other) const;
  bool operator==(int other);
  bool operator<=(UnsignedMorsel in);
  Address operator&(const Address& other);
  Address operator&(uint64_t other);
  Address operator|(const Address& other);
  UnsignedMorsel asUnsignedMorsel();
  Address operator/(const Address& other);
  Address operator/(int rhs);
  Address& resize(unsigned int newsize);
  unsigned int size();
};

size_t hash_value(const Address& in);
#endif
