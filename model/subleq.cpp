#include "subleq.h"

subleq::subleq(int byte_in, int address_in) : cpu(byte_in, address_in, 0)
{
}

void subleq::step(int inst)
{
	    switch (inst)
	    {
	        case 0: // subleq for now, until loading alternative
	                // instruction sets is implemented
	                
	            {
							int a = view(view(getip()+1));
							int b = view(view(getip()+2));
							int c = view(getip()+3);
	            printf("loaded %d %d %d\n", a, b, c);
	            printf("diff is %d\n", b-a);

	            load( view(inst+2), b-a);
/*
	            if (b-a <= 0)
	                mycpu->setip(c);
	            else
	                mycpu->setip(inst+1);
*/
							setip( b-a <= 0 ? c : inst+1 );
	            }

	            break;

	        default:
	            break;
	    }
}
