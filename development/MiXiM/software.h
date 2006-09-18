#ifndef __SOFTWARE_H__
#define __SOFTWARE_H__

#include "mixim.h"

/** Base class for software running on nodes. */
class MiximSoftwareModule: public MiximBaseModule {
	Module_Class_Members(MiximSoftwareModule, MiximBaseModule, 0);

	protected:
		/** Simulate processor cycles use.
		    @param cycles The number of cycles to be used in simulation.

		    This routine can be used to simulate that code needs cpu-cycles to
		    execute. The cycles used will then be taken into account in
		    calculating the amount of power used by the processor.
		*/
		virtual void eatCycles(unsigned cycles) = 0;
		/** CPU cycles used by running the software. */
		unsigned cpu_cycles;

		MiximSoftwareModule() : cpu_cycles(0) {}
};

#endif

