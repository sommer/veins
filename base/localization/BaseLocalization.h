/* -*- mode:c++ -*- ********************************************************
 * file:        BaseLocalization.h
 *
 * author:      Peterpaul Klein Haneveld
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     mixim framework
 * description: localization layer: general class for the network layer
 *              subclass to create your own localization layer
 **************************************************************************/


#ifndef BASELOC_H
#define BASELOC_H

#include <BaseLayer.h>

#include "BaseArp.h"
#include "LocPkt_m.h"

#include <list>
#include <string>

using std::string;
/**
 * @brief Base class for maintaining node information for the anchor and
 * neighbor lists.
 *
 * @ingroup localization
 * @author Peterpaul Klein Haneveld
 *
 * This class contains the same fields as the LocPkt message.
 * In case one needs more information, one can extend this class or create
 * a new class with node information. The corresponding methods in 
 * BaseLocalization need to be overwritten.
 */
class NodeInfo {
public:
	NodeInfo(LocPkt * pkt, Coord basePos) 
		: id(pkt->getId()),
		  isAnchor(pkt->getIsAnchor()),
		  pos(pkt->getPos()),
		  distance(basePos.distance(pkt->getPos())) {}

	NodeInfo(int i, bool a, Location p, double d)
		: id(i),
		  isAnchor(a),
		  pos(p),
		  distance(d) {}

	virtual std::string info() const {
		std::stringstream os;
		os << id << "@" << pos.info() << "@" << distance;
		return os.str();
	}

	
	/** Unique identifier of this node. */
	int id;
	/** Weather this node is an anchor node or not. */
	bool isAnchor;
	/** The (estimated) location of this node. */
	Location pos;
	/** The (estimated) distance between this node and the owner. 
	 * @TODO This must be determined base on RSSI once that's added
	 * to Mixim. */
	double distance; 
};

using std::list;

/**
 * @brief Base class for the localization layer
 * 
 * @ingroup localization
 * @author Peterpaul Klein Haneveld
 */
class BaseLocalization:public BaseLayer {

      protected:
	/**
	 * @brief Messagetypes used in the localization layer.
	 *
	 * Should one need more message types, implement this in
	 * the subclass of BaseLocalization like this:
	 *
	 * <code>
	 * enum {
	 * 	LOCALIZATION_MSG = APPLICATION_MSG + 1,
	 * 	...
	 * };
	 * </code>
	 */ 
	enum 
	{
		APPLICATION_MSG = 0,
	};

	int headerLength; /**< @brief Length of the LocPkt header */
	bool isAnchor; /**< @brief Specifies weather this node is an anchor
			* node or not */
	int id; /**< @brief This node's number */
	Location pos; /**< @brief The current estimated location of this node.
		       *
		       * A location also contains the timestamp of calculation,
		       * and a confidence value.
		       */

	list<NodeInfo *> neighbors; /**< @brief The neighbor list */
	list<NodeInfo *> anchors; /**< @brief The anchor list */

      public:
	 Module_Class_Members(BaseLocalization, BaseLayer, 0);

	/**
	 * @brief Initialization of the module and some variables
	 */
	virtual void initialize(int);
	/**
	 * @brief Finalization of the module
	 */
	virtual void finish();
	/**
	 * @brief Returns the actual coordinates of this node.
	 *
	 * In case one also needs the current timestamp, call
	 * getLocation().
	 * @return The current absolute position
	 */
	Coord getPosition();
	/**
	 * @brief Returns the current actual location of this node.
	 *
	 * The current simulation time is set as timestamp.
	 * @return The current absolute location
	 */
	Location getLocation();
	/**
	 * @brief Returns the latest location estimation, with
	 * the timestamp when this position was calculated.
	 * @return The latest estimated location
	 */
	Location getLocationEstimation();

      protected:
	/** 
	 * @name Handle Messages
	 * @brief Functions to redefine by the programmer
	 *
	 * These are the functions provided to add own functionality to your
	 * modules. These functions are called whenever a self message or a
	 * data message from the upper or lower layer arrives respectively.
	 */
	/*@{ */

	/** @brief Handle messages from upper layer */
	 virtual void handleUpperMsg(cMessage * msg);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerMsg(cMessage * msg);

	/** @brief Handle self messages */
	virtual void handleSelfMsg(cMessage * msg) {
		error("BaseLocalization does not handle self messages");
	};

	/** @brief Handle control messages from lower layer */
	virtual void handleLowerControl(cMessage * msg) {
		error("BaseLocalization does not handle control messages");
	};

	/** @brief Handle control messages from lower layer */
	virtual void handleUpperControl(cMessage * msg) {
		error("BaseLocalization does not handle control messages");
	};

	/** @brief Send a message to another node.
	 *
	 * Use this method in the implementation of a localization
	 * algorithm to send messages. It can easily be overwritten
	 * as it isn't used anywhere in the BaseLocalization class.
	 */
	void sendMsg(cMessage *);
	
	/** @brief Handle a message sent by the Localization layer.
	 *
	 * Overwrite this method in localization algorithms if you
	 * use sendMsg() to send messages from the localization layer.
	 * The received message is exactly the same as the one handed
	 * to sendMsg().
	 */
	virtual void handleMsg(cMessage *) {
		error("Subclasses of BaseLocalization should implement handleMsg()!");
	}
	/*@} */

	/**
	 * @brief Decapsulates messages received from the network layer.
	 */
	virtual cMessage *decapsMsg(cMessage *);

	/**
	 * @brief Encapsulates application layer messages.
	 */
	virtual cMessage *encapsMsg(cMessage *, int);

	/** @name Localization methods
	 * @brief Localization functions that can be redefined by
	 * the programmer.
	 */

	/** @brief Perform actions on the anchor list
	 * 
	 * When the anchor doesn't need to be stored, this function must
	 * delete the node.
	 *  @return the anchor if it should be stored, NULL otherwise */
	virtual NodeInfo * handleNewAnchor(NodeInfo * node) { return node; }
	/** @brief Perform actions on the neighbor list
	 *
	 * When the node doesn't need to be stored, this function must
	 * delete the node.
	 *  @return the node if the new neighbor should be stored, NULL otherwise */
	virtual NodeInfo * handleNewNeighbor(NodeInfo * node) { return node; }
	/** @brief Perform actions on the updated neighbor list */
	virtual void handleMovedNeighbor(NodeInfo *) {}
	/*@} */

private:
	/*@{ */
	/** @brief Check if the sender of this position exists in the
	 * anchor list.
	 *
	 * This method is called in decapsMsg() when the sender of the
	 * message is an anchor. If the anchor list doesn't already
	 * have this point it is added to the anchor list and
	 * handleNewAnchor() is called.
	 * @param msg The received message (LocPkt)
	 */
	virtual void newAnchor(cMessage * msg);
	/** @brief Check if the sender of this message exists in the
	 * neighbor list or if the neighbor has an updated position.
	 *
	 * This method is called in decapsMsg() when the sender of the
	 * message is not an anchor. If the neighbor list doesn't contain
	 * this node yet, it is added to the list and handleNewNeighbor()
	 * is called, otherwise the positions are compared and if the
	 * position is updated handleMovedNeighbor() is called.
	 * @param msg The received message (LocPkt)
	 */
	virtual void newNeighbor(cMessage * msg);
};

#endif
