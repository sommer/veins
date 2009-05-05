#ifndef PATHLOSSMODEL_H_
#define PATHLOSSMODEL_H_

#include "AnalogueModel.h"
#include "Mapping.h"
#include "BaseWorldUtility.h"

#include <cstdlib>

class SimplePathlossModel;

/**
 * @brief Mapping that represents a Pathloss-function.
 *
 * SimplePathlossConstMapping is subclassed from SimpleConstMapping for convenience.
 * In this simple pathloss implementation, we assume one attenuation value
 * being constant over the signals duration.
 *
 * @ingroup analogueModels
 */
class SimplePathlossConstMapping : public SimpleConstMapping
{

protected:
	const double distFactor;
	SimplePathlossModel* model;
	bool hasFrequency;

public:
	/**
	 * @brief initializes the PathlossMapping for the passed dimensions,
	 * its model and the distance factor for the signal to attenuate.
	 */
	SimplePathlossConstMapping(const DimensionSet& dimensions,
							   SimplePathlossModel* model,
							   const double distFactor);

	/**
	 * @brief Calculates attenuation from the distance factor and
	 * the current positions frequency (or the models carrier frequency
	 * if we are using using signals without frequency domain.
	 */
	virtual double getValue(const Argument& pos) const;

	/**
	 * @brief creates a clone of this mapping. This method has to be implemented
	 * by every subclass. But most time the implementation will look like the
	 * implementation of this method (except of the class name).
	 */
	ConstMapping* constClone() const
	{
		return new SimplePathlossConstMapping(*this);
	}

};


/**
 * @brief Basic implementation of a SimplePathlossModel that uses
 * SimplePathlossConstMapping (that is subclassed from SimpleConstMapping) as attenuation-Mapping.
 *
 */
class SimplePathlossModel : public AnalogueModel
{
protected:
	friend class SimplePathlossConstMapping;

	/** @brief shortcut to frequency dimension */
	static Dimension frequency;

	/** @brief Shortcut to time domain.*/
	static DimensionSet timeDomain;

	/** @brief Shortcut to time x frequency domain.*/
	static DimensionSet timeFreqDomain;

	/** @brief Path loss coefficient. **/
    double pathLossAlphaHalf;

    /** @brief carrier frequency needed for calculation */
    double carrierFrequency;

    /** @brief stores my Move pattern */
    const Move& myMove;

	/** Information needed about the playground */
	const bool useTorus;
	const Coord& playgroundSize;

	/** @brief Whether debug messages should be displayed. */
	bool debug;

public:
	/**
	 * @brief Initializes the analogue model. myMove and playgroundSize
	 * need to be valid as long as this instance exists.
	 *
	 * The constructor needs some specific knowledge in order to create
	 * its mapping properly:
	 *
	 * 1. the coefficient alpha (specified e.g. in config.xml and passed
	 *    in constructor call)
	 * 2. the carrier frequency
	 * 3. a pointer to the hosts move pattern
	 * 4. information about the playground the host is moving in
	 *
	 */
	SimplePathlossModel(double alpha, double carrierFrequency, const Move* myMove,
					bool useTorus, const Coord& playgroundSize, bool debug):
		pathLossAlphaHalf(alpha * 0.5),
		carrierFrequency(carrierFrequency),
		myMove(*myMove),
		useTorus(useTorus),
		playgroundSize(playgroundSize),
		debug(debug)
	{

	}

	/**
	 * @brief Filters a specified Signal by adding an attenuation
	 * over time to the Signal.
	 *
	 */
	virtual void filterSignal(Signal& s);

	/**
	 * @brief Method to calculate the attenuation value for pathloss.
	 *
	 * Functionality is similar to pathloss-calculation in BasicSnrEval from
	 * Mobility-frame work.
	 *
	 */
	virtual double calcPathloss(const Coord& myPos, const Coord& sendersPos);
};

#endif /*PATHLOSSMODEL_H_*/
