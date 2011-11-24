/* -*- mode:c++ -*- ********************************************************
 * file:        MoBANLocal.h
 *
 * author:      Majid Nabi <m.nabi@tue.nl>
 *
 *              http://www.es.ele.tue.nl/nes
 *
 *
 * copyright:   (C) 2010 Electronic Systems group(ES),
 *              Eindhoven University of Technology (TU/e), the Netherlands.
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:    MoBAN (Mobility Model for wireless Body Area Networks)
 * description:     Implementation of the local module of the MoBAN mobility model
 ***************************************************************************
 * Citation of the following publication is appreciated if you use MoBAN for
 * a publication of your own.
 *
 * M. Nabi, M. Geilen, T. Basten. MoBAN: A Configurable Mobility Model for Wireless Body Area Networks.
 * In Proc. of the 4th Int'l Conf. on Simulation Tools and Techniques, SIMUTools 2011, Barcelona, Spain, 2011.
 *
 * BibTeX:
 *		@inproceedings{MoBAN,
 * 		author = "M. Nabi and M. Geilen and T. Basten.",
 * 	 	title = "{MoBAN}: A Configurable Mobility Model for Wireless Body Area Networks.",
 *    	booktitle = "Proceedings of the 4th Int'l Conf. on Simulation Tools and Techniques.",
 *    	series = {SIMUTools '11},
 *    	isbn = {978-963-9799-41-7},
 *	    year = {2011},
 *    	location = {Barcelona, Spain},
 *	    publisher = {ICST} }
 *
 **************************************************************************/


#include "MiXiMDefs.h"
#include "BaseMobility.h"

/**
 * @brief This is the local mobility module of MoBAN. It should be instantiated in each node that belongs to a WBAN. The NED parameter "coordinatorIndex"
 * determine to which WBAN (MoBANCoordinator) it belongs.
 * The current implementation uses the Random Walk Mobility Model (RWMM) for individual (local) movement with a sphere around the node, with given speed
 * and sphere radius of the current posture. The reference point of the node it the current posture, the sphere radius, and the speed is given by the
 * corresponding coordinator through the signaling system. RWMM determines the location of node at ant time relative to the given reference point.
 *
 * @ingroup mobility
 * @ingroup MoBAN
 * @author Majid Nabi
 */
class MIXIM_API MoBANLocal : public BaseMobility
{
  protected:

    /** @brief Reference point of the node in the current posture. It is gotten from the MoBAN coordinator through signaling system */
    Coord referencePoint;

    /** @brief The radius of local mobility of the node in the current posture. It is gotten from the MoBAN coordinator through signaling system. */
    double radius;

    /** @brief The speed of local mobility of the node in the current posture. It is gotten from the MoBAN coordinator through signaling system. */
    double speed;

	/** @brief parameters to handle the movement of the host*/
    /*@{*/
	/** @brief Size of a step*/
    Coord stepSize;
    /** @brief Total number of steps */
    int numSteps;
    /** @brief Number of steps already moved*/
    int step;
    /*@}*/

    /** @brief The relative target position of the current move */
    Coord targetPos;

    /** @brief The relative position of the node in the next step */
    Coord stepTarget;

  public:
    /** @brief Variable to keep the category of the information that the coordinator publishes. */
    const static simsignalwrap_t catBBMoBANMsgSignal;

    /** @brief Initializes the parameters of the local mobility module.*/
    virtual void initialize(int);

  protected:

    /** @brief Move the node*/
    virtual void makeMove();

    /** @brief Selects a target position for the next move and set the corresponding variables*/
	virtual void setTargetPosition();

    /** @brief Function which is called when something is written to the signaling system of this node */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    /** @brief Gets a position and return the nearest point inside the simulation area if the point is outside the area*/
    Coord insideWorld(Coord apoint);


};
