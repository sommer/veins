/* -*- mode:c++ -*- ********************************************************
 * file:        ExampleLocalization.h
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
 * description: example localization class
 **************************************************************************/

#ifndef EXAMPLE_LOCALIZATION_H
#define EXAMPLE_LOCALIZATION_H

#include "BaseLocalization.h"
#include "LocPkt_m.h"

#define MAX_NEIGHBOURS 10

struct nghbor_info {
	int idx;
	double distance;
	Coord position;
};

class ExampleLocalization:public BaseLocalization {
      public:
	Module_Class_Members(ExampleLocalization, BaseLocalization, 0);

	virtual void initialize(int);
	virtual void finish();
	virtual void estimatePosition();

	enum APPL_MSG_TYPES {
		MSG_POSITION
	};

      private:
	double accuracy;
	cLinkedList neighbors;
	Coord position;
	void updateNeighbor(cMessage * msg);

      protected:
	virtual void handleLowerMsg(cMessage *);
	virtual void sendPosition();

};

#endif				/* EXAMPLE_LOCALIZATION_H */
