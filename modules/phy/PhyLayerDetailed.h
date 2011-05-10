
#ifndef PHYLAYERDETAILED_H_
#define PHYLAYERDETAILED_H_

#include "PhyLayerBattery.h"
#include "RadioDetailed.h"


class PhyLayerDetailed : public PhyLayerBattery {

protected:
	double onCurrent, setupOnCurrent;
	RadioDetailed* radioDetailed;
	virtual Radio* initializeRadio(); 
	virtual void setSwitchingCurrent(int from, int to);
	/** @brief duration of PHY header (after which we can decide if we can synchronize on the frame). */
	double  phyHeaderDuration;

public:
	/** @brief Loads analogue models. */
	AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

	/** @brief Initializes PER Model */
	AnalogueModel* initializePERModel(ParameterMap& params);

};

#endif
