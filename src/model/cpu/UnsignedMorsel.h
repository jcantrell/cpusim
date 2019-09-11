#ifndef UNSIGNEDMORSEL_H
#define UNSIGNEDMORSEL_H
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include <iostream>
class Address;
using namespace std;
using namespace boost;
class UnsignedMorsel 
{
  private:
    dynamic_bitset<> bs;
  public:
  UnsignedMorsel() : bs(64,0ul) {}
  size_t count()
  {
    return bs.count();
  }
  UnsignedMorsel(dynamic_bitset<> in)
  {
    bs = in;
  }
  UnsignedMorsel(unsigned long long int in)
  {
    *this = in;
  }
  UnsignedMorsel operator+(const UnsignedMorsel& other) 
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
    
    return UnsignedMorsel(out);
  }
  UnsignedMorsel operator+(int rhs)
  {
    UnsignedMorsel rhs_morsel(rhs);
    return *this + rhs_morsel;
  }
  UnsignedMorsel operator-(const UnsignedMorsel& other)
  {
		UnsignedMorsel lhs(*this);
		UnsignedMorsel rhs(other);
		if (rhs.bs.size() < lhs.bs.size())
			rhs.bs.resize(lhs.bs.size());
		if (lhs.bs.size() < rhs.bs.size())
			lhs.bs.resize(rhs.bs.size());
		rhs.bs.flip();
    rhs = rhs + 1;
		UnsignedMorsel result;
		result = lhs + rhs;
		result.bs.resize(lhs.size());
		return result;
  }
  friend UnsignedMorsel operator-(int lhsInt, const UnsignedMorsel& other)
  {
    UnsignedMorsel lhs(lhsInt);
    return lhs - other;
  }
  UnsignedMorsel operator-=(const UnsignedMorsel& other)
  {
    *this = *this - other;
    return *this;
  }
  UnsignedMorsel& operator++(int)
  {
    *this = (*this) + 1;
    return *this;
  }
  UnsignedMorsel& operator=(unsigned long long int in)
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
  UnsignedMorsel& operator=(const UnsignedMorsel& other)
  {
    bs = other.bs;
    return *this;
  }
  string asString() const 
  {
    UnsignedMorsel copy(*this);
    UnsignedMorsel reversed;
    reversed = 0;
    for (int count=copy.size();count != 0;count--)
    {
      reversed <<= 1;
      reversed = reversed | (copy & 1);
      copy = copy >> 1;
    }
    int tCount = 7;
    while (reversed.bs.size() % 8 != 0 && (tCount >=0))
    {
      reversed.bs.resize(reversed.bs.size()+1);
      reversed.bs = reversed.bs<<1;
      tCount--;
    }
    stringstream out;
    char const hex[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c',
      'd','e','f'};
    for (int count=reversed.size();count != 0;count-=8)
    {
      UnsignedMorsel chunk(0);

      for (int i=0;i<8;i++)
      {
        chunk = (chunk<<1) | (reversed & 0x1);
        reversed = (reversed >> 1);
      }
      out << hex[(chunk.asChar() & 0xF0) >> 4] 
          << hex[chunk.asChar() & 0xF];
    }
      return out.str();
  }
  friend std::ostream& operator<<( std::ostream& stream, const UnsignedMorsel& addr ) 
  {
    stream << addr.asString();
    return stream;
  }
  bool operator<(const UnsignedMorsel other) const
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
    UnsignedMorsel otherUnsignedMorsel;
    otherUnsignedMorsel = other;
    return *this < otherUnsignedMorsel;
  }
  bool operator<=(const UnsignedMorsel& other)
  {
    return *this < other || *this == other; 
  }
  bool operator<=(int other)
  {
    return *this < other || *this == other;
  }
  friend bool operator<=(int lhs, const UnsignedMorsel& rhs)
  {
    UnsignedMorsel lhsUnsignedMorsel(lhs);
    return lhsUnsignedMorsel <= rhs;
  }
  bool operator>(int other)
  {
    UnsignedMorsel otherUnsignedMorsel;
    otherUnsignedMorsel = other;
    return *this > otherUnsignedMorsel;
  }
  friend bool operator>(int lhs, UnsignedMorsel rhs)
  {
    UnsignedMorsel lhsUnsignedMorsel;
    lhsUnsignedMorsel = lhs;
    return lhsUnsignedMorsel > rhs;
  }
  bool operator>(UnsignedMorsel other)
  {
    return !((*this < other) || (*this == other));
  }
  bool operator>=(const UnsignedMorsel& other)
  {
    return *this > other || *this == other;
  }
  UnsignedMorsel operator/(const UnsignedMorsel& other)
  {
    UnsignedMorsel dividend(*this);
    UnsignedMorsel divisor(other);
    UnsignedMorsel quotient(0);
    UnsignedMorsel remainder(0);
    UnsignedMorsel radix(2);

    if (divisor > dividend) return UnsignedMorsel(0);
    if (divisor == dividend) return UnsignedMorsel(1);
    if (dividend <= divisor * radix)
    {
      while (dividend > divisor || dividend == divisor)
      {
        dividend = dividend - divisor;
        quotient = quotient + 1;
      }
      return quotient;
    }

    for (int i=dividend.bs.size()-1;i>=0;i--)
    {
      remainder = remainder * radix + UnsignedMorsel(dividend.bs[i]);

      //push(remainder/divisor);
      quotient.resize(quotient.size()+1);
      quotient = (quotient << 1) | (remainder/divisor);
      
      remainder = remainder%divisor;
    }
    return quotient;
/*
    UnsignedMorsel numerator(*this);
    UnsignedMorsel quotient;
    quotient = 0;
    while (numerator > other || numerator == other)
    {
      numerator = numerator - other;
      quotient = quotient + 1;
    }
    return quotient;
*/
  }
  UnsignedMorsel operator%(const UnsignedMorsel& other)
  {
    UnsignedMorsel numerator(*this);
    UnsignedMorsel quotient( (*this) / other );
    UnsignedMorsel remainder;
    remainder = (*this) - (quotient * other);
    return remainder;
  }
  unsigned int size() const
  {
    return bs.size();
  }
  bool operator==(UnsignedMorsel other) const 
  { 
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    return lhs.bs==rhs.bs; 
  }
  bool operator==(int other) const {
    UnsignedMorsel result;
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
  UnsignedMorsel operator&(const UnsignedMorsel& other)
  {
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    UnsignedMorsel result;
    result.bs = lhs.bs & rhs.bs;
    return result;
  }
  UnsignedMorsel operator&(int otherInt)
  {
    UnsignedMorsel other(otherInt);
    return (*this) & other;
  }
  UnsignedMorsel operator|(const UnsignedMorsel& other)
  {
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    UnsignedMorsel result;
    result.bs = lhs.bs | rhs.bs;
    return result;
  }
  UnsignedMorsel operator|=(const UnsignedMorsel& other)
  {
    UnsignedMorsel result;
    result = *this | other;
    *this = result;
    return (*this);
  }
  UnsignedMorsel operator~()
  {
    UnsignedMorsel result(*this);
    result.bs = ~bs;
    return result;
  }
  UnsignedMorsel operator^(const UnsignedMorsel& other) const
  {
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    UnsignedMorsel result;
    result.bs = lhs.bs ^ rhs.bs;
    return result;
  }
  UnsignedMorsel operator<<(const UnsignedMorsel& other)
  {
    bool debugFlag;
    if (*this == UnsignedMorsel(0xa5))
      debugFlag = true;

    UnsignedMorsel result(*this);
    UnsignedMorsel shift(other);
    while (!(shift == 0))
    {
      result.bs = result.bs << 1;
      shift = shift - 1;
    }
    return result;
  }
  UnsignedMorsel operator<<(int other)
  {
    UnsignedMorsel otherUnsignedMorsel;
    otherUnsignedMorsel = other;
    return *this << otherUnsignedMorsel;
  }
  UnsignedMorsel operator>>(int other)
  {
    UnsignedMorsel otherUnsignedMorsel;
    otherUnsignedMorsel = other;
    return *this >> otherUnsignedMorsel;
  }
  UnsignedMorsel operator>>(const UnsignedMorsel& other)
  {
    UnsignedMorsel result(*this);
    UnsignedMorsel decrementor(other);
    while (!(decrementor == 0))
    {
      result.bs = result.bs >> 1;
      decrementor = decrementor - 1;
    }
    return result;
  }
  UnsignedMorsel operator*(const UnsignedMorsel& other)
  {
    UnsignedMorsel accumulator(0);
    UnsignedMorsel decrementor(other);
    UnsignedMorsel zero(0);
    while (!(zero==decrementor))
    {
      accumulator = accumulator + (*this);
      decrementor = decrementor - 1;
    }
    return accumulator;
  }
  friend UnsignedMorsel operator*(int lhs, const UnsignedMorsel& other)
  {
    UnsignedMorsel lhsUnsignedMorsel(lhs);
    return lhsUnsignedMorsel*other;
  }
  UnsignedMorsel& operator<<=(uint64_t in)
  {
    bs = bs << in;
    return *this;
  }
  bool operator!=(const UnsignedMorsel& other)
  {
    return !(*this == other);
  }
  bool operator!=(unsigned long long int in)
  {
    UnsignedMorsel other(in);
    return !(*this == other);
  }
  float asFloat() const
  {
    UnsignedMorsel a(*this);
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
  UnsignedMorsel& resize(unsigned int newsize)
  {
    bs.resize(newsize);
    return *this;
  }
};
#endif
