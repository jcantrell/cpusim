#include "subleq.h"
#include "model/cpu/SignedMorsel.h"

subleq::subleq(unsigned int byte_in, UnsignedMorsel address_in) : cpu(byte_in, address_in, 0)
{
}

void subleq::step(unsigned int inst)
{
  if (inst == 0)
  {
    SignedMorsel a = SignedMorsel(view(view(getip()+1)));
    SignedMorsel b = SignedMorsel(view(view(getip()+2)));
    UnsignedMorsel c = view(getip()+3);
    load( view(getip()+2),(b-a).asUnsignedMorsel());
    setip( (b-a <= 0) ? c : getip()+4 );
  } else {
  }
}
