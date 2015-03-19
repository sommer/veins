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
    double timeoutMsgInterval;
    cMessage* timeoutMsg;

	public:
        virtual void finish();
		virtual void initialize(int stage);
	    enum WaveApplMessageKinds {
	        UPDATE_STATS,
	        CHECK_TIMEOUT
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

		    /** @brief variable containing the vehicle type */
		    std::string myType;

		    /** @brief variable storing vehicle type. Will be feed to scalar later */
		    vType vTypeInt;

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
