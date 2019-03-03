/*******************************************************************************
 * @author  Joseph Kamel
 * @email   josephekamel@gmail.com
 * @date    28/11/2018
 * @version 2.0
 *
 * SCA (Secure Cooperative Autonomous systems)
 * Copyright (c) 2013, 2018 Institut de Recherche Technologique SystemX
 * All rights reserved.
 *******************************************************************************/

#ifndef __VEINS_MDParameters_H_
#define __VEINS_MDParameters_H_
#include <veins/modules/application/f2md/mdEnumTypes/AttackTypes.h>
#include <veins/modules/application/f2md/mdEnumTypes/MbTypes.h>
#include <veins/modules/application/f2md/mdEnumTypes/MdAppTypes.h>
#include <veins/modules/application/f2md/mdEnumTypes/MdChecksVersionTypes.h>
#include <veins/modules/application/f2md/mdEnumTypes/PseudoChangeTypes.h>
#include <veins/modules/application/f2md/mdEnumTypes/ReportTypes.h>


//Detection Parameters
#define MAX_CONFIDENCE_RANGE 10 //3
#define MAX_PLAUSIBLE_RANGE 420 // 417

#define MAX_TIME_DELTA 1.1
#define MAX_DELTA_INTER 1.0 //1.001
#define MAX_SA_RANGE 210 // 420/2

#define MIN_MAX_SPEED 40 // 40
#define MIN_MAX_ACCEL 3 // 3
#define MIN_MAX_DECEL 4.5 // 4.5
#define MAX_MGT_RNG 4 // 3
#define MAX_MGT_RNG_DOWN 5.5 // 5.36874
#define MAX_MGT_RNG_UP 2.15// 0.129
#define MAX_BEACON_FREQUENCY 0.99 //0.99998
#define MAX_DISTANCE_FROM_ROUTE 2 //4.5
#define MAX_NON_ROUTE_SPEED -1 //3
#define MAX_HEADING_CHANGE 90 //90
#define DELTA_BSM_TIME 5 // application
#define DELTA_REPORT_TIME 5 // history report
#define POS_HEADING_TIME 1.1
//Detection Parameters


//Storage Parameters
#define MAX_BSM_LENGTH 20
#define MAX_MDM_LENGTH 20
#define MAX_NODES_LENGTH 200
#define MAX_TARGET_LENGTH 1000
#define MAX_TARGET_TIME 2
#define MAX_ACCUSED_LENGTH 1000
#define MAX_ACCUSED_TIME 2
#define MAX_INTER_NUM 10
//Storage Parameters

//Attacks Parameters
#define RandomPosX 3900.0
#define RandomPosY 1700.0
#define RandomPosOffsetX 70.0
#define RandomPosOffsetY 70.0
#define RandomSpeedX 40.0
#define RandomSpeedY 40.0
#define RandomSpeedOffsetX 7.0
#define RandomSpeedOffsetY 7.0
#define StopProb 0.01
#define StaleMessages_Buffer 9 //less than 20
#define DosMultipleFreq 4 // times faster
#define DosMultipleFreqSybil 1 // times faster
#define SybilVehNumber 9 // max 10
#define SelfSybil true // default true
#define SybilDistanceX 5 // 5 meters
#define SybilDistanceY 2 // 2 meters
#define MAX_SYBIL_NUM 10
//Attacks Parameters

//pseudonym Parameters
#define Period_Change_Time 240 //seconds
#define Tolerance_Buffer 10 // nbr of messages
#define Period_Change_Distance 80 //meters
#define Random_Change_Chance 0.1 // 0.01 over 1 = 1% chance
#define MAX_PSEUDO_LIST 200
//pseudonym Parameters

#endif
