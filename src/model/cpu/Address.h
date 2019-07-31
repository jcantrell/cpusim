#ifndef ADDRESS_H
#define ADDRESS_H
#include <string>
#include "./morsel.h"
#include <boost/functional/hash.hpp>
using namespace std;
class Address {
  private:
    Morsel m;
  public:
  Address();
  Address(const Address &other);
  Address(Morsel in);
  Address& operator=(int in);
  int asInt() const;
  string asString() const;
  friend std::ostream& operator<<(std::ostream& stream, const Address& addr);
  Address operator*(const Address& other);
  friend Address operator*(int lhs, const Address& rhs);
  Address operator*(int in);
  Address operator<<(int other);
  bool operator<(const Address other) const;
  bool operator<(int other);
  bool operator>(int other);
  bool operator<=(int other);
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
  Address operator&(const Address& other);
  Address operator&(uint64_t other);
  Address operator|(const Address& other);
  Morsel asMorsel();
  Address operator/(const Address& other);
  Address operator/(int rhs);
  Address& resize(unsigned int newsize);
  unsigned int size();
};

  size_t hash_value(Address const& in);
#endif
