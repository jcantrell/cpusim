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
  Address& operator=(int in) {
    m = in;
    return *this;
  }
  string asString() const {
    return "";
  }
  friend std::ostream& operator<<(std::ostream& stream, const Address& addr) {
    stream << addr.asString() ;
    return stream;
  }
  bool operator<(Address other) {
    return m < other.m;
  }
  Address operator+(int in) {
    Address result;
    result = in;
    result = result + *this;
    return result;
  }
  Address operator+(Address other) {
    Address result;
    result.m = m + other.m;
    return result;
  }
  Address& operator+=(int in) {
    m = m + in;
    return *this;
  }

};

template <typename Block, typename Alloc>
struct hash<boost::dynamic_bitset<Block, Alloc>> {
  size_t operator()(boost::dynamic_bitset<Block, Alloc> const& bs) const {
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
#endif
