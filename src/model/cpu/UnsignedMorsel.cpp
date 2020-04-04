#include "UnsignedMorsel.h"
  size_t UnsignedMorsel::count()
  {
    return bs.count();
  }
  UnsignedMorsel::UnsignedMorsel(dynamic_bitset<> in)
  {
    bs = in;
  }
  UnsignedMorsel::UnsignedMorsel(unsigned long long int in)
  {
    *this = in;
  }
  UnsignedMorsel::UnsignedMorsel(const SignedMorsel in)
  {
    SignedMorsel other(in);
    UnsignedMorsel rev(0);
    rev.resize(other.size());
    
    int c=0;
    while (other != 0)
    {
      c++;
      rev = ((rev.pb(1)) | (((other & 1) == 1) ? 1 : 0));
      other = (other >> 1);
    }
    UnsignedMorsel res(0);
    res.resize(other.size());
    for (;c>0;c--)
    {
      res = ((res.pb(1)) | (((rev & 1) == 1) ? 1 : 0));
      rev = (rev >> 1);
    }
    *this = res;
  }
  UnsignedMorsel& UnsignedMorsel::operator+=(int in)
  {
    *this = *this + static_cast<unsigned int>(in);
    return *this;
  }
/*
  UnsignedMorsel::UnsignedMorsel(SignedMorsel in)
  {
*/
/*
    UnsignedMorsel other(in);
    SignedMorsel res(0);
    res.resize(other.size());
    
    while (other != 0)
    {
      res = res + 1;
      other = other - 1;
    }
    *this = res;
*/
/*
    UnsignedMorsel other(in);
    SignedMorsel res(0);
    res.resize(other.size());
    
    while (other != 0)
    {
      cout << "STEP 4 before: " << res << endl;
      res = res << SignedMorsel(1);
      cout << "STEP 4 after: " << res << endl;
      UnsignedMorsel t;
      cout << "STEP 4.1: " << other << endl;
      t = (other&1);
      cout << "STEP 4.2: " << t << endl;
      res = res | t;
      cout << "STEP 4.3: " << res << endl;
      cout << "STEP 5 other: " << other << endl;
      other = other >> 1;
      cout << "STEP 5 after: " << other << endl;
      cout << "STEP 5 comp: " << (other != 0) << endl;
    }
  cout << "CAST 6 FIXRX" << endl;
    *this = res;
  cout << "CAST 7 FIXRX" << endl;
  }
*/
  UnsignedMorsel UnsignedMorsel::operator+(const UnsignedMorsel& other) 
  {
    dynamic_bitset<> out;
    bool a, b, carry;
    unsigned int i;
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
  UnsignedMorsel UnsignedMorsel::operator+(unsigned int rhs)
  {
    UnsignedMorsel rhs_morsel(rhs);
    return *this + rhs_morsel;
  }
  UnsignedMorsel UnsignedMorsel::operator-(const UnsignedMorsel& other)
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
    //cout << "lhs: " << (*this) << " rhs: " << other << endl;
    //cout << lhs << " " << rhs;
		result = lhs + rhs;
    //cout << result << endl;
		result.bs.resize(lhs.size());
		return result;
  }
  UnsignedMorsel operator-(unsigned int lhsInt, const UnsignedMorsel& other)
  {
    UnsignedMorsel lhs(lhsInt);
    return lhs - other;
  }
  UnsignedMorsel UnsignedMorsel::operator-=(const UnsignedMorsel& other)
  {
    *this = *this - other;
    return *this;
  }
  UnsignedMorsel& UnsignedMorsel::operator++(int)
  {
    *this = (*this) + 1;
    return *this;
  }
  UnsignedMorsel& UnsignedMorsel::operator=(unsigned long long int in)
  {
    unsigned long long int in_copy = in;
    unsigned size = 0;
    unsigned sz = bs.size();
    do
    {
      bs.push_back( in & 1 );
      in >>= 1;
      size++;
    } while (in != 0);
    bs = (bs >> (bs.size() - size)) ;
    //cout << "after = " << in_copy << " is " << bs << endl;
    bs.resize(size);
    return *this;
  }
  UnsignedMorsel& UnsignedMorsel::operator=(const UnsignedMorsel& other)
  {
    bs = other.bs;
    return *this;
  }
  string UnsignedMorsel::asString() const 
  {
    UnsignedMorsel copy(*this);
    UnsignedMorsel reversed;
    reversed = 0;
    for (unsigned int count=copy.size();count != 0;count--)
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
    for (unsigned int count=reversed.size();count != 0;count-=8)
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
  std::ostream& operator<<( std::ostream& stream, const UnsignedMorsel& addr ) 
  {
    stream << addr.asString();
    return stream;
  }
  bool UnsignedMorsel::operator<(const UnsignedMorsel other) const
  {
    int this_index = bs.size()-1;
    int other_index = other.bs.size()-1;
    for (;this_index > other_index; this_index--)
    {
      if (bs[static_cast<unsigned>(this_index)] == 1)
        {
          return false;
        }
    }
    for (;other_index > this_index; other_index--)
    {
      if (other.bs[static_cast<unsigned>(other_index)] == 1)
        { 
          return true;
        }
    }
    for (;this_index >= 0 && other_index >= 0; this_index--, other_index--)
    {
      if ((bs[static_cast<unsigned>(this_index)] != other.bs[static_cast<unsigned>(other_index)]))
      {
        if (bs[static_cast<unsigned>(this_index)]==0)
        {
          return true;
        } else {
          return false;
        }
      } 
    }
    return false;
  }
  bool UnsignedMorsel::operator<(unsigned int other)
  {
    UnsignedMorsel otherUnsignedMorsel;
    otherUnsignedMorsel = other;
    return *this < otherUnsignedMorsel;
  }
  bool UnsignedMorsel::operator<=(const UnsignedMorsel& other)
  {
    return *this < other || *this == other; 
  }
  bool UnsignedMorsel::operator<=(unsigned int other)
  {
    return *this < other || *this == other;
  }
  bool operator<=(unsigned int lhs, const UnsignedMorsel& rhs)
  {
    UnsignedMorsel lhsUnsignedMorsel(lhs);
    return lhsUnsignedMorsel <= rhs;
  }
  bool UnsignedMorsel::operator>(unsigned int other)
  {
    UnsignedMorsel otherUnsignedMorsel;
    otherUnsignedMorsel = other;
    return *this > otherUnsignedMorsel;
  }
  bool operator>(unsigned int lhs, UnsignedMorsel rhs)
  {
    UnsignedMorsel lhsUnsignedMorsel;
    lhsUnsignedMorsel = lhs;
    return lhsUnsignedMorsel > rhs;
  }
  bool UnsignedMorsel::operator>(UnsignedMorsel other)
  {
    return !((*this < other) || (*this == other));
  }
  bool UnsignedMorsel::operator>=(const UnsignedMorsel& other)
  {
    return *this > other || *this == other;
  }
  UnsignedMorsel UnsignedMorsel::operator/(const UnsignedMorsel& other)
  {
    //cout << "CALLED div: " << asString() << " , " << other << endl;

    UnsignedMorsel dividend(*this);
    UnsignedMorsel divisor(other);
    UnsignedMorsel quotient(0);
    UnsignedMorsel remainder(0);
    UnsignedMorsel radix(2);
    //cout << "dividend: " << dividend.asString() << " ,divisor: " << divisor.asString() << " cmd: " << (divisor > dividend) << " dividend bs: " << dividend.bs << endl;

    if (divisor > dividend) {
    //cout << "result: " << 0 << endl;
return UnsignedMorsel(0);
}
    if (divisor == dividend){
    //cout << "result: " << 1 << endl;
 return UnsignedMorsel(1);
}
    //cout << "dividend: " << dividend << " div*radix: " << divisor*radix << endl << " cmp <=: " << (dividend <= divisor*radix) << endl;
    if (dividend <= divisor * radix)
    {
      while (dividend > divisor || dividend == divisor)
      {
        dividend = dividend - divisor;
        quotient = quotient + 1;
      }
      //cout << "returning div: " << *this << " , " << other << endl;
      //cout << "result: " << quotient << endl;
      return quotient;
    }

    for (int i=dividend.bs.size()-1;i>=0;i--)
    {
      //cout << "rem: " << remainder << " "
       //    << "radix: " << radix << " i: " << i << " " << UnsignedMorsel(dividend.bs[static_cast<unsigned>(i)]) << " " 
        //   << "quotient: " << quotient << " " 
      //     << "dividend: " << dividend.bs
           //<< endl;
      remainder = remainder * radix + UnsignedMorsel(dividend.bs[static_cast<unsigned>(i)]);

      quotient.resize(quotient.size()+1);
      quotient = (quotient << 1) | (remainder/divisor);
      
      remainder = remainder%divisor;
    }
/*
    cout << endl;
      cout << "after rem: " << remainder << " "
           << "after radix: " << radix << " "
           << "after quotient: " << quotient
           << endl;
    cout << "result: " << quotient << endl;
    //cout << "calling div: " << *this << " , " << other << endl;
*/
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
  UnsignedMorsel UnsignedMorsel::operator%(const UnsignedMorsel& other)
  {
    UnsignedMorsel numerator(*this);
    UnsignedMorsel quotient( (*this) / other );
    UnsignedMorsel remainder;
    remainder = (*this) - (quotient * other);
    return remainder;
  }
  unsigned int UnsignedMorsel::size() const
  {
    return bs.size();
  }
  bool UnsignedMorsel::operator==(UnsignedMorsel other) const 
  { 
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    return lhs.bs==rhs.bs; 
  }
  bool UnsignedMorsel::operator==(unsigned int other) const {
    UnsignedMorsel result;
    result = other;
    return *this == result;
  }
  unsigned int UnsignedMorsel::asInt() const
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
  UnsignedMorsel UnsignedMorsel::operator&(const UnsignedMorsel& other)
  {
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    UnsignedMorsel result;
    result.bs = lhs.bs & rhs.bs;
    return result;
  }
  UnsignedMorsel UnsignedMorsel::operator&(unsigned int otherInt)
  {
    UnsignedMorsel other(otherInt);
    return (*this) & other;
  }
  UnsignedMorsel UnsignedMorsel::operator|(const UnsignedMorsel& other)
  {
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    UnsignedMorsel result;
    result.bs = lhs.bs | rhs.bs;
    return result;
  }
  UnsignedMorsel UnsignedMorsel::operator^(const unsigned int& otherInt)
  {
    UnsignedMorsel other(otherInt);
    return (*this) ^ other;
  }
  UnsignedMorsel UnsignedMorsel::operator^(const UnsignedMorsel& other)
  {
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    UnsignedMorsel result;
    result.bs = lhs.bs ^ rhs.bs;
    return result;
  }
  UnsignedMorsel UnsignedMorsel::operator|=(const UnsignedMorsel& other)
  {
    UnsignedMorsel result;
    result = *this | other;
    *this = result;
    return (*this);
  }
  UnsignedMorsel UnsignedMorsel::operator~()
  {
    UnsignedMorsel result(*this);
    result.bs = ~bs;
    return result;
  }
  UnsignedMorsel UnsignedMorsel::operator^(const UnsignedMorsel& other) const
  {
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (lhs.bs.size() < rhs.bs.size()) lhs.bs.resize(rhs.bs.size());
    if (rhs.bs.size() < lhs.bs.size()) rhs.bs.resize(lhs.bs.size());
    UnsignedMorsel result;
    result.bs = lhs.bs ^ rhs.bs;
    return result;
  }
  UnsignedMorsel UnsignedMorsel::operator<<(const UnsignedMorsel& other)
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
  UnsignedMorsel UnsignedMorsel::operator<<(unsigned int other)
  {
    UnsignedMorsel otherUnsignedMorsel;
    otherUnsignedMorsel = other;
    return *this << otherUnsignedMorsel;
  }
  UnsignedMorsel UnsignedMorsel::pb(unsigned int other)
  {
    resize(size() + other);
    *this = *this << other;
    return *this;
  }
  UnsignedMorsel UnsignedMorsel::operator>>(unsigned int other)
  {
    UnsignedMorsel otherUnsignedMorsel;
    otherUnsignedMorsel = other;
    return *this >> otherUnsignedMorsel;
  }
  UnsignedMorsel UnsignedMorsel::operator>>(const UnsignedMorsel& other)
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
  UnsignedMorsel UnsignedMorsel::operator*(const UnsignedMorsel& other)
  {
    UnsignedMorsel lhs(*this);
    UnsignedMorsel rhs(other);
    if (lhs == 0 || rhs == 0) return UnsignedMorsel(0);
    if (lhs == 1) return rhs;
    if (rhs == 1) return lhs;
    UnsignedMorsel accum(0);
    for (unsigned i=0;i<other.size();i++) {
      accum = accum + (lhs*(rhs&1u)).pb(i);
      rhs = rhs>>1;
    }
    return accum;
  }
  UnsignedMorsel operator*(unsigned int lhs, const UnsignedMorsel& other)
  {
    UnsignedMorsel lhsUnsignedMorsel(lhs);
    return lhsUnsignedMorsel*other;
  }
  UnsignedMorsel& UnsignedMorsel::operator<<=(uint64_t in)
  {
    bs = bs << in;
    return *this;
  }
  bool UnsignedMorsel::operator!=(const UnsignedMorsel& other)
  {
    return !(*this == other);
  }
  bool UnsignedMorsel::operator!=(unsigned long long int in)
  {
    UnsignedMorsel other(in);
    return !(*this == other);
  }
  float UnsignedMorsel::asFloat() const
  {
    UnsignedMorsel a(*this);
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
      temp.in = temp.in | a.asChar();
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
  unsigned char UnsignedMorsel::asChar()
  {
    unsigned char result = bs.to_ulong() & 0xFF;
    return result;
  }
  UnsignedMorsel& UnsignedMorsel::resize(unsigned int newsize)
  {
    bs.resize(newsize);
    return *this;
  }
size_t hash_value(const UnsignedMorsel& in)
{
  size_t hash = boost::hash_value(in.asInt());
  return hash;
}
