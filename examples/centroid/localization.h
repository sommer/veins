/* -*- mode:c++ -*- ********************************************************
 * file:        localization.h
 *
 * author:      Aline Baggio
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
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
 * description: data types for the localization module
 **************************************************************************/

#ifndef __LOCALIZATION_H__
#define __LOCALIZATION_H__

/** 3D coordinates of an anchor with node id */
struct PositionData {
  int id;       // anchor id
  double x;     // anchor x coordinate
  double y;     // anchor y coordinate
  double z;     // anchor z coordinate
  double range; // anchor radio range
  double ts;    // coordinate reception time
};

#endif

