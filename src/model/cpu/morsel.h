#ifndef MORSEL_H
#define MORSEL_H
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include "Address.h"
using namespace std;
using namespace boost;
class Morsel 
{
  private:
    dynamic_bitset<> bs;
  public:
  Morsel() : bs(64,0ul) {}
  size_t count()
  {
    return bs.count();
  }
  Morsel(dynamic_bitset<> in)
  {
    bs = in;
  }
  Morsel(unsigned long long int in)
  {
    *this = in;
  }
  Morsel operator+(const Morsel& other) 
  {
    Morsel out;
    bool a, b, carry;
    int i;
    for (i=0, carry=false;i<other.bs.size() && i<bs.size();i++) {
      a = bs[i];
      b = other.bs[i];
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
      out.bs.push_back( a xor b xor carry );
    }
    for (;i<other.bs.size();i++) {
      a = false;
      b = other.bs[i];
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
      out.bs.push_back( a xor b xor carry );
    }
    for (;i<other.bs.size() && i<bs.size();i++) {
      a = bs[i];
      b = false;
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
      out.bs.push_back( a xor b xor carry );
    }
    
    return out;
  }
  Morsel operator+( int rhs )
  {
    Morsel rhs_morsel;
    rhs_morsel = rhs;
    return *this + rhs_morsel;
  }
  Morsel operator-(const Morsel& other)
  {
    dynamic_bitset<> inverted_bs;
    inverted_bs = other.bs;
    inverted_bs.flip();
    Morsel inverse(inverted_bs);
    inverse = inverse + 1;
    return *this + inverse;
  }
  friend Morsel operator-(int lhsInt, const Morsel& other)
  {
    Morsel lhs;
    lhs = lhsInt;
    return lhs - other;
  }
  Morsel operator-=(const Morsel& other)
  {
    *this = *this - other;
    return *this;
  }
  Morsel& operator++(int)
  {
    *this = (*this) + 1;
    return *this;
  }
  Morsel& operator=(unsigned long long int in)
  {
    //bs = in;
    while (in != 0) {
      bs.push_back( in & 1 );
      in >>= 1;
    }
    return *this;
  }
  Morsel& operator=(const Morsel& other)
  {
    bs = other.bs;
    return *this;
  }
  string asString() const 
  {
    return "";
  }
  friend std::ostream& operator<<( std::ostream& stream, const Morsel& addr ) 
  {
    stream << addr.asString();
    return stream;
  }
  bool operator<(Morsel other)
  {
    int this_index = 0;
    int other_index = 0;
    for (;this_index < (bs.size() - other.size());this_index++) {
      if (bs[this_index] == 1)
        return true;
    }
    for (;other_index < (other.size() - bs.size()); other_index++) {
      if (other.bs[other_index] == 1)
        return true;
    }
    for (;this_index < size() && other_index < other.size(); 
          this_index++, other_index++)
    {
      if (bs[this_index] != other.bs[other_index]) {
        if (bs[this_index] == 1) return false;
      } 
    }
    return false;
  }
  bool operator<(int other)
  {
    Morsel otherMorsel;
    otherMorsel = other;
    return *this < otherMorsel;
  }
  bool operator<=(const Morsel& other)
  {
    return *this < other || *this == other; 
  }
  bool operator<=(int other)
  {
    return *this < other || *this == other;
  }
  friend bool operator<=(int lhs, const Morsel& rhs)
  {
    Morsel lhsMorsel(lhs);
    return lhsMorsel <= rhs;
  }
  bool operator>(int other)
  {
    Morsel otherMorsel;
    otherMorsel = other;
    //return !((*this < otherMorsel) || (*this == otherMorsel));
    return *this > otherMorsel;
  }
  friend bool operator>(int lhs, Morsel rhs)
  {
    Morsel lhsMorsel;
    lhsMorsel = lhs;
    return lhsMorsel > rhs;
  }
  bool operator>(Morsel other)
  {
    return !((*this < other) || (*this == other));
  }
  bool operator>=(const Morsel& other)
  {
    return *this > other || *this == other;
  }
  Morsel operator/(const Morsel& other)
  {
    Morsel numerator(*this);
    Morsel quotient;
    quotient = 0;
    while (numerator > other || numerator == other)
    {
      numerator = numerator - other;
      quotient = quotient + 1;
    }
    return quotient;
  }
  Morsel operator%(const Morsel& other)
  {
    Morsel numerator(*this);
    Morsel quotient;
    quotient = 0;
    while (numerator > other || numerator == other)
    {
      numerator = numerator - other;
      quotient = quotient + 1;
    }
    return numerator;
  }
  unsigned int size() 
  {
    return bs.size();
  }
  bool operator==(Morsel other) const 
  { 
    return bs==other.bs; 
  }
  bool operator==(int other) const {
    Morsel result;
    result = other;
    return *this == result;
  }
  int asInt() const
  {
    dynamic_bitset<> copy = bs;
    int i = 0;
    i = ~i;
    while (copy.count() != 0)
    {
      i = (i << 1) & bs[0];
      copy.pop_back();
    }
    return i;
  }
  Morsel operator&(const Morsel& other)
  {
    Morsel result(*this);
    result.bs = bs & other.bs;
    return result;
  }
  Morsel operator&(int otherInt)
  {
    Morsel other;
    other = otherInt;
    Morsel result(*this);
    result.bs = bs & other.bs;
    return result;
  }
  Morsel operator|(const Morsel& other)
  {
    Morsel result(*this);
    result.bs = bs | other.bs;
    return result;
  }
  Morsel operator|=(const Morsel& other)
  {
    bs = bs | other.bs;
    return (*this);
  }
  Morsel operator~()
  {
    Morsel result(*this);
    result.bs = ~bs;
    return result;
  }
  Morsel operator^(const Morsel& other)
  {
    Morsel result(*this);
    result.bs = bs ^ other.bs;
    return result;
  }
  Morsel operator<<(const Morsel& other)
  {
    Morsel result(*this);
    result.bs = bs << other.asInt();
    return result;
  }
  Morsel operator<<(int other)
  {
    Morsel otherMorsel;
    otherMorsel = other;
    return *this << otherMorsel;
  }
  Morsel operator>>(int other)
  {
    Morsel otherMorsel;
    otherMorsel = other;
    return *this >> otherMorsel;
  }
  Morsel operator>>(const Morsel& other)
  {
    Morsel result(*this);
    result.bs = bs >> other.asInt();
    return result;
  }
  size_t hashVal()
  {
    //size_t seed = boost::hash_value(bs.size());
    //std::vector< blocks(bs.num_blocks());
    //boost::hash_range(seed, blocks.begin(), blocks.end());
    //return seed;
    return asInt();
  }
  // TODO: Implement multiplication
  Morsel operator*(const Morsel& other)
  {
    Morsel result(*this);
    return result;
  }
  friend Morsel operator*(int lhs, const Morsel& other)
  {
    Morsel lhsMorsel(lhs);
    return lhsMorsel*other;
  }
  Morsel& operator<<=(uint64_t in)
  {
    bs = bs << in;
    return *this;
  }
  bool operator!=(const Morsel& other)
  {
    return !(*this == other);
  }
  bool operator!=(unsigned long long int in)
  {
    Morsel other(in);
    return !(*this == other);
  }
  // TODO: Implement floats
  float asFloat()
  {
    float result = 3.14;
    return result;
  }
  char asChar()
  {
    char result = bs.to_ulong() & 0xFF;
    return result;
  }
/*
  operator bool()
  {
    Morsel zero(0);
    return !(*this == zero);
  }
*/
};
#endif
