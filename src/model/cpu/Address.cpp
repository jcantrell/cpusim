#include "./Address.h"

Address::Address()
  {
  }
Address::Address(const Address &other)
  {
    m = other.m;
  }
Address::Address(UnsignedMorsel in)
  {
    m = in;
  }
Address& Address::operator=(unsigned int in) 
{
    m = in;
    return *this;
  }
int Address::asInt() const
  {
    return static_cast<int>(m.asInt());
  }
string Address::asString() const
  {
    return m.asString();
  }
std::ostream& operator<<(std::ostream& stream, const Address& addr) 
{
    stream << addr.asString() ;
    return stream;
}
Address Address::operator*(const Address& other)
{
    Address lhs(m);
    lhs.m = m * other.m;
    return lhs;
}
Address operator*(unsigned int lhs, const Address& rhs)
  {
    Address lhs_addr;lhs_addr = lhs;
    return lhs_addr * rhs;
  }
Address Address::operator*(int in)
  {
    Address other(static_cast<unsigned int>(in));
    return (*this) * other;
  }
  Address Address::operator<<(unsigned int other)
  {
    Address result(*this);
    result.m = result.m << other;
    return result;
  }
  bool Address::operator<(const Address other) const
  {
    return m < other.m;
  }
  bool Address::operator<(unsigned int other)
  {
    return m < other;
  }
  bool Address::operator>(unsigned int other)
  {
    return m > other;
  }
  bool Address::operator<=(unsigned int other)
  {
    return m < other || m == other;
  }
  bool operator>(unsigned int other, Address rhs) {
    //return other > rhs.m;
    return rhs < static_cast<unsigned int>(other);
  }
  Address Address::operator+(unsigned int in)
  {
    Address result;
    result = in;
    result = result + *this;
    return result;
  }
  Address Address::operator+(Address other)
  {
    Address result;
    result.m = m + other.m;
    return result;
  }
  Address& Address::operator+=(int in)
  {
    m = m + static_cast<unsigned int>(in);
    return *this;
  }
  Address Address::operator-(Address other)
  {
    Address result;
    result.m = m - other.m;
    return result;
  }
  Address Address::operator-(uint64_t other)
  {
    Address rhs;
    rhs = other;
    Address result;
    result.m = m - rhs.m;
    return result;
  }
  Address& Address::operator++(int) 
  {
    m++;
    return *this;
  }
  Address Address::operator%(unsigned int in)
  {
    Address result;
    result = in;
    result.m = m % result.m;
    return result;
  }
  bool Address::operator==(const Address& other) const
  {
    return m == other.m;
  }
  bool Address::operator==(int other)
  {
    Address otherAddr;
    otherAddr = static_cast<unsigned int>(other);
    return *this == otherAddr;
  }
  bool Address::operator<=(UnsignedMorsel in)
  {
    return m <= in;
  }
  Address Address::operator&(const Address& other)
  {
    Address result(*this);
    result.m = result.m & other.m;
    return result;
  }
  Address Address::operator&(uint64_t other)
  {
    Address otherAddr;
    otherAddr = other;
    Address result;
    result = (*this) & otherAddr;
    return result;
  }
  Address Address::operator|(const Address& other)
  {
    Address result(*this);
    result.m = result.m | other.m;
    return result;
  }
  UnsignedMorsel Address::asUnsignedMorsel()
  {
    return m;
  }
  Address Address::operator/(const Address& other)
  {
    Address result(*this);
    result.m = result.m / other.m;
    return result;
  }
  Address Address::operator/(int rhs)
  {
    Address other(static_cast<unsigned int>(rhs));
    Address result(*this);
    result.m = result.m / other.m;
    return result;
  }
  Address & Address::resize(unsigned int newsize)
  {
    m.resize(newsize);
    return *this;
  }
  unsigned int Address::size()
  {
    return m.size();
  }

size_t hash_value(const Address& in)
{
  size_t hash = boost::hash_value(in.asInt());
  return hash;
}
