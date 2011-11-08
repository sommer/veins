#ifndef RANDOMFREQTIMEMODEL_H_
#define RANDOMFREQTIMEMODEL_H_

#include "AnalogueModel.h"
#include "Mapping.h"

#include <cstdlib>

/**
 * @brief Sample implementation of an AnalogueModel which uses
 * MultiDimMapping as AttenuationMapping.
 *
 * This class is a sample which shows how to use the default Mapping implementation
 * to implement a signal attenuation over more dimensions then just the
 * time.
 *
 * @ingroup analogueModels
 * @ingroup exampleAM
 */
class RandomFreqTimeModel : public AnalogueModel
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
	 * In this case we want to work on "time" and "frequency".
	 * The DimensionSet provides constructors for up to three initial
	 * dimensions. If we need more we will have to add them after construction
	 * by calling the DimensionSets "addDimension()"-method.
	 *
	 * Note: Using "Dimension("time")" instead of "Dimension::time()" whould
	 * work also, but using "Dimension::time()" spares us a string comparison
	 * and should therefore be prefered instead of using "Dimension("time")".
	 * Besides using Dimension::time() is more typo resistant since the
	 * compiler can check if the static member actually exists.
	 */
	RandomFreqTimeModel(int seed = 23):
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

#endif /*RANDOMFREQTIMEMODEL_H_*/
