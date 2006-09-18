#ifndef __MIXIM_H__
#define __MIXIM_H__

using namespace std;

#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS

#include <vector>
#include <cassert>

#include <omnetpp.h>

#include "message.h"

#ifdef _MSC_VER
#if !defined(vsnprintf)
#define vsnprintf _vsnprintf
#endif
#endif // WIN32

/** @defgroup RadioMacros Macros for the radio.
	@todo Make these macros generic (not just RFM1001 specific).
	@{
*/
#define BYTE_TIME		(10.0 / 115200.0)
#define MODULATION_FACTOR	(1.5)
#define PREAMBLE_TIME		(3.0 * BYTE_TIME)
#define FRAME_DATA_TIME(x)	(BYTE_TIME * (1.0 + MODULATION_FACTOR * ((x) * 1.0 + 2.0)))
#define EXTRA_TRANSMIT_TIME	(0.5 * BYTE_TIME)
/** @} */

/** 3D coordinates. */
struct Position {
	int x;
	int y;
	int z;
};

/** Macro to assert that an expression has a specified object type.
    @param x The expression to test.
    @param t The type to check for.
*/
#define assert_type(x,t) assert(dynamic_cast<t>(x)!=NULL)

/** Base class for all MiXiM classes. */
class MiximBaseModule: public cSimpleModule {
	Module_Class_Members(MiximBaseModule, cSimpleModule, 0);

	public:
		/** Message selection constants. */
		enum {
			PRINT_APP = 0x0001,
			PRINT_ROUTING = 0x0002,
			PRINT_MAC = 0x0004,
			PRINT_RADIO = 0x0008,
			PRINT_PROP = 0x0010,
			PRINT_STATS = 0x0020,
			PRINT_INIT = 0x0040,
			PRINT_MAPPER = 0x0080,
			PRINT_CRIT = 0x8000,
		};
		/** Cleanup.
	
		    When overriding this method, don't forget to call the superclass's
		    @b finish() method.
		*/
		virtual void finish();
		virtual ~MiximBaseModule();
		/** Initialization.
	
		    When overriding this method, don't forget to call the superclass's
		    @b initialize() method.
		*/
		virtual void initialize();

		/** Get a string-type implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		std::string getStringParameter(const char *parameter, std::string defaultValue = "");
		/** Get a time-value implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		double getTimeParameter(const char *parameter, double defaultValue = 0.0);
		/** Get a double-type implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		double getDoubleParameter(const char *parameter, double defaultValue = 0.0);
		/** Get a long-type implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		long getLongParameter(const char *parameter, long defaultValue = 0);
		/** Get a bool-type implementation-parameter.
		    @param parameter The parameter name.
		    @param defaultValue The default value to be used when the parameter
		        has been omitted from the ini-file(s).
			@return The value of the parameter or @a defaultValue.
		
		    See @b getParameter() for details.
		*/
		bool getBoolParameter(const char *parameter, bool defaultValue = false);

		/* timer stuff */

		/** Initialise a set of timers for this protocol layer
		    @param count Number of timers used by this layer
		 */	
		void initTimers(unsigned int count);

		/** Set one of the timers to fire at a point in the future.
		    If the timer has already been set then this discards the old information.
			Must call @b initTimers() before using.
			@param index Timer number to set. Must be between 0 and the value given to @b initTimers()
			@param when Time in seconds in the future to fire the timer
		 */
		void setTimer(unsigned int index, double when);

		/** Cancel an existing timer set by @b setTimer()
		    If the timer has not been set, or has already fires, this does nothing
			Must call @b initTimers() before using.
			@param index Timer to cancel. Must be between 0 and the value given to @b initTimers()
		 */
		void cancelTimer(unsigned int index);

		/** Fires on expiration of a timer.
		    Fires after a call to @b setTimer(). Subclasses should override this.
			@param index Timer number that fired. Will be between 0 and the value given to @b initTimers()
		*/	

		virtual void handleTimer(unsigned int count);

		/** Filters out timer messages from data messages.
		    Timers generate @b handleTimer() events, messages generate @b handleEvent() calls. Subclasses shouldn't override this
			@param msg Incoming message
		*/	
		virtual void handleMessage(cMessage *msg);

		/** Fires on incoming data messages.
		    Subclasses should override this.
			@param msg Incoming message.
		*/	
		virtual void handleEvent(cMessage *msg);

	protected:
		/** Print a message with time stamp.
		    @param which The type of message.
		    @param fmt printf(3) format string.
	
		    The message is printed preceeded by the time and the module that
		    includes this function and followed by a newline. @a which is used
		    to determine whether to actually print this message or not. This
		    allows control over the amount of output from the simulator.
		*/
		void printf(int which, const char *fmt, ...);
		/** Print a message without a trailing newline.
		    @param which The type of message.
		    @param fmt printf(3) format string.

		    Same as @b printf(), but with a trailing space instead of a newline.
		    This allows printinf of a message in several steps. See also @b
		    printf_clr().
		*/
		void printf_nr(int which, const char *fmt, ...);
		/** Continue a message started with @b printf_nr().
		    @param which The type of message.
		    @param fmt printf(3) format string.
		*/
		void printf_clr(int which, const char *fmt, ...);
		/** Print a message without preceding time and node information. */
		void printfNoInfo(int which, const char *fmt, ...);
		/** Print a message without preceding time and node information or trailing newline. */
		void printfNoInfo_nr(int which, const char *fmt, ...);
		
		/** Determine the absolute (Euclidean) distance between two
		        @link Position Position's @endlink.
		    @param one First position.
		    @param two Second position.
		    @return The absolute (Euclidean) distance between @link Position
		        Position's @endlink one and two.
		*/
		double absDistance(Position one, Position two);
		/** Add Joules to the power consumed by this module.
		    @param joule The amount of Joules to add.
		*/
		virtual void powerConsumed(double joule);

		/** Get an implementation parameter.
		    @param parameter The name of the parameter.
		    @return The string value of the paramter.
		
		    The @a parameter will be prefixed with "mixim.<class name>.". This
		    string will then be used to fetch a value from the ini-file(s).
		
		    These parameters are intended for implementation dependant
		    parameters that cannot be included in the NED specification. For
		    example, T-MAC has several such parameters.
		*/
		virtual std::string getParameter(const char *parameter);

	private:
		/** The amount of Joules consumed by this module. */
		double power_consumed;
		/** Internal printing routine.
		    @param fmt printf(3) format string.
		    @param list A @b va_list of arguments.
		    @param newline Boolean indicating whether to print a newline.
	
		    This function is used by @b printf() and @b printf_nr() to print
		    a message. The message is printed preceeded by the time and the
		    module that includes this function. If the boolean @a newline is
		    set, the message will be followed by a newline, otherwise a space
		    is printed. Printing is performed through vsnprintf, to a buffer of
		    size 1000. Larger message strings will be truncated.
		*/
		void internal_printf(const char* fmt, va_list list, bool newline);
	
		/** Mask that defines what messages to print. */
		static int prints;
		/** Boolean indicating whether @b prints has been initialised. */
		static bool globalsInitialised;

		/** Number of timers. Initialised to 0. Set by @b init_timers() */
		unsigned int timer_count;
		/** Timer message array */
		cMessage *timers;

};

/** Symbolic names for the different random number generators used. */
enum {
	/* Define MAC as 1 to prevent inadvertent use of MAC's RNG. */
	RNG_MAC = 1,
	RNG_ROUTING,
	RNG_APP,
	RNG_APP_AUX,
	RNG_NODE,
	
	/* Add all RNG's before this one. */
	RNG_MAX
};

#endif

