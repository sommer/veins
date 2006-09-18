#ifndef __NODE_H__
#define __NODE_H__

#include "software.h"

/** Base class for defining node types */
class Node : public MiximSoftwareModule {
	Module_Class_Members(Node, MiximSoftwareModule, 0);

	public:
		virtual void initialize();
		virtual void finish();
		/** Simulate processor cycles use.
		    @param cycles The number of cycles to be used in simulation.

		    This routine can be used to simulate that code needs cpu-cycles to
		    execute. The cycles used will then be taken into account in
		    calculating the amount of power used by the processor.
		*/
		virtual void eatCycles(unsigned cycles);
		/** Get the current time as indicated by the node's crystal clock.
		    @return The current time in clock ticks.

		    Most nodes feature a 32KHz crystal. This function will retrieve the
		    current value of the simulated clock.
		*/
		unsigned short getCurrentTime(void);

		/** Get the simulation time for @a ticks clock ticks into the future.
		    @param ticks The number of clock ticks to be taken into account.
		    @return The simulation time at which the clock will have ticked
		        @a ticks times.
		
		    Note: this function adds a very small amount of time over the time
		    indicated by the @a ticks. This is to prevent the returned time to
			be slightly smaller than the actual tick.
		*/
		double getThenTime(unsigned short ticks);

		/** Get the ID of this node.
		    @return The ID of this node.
		*/
		int getNodeId(void);
		/** Get the position of this node.
		    @return The @b Position of this node.
		*/
		Position getPosition(void);
		/** Get the maximum range of the radio.
		    @return The maximum radio range of this node.
		*/
		virtual double getMaxRadioRange(void) { return radioRange; }

	protected:
		/** Implementation defined joules per CPU cycle.
	
		    To be set in all node implementation constructors.
		*/
		double joulesPerCycle;
	
		/** The radio range setting (net.radioRange). */
		double radioRange;
	
	private:
		/** Skew of the crystal clock.
	
		    This represents the offset relative to the global time. It is the
		    fraction of a clock tick that this clock is ahead of the base clock.
		*/
		double clock_skew;
		/** ID of this node. */
		int node_id;
		/** Position of this node. */
		Position currentPosition;
};

#endif

