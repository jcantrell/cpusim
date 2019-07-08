#ifndef FAKEMORSEL_H
#define FAKEMORSEL_H
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>

#define BSET 1
class fakemorsel
{
  private:
    boost::dynamic_bitset<> bs;
    int i;
  public:
#if BSET
    //boost::dynamic_bitset<> hashValue() const { return bs; }
    size_t hashValue() const { return i; }
#else
    size_t hashValue() const { return i; }
#endif
    bool operator==(const fakemorsel &rhs) const { return bs==rhs.bs; }
};

/*
namespace std {
  template <>
  struct hash<fakemorsel>
  {
    size_t operator()(const fakemorsel& key) const
    {
#if BSET
      return (hash<boost::dynamic_bitset<>>()(key.hashValue()));
#else
      return (hash<int>()(key.hashValue()));
#endif
    }
  };
}
*/

/*
#if BSET
namespace boost {
  template <typename B, typename A>
  std::size_t hash_value(const boost::dynamic_bitset<B, A>&bs)
  {
    std::vector<B, A> v;
    boost::to_block_range(bs, std::back_inserter(v));
    return boost::hash_value(v);
  }
}
#if 0
namespace std {
	template <typename Block, typename Alloc>
	struct hash<dynamic_bitset<Block, Alloc>> {
	  size_t operator()(dynamic_bitset<Block, Alloc> const& bs) const {
	    size_t seed = hash_value(bs.size());
	    vector<Block> blocks(bs.num_blocks());
	    hash_range(seed, blocks.begin(), blocks.end());
	    return seed;
	  }
  };
}
#endif
#endif
*/
#endif
