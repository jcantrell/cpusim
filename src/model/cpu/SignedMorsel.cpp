#include "SignedMorsel.h"
  size_t SignedMorsel::count()
  {
    return bs.count();
  }
  SignedMorsel::SignedMorsel(dynamic_bitset<> in)
  {
    bs = in;
  }
  SignedMorsel::SignedMorsel(unsigned long long int in)
  {
    *this = in;
  }
  SignedMorsel::SignedMorsel(const UnsignedMorsel in)
  {
    UnsignedMorsel other(in);
    SignedMorsel res(0);
    res.resize(other.size());
    
    while (other != 0)
    {
      res = res * 2;
      other = other / 2;
    }
    *this = res;
  }
  SignedMorsel SignedMorsel::operator+(const SignedMorsel& other) 
  {
    dynamic_bitset<> out;
    bool a, b, carry;
    int i;
    for (i=0, carry=false;static_cast<unsigned>(i)<other.size() && static_cast<unsigned>(i)<bs.size();i++) {
      a = bs[static_cast<unsigned>(i)];
      b = other.bs[static_cast<unsigned>(i)];
      out.push_back( a xor b xor carry );
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
    }
    for (;static_cast<unsigned>(i)<other.bs.size();i++) {
      a = false;
      b = other.bs[static_cast<unsigned>(i)];
      out.push_back( a xor b xor carry );
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
    }
    for (;static_cast<unsigned>(i)<static_cast<unsigned>(bs.size());i++) {
      a = bs[static_cast<unsigned>(i)];
      b = false;
      out.push_back( a xor b xor carry );
      carry = ( (b&&carry) || (a&&carry) || (a&&b) );
    }
		if (carry)
			out.push_back(carry);
    
    return SignedMorsel(out);
  }
  SignedMorsel SignedMorsel::operator+(int rhs)
  {
    SignedMorsel rhs_morsel(static_cast<unsigned int>(rhs));
    return *this + rhs_morsel;
  }
  SignedMorsel SignedMorsel::operator-(const SignedMorsel& other)
  {
		SignedMorsel lhs(*this);
		SignedMorsel rhs(other);
		if (rhs.bs.size() < lhs.bs.size())
			rhs.bs.resize(lhs.bs.size());
		if (lhs.bs.size() < rhs.bs.size())
			lhs.bs.resize(rhs.bs.size());
		rhs.bs.flip();
    rhs = rhs + 1;
		SignedMorsel result;
		result = lhs + rhs;
		result.bs.resize(lhs.size());
		return result;
  }
  SignedMorsel operator-(int lhsInt, const SignedMorsel& other)
  {
    SignedMorsel lhs(static_cast<unsigned int>(lhsInt));
    return lhs - other;
  }
  SignedMorsel SignedMorsel::operator-=(const SignedMorsel& other)
  {
    *this = *this - other;
    return *this;
  }
  SignedMorsel& SignedMorsel::operator++(int)
  {
    *this = (*this) + 1;
    return *this;
  }
  SignedMorsel& SignedMorsel::operator=(unsigned long long int in)
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
  SignedMorsel& SignedMorsel::operator=(const SignedMorsel& other)
  {
    bs = other.bs;
    return *this;
  }
  string SignedMorsel::asString() const 
  {
    SignedMorsel copy(*this);
    SignedMorsel reversed;
    reversed = 0;
    for (int count=static_cast<int>(copy.size());count != 0;count--)
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
    for (int count=static_cast<int>(reversed.size());count != 0;count-=8)
    {
      SignedMorsel chunk(0);

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
  std::ostream& operator<<( std::ostream& stream, const SignedMorsel& addr ) 
  {
    stream << addr.asString();
    return stream;
  }
  bool SignedMorsel::operator<(const SignedMorsel other) const
  {
    int this_index = bs.size()-1;
    int other_index = other.bs.size()-1;
    for (;this_index > other_index; this_index--)
    {
      if (bs[static_cast<unsigned int>(this_index)] == 1)
        {
          return false;
        }
    }
    for (;other_index > this_index; other_index--)
    {
      if (other.bs[static_cast<unsigned int>(other_index)] == 1)
        { 
          return true;
        }
    }
    for (;this_index >= 0 && other_index >= 0; this_index--, other_index--)
    {
      if ((bs[static_cast<unsigned int>(this_index)] != other.bs[static_cast<unsigned int>(other_index)]))
      {
        if (bs[static_cast<unsigned int>(this_index)]==0)
        {
          return true;
        } else {
          return false;
        }
      } 
    }
    return false;
  }
  bool SignedMorsel::operator<(int other)
  {
    SignedMorsel otherSignedMorsel;
    otherSignedMorsel = static_cast<unsigned int>(other);
    return *this < otherSignedMorsel;
  }
  bool SignedMorsel::operator<=(const SignedMorsel& other)
  {
    return *this < other || *this == other; 
  }
  bool SignedMorsel::operator<=(int other)
  {
    return *this < other || *this == other;
  }
  bool operator<=(int lhs, const SignedMorsel& rhs)
  {
    SignedMorsel lhsSignedMorsel(static_cast<unsigned int>(lhs));
    return lhsSignedMorsel <= rhs;
  }
  bool SignedMorsel::operator>(int other)
  {
    SignedMorsel otherSignedMorsel;
    otherSignedMorsel = static_cast<unsigned int>(other);
    return *this > otherSignedMorsel;
  }
  bool operator>(int lhs, SignedMorsel rhs)
  {
    SignedMorsel lhsSignedMorsel;
    lhsSignedMorsel = static_cast<unsigned int>(lhs);
    return lhsSignedMorsel > rhs;
  }
  bool SignedMorsel::operator>(SignedMorsel other)
  {
    return !((*this < other) || (*this == other));
  }
  bool SignedMorsel::operator>=(const SignedMorsel& other)
  {
    return *this > other || *this == other;
  }
  SignedMorsel SignedMorsel::operator/(const SignedMorsel& other)
  {
    SignedMorsel dividend(*this);
    SignedMorsel divisor(other);
    SignedMorsel quotient(0);
    SignedMorsel remainder(0);
    SignedMorsel radix(2);

    if (divisor > dividend) return SignedMorsel(0);
    if (divisor == dividend) return SignedMorsel(1);
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
      remainder = remainder * radix + SignedMorsel(dividend.bs[static_cast<unsigned int>(i)]);

      //push(remainder/divisor);
      quotient.resize(quotient.size()+1);
      quotient = (quotient << 1) | (remainder/divisor);
      
      remainder = remainder%divisor;
    }
    return quotient;
/*
    SignedMorsel numerator(*this);
    SignedMorsel quotient;
    quotient = 0;
    while (numerator > other || numerator == other)
    {
      numerator = numerator - other;
      quotient = quotient + 1;
    }
    return quotient;
*/
  }
  SignedMorsel SignedMorsel::operator%(const SignedMorsel& other)
  {
    SignedMorsel numerator(*this);
    SignedMorsel quotient( (*this) / other );
    SignedMorsel remainder;
    remainder = (*this) - (quotient * other);
    return remainder;
  }
  unsigned int SignedMorsel::size() const
  {
    return bs.size();
  }
  bool SignedMorsel::operator==(SignedMorsel other) const 
  { 
    SignedMorsel lhs(*this);
    SignedMorsel rhs(other);
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    return lhs.bs==rhs.bs; 
  }
  bool SignedMorsel::operator==(int other) const {
    SignedMorsel result;
    result = static_cast<unsigned int>(other);
    return *this == result;
  }
  unsigned int SignedMorsel::asInt() const
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
  SignedMorsel SignedMorsel::operator&(const SignedMorsel& other)
  {
    SignedMorsel lhs(*this);
    SignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    SignedMorsel result;
    result.bs = lhs.bs & rhs.bs;
    return result;
  }
  SignedMorsel SignedMorsel::operator&(int otherInt)
  {
    SignedMorsel other(static_cast<unsigned int>(otherInt));
    return (*this) & other;
  }
  SignedMorsel SignedMorsel::operator|(const SignedMorsel& other)
  {
    SignedMorsel lhs(*this);
    SignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    SignedMorsel result;
    result.bs = lhs.bs | rhs.bs;
    return result;
  }
  SignedMorsel SignedMorsel::operator|=(const SignedMorsel& other)
  {
    SignedMorsel result;
    result = *this | other;
    *this = result;
    return (*this);
  }
  SignedMorsel SignedMorsel::operator~()
  {
    SignedMorsel result(*this);
    result.bs = ~bs;
    return result;
  }
  SignedMorsel SignedMorsel::operator-()
  {
    SignedMorsel result(0);
    result = result - *this;
    return result;
  }
  SignedMorsel SignedMorsel::operator^(const SignedMorsel& other) const
  {
    SignedMorsel lhs(*this);
    SignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    SignedMorsel result;
    result.bs = lhs.bs ^ rhs.bs;
    return result;
  }
  SignedMorsel SignedMorsel::operator<<(const SignedMorsel& other)
  {
    bool debugFlag;
    if (*this == SignedMorsel(0xa5))
      debugFlag = true;

    SignedMorsel result(*this);
    SignedMorsel shift(other);
    while (!(shift == 0))
    {
      result.bs = result.bs << 1;
      shift = shift - 1;
    }
    return result;
  }
  SignedMorsel SignedMorsel::operator<<(int other)
  {
    SignedMorsel otherSignedMorsel;
    otherSignedMorsel = static_cast<unsigned int>(other);
    return *this << otherSignedMorsel;
  }
  SignedMorsel SignedMorsel::pb(int other)
  {
    resize(size() + static_cast<unsigned>(other));
    *this = *this << other;
    return *this;
  }
  SignedMorsel SignedMorsel::operator>>(int other)
  {
    SignedMorsel otherSignedMorsel;
    otherSignedMorsel = static_cast<unsigned int>(other);
    return *this >> otherSignedMorsel;
  }
  SignedMorsel SignedMorsel::operator>>(const SignedMorsel& other)
  {
    SignedMorsel result(*this);
    SignedMorsel decrementor(other);
    while (!(decrementor == 0))
    {
      result.bs = result.bs >> 1;
      decrementor = decrementor - 1;
    }
    return result;
  }
  SignedMorsel SignedMorsel::operator*(const SignedMorsel& other)
  {
    SignedMorsel accumulator(0);
    SignedMorsel decrementor(other);
    SignedMorsel zero(0);
    while (!(zero==decrementor))
    {
      accumulator = accumulator + (*this);
      decrementor = decrementor - 1;
    }
    return accumulator;
  }
  SignedMorsel operator*(int lhs, const SignedMorsel& other)
  {
    SignedMorsel lhsSignedMorsel(static_cast<unsigned int>(lhs));
    return lhsSignedMorsel*other;
  }
  SignedMorsel& SignedMorsel::operator<<=(uint64_t in)
  {
    bs = bs << in;
    return *this;
  }
  bool SignedMorsel::operator!=(const SignedMorsel& other)
  {
    return !(*this == other);
  }
  bool SignedMorsel::operator!=(unsigned long long int in)
  {
    SignedMorsel other(in);
    return !(*this == other);
  }
  float SignedMorsel::asFloat() const
  {
    SignedMorsel a(*this);
    union myfloat
    {
      float fl;
      unsigned long long in;
    };

    unsigned int count = 8;
    union myfloat temp;
    union myfloat result; result.in = 0;
    while (a != 0 && count != 0)
    {
      temp.in <<= 8;
      temp.in = temp.in | (static_cast<unsigned char>(a.asChar()));
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
  unsigned char SignedMorsel::asChar()
  {
    unsigned char result = bs.to_ulong() & 0xFF;
    return result;
  }
SignedMorsel& SignedMorsel::resize(unsigned int newsize)
  {
    bs.resize(newsize);
    return *this;
  }
