/***************************************************************************
                          CHannelInfoTest.cc  -  description
                             -------------------
    begin                : Fri Jan 11 2008
    copyright            : (C) 2007 by wessel
    email                : wessel@tkn.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <omnetpp.h>
#include <ChannelInfo.h>
#include <asserts.h>

/**
 * Unit test for isInRectangle method of class Coord
 *
 * - test with one AirFrame
 * - test with removed AirFrame
 */
void testIntersections() {
	
	ChannelInfo testChannel;

	//test with one AirFrame
	AirFrame* frame1 = new AirFrame();
	frame1->setDuration(2.0);
	
	testChannel.addAirFrame(frame1, 1.0);
	
	ChannelInfo::AirFrameVector v;
	testChannel.getAirFrames(0.0, 0.9, v);	
	assertTrue("No intersecting AirFrames before single AirFrame.", v.empty());
	
	v.clear();
	testChannel.getAirFrames(3.1, 3.9, v);	
	assertTrue("No intersecting AirFrames after single AirFrame.", v.empty());
	
	v.clear();
	testChannel.getAirFrames(0.5, 1.5, v);	
	assertFalse("Cut with start should intersect.", v.empty());
	assertEqual("Cut with start should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(2.5, 3.5, v);	
	assertFalse("Cut with end should intersect.", v.empty());
	assertEqual("Cut with end should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(1.5, 2.5, v);	
	assertFalse("Interval total in AirFrame duration should intersect.", v.empty());
	assertEqual("Interval total in AirFrame duration should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(0.5, 3.5, v);	
	assertFalse("AirFrame total in interval duration should intersect.", v.empty());
	assertEqual("AirFrame total in interval duration should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(3.0, 3.9, v);	
	assertFalse("Upper border should count as intersect.", v.empty());
	assertEqual("Upper border intersection should return single AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(0.0, 1.0, v);	
	assertFalse("Lower border should count as intersect.", v.empty());
	assertEqual("Lower border intersection should return single AirFrame.", frame1, v.front());
	
	
	//add another AirFrame
	AirFrame* frame2 = new AirFrame();
	frame2->setDuration(1.0);
	testChannel.addAirFrame(frame2, 2.5);
	
	v.clear();
	testChannel.getAirFrames(0.5, 2.44, v);	
	assertEqual("Interval before second AirFrame should return only first.", 1, v.size());
	assertEqual("Interval before second AirFrame should return the first AirFrame.", frame1, v.front());
	
	v.clear();
	testChannel.getAirFrames(3.05, 4.0, v);	
	assertEqual("Interval after first AirFrame should return only second.", 1, v.size());
	assertEqual("Interval after first AirFrame should return the second AirFrame.", frame2, v.front());
	
	v.clear();
	testChannel.getAirFrames(2.44, 2.55, v);	
	assertEqual("Interval inside both AirFrames should return both.", 2, v.size());
	bool bothReturned =    (v.front() == frame1 && v.back() == frame2) 
						|| (v.front() == frame2 && v.back() == frame1);
	assertTrue("Interval inside both AirFrame should return both.", bothReturned);
	
	//remove first one
	testChannel.removeAirFrame(frame1);
	
	v.clear();
	testChannel.getAirFrames(2.51, 2.9, v);	
	assertEqual("Interval inside both AirFrame should return also the deleted.", 2, v.size());
	bothReturned =    (v.front() == frame1 && v.back() == frame2) 
							|| (v.front() == frame2 && v.back() == frame1);
	assertTrue("Interval inside both AirFrame should return also the deleted.", bothReturned);
	
	//add another AirFrame which intersects with second but not with first
	
	AirFrame* frame3 = new AirFrame();
	frame3->setDuration(1.5);
	testChannel.addAirFrame(frame3, 3.5);
	
	v.clear();
	testChannel.getAirFrames(2.51, 3.5, v);	
	assertEqual("Interval inside all AirFrame should return all.", 3, v.size());
	
	//remove second AirFrame
	testChannel.removeAirFrame(frame2);
	
	v.clear();
	testChannel.getAirFrames(1.51, 2.0, v);	
	assertTrue("Interval before second frame should be empty (first one is deleted).", v.empty());
	
	v.clear();
	testChannel.getAirFrames(3.5, 3.6, v);	
	assertEqual("Interval inside second and third AirFrame should return also the second (deleted).", 2, v.size());
	bothReturned =    (v.front() == frame2 && v.back() == frame3) 
							|| (v.front() == frame3 && v.back() == frame2);
	assertTrue("Interval inside both AirFrame should return also the deleted.", bothReturned);
	
	//remove third AirFrame
	testChannel.removeAirFrame(frame3);
	
	v.clear();
	testChannel.getAirFrames(0.0, 10.0, v);	
	assertTrue("There shouldn't be anymore AirFrames.", v.empty());
	
}

int main() {
    testIntersections();
}

