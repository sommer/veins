//**************************************************************************
// * file:        REDMacPkt.msg
// *
// * author:      Jochen Adamek
// * copyright:   (c) by 'the Glimmer Twins'
// *              Telecommunication Networks Group
// *              TU-Berlin
// * email:       adamek@tkn.tu-berlin.de
// **************************************************************************
// * part of:     framework implementation developed by tkn
// *************************************************************************/

cplusplus {{
#include "MacPkt_m.h"
}};

class MacPkt;

//
// Defines all fields of an RED Mac frame
//
message REDMacPkt extends MacPkt
{
   fields:
       int token;
       double time; //processing delay of message
       int type;
       int repetitionCounter; // maximum number of following repetitions
       
};

