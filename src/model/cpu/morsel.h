#ifndef MORSEL_H
#define MORSEL_H
#include <boost/dynamic_bitset.hpp>
using namespace std;
using namespace boost;
class Morsel 
{
  private:
    dynamic_bitset<> bs;
  public:
  Morsel() : bs(64,0ul) {}
  Morsel operator+(Morsel& other) 
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
  Morsel& operator=(int in)
  {
    //bs = in;
    while (in != 0) {
      bs.push_back( in & 1 );
      in >>= 1;
    }
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
  unsigned int size() 
  {
    return bs.size();
  }
};

template <>
struct hash<Morsel>
{
  size_t operator()(const Morsel& in) const
  {
    return (hash<boost::dynamic_bitset>(in.bs));
  }
};
#endif
