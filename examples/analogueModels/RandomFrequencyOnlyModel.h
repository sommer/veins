#ifndef RANDOMFREQUENCYONLYMODEL_H_
#define RANDOMFREQUENCYONLYMODEL_H_

#include "AnalogueModel.h"
#include "Mapping.h"

#include <cstdlib>

/**
 * @brief Sample implementation of an AnalogueModel which uses
 * MultiDimMapping as AttenuationMapping.
 *
 * This class is a sample which shows how to use the default Mapping implementation
 * to implement a signal attenuation over only one dimension but which is not the time.
 * Since every Mapping has to be defined in time we can't really create a Mapping
 * with only frequency as Dimension. But we can just create a two dimensional
 * Mapping over frequency and time which is constant in time space. This means
 * every time we want to set a value of the Mapping we will just pass 0 as parameter
 * for the time dimension.
 *
 * @ingroup analogueModels
 * @ingroup exampleAM
 */
class RandomFrequencyOnlyModel : public AnalogueModel
{
protected:

	/** @brief shortcut to the frequency dimension, to avoid using
	 * 'Dimension("frequency")' every time. */
	const Dimension frequency;

	/** @brief stores the dimensions this analogue model applies to.*/
	const DimensionSet dimensions;

public:
	/**
	 * @brief Initializes the analogue model.
	 *
	 * The only thing we have to do in the constructor and which whould
	 * probably have to be done for every other AnalogueModel is
	 * setting the the DimensionSet this AnalogueModel will work with.
	 *
	 * In this case we want to work on "time (constant)" and "frequency".
	 * The DimensionSet provides constructors for up to three initial
	 * dimensions. If we need more we will have to add them after construction
	 * by calling the DimensionSets "addDimension()"-method.
	 *
	 * Note: Using "Dimension("time")" instead of "Dimension::time()" would
	 * work also, but using "Dimension::time()" saves us a string comparison
	 * and should therefore be prefered instead of using "Dimension("time")".
	 */
	RandomFrequencyOnlyModel(int seed = 23):
		frequency("frequency"),
		dimensions(Dimension::time, frequency) {

		//sets the seed for random number generation. The PhyLayer
		//(which created the analogue models) gets the seed from the
		//configuration parameters inside the xml-config
		srand(seed);
	}

	/**
	 * @brief Has to be overriden by every implementation.
	 *
	 * Filters a specified AirFrame's Signal by adding an attenuation
	 * over time to the Signal.
	 */
	virtual void filterSignal(AirFrame *, const Coord&, const Coord&);
};

#endif /*RANDOMSPACEONLYMODEL_H_*/
