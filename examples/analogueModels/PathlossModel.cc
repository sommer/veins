#include "PathlossModel.h"


/**
 * @brief The actual filtering method.
 * 
 * The attenuation value(s) is(are) calculated and put to the newly created
 * mapping.
 * 
 */
void PathlossModel::filterSignal(Signal& s){
	
	/** Get start of the signal */ 
	simtime_t sStart = s.getSignalStart();
	simtime_t sEnd = s.getSignalLength() + sStart;
	
	/** claim the Move pattern of the sender from the Signal */
	Move sendersMove = s.getMove();
	
	/** Calculate the attenuation value */
	double attValue = calcPathloss(myMove.getPositionAt(sStart),
									sendersMove.getPositionAt(sStart));
	
	/*
	 * Create a proper mapping.
	 * 
	 * We assume one constant attenuation Value for the whole duration
	 * of the Signal. That is why we pass timepoints for start and end
	 * and a default interval of 1.
	 * 
	 */
	PathlossConstMapping* attMapping = new PathlossConstMapping(dimensions,
											Argument(sStart),
											Argument(sEnd),
											Argument(1),
											attValue);
			
	/* at last add the created attenuation mapping to the signal */
	s.addAttenuation(attMapping);
}

/**
 * @brief Method to calculate the attenuation value for pathloss.
 * 
 * Functionality is similar to pathloss-calculation in BasicSnrEval
 * 
 */
double PathlossModel::calcPathloss(const Coord& myPos, const Coord& sendersPos)
{
	/*
	 * maybe we can reuse an already calculated value for the square-distance
	 * at this point.
	 * 
	 */
	double sqrdistance = 0.0;
	
	if (useTorus)
	{
		sqrdistance = myPos.sqrTorusDist(sendersPos, playgroundSize);
	} else
	{
		sqrdistance = myPos.sqrdist(sendersPos);
	}
	
	ev << "PathlossModel: sqrdistance is: " << sqrdistance << endl;
	
	double attenuation = 1.0;
	// wavelength in metres
	double wavelength = (BaseWorldUtility::speedOfLight/carrierFrequency);
	
	ev << "PathlossModel: wavelength is: " << wavelength << endl;
	
	if (sqrdistance > 1.0)
	{
		attenuation = (wavelength * wavelength) / (16.0 * M_PI * M_PI)
						* (pow(sqrdistance, -1.0*pathLossAlphaHalf));
	}
	
	ev << "PathlossModel: attenuation is: " << attenuation << endl;
	
	return attenuation;
}
