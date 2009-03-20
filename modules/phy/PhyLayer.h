/*
 * PhyLayer.h
 *
 *  Created on: 11.02.2009
 *      Author: karl wessel
 */

#ifndef PHYLAYER_H_
#define PHYLAYER_H_

#include "BasePhyLayer.h"

/**
 * @brief Provides initialisation for several AnalogueModels and Deciders
 * from modules directory.
 *
 * Knows the following AnalogueModels:
 * - SimplePathlossModel
 * - LogNormalShadowing
 * - JakesFading
 *
 * Knows the following Deciders
 * - Decider80211
 * - SNRThresholdDecider
 *
 * @ingroup phyLayer
 */
class PhyLayer: public BasePhyLayer {
protected:
	/**
	 * @brief Creates and returns an instance of the AnalogueModel with the
	 * specified name.
	 *
	 * Is able to initialize the following AnalogueModels:
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
	 * @brief Initializes a new SNRThresholdDecider from the passed parameter map.
	 */
	virtual Decider* initializeSNRThresholdDecider(ParameterMap& params);
};

#endif /* PHYLAYER_H_ */
