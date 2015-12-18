#include "veins/base/connectionManager/ConnectionManager.h"

#include <cmath>

#include "veins/base/modules/BaseWorldUtility.h"

#ifndef ccEV
#define ccEV EV << getName() << ": "
#endif

Define_Module( ConnectionManager );

double ConnectionManager::calcInterfDist()
{
	double interfDistance;

	//the minimum carrier frequency for this cell
	double carrierFrequency = par("carrierFrequency").doubleValue();
	//maximum transmission power possible
	double pMax             = par("pMax").doubleValue();
	if (pMax <=0) {
		error("Max transmission power is <=0!");
	}
	//minimum signal attenuation threshold
	double sat              = par("sat").doubleValue();
	//minimum path loss coefficient
	double alpha            = par("alpha").doubleValue();

	double waveLength     = (BaseWorldUtility::speedOfLight()/carrierFrequency);
	//minimum power level to be able to physically receive a signal
	double minReceivePower = pow(10.0, sat/10.0);

	interfDistance = pow(waveLength * waveLength * pMax
					       / (16.0*M_PI*M_PI*minReceivePower),
					     1.0 / alpha);

	ccEV << "max interference distance:" << interfDistance << endl;

	return interfDistance;
}

