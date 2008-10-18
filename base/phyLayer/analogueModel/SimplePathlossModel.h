#ifndef PATHLOSSMODEL_H_
#define PATHLOSSMODEL_H_

#include "AnalogueModel.h"
#include "Mapping.h"
#include "BaseWorldUtility.h"

#include <cstdlib>

/**
 * @brief Mapping that represents a Pathloss-function.
 *
 * SimplePathlossConstMapping is subclassed from SimpleConstMapping for convenience.
 * In this simple pathloss implementation, we assume one attenuation value
 * being constant over the signals duration.
 *
 */
class SimplePathlossConstMapping : public SimpleConstMapping
{

protected:
	const double attValue;

public:
	/**
	 * @brief initializes the PathlossMapping for the passed dimensions,
	 * an interval the mapping is defined over and the actual constant
	 * attenuation value.
	 *
	 * The passed interval is needed by the base class SimpleConstMapping
	 * to be able to create an iterator for this Mapping. The interval
	 * tells the base class where the iterator should start, which
	 * step size it should use to iterate and where to end.
	 */
	SimplePathlossConstMapping(const DimensionSet& dimensions,
						const Argument& start,
						const double _attValue) :
		SimpleConstMapping(dimensions, start),
		attValue(_attValue)
	{

	}

	/**
	 * @brief simply return constant attenuation value
	 */
	virtual double getValue(const Argument& pos) const
	{

		/* In a more complex PathlossMapping we could use the value of the passed
		 * Argument in the frequency dimension to calculate the attenuation
		 * for the actual frequency correctly. But at this Mapping we assume
		 * the frequncy of every signal the same.*/
		return attValue;
	}

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

	/** @brief stores the dimensions this analogue model applies to.*/
	const DimensionSet dimensions;

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
	 * @brief Initializes the analogue model. _myMove and _playgroundSize
	 * need to be valid as long as this instance exists.
	 *
	 * The constructor needs some specific knowlegde in order to create
	 * its mapping properly:
	 *
	 * 1. the coefficient alpha (specified e.g. in config.xml and passed
	 *    in constructor call)
	 * 2. the carrier frequency
	 * 3. a pointer to the hosts move pattern
	 * 4. information about the playground the host is moving in
	 *
	 */
	SimplePathlossModel(double alpha, double _carrierFrequency, const Move* _myMove,
					bool _useTorus, const Coord& _playgroundSize, bool debug):
		dimensions(Dimension::time),
		pathLossAlphaHalf(alpha * 0.5),
		carrierFrequency(_carrierFrequency),
		myMove(*_myMove),
		useTorus(_useTorus),
		playgroundSize(_playgroundSize),
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
