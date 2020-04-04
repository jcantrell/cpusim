#include "SignedMorsel.h"
class UnsignedMorsel;
  size_t SignedMorsel::count()
  {
    return um.count();
  }
  SignedMorsel::SignedMorsel(dynamic_bitset<> in)
  {
    um = UnsignedMorsel(in);
  }
  SignedMorsel::SignedMorsel(unsigned long long int in)
  {
    *this = in;
  }
  SignedMorsel::SignedMorsel(const UnsignedMorsel in)
  {
    um = in;
  }
  SignedMorsel SignedMorsel::operator+(const SignedMorsel& other) 
  {
    return UnsignedMorsel(um+other.um);
  }
  SignedMorsel SignedMorsel::operator+(int rhs)
  {
    return *this + SignedMorsel(static_cast<unsigned int>(rhs));
  }
  SignedMorsel SignedMorsel::operator-(const SignedMorsel& other)
  {
    return SignedMorsel(um-other.um);
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
    um=in;
    return *this;
  }
  SignedMorsel& SignedMorsel::operator=(const SignedMorsel& other)
  {
    um = other.um;
    return *this;
  }
  UnsignedMorsel SignedMorsel::asUnsignedMorsel() { return um; }
  string SignedMorsel::asString() const 
  {
      return um.asString();
  }
  std::ostream& operator<<( std::ostream& stream, const SignedMorsel& addr ) 
  {
    stream << addr.asString();
    return stream;
  }
  bool SignedMorsel::operator<(const SignedMorsel other) const
  {
    bool sign_bit_l = ((SignedMorsel(*this)>>(size()-1ul)) == 1ul);
    bool sign_bit_r = ((SignedMorsel(other)>>(other.size()-1ul)) == 1ul);
    if (sign_bit_l == 0 && sign_bit_r == 0) return um<other.um;
    if (sign_bit_l == 1 && sign_bit_r == 0) return true;
    if (sign_bit_l == 0 && sign_bit_r == 1) return false;
    //if (sign_bit_l == 1 && sign_bit_r == 1)
    {
      UnsignedMorsel l = (~SignedMorsel(*this)+1).asUnsignedMorsel();
      UnsignedMorsel r = (~SignedMorsel(other)+1).asUnsignedMorsel();
      return l<r;
    } 
  }
  bool SignedMorsel::operator<(int other)
  {
    SignedMorsel otherSignedMorsel(static_cast<unsigned int>(other));
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
    return SignedMorsel(um/other.um);
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
    return um.size();
  }
  bool SignedMorsel::operator==(SignedMorsel other) const 
  { 
    return um==other.um;
  }
  bool SignedMorsel::operator==(int other) const {
    SignedMorsel result;
    result = static_cast<unsigned int>(other);
    return *this == result;
  }
  unsigned int SignedMorsel::asInt() const
  {
    return um.asInt();
  }
  SignedMorsel SignedMorsel::operator&(const SignedMorsel& other)
  {
    return um & other.um;
  }
  SignedMorsel SignedMorsel::operator&(int otherInt)
  {
    SignedMorsel other(static_cast<unsigned int>(otherInt));
    return (*this) & other;
  }
  SignedMorsel SignedMorsel::operator|(const SignedMorsel& other)
  {
    return SignedMorsel(um | other.um);
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
    return SignedMorsel(~um);
  }
  SignedMorsel SignedMorsel::operator-()
  {
    SignedMorsel result(0);
    result = result - *this;
    return result;
  }
  SignedMorsel SignedMorsel::operator^(const SignedMorsel& other) const
  {
    return SignedMorsel(um^other.um);
  }
  SignedMorsel SignedMorsel::operator<<(const SignedMorsel& other)
  {
    return SignedMorsel(um<<other.um);
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
    return SignedMorsel(um>>other.um);
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
    um = um << in;
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
    return um.asChar();
  }
SignedMorsel& SignedMorsel::resize(unsigned int newsize)
  {
    um.resize(newsize);
    return *this;
  }
