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
  Address()
  {
  }
  Address(const Address &other)
  {
    m = other.m;
  }
  Address(Morsel in)
  {
    m = in;
  }
  Address& operator=(int in) {
    m = in;
    return *this;
  }
  int asInt() const
  {
    return m.asInt();
  }
  string asString() const
  {
    return "";
  }
  friend std::ostream& operator<<(std::ostream& stream, const Address& addr) {
    stream << addr.asString() ;
    return stream;
  }
  Address operator*(const Address& other)
  {
    Address lhs(this*);
    lhs.m = m * other.m;
  }
  Address operator<<(int other)
  {
    Address result(*this);
    result.m = result.m << other;
    return result;
  }
  bool operator<(Address other)
  {
    return m < other.m;
  }
  bool operator<(int other)
  {
    return m < other;
  }
  bool operator>(int other)
  {
    return m > other;
  }
  bool operator<=(int other)
  {
    return m < other || m == other;
  }
  friend bool operator>(int other, Address rhs) {
    return other > rhs.m;
  }
  Address operator+(int in)
  {
    Address result;
    result = in;
    result = result + *this;
    return result;
  }
  Address operator+(Address other)
  {
    Address result;
    result.m = m + other.m;
    return result;
  }
  Address& operator+=(int in)
  {
    m = m + in;
    return *this;
  }
  Address& operator++(int) 
  {
    m++;
    return *this;
  }
  Address operator%(int in)
  {
    Address result;
    result = in;
    result.m = m % result.m;
    return result;
  }
  bool operator==(const Address& other) const
  {
    return m == other.m;
  }
  bool operator==(int other)
  {
    Address otherAddr;
    otherAddr = other;
    return *this == otherAddr;
  }
  size_t hashVal()
  {
    return m.hashVal();
  }
  Address operator&(Address& other)
  {
    Address result(*this);
    result.m = result.m & other.m;
    return result;
  }
  Address operator&(uint64_t other)
  {
    Address otherAddr;
    otherAddr = other;
    Address result;
    result = (*this) & otherAddr;
    return result;
  }
  Address operator|(Address& other)
  {
    Address result(*this);
    result.m = result.m | other.m;
    return result;
  }
  Morsel asMorsel()
  {
    return m;
  }
};

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
#endif
