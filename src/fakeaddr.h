#ifndef FAKEADDR_H
#define FAKEADDR_H
#include "./fakemorsel.h"

class fakeaddr
{
  private:
    fakemorsel m;
  public:
    fakemorsel hashValue() const {return m;}
};

namespace std {
  template <>
  struct hash<fakeaddr>
  {
    size_t operator()(const fakeaddr& key) const
    {
      return (hash<fakemorsel>()(key.hashValue()));
    }
  };
}
#endif
