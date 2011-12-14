/*
 * PhyLayer.h
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#ifndef PHYLAYER_H_
#define PHYLAYER_H_

#include "MiXiMDefs.h"
#include "BasePhyLayer.h"

/**
 * @brief Provides initialisation for several AnalogueModels and Deciders
 * from modules directory.
 *
 * Knows the following AnalogueModels:
 * - SimplePathlossModel
 * - LogNormalShadowing
 * - JakesFading
 * - SimpleObstacleModel
 *
 * Knows the following Deciders
 * - Decider80211
 * - SNRThresholdDecider
 *
 * @ingroup phyLayer
 */
class MIXIM_API PhyLayer: public BasePhyLayer {
protected:
	enum ProtocolIds {
		IEEE_80211 = 12123,
		IEEE_802154_NARROW,
	};
	/**
	 * @brief Creates and returns an instance of the AnalogueModel with the
	 * specified name.
	 *
	 * Is able to initialize the following AnalogueModels:
	 * - SimplePathlossModel
	 * - LogNormalShadowing
	 * - JakesFading
	 * - SimpleObstacleModel
	 */
	virtual AnalogueModel* getAnalogueModelFromName(std::string name, ParameterMap& params);

	/**
	 * @brief Creates and initializes a SimplePathlossModel with the
	 * passed parameter values.
	 */
	AnalogueModel* initializeSimplePathlossModel(ParameterMap& params);

	/**
	 * @brief Creates and initializes a LogNormalShadowing with the
	 * passed parameter values.
	 */
	AnalogueModel* initializeLogNormalShadowing(ParameterMap& params);

	/**
	 * @brief Creates and initializes a JakesFading with the
	 * passed parameter values.
	 */
	AnalogueModel* initializeJakesFading(ParameterMap& params);

	/**
	 * @brief Creates and initializes a BreakpointPathlossModel with the
	 * passed parameter values.
	 */
	virtual AnalogueModel* initializeBreakpointPathlossModel(ParameterMap& params);

    /**
	 * @brief Creates and initializes a SimpleObstacleShadowing with the
	 * passed parameter values.
	 */
	AnalogueModel* initializeSimpleObstacleShadowing(ParameterMap& params);

	/**
	 * @brief Creates a simple Packet Error Rate model that attenuates a percentage
	 * of the packets to zero, and does not attenuate the other packets.
	 *
	 */
	virtual AnalogueModel* initializePERModel(ParameterMap& params);

	/**
	 * @brief Creates and initializes a TwoRayInterferenceModel with the
	 * passed parameter values.
	 */
	AnalogueModel* initializeTwoRayInterferenceModel(ParameterMap& params);

	/**
	 * @brief Creates and returns an instance of the Decider with the specified
	 * name.
	 *
	 * Is able to initialize the following Deciders:
	 *
	 * - Decider80211
	 * - SNRThresholdDecider
	 */
	virtual Decider* getDeciderFromName(std::string name, ParameterMap& params);

	/**
	 * @brief Initializes a new Decider80211 from the passed parameter map.
	 */
	virtual Decider* initializeDecider80211(ParameterMap& params);

	/**
	 * @brief Initializes a new Decider802154Narrow from the passed parameter map.
	 */
	virtual Decider* initializeDecider802154Narrow(ParameterMap& params);

	/**
	 * @brief Initializes a new SNRThresholdDecider from the passed parameter map.
	 */
	virtual Decider* initializeSNRThresholdDecider(ParameterMap& params);

};

#endif /* PHYLAYER_H_ */
