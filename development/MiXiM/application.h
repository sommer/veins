#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "software.h"
#include "node.h"

/** Base class for application classes. */
class Application : public MiximSoftwareModule {
	Module_Class_Members(Application, MiximSoftwareModule, 0);

	public:
		virtual void initialize();
		virtual void finish();
		/** Simulate processor cycles use.
		    @param cycles The number of cycles to be used in simulation.

		    This routine can be used to simulate that code needs cpu-cycles to
		    execute. The cycles used will then be taken into account in
		    calculating the amount of power used by the processor.
	
		    Note that this simply calls the eatCycles routine in @b Node.
		*/
		virtual void eatCycles(unsigned cycles);

	protected:
		/** Pointer to the enclosing @b Node module. */
		Node* node;
};

#endif

