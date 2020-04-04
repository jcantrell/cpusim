#ifndef SUBLEQ_H
#define SUBLEQ_H

#include "model/cpu/cpu.h"

class subleq : public cpu {
	public:
		subleq(unsigned int byte_in, UnsignedMorsel address_in);
		virtual void step(unsigned int inst);
};

#endif
