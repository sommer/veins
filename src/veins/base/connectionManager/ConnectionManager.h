#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"

/**
 * @brief BaseConnectionManager implementation which only defines a
 * specific max interference distance.
 *
 * Calculates the maximum interference distance based on the transmitter
 * power, wavelength, pathloss coefficient and a threshold for the
 * minimal receive Power.
 *
 * @ingroup connectionManager
 */
class MIXIM_API ConnectionManager : public BaseConnectionManager
{
protected:

	/**
	 * @brief Calculate interference distance
	 *
	 * You may want to overwrite this function in order to do your own
	 * interference calculation
	 */
	virtual double calcInterfDist();
};

#endif /*CONNECTIONMANAGER_H_*/
