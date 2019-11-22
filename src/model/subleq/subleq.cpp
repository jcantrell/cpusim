#include "subleq.h"

subleq::subleq(unsigned int byte_in, Address address_in) : cpu(byte_in, address_in, 0)
{
  printf("subleq constructor called!\n");
}

void subleq::step(unsigned int inst)
{
  printf("subleq step called!\n");
	    //switch (inst)
	    //{
      if (inst == 0)
      {
	        //case 0: // subleq for now, until loading alternative
	                // instruction sets is implemented
	                
	            {
							UnsignedMorsel a = view(Address(view(getip()+1)));
							UnsignedMorsel b = view(Address(view(getip()+2)));
							UnsignedMorsel c = view(getip()+3);
	            printf("loaded %s %s %s\n", 
                a.asString().c_str(), b.asString().c_str(), 
                c.asString().c_str());
	            printf("diff is %s\n", (b-a).asString().c_str());

/*
              Morsel instCopy(inst);
              Morsel t_inst = instCopy+2;
              Morsel t_view = view(instCopy+2);
              Address t_addr(t_view);
              Morsel t_diff;
              t_diff = b-a;
              load( t_addr, t_diff );
*/

	            load( Address(view(Address(inst+2))), b-a);
/*
	            if (b-a <= 0)
	                mycpu->setip(c);
	            else
	                mycpu->setip(inst+1);
*/
	            printf("c is %s\n", c.asString().c_str());
	            //printf("inst+1 is %s\n", (inst+1).asString().c_str());
	            printf("inst+1 is %d\n", (inst+1));
							setip( b-a <= 0 ? c : inst+1 );
	            printf("ip is %s\n", getip().asString().c_str());
	            }


      } else {
	    }
}
