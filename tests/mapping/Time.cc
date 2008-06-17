
#include "Time.h"



Time::Time() {

}

void Time::start() {

	// get current time

	_tstart = clock();
}

double Time::elapsed() {
		return (double)((clock() - _tstart)) * 1000.0 / CLOCKS_PER_SEC;
}

