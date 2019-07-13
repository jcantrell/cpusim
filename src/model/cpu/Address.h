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
  bool operator<(Address other);
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
  size_t hashVal();
  Address operator&(Address& other);
  Address operator&(uint64_t other);
  Address operator|(Address& other);
  Morsel asMorsel();
  Address operator/(Address& other);
  Address operator/(int rhs);
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
  size_t hash_value(Address const& in);
#if 0
  size_t hash_value(Address const& in)
  {
    size_t hash = boost::hash_value(in.asInt());
    return hash;
  }
#endif

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
