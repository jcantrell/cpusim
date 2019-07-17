#include "./Address.h"

Address::Address()
  {
  }
Address::Address(const Address &other)
  {
    m = other.m;
  }
Address::Address(Morsel in)
  {
    m = in;
  }
Address& Address::operator=(int in) 
{
    m = in;
    return *this;
  }
int Address::asInt() const
  {
    return m.asInt();
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
Address operator*(int lhs, const Address& rhs)
  {
    Address lhs_addr;lhs_addr = lhs;
    return lhs_addr * rhs;
  }
Address Address::operator*(int in)
  {
    Address other(in);
    return (*this) * other;
  }
  Address Address::operator<<(int other)
  {
    Address result(*this);
    result.m = result.m << other;
    return result;
  }
  bool Address::operator<(Address other)
  {
    return m < other.m;
  }
  bool Address::operator<(int other)
  {
    return m < other;
  }
  bool Address::operator>(int other)
  {
    return m > other;
  }
  bool Address::operator<=(int other)
  {
    return m < other || m == other;
  }
  bool operator>(int other, Address rhs) {
    return other > rhs.m;
  }
  Address Address::operator+(int in)
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
    m = m + in;
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
  Address Address::operator%(int in)
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
    otherAddr = other;
    return *this == otherAddr;
  }
  bool Address::operator<=(Morsel in)
  {
    return m <= in;
  }
  size_t Address::hashVal()
  {
    return m.hashVal();
  }
  Address Address::operator&(Address& other)
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
  Address Address::operator|(Address& other)
  {
    Address result(*this);
    result.m = result.m | other.m;
    return result;
  }
  Morsel Address::asMorsel()
  {
    return m;
  }
  Address Address::operator/(Address& other)
  {
    Address result(*this);
    result.m = result.m / other.m;
    return result;
  }
  Address Address::operator/(int rhs)
  {
    Address other(rhs);
    Address result(*this);
    result.m = result.m / other.m;
    return result;
  }

/*
namespace boost {
  template <typename Block, typename Alloc> 
  struct hash<boost::dynamic_bitset<Block, Alloc>>
  {
    size_t operator()(boost::dynamic_bitset<Block, Alloc> const& bs) const
    {
      size_t seed = boost::hash_value <Block>(bs.size());
      std::vector<Block> blocks(bs.num_blocks());
      boost::hash_range(seed, blocks.begin(), blocks.end());
      return seed;
    }
  };
  //template <typename Block>
  size_t hash_value(Address const& in)
  {
    size_t hash = boost::hash_value(in.asInt());
    return hash;
  }
}
*/
size_t hash_value(Address const& in)
{
  size_t hash = boost::hash_value(in.asInt());
  return hash;
}
/*

template <typename Block, typename Alloc>
struct hash<boost::dynamic_bitset<Block, Alloc>> {
  size_t operator()(boost::dynamic_bitset<Block, Alloc> const& bs) const
  {
    size_t seed = boost::hash_value(bs.size());
    std::vector<Block> blocks(bs.num_blocks());
    boost::hash_range(seed, blocks.begin(), blocks.end());
    return seed;
  }
};

template <typename Address>
struct hash<Address>
{
  std::size_t operator()(const Address& key) const
  {
    using std::size_t;
    using std::hash;
    using std::string;
    return (hash<Morsel>(key.m));
  }
};
*/
