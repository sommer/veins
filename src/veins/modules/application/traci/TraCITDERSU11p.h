//
// Copyright (C) 2015 Daniel Febrian Sengkey <danielsengkey.cio13@mail.ugm.ac.id>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Part of my Master of Engineering thesis project.
// Copyright (C) 2015 Daniel Febrian Sengkey <danielsengkey.cio13@mail.ugm.ac.id>
// Department of Electrical Engineering and Information Technology, Gadjah Mada University
// Yogyakarta, INDONESIA
//
// Most parts of this file was derived from TraCIDemo11p.{cc,h} of Veins by Dr.-Ing. Christoph Sommer

#ifndef TraCITDERSU11p_H
#define TraCITDERSU11p_H

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "veins/modules/world/annotations/AnnotationManager.h"

using Veins::AnnotationManager;

/**
 * @brief
 * IVC with Traffic Density Estimation Capability
 * Trying to exploit the exchanged beacon
 * as vehicle existence indicator.
 *
 * @author Daniel Febrian Sengkey
 */

class TraCITDERSU11p : public BaseWaveApplLayer {
private:
    /** @brief data collector  for storing number of detected vehicles*/
    simsignal_t vehNumberLV, vehNumberHV, vehNumberMC;

    struct knownVehicle {
        int id;
        std::string vType;
        simtime_t lastSeenAt;
    };

    /** @brief controlling vehicle list information */
    bool debugAppTDE;

    /** @brief enable/disable traffic density estimation */
    bool enableTDE;

    /** @brief Declaring enumerated values for each type of vehicle.
     * This values have no use with application layer but important for post-processing.
     */
    enum vType { MC, LV, HV };

    /** Interval for statistics signals update */
    double   statInterval;
    cMessage* updateStatMsg;

    /** @brief Timeout for registering a node in list */
    simtime_t timeoutInterval;

    /** @brief Time limit (s) before a node deleted from the list */
    double timeoutMsgInterval;
    cMessage* timeoutMsg;

	public:
        virtual void finish();
		virtual void initialize(int stage);

		/** @brief Enumerated values for handling self messages. */
		enum WaveApplMessageKinds {
	        UPDATE_STATS,
	        CHECK_TIMEOUT,
	        CHECK_ROAD,
	        INTERVENTION_TEST
	    };

	protected:
		AnnotationManager* annotations;
		BaseMobility* mobi;
		bool sentMessage;

	protected:
		/** @brief maximum number of listed vehicles for each node */
		    static const short maxVehicles = 100;

		    /** @brief array of listed vehicles */
		    knownVehicle listedVehicles[100];

		    short
		    /** @brief counting variable for light vehicle */
		    currentNumberofDetectedLV,

		    /** @brief counting variable for heavy vehicle */
		    currentNumberofDetectedHV,

		    /** @brief counting variable for motorcylce */
		    currentNumberofDetectedMC,

		    /** @brief variable  that stores total number of detected vehicles */
		    currentNumberofTotalDetectedVehicles;

		    /** @brief Passenger car equivalent for motorcycle. */
		    double pceMC;

		    /* @brief Passenger car equivalent for light vehicle. */
		    double pceLV;

		    /** @brief Passenger car equivalent for heavy vehicle. */
		    double pceHV;

//		    /** @brief variable containing the vehicle type */
//		    std::string myType;

		    /** @brief variable storing vehicle type. Will be feed to scalar later */
		    vType vTypeInt;

		    /** @brief Road basic capacity, value for a single lane. Symbolized as C0 in MKJI */
		    double basicCapacity;

		    /** @brief Correction factor for road width (FCw).*/
		    double correctionFactorWidth;

		    /** @brief Correction factor for side friction (FCsf).*/
		    double correctionFactorSideFriction;

		    /** @brief Correction factor for city size (FCcs).*/
		    double correctionFactorCitySize;

		    /** @brief Road capacity, based on calculation of previous variables. */
		    double roadCapacity;

		    /** @brief Number of lanes */
		    double numLanes;

		    /** @brief Time slot for calculation. Division between update (monitoring interval) and number of seconds in an hour. */
		    double timeSlot;

		    /**
		     * @brief A function to calculate road capacity. During initialization this method will be called to initialized related variable.
		     * @param[in] bc basic capacity
		     * @param[in] cfw correction factor for width of the road
		     * @param[in] csf correction factor for side friction
		     * @param[in] cfcs correction factor for city size
		     * @param[out] Edge capacity in passenger car unit (pce)
		     *
		     *@return Value of the edge capacity
		     */
		    double calculateCapacity(double bc, double cfw, double csf, double cfcs, double numLanes);

            /**
             *  @brief Traffic volume. Number of vehicle per time unit, updated regularly
             *  @param[in] nMC current number of indexed motorcycles
             *  @param[in] nLV current number of indexed light vehicles
             *  @param[in] nHV current number of indexed heavy vehicles
             *
             *   @return Traffic volume at the time t in passenger car unit
             */
            double trafficVolume();

            /** @brief Signal for recording traffic volume */
            simsignal_t trafficVolumeSignal;

            /** @brief Time interval for congestion information update */
            simtime_t updateRoadConditionInterval;

            /** @brief Self message for road condition update */
            cMessage* RoadConditionUpdateMsg;

            /** @brief Road condition information */
            void updateRoadCondition();

		    /**
		     * @brief VCRatio. Ratio between volume and capacity
		     * @param[in] volume Traffic volume which collected regularly
		     * @param[out] VCRatio Value of ratio between traffic volume and road capacity.
		     */
		    double VCRatio(double volume);
		    simsignal_t VCRatioSignal;

		    /** @brief Road ID where this RSU is located. */
		    std::string roadId;

		    /** @brief Enable this variable to test the traffic intervention. */
		    bool trafficInterventionTestEnabled;

		    /** @brief When the intervention will start? */
		    simtime_t interventionTime;

		    /** @brief Self-message for scheduled intervention. */
		    cMessage* scheduledIntervention;

		    /** @brief Method to execute while receiving a beacon */
		    virtual void onBeacon(WaveShortMessage* wsm);

		    /** @brief Method to execute while receiving a data */
		    virtual void onData(WaveShortMessage* wsm);

		void sendMessage(std::string blockedRoadId);
		virtual void sendWSM(WaveShortMessage* wsm);

		   /** @brief
		     * Method to append vehicles to the list
		     * @param carId is used to pass the address of sender node.
		     * @param firstEmptyArrayIndex is used to pass the first empty array index to be found.
		     */
		    virtual void append2List(short carId, short firstEmptyArrayIndex, simtime_t messageTime, std::string vType);

		    /** @brief
		     * This method is used to determine if a vehicle already listed or not.
		     * @param carId is used to pass the address of sender node.
		     * @param counter is used to pass the number of listed vehicles as in currentNumberofVehicles.
		     * @return indexing status of a car
		     * @see currentNumberofVehicles
		     */
		    virtual bool indexedCar(short carId, short counter);

		    /**
		     * @brief
		     * Printing list of detected vehicles and related information.
		     * @param counter last array element.
		     */
		    virtual void showInfo(short counter);

		    /**
		     * @brief
		     * Update the last seen time of an indexed vehicle
		     */
		    virtual void updateLastSeenTime(short carId, short counter, simtime_t messageTime);

		    /** Overloading handleSelfMsg to include vType */
		    virtual void handleSelfMsg(cMessage* msg);

		    /** Handling regular update of statistics signals */
		    virtual void updateStats();

		    /** delete timed out vehicle from the list and restructure the array */
		    virtual void deleteAndRestructureArray();
};

#endif
