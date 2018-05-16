/*
 * SampledAntenna2D.h
 *
 *  Created on: Apr 16, 2017
 *      Author: Alexander Brummer
 */

#ifndef SRC_VEINS_MODULES_PHY_SAMPLEDANTENNA2D_H_
#define SRC_VEINS_MODULES_PHY_SAMPLEDANTENNA2D_H_

#include "veins/base/phyLayer/Antenna.h"
#include "veins/base/phyLayer/MappingUtils.h"
#include <vector>

using namespace std;

/**
 * @brief
 * This class represents an antenna whose gain is interpolated from given samples for the horizontal and vertical plane.
 * The respective gain is therefore dependent on the azimuth and elevation angle.
 * The applied 3D antenna interpolation method is given in Leonor et al.: "A Three-Dimensional Directive Antenna Pattern
 * Interpolation Method"
 *
 * A detailed explanation of how this method is applied in the context of 3D VANET simulation and example simulation results
 * can be found in:
 * Alexander Brummer, Reinhard German and Anatoli Djanatliev, "On the Necessity of Three-Dimensional Considerations
 * in Vehicular Network Simulation," 14th IEEE/IFIP Conference on Wireless On demand Network Systems and Services
 * (WONS 2018), Isola 2000, France, February 2018
 *
 * The user has to provide the samples, which are assumed to be distributed equidistantly (for both planes, respectively).
 * As the power is assumed to be relative to an isotropic radiator, the values have to be given in dBi.
 * The values are stored in two Mappings automatically supporting linear interpolation between samples.
 * Optional randomness in terms of sample offsets and antenna rotation is supported.
 *
 * * An example antenna.xml for this Antenna can be the following:
 * @verbatim
    <?xml version="1.0" encoding="UTF-8"?>
    <root>
        <Antenna type="SampledAntenna2D" id="antenna1">
            <!-- Write the azimuth samples in the azi-samples value attribute, separated by spaces. The values will be -->
            <!-- distributed equidistantly, e.g. 4 values will be placed at 0° (front), 90° (right), 180° (rear) -->
            <!-- and 270° (left)-->
            <parameter name="azi-samples" type="string" value="10 0 -10 -20"/>

            <!-- Options for random offsets are as follows (also accounts for ele-random-offsets). -->
            <!-- The mean of the given distribution has to be 0 (so that the overall offset is close to 0dBi) -->
            <!-- <parameter name="azi-random-offsets" type="string" value="uniform a b"/> -->
            <!-- <parameter name="azi-random-offsets" type="string" value="normal mean stddev"/> -->
            <!-- <parameter name="azi-random-offsets" type="string" value="triang a b c"/> -->
            <parameter name="azi-random-offsets" type="string" value="uniform -0.01 0.01"/>

            <!-- Options for random rotation of the pattern are the same, but mean doesn't have to be 0 -->
            <!-- (also accounts for ele-random-rotation). -->
            <parameter name="azi-random-rotation" type="string" value="uniform -1 1"/>

            <!-- Write the elevation samples in the ele-samples value attribute, separated by spaces. The values will be -->
            <!-- distributed equidistantly, e.g. 4 values will be placed at 0° (top), 90° (front), 180° (bottom) -->
            <!-- and 270° (rear)-->
            <parameter name="ele-samples" type="string" value="-30 10 -40 -10"/>

            <parameter name="ele-random-offsets" type="string" value="uniform -0.01 0.01"/>
            <parameter name="ele-random-rotation" type="string" value="uniform -1 1"/>
        </Antenna>
    </root>
   @endverbatim
 *
 *
 * @author Alexander Brummer
 *
 *
 * @see Antenna
 * @see BasePhyLayer
 */
class SampledAntenna2D: public Antenna {
public:
    /**
     * @brief Constructor for the sampled antenna.
     *
     * @param aziValues            - contains the samples representing the antenna's azimuth plane
     * @param aziOffsetType        - name of random distribution to use for the random offset of the azimuth plane's samples
     * @param aziOffsetParams      - contains the parameters for the azimuth offset random distribution
     * @param aziRotationType      - name of random distribution to use for the random rotation of the whole azimuth plane
     * @param aziRotationParams    - contains the parameters for the azimuth rotation random distribution
     * @param eleValues            - contains the samples representing the antenna's elevation plane
     * @param eleOffsetType        - name of random distribution to use for the random offset of the elevation plane's samples
     * @param eleOffsetParams      - contains the parameters for the elevation offset random distribution
     * @param eleRotationType      - name of random distribution to use for the random rotation of the whole elevation plane
     * @param eleRotationParams    - contains the parameters for the elevation rotation random distribution
     * @param rng                  - pointer to the random number generator to use
     */
    SampledAntenna2D(vector<double>& aziValues, string aziOffsetType, vector<double>& aziOffsetParams, string aziRotationType, vector<double>& aziRotationParams,
                     vector<double>& eleValues, string eleOffsetType, vector<double>& eleOffsetParams, string eleRotationType, vector<double>& eleRotationParams,
                     cRNG* rng);

    /**
     * @brief Destructor of the SampledAntenna2D.
     *
     * Deletes the mappings used for storing the antenna samples.
     */
    virtual ~SampledAntenna2D();

    /**
     * @brief Calculates this antenna's gain based on the direction the signal is coming from/sent in.
     *
     * @param ownPos        - coordinates of this antenna
     * @param ownOrient     - states the direction the antenna (i.e. the car) is pointing at
     * @param otherPos      - coordinates of the other antenna which this antenna is currently communicating with
     * @return Returns the gain this antenna achieves depending on the computed direction.
     */
    double getGain(Coord ownPos, Coord ownOrient, Coord otherPos);

    double getLastAzi();

    double getLastEle();

private:
    /**
     * @brief Mapping which is used to store the antenna's azimuth plane samples. Provides automatic linear interpolation.
     */
    Mapping* aziSamples;

    /**
     * @brief Mapping which is used to store the antenna's elevation plane samples. Provides automatic linear interpolation.
     */
    Mapping* eleSamples;

    /**
     * @brief An optional random rotation of the azimuth plane is stored in this field and applied every time
     * the gain has to be calculated.
     */
    double aziRotation;

    /**
     * @brief An optional random rotation of the elevation plane is stored in this field and applied every time
     * the gain has to be calculated.
     */
    double eleRotation;

    double lastAzimuth;
    double lastElevation;

};

#endif /* SRC_VEINS_MODULES_PHY_SAMPLEDANTENNA2D_H_ */
