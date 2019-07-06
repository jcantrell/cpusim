#ifndef FAKEMORSEL_H
#define FAKEMORSEL_H
#include <boost/dynamic_bitset.hpp>


class fakemorsel
{
  private:
    //boost::dynamic_bitset<> bs;
    int bs;
  public:
    size_t hashValue() const {
      return bs;
    }
};

namespace std {
  template <>
  struct hash<fakemorsel>
  {
    size_t operator()(const fakemorsel& key) const
    {
      return (hash<int>()(key.hashValue()));
    }
  };
}
#endif
