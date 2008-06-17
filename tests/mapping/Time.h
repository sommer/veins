#ifndef TIME_H_
#define TIME_H_

#include <time.h>




/**
* @brief multi-platform high performance timer
*
* measure time up to a 10th of a millisecond 
*/
class Time {
public:

	Time();
	/** 
	* @brief start the timer
	*/
	void start();

	/**
	* @brief get time since last start
	* @return elapsed time in seconds
	*/
	double elapsed();

protected:
	// store start
	clock_t		_tstart; //!< start time

};

#endif
