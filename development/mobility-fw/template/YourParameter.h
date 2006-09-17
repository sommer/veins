/***************************************************************************
 * file:        YourParameter.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: template file for a blackboard parameter
 ***************************************************************************
 * changelog:   $Revision: 382 $
 *              last modified:   $Date: 2006-07-31 16:10:24 +0200 (Mo, 31 Jul 2006) $
 *              by:              $Author: koepke $
 **************************************************************************/

#include <Blackboard.h>
#include <sstream>

class YourParameter : public BBItem
{
    /**
     * This macro helps the black board to track the inheritance structure
     */
    BBITEM_METAINFO(BBItem);
    
 public:
    
    /** you need at least a proper constructor, define the rest to suit your
     * needs.
     *
     * A general hint: the blackbord comes in handy for performance
     * evaluations. Publish everything that is interesting from your protocol
     * and use a second module to evaluate it. This way you can leave the
     * performance evaluation code inside the code that you want to make
     * publically available */
    YourParameter() : BBItem() {};

    /** When this function is properly defined, it visualizes the state of the
     * parameter on TK */
    std::string info() const {
        std::ostringstream ost;
        ost << " YourParameter has the following value: ";
        return ost.str();
    }
};

#endif
