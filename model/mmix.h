#ifndef SUBLEQ_H
#define SUBLEQ_H

#include "model/cpu.h"

class mmix : public cpu {
	public:
		subleq(int byte_in, int address_in);
		void step(int inst);
};

#endif
