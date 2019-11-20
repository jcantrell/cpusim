#ifndef SUBLEQ_H
#define SUBLEQ_H

#include "model/cpu/cpu.h"

class subleq : public cpu {
	public:
		subleq(int byte_in, Address address_in);
		virtual void step(UnsignedMorsel inst);
};

#endif
