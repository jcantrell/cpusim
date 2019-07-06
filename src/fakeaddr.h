#ifndef FAKEADDR_H
#define FAKEADDR_H
#include "./fakemorsel.h"
using namespace std;

class fakeaddr
{
  private:
    fakemorsel m;
  public:
    fakemorsel hashValue() const {return m;}
};

  template <>
  struct hash<fakeaddr>
  {
    size_t operator()(const fakeaddr& key) const
    {
      return (hash<fakemorsel>()(key.hashValue()));
    }
  };
#endif
