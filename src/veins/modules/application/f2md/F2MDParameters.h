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

#define MAX_TIME_DELTA 3.1
#define MAX_DELTA_INTER 2.0 //1.001
#define MAX_SA_RANGE 210 // 420/2
#define MAX_SA_TIME 2.1 // 2.1

#define MAX_KALMAN_TIME 3.1
#define KALMAN_POS_RANGE 1.0
#define KALMAN_SPEED_RANGE 4.0
#define KALMAN_MIN_POS_RANGE 4.0
#define KALMAN_MIN_SPEED_RANGE 1.0

#define MIN_MAX_SPEED 40 // 40
#define MIN_MAX_ACCEL 3 // 3
#define MIN_MAX_DECEL 4.5 // 4.5
#define MAX_MGT_RNG 4 // 3
#define MAX_MGT_RNG_DOWN 6.2 // 6.1556
#define MAX_MGT_RNG_UP 2.1 // 0.8378

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
#define parVar 0.55 // 55%

#define RandomPosOffsetX 70.0
#define RandomPosOffsetY 70.0
#define RandomSpeedX 40.0
#define RandomSpeedY 40.0
#define RandomSpeedOffsetX 7.0
#define RandomSpeedOffsetY 7.0
#define RandomAccelX 2.0
#define RandomAccelY 2.0
#define StopProb 0.05
#define StaleMessages_Buffer 60 //less than (120 - parVar*120)
#define DosMultipleFreq 4 // times faster
#define DosMultipleFreqSybil 2 // times faster
#define ReplaySeqNum 6
#define SybilVehNumber 5 // max 10
#define SelfSybil false // default true
#define SybilDistanceX 5 // 5 meters
#define SybilDistanceY 2 // 2 meters
#define MAX_SYBIL_NUM 20
#define MAX_STALE_NUM 120  //2 minutes
//Attacks Parameters

//pseudonym Parameters
#define Period_Change_Time 240 //seconds
#define Tolerance_Buffer 10 // nbr of messages
#define Period_Change_Distance 80 //meters
#define Random_Change_Chance 0.1 // 0.01 over 1 = 1% chance
#define MAX_PSEUDO_LIST 100
//pseudonym Parameters

//report parameters
#define InitialHistory 5
#define CollectionPeriod 10
#define UntolerancePeriod 5
#define MAX_REP_PSEUDOS 1000
//report parameters

#endif
