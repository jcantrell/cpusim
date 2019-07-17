#ifndef MORSEL_H
#define MORSEL_H
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
//#include "Address.h"
class Address;
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
    dynamic_bitset<> out;
    bool a, b, carry;
    int i;
    for (i=0, carry=false;i<other.size() && i<bs.size();i++) {
      a = bs[i];
      b = other.bs[i];
      out.push_back( a xor b xor carry );
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
    }
    for (;i<other.bs.size();i++) {
      a = false;
      b = other.bs[i];
      out.push_back( a xor b xor carry );
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
    }
    for (;i<bs.size();i++) {
      a = bs[i];
      b = false;
      out.push_back( a xor b xor carry );
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
    }
		if (carry)
			out.push_back(carry);
    
    return Morsel(out);
  }
  Morsel operator+(int rhs)
  {
    Morsel rhs_morsel(rhs);
    return *this + rhs_morsel;
  }
  Morsel operator-(const Morsel& other)
  {
		Morsel lhs(*this);
		Morsel rhs(other);
		if (rhs.bs.size() < lhs.bs.size())
			rhs.bs.resize(lhs.bs.size());
		if (lhs.bs.size() < rhs.bs.size())
			lhs.bs.resize(rhs.bs.size());
		rhs.bs.flip();
    rhs = rhs + 1;
		Morsel result;
		result = lhs + rhs;
		result.bs.resize(lhs.size());
		return result;
  }
  friend Morsel operator-(int lhsInt, const Morsel& other)
  {
    Morsel lhs(lhsInt);
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
    unsigned long long int in_copy = in;
    unsigned size = 0;
    do
    {
      bs.push_back( in & 1 );
      in >>= 1;
      size++;
    } while (in != 0);
    bs = (bs >> (bs.size() - size)) ;
    bs.resize(size);
    return *this;
  }
  Morsel& operator=(const Morsel& other)
  {
    bs = other.bs;
    return *this;
  }
  string asString() const 
  {
    Morsel copy(*this);
    Morsel reversed;
    reversed = 0;
    while (copy != 0)
    {
      reversed <<= 1;
      reversed = reversed | (copy & 1);
      copy = copy >> 1;
    }
    while (reversed.bs.size() % 8 != 0)
    {
      reversed = reversed * 2;
    }
    stringstream out;
    for (int count=reversed.size();count != 0;count-=8)
    {
      Morsel chunk(0);

      for (int i=0;i<8;i++)
      {
        chunk = (chunk<<1) | (reversed & 0x1);
        reversed = (reversed >> 1);
      }
      out << hex << (int)chunk.asChar();
    }

//		string buffer;
//		to_string(bs, buffer);
//    return buffer;
      return out.str();
  }
  friend std::ostream& operator<<( std::ostream& stream, const Morsel& addr ) 
  {
    stream << addr.asString();
    return stream;
  }
  bool operator<(Morsel other)
  {
    int this_index = bs.size()-1;
    int other_index = other.bs.size()-1;
    for (;this_index > other_index; this_index--)
    {
      if (bs[this_index] == 1)
        {
          return false;
        }
    }
    for (;other_index > this_index; other_index--)
    {
      if (other.bs[other_index] == 1)
        { 
          return true;
        }
    }
    for (;this_index >= 0 && other_index >= 0; this_index--, other_index--)
    {
      if ((bs[this_index] != other.bs[other_index]))
      {
        if (bs[this_index]==0)
        {
          return true;
        } else {
          return false;
        }
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
    Morsel quotient( (*this) / other );
    Morsel remainder;
    remainder = (*this) - (quotient * other);
    return remainder;
  }
  unsigned int size() const
  {
    return bs.size();
  }
  bool operator==(Morsel other) const 
  { 
    Morsel lhs(*this);
    Morsel rhs(other);
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    return lhs.bs==rhs.bs; 
  }
  bool operator==(int other) const {
    Morsel result;
    result = other;
    return *this == result;
  }
  unsigned int asInt() const
  {
    dynamic_bitset<> copy = bs;
    unsigned int i = 0;
    while (copy.size() != 0)
    {
      i = (i << 1) | copy[copy.size()-1];
      copy.pop_back();
    }
    return i;
  }
  Morsel operator&(const Morsel& other)
  {
    Morsel lhs(*this);
    Morsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    Morsel result;
    result.bs = lhs.bs & rhs.bs;
    return result;
  }
  Morsel operator&(int otherInt)
  {
    Morsel other(otherInt);
    return (*this) & other;
  }
  Morsel operator|(const Morsel& other)
  {
    Morsel lhs(*this);
    Morsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    Morsel result;
    result.bs = lhs.bs | rhs.bs;
    return result;
  }
  Morsel operator|=(const Morsel& other)
  {
    Morsel result;
    result = *this | other;
    *this = result;
    return (*this);
  }
  Morsel operator~()
  {
    Morsel result(*this);
    result.bs = ~bs;
    return result;
  }
  Morsel operator^(const Morsel& other) const
  {
    Morsel lhs(*this);
    Morsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    Morsel result;
    result.bs = lhs.bs ^ rhs.bs;
    return result;
  }
  Morsel operator<<(const Morsel& other)
  {
    Morsel result(*this);
    Morsel shift(other);
    while (!(shift == 0))
    {
      result.bs = result.bs << 1;
      shift = shift - 1;
    }
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
    Morsel decrementor(other);
    while (!(decrementor == 0))
    {
      result.bs = result.bs >> 1;
      decrementor = decrementor - 1;
    }
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
  Morsel operator*(const Morsel& other)
  {
    Morsel accumulator(0);
    Morsel decrementor(other);
    Morsel zero(0);
    while (!(zero==decrementor))
    {
      accumulator = accumulator + (*this);
      decrementor = decrementor - 1;
    }
    return accumulator;
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
  float asFloat() const
  {
    Morsel a(*this);
    union myfloat
    {
      float fl;
      unsigned long long in;
    };

    unsigned int count = 8;
    union myfloat temp;
    union myfloat result = { .in=0 };
    while (a != 0 && count != 0)
    {
      temp.in <<= 8;
      temp.in = temp.in | ((unsigned char)a.asChar());
      a = a>>8;
      count--;
    }
    while (count != 8)
    {
      result.in <<= 8;
      result.in = result.in | (temp.in & 0xFF);
      temp.in >>= 8;
      count++;
    } 
    return result.fl;
  }
  unsigned char asChar()
  {
    unsigned char result = bs.to_ulong() & 0xFF;
    return result;
  }
};
#endif