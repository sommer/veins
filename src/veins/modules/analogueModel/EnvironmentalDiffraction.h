#ifndef ENVIRONMENTALDIFFRACTION_H_
#define ENVIRONMENTALDIFFRACTION_H_

#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/messages/AirFrame_m.h"
#include "veins/modules/utility/NBHeightMapper.h"

using Veins::AirFrame;

/**
 * @brief Implementation of an environmental diffraction model.
 * Generates map of knife-edges along the signal path and applies multiple knife-edge model
 * as suggested in ITU-R Recommendation P.526-11: Propagation by diffraction (10/2009).
 * The terrain shape is analyzed by querying the DEM. Moreover, other vehicles in the LOS
 * are considered as potential obstacles. The user can switch consideration of both parts
 * on and off in the confix.xml file.
 *
 * A detailed explanation of this model in the context of 3D VANET simulation and example simulation results
 * can be found in:
 * Alexander Brummer, Reinhard German and Anatoli Djanatliev, "On the Necessity of Three-Dimensional Considerations
 * in Vehicular Network Simulation," 14th IEEE/IFIP Conference on Wireless On demand Network Systems and Services
 * (WONS 2018), Isola 2000, France, February 2018
 *
 * An example config.xml for this AnalogueModel can be the following:
 * @verbatim
    <AnalogueModel type="EnvironmentalDiffraction">
        <!-- The signal frequency -->
        <parameter name="carrierFrequency" type="double" value="5.890e+9"/>
        <!-- States whether the terrain characteristics should be considered -->
        <parameter name="considerDEM" type="bool" value="true"/>
        <!-- File name of the DEM to query the terrain's elevation -->
        <parameter name="demFiles" type="string" value="GENERATED.tif"/>
        <!-- True if the DEM is in raster format (GeoTiff) -->
        <parameter name="isRasterType" type="bool" value="true"/>
        <!-- The spacing of queried height profile points of the terrain along the LOS -->
        <parameter name="spacing" type="double" value="1"/>
        <!-- States whether other vehicles should be considered -->
        <parameter name="considerVehicles" type="bool" value="true"/>
    </AnalogueModel>
   @endverbatim
 *
 * @author Alexander Brummer
 *
 * @ingroup analogueModels
 */
class EnvironmentalDiffraction: public AnalogueModel {
private:
    /** @brief The signal wavelength. **/
    double wavelength;

    /** @brief States whether the terrain characteristics should be considered. **/
    bool considerDEM;

    /** @brief The spacing of queried height profile points of the terrain along the LOS. **/
    double spacing;

    /** @brief States whether other vehicles should be considered. **/
    bool considerVehicles;

    /** @brief Cell size for the DEM cache. **/
    double demCellSize;

    /** @brief The size of the playground. **/
    static const Coord* pgs;

    /** @brief Pointer to the DEM cache (array storing height values queried before). **/
    static double* demCache;

    /** @brief Number of rows in the DEM cache, depends on the playground size and grid granularity. **/
    static size_t cacheRows;

    /** @brief Number of columns in the DEM cache, depends on the playground size and grid granularity. **/
    static size_t cacheCols;


    /**
     * @brief calculates the attenuation due to environmental diffraction
     *
     * @param senderPos the sender's position
     * @param receiverPos the receiver's position
     * @return value of attenuation in linear units (non-dB)
     */
    double calcAttenuation(const Coord& senderPos, const Coord& receiverPos);

    /**
     * @brief checks if vehicle intersects the LOS
     *
     * @param pos the vehicle's position
     * @param orient the vehicle's orientation
     * @param senderPos the sender's position
     * @param receiverPos the receiver's position
     * @return pair of distance to sender and height; if no intersection is detected, (-1.0, -1.0) is returned
     */
    std::pair<double, double> isInLOS(const Coord& pos, const Coord& orient, const Coord& senderPos, const Coord& receiverPos);

    /**
     * @brief checks if and where segments intersect, similar to the function used for SimpleObstacleShadowing
     *
     * @param p1From the first segment's starting point
     * @param p1To the first segment's end point
     * @param p2From the second segment's starting point
     * @param p2To the second segment's end point
     * @return true if segments intersect
     */
    bool segmentsIntersect(Coord p1From, Coord p1To, Coord p2From, Coord p2To);

    /**
     * @brief Finds the maximum v diffraction parameter in the given knife-edge map
     *
     * @param map the knife-edge map
     * @param a lower distance value to start search within the map
     * @param b upper distance value to search until
     * @return the pair of distance and v with the highest v in the given interval
     */
    std::pair<double, double> getHighestV(const std::map<double, double>& map, double a, double b);

    /**
     * @brief Calculates the value of the J function yielding the attenuation.
     *
     * @param v the diffraction parameter
     * @return the value of attenuation in dB
     */
    double getJFuncValue(double v);

public:
    /**
     * @brief Initializes the analogue model.
     *
     * The constructor needs some specific knowledge in order to create
     * its mapping properly:
     *
     * @param carrierFrequency the carrier frequency
     * @param considerDEM states whether the terrain characteristics should be considered
     * @param demFiles the filename(s) of the DEM
     * @param isRasterType whether the DEM is in raster format (GeoTiff)
     * @param spacing the spacing of queried height profile points of the terrain along the LOS
     * @param considerVehicles states whether other vehicles should be considered
     */
    EnvironmentalDiffraction(double carrierFrequency, bool considerDEM, std::vector<std::string> demFiles,
            bool isRasterType, double demCellSize, double spacing, bool considerVehicles);

    /**
     * @brief Filters a specified AirFrame's Signal by adding the resulting attenuation.
     * @param frame the Airframe in question
     * @param senderPos the sender's position
     * @param receiverPos the receiver's position
     */
    virtual void filterSignal(AirFrame* frame, const Coord& senderPos, const Coord& receiverPos);
};

#endif /* ENVIRONMENTALDIFFRACTION_H_ */
