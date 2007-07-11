/* -*- mode:c++ -*- ********************************************************
 * file:        REDMacLayer.cc
 *
 * author:      Jochen Adamek, Andreas Köpke
 *
 * copyright:   (C) 2004,2005,2006
 *              Telecommunication Networks Group (TKN) at Technische
 *              Universitaet Berlin, Germany.
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
 ***************************************************************************/
 
#include "REDMacLayer.h"
#include "NicControlType.h"
#include "FWMath.h"
#include "RSSI.h"
#include "MacControlInfo.h"


Define_Module( REDMacLayer )

  void REDMacLayer::initialize(int stage)

{
  BasicMacLayer::initialize(stage);

  if(stage==0){
    	
    busyRSSI = hasPar("busyRSSI") ? par("busyRSSI").doubleValue() : -90;
    maxPacketLength = hasPar("maxPacketLength") ? par("maxPacketLength") : 50;//verändern

    radioState = RadioState::RECV;
    rssi = 0;
    RadioState cs;
    RSSI rs;
    REDMacControlInfo cInfo;
    mediumState = MediumIndication::IDLE;
    MediumIndication ind;
    	
    catRadioState = bb->subscribe(this, &cs, parentModule()->id());
    catRSSI = bb->subscribe(this, &rs, parentModule()->id());
    catIndication = bb->subscribe(this, &ind, parentModule()->id());
    
    radio = SingleChannelRadioAccess().get();
		
    // initialize the timer
    timer = new cMessage("timer");
    sampleTimer = new cMessage("wakeup");
    rssiStableTimer = new cMessage("rssistable");
    timeOut = new cMessage("timeOut");
        
    restLaufzeit = 0;
    congestionLevel = 0;
    error = FAIL;
    shortRetryCounter = 0;
    longRetryCounter = 0;
    checkCounter = 0.0;
    rxTime = 0.0;
    sim_time = 0.0;
    teamgeistType = 0;
    txBufPtr = NULL;
    ackMsg = NULL;
    flags = 0;
    	
    	
        
    sleepTime = DEFAULT_SLEEP_TIME;
    for(unsigned i=0; i < MSG_TABLE_ENTRIES; i++){
      knownMsgTable[i].age = MAX_AGE;
    }    
    for(MIN_BACKOFF_MASK = 1; MIN_BACKOFF_MASK < sleepTime;){
      MIN_BACKOFF_MASK = (MIN_BACKOFF_MASK << 1) +1;
    }
    seqNo = intrand(0xFFFF) % TOKEN_ACK_FLAG;
    macState = INIT;
  }
  else if(stage==1) {
    
    int channel;
    channel = hasPar("defaultChannel") ? par("defaultChannel") : 0;

    radio->setBitrate(12482); // put all stuff (UART 4b6b etc. into net bit rate)
    radio->setActiveChannel(channel);
    	
    splitControlStart();
    myMacAddr = parentModule()->id();
    	
    	
    EV << " busyRSSI = " << busyRSSI << endl;
    busyRSSI = FWMath::dBm2mW(busyRSSI);
    // physical header: 6 bytes (uncoded -> 4bytes) + RedMac: 12 bytes + 2
    // to do: adjust snreval headerlength to 6*8 o#mnetpp.ini
    headerLength = 18 * 8;
  }
}

void REDMacLayer::handleUpperMsg(cMessage *msg)
{  
  macSend(msg,msg->byteLength());
}

void REDMacLayer::handleLowerMsg(REDMacPkt *pkt)	
{
  Error_t er = SUCCESS;
  //subrtract the size of the header
  int len = (pkt->byteLength() - headerLength);
  receiveDone(pkt,len,er); 
}

void REDMacLayer::handleUpperControl(REDMacPkt *pkt){
	
}
 
void REDMacLayer::handleSelfMsg(cMessage *msg)
{	
  if(msg == timer){
    EV << " timer fired " << endl;
    timerFired();
  }
  else if(msg == sampleTimer){
    EV << " sampleTimer fired " << endl;
    sampleTimerFired();
  }
  else if(msg == rssiStableTimer){
    EV << " rssiStableTimer fired " << endl;
    rssiStable();	
  }
  else if(msg == timeOut){
    //receiveDone(txBufPtr,len,FAIL);
  }
}

void REDMacLayer::handleLowerControl(cMessage *msg)
{
  if(msg->kind() == NicControlType::TRANSMISSION_OVER) {
    EV << " transmission over" << endl;
    packetSendDone(txBufPtr,SUCCESS);
  }
  else {
    EV << "control message with wrong kind -- deleting\n";
  }
  delete msg;
}

void REDMacLayer::packetSendDone(REDMacPkt *pkt, Error_t error){
  if(macState == TX) {
    EV << "TX --> RX_ACK" << endl;
    macState = RX_ACK;
    setRxMode();
    scheduleAt(simTime() + startOneShot(RX_ACK_TIMEOUT), timer);
    checkCounter = 0;
  }
  else if(macState == TX_ACK) {
    checkCounter = 0;
    macState = RX;
    setRxMode();
    EV << "repCounter: " << repCounter << endl;
  }
    	
}

REDMacLayer::Error_t REDMacLayer::packetSend(REDMacPkt *pkt){	
  //macLength = (pkt->getByteLength())* (2.0/3.0)/8;
  error = SUCCESS;
  sendDown(static_cast<REDMacPkt *>(pkt->dup()));
  EV << " Passes the message down to the REDMAC Layer " << endl;	
  return error;
}


REDMacPkt* REDMacLayer::receiveDone(REDMacPkt* pkt, int len, Error_t error){
	    
  action = STOP;
  bool isCnt;
  double nav = 0;
  cInfo = new REDMacControlInfo(); // TODO: nur dort allozieren wo angehängt 
		        
  if(macState == RX_P) {
    EV << " macState in RX_P-modus " << endl;
    if(error == SUCCESS) {
      EV << " SUCCESS " << endl;
      isCnt = isControl(pkt);
      if(msgIsForMe(pkt)) {
	if(!isCnt) {
	  EV << "" << endl;
	  if(isNewMsg(pkt)) {
	    EV << " new message arrived " << endl;
	    if(isOwner()) {
	      cInfo->setStrength(snr);
	      pkt->setControlInfo(cInfo);
	      cInfo = NULL;
	    }
	    else {
	      cInfo->setStrength(1);
	      pkt->setControlInfo(cInfo);
	      cInfo = NULL;
	    }
	    pkt->setTime(calcGeneratedTime(pkt));
	    cInfo->setAck(WAS_NOT_ACKED);
	    pkt->setControlInfo(cInfo);
	    //cInfo = NULL; 
	    macReceiveDone(pkt);
	    cInfo = NULL;
	    rememberMsg(pkt);
	  }
	  if((needsAckRx(pkt)) && (action != RX)) {
	    EV << " needsAckRx and not RX " << endl;
	    action = CCA_ACK;
	  }
	  else {
	    EV << " no change to CCA_ACK " << endl;
	    if(action != RX) {
	      nav = startOneShot((pkt->getRepetitionCounter() *
				  (SUB_HEADER_TIME + pkt->byteLength()*BYTE_TIME +
				   SUB_FOOTER_TIME + RX_ACK_TIMEOUT + TX_SETUP_TIME) + ACK_DURATION));
	      action = SLEEP;
	    }
	  }
	}
	else {
	  action = RX;
	}
      }
      else {
	EV << " msg is not for me " << endl;
	action = SLEEP;
	if(!isCnt) {
	  nav = startOneShot((pkt->getRepetitionCounter() *
			      (SUB_HEADER_TIME + pkt->byteLength()*BYTE_TIME +
			       SUB_FOOTER_TIME + RX_ACK_TIMEOUT + TX_SETUP_TIME) +
			      ACK_DURATION));
	}
      }
    }
    else {
      EV << " RX " << endl;
      action = RX;
    }
  }
  else if(macState == RX_ACK_P) {
    if(error == SUCCESS) {
      if(ackIsForMe(pkt)) {
	EV << " ack is for me " << endl;
	cInfo->setStrength(FWMath::dBm2mW(rssi)); //TODO: dB umrechnen
	cInfo->setAck(WAS_ACKED);
	pkt->setControlInfo(cInfo);
	cInfo = NULL;
	if((isFlagSet(&flags, TEAMGEIST_ACTIVE)) && (pkt->getType() == teamgeistType)){
	  gotAck(pkt, pkt->getSrcAddr(), cInfo->getStrength());
	  if(cInfo==NULL) delete cInfo;

	}
	EV << " teamgeist not active " << endl;
	signalSendDone(SUCCESS);
	EV << " signalSendDone " << pkt->getSrcAddr() << endl;
	action = SLEEP;
      }
      else {
	updateLongRetryCounters();
	action = RX;
      }
    }
    else {
      if(timer->isScheduled())  {
	EV << " timer scheduled " << endl;
	action = RX_ACK;
      }
      else {
	EV << " timer not scheduled " << endl;
	updateLongRetryCounters();
	action = RX;
      }
    }
  }
  else {
    EV << " action = INIT " << endl;
    action = INIT;
  }
  if(action == CCA_ACK) {
    macState = CCA_ACK;
    if(intrand(0xFFFF) & 2) {
      scheduleAt(simTime() + startOneShot(RX_SETUP_TIME - TX_SETUP_TIME + RECEIVE_DONE_TIME + ADDED_DELAY), timer); //umwandeln

    }
    else {
      macState = TX_ACK;
      scheduleAt(simTime() + startOneShot(RX_SETUP_TIME - TX_SETUP_TIME	+ RECEIVE_DONE_TIME), timer); //durch Konstanten ersetzen
    }
    prepareAck(pkt);
  }
  else if(action == RX_ACK) {
    macState = RX_ACK;
  }
  else if(action == RX) {
    macState = RX;
    checkCounter = 0;
    scheduleAt(simTime() + startOneShot(DATA_DETECT_TIME), timer);
  }
  else if(action == SLEEP) {
    macState = SLEEP;
    if(isFlagSet(&flags, RESUME_BACKOFF)){
      double help = startOneShot(restLaufzeit);
      if(nav > help) help += nav;
      restLaufzeit = static_cast<int>(help*32768);
    }
    else {
      setFlag(&flags, RESUME_BACKOFF);
      restLaufzeit = intrand(0xFFFF) & ZERO_BACKOFF_MASK;
    }
    setSleepMode();
  }
  else if(action == INIT) {
    clearFlag(&flags, UNHANDLED_PACKET);
  }
  else {
    EV << " another state " << endl;
  }
       
  delete pkt;
  return NULL;
}
    
	


void REDMacLayer::receiveDetected(){
  setFlag(&flags, ACTION_DETECTED);
  EV << "receiveDetected" << endl;
  if(macState <= CCA_ACK) {
    scheduleAt(simTime() + (startOneShot(BYTE_TIME) * maxPacketLength), timeOut); //50Byte
    if(macState == CCA) computeBackoff();
    if(macState != RX_ACK) {
      macState = RX_P;
    } else {
      macState = RX_ACK_P;
                
    }
  }
  else if(macState == INIT) {
    EV << " INIT " << endl; 
    setFlag(&flags, UNHANDLED_PACKET);
  }
}  
  
void REDMacLayer::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
  Enter_Method("receiveBBItem: cat %i, scope: %i", category, scopeModuleId);
  BasicMacLayer::receiveBBItem(category, details, scopeModuleId);
	
  if(category == catRadioState){
    radioState = static_cast<const RadioState *>(details)->getState();
    EV << "radioState: " << radioState << endl;
    if(radioState == RadioState::RECV) {
      EV << " RadioState is RECV " << endl;
      rxModeDone();
      scheduleAt(simTime() + 0.5, rssiStableTimer); //halbe ms
    }
    else if(radioState == RadioState::SEND) {
      EV << " radio in SEND state, sendDown packet " << endl;
      txModeDone();
    }
    else if(radioState == RadioState::SLEEP) {
      EV << " radio in SLEEP state " << endl;
      sleepModeDone();	
    }
  }
  else if(category == catRSSI){
    rssi = static_cast<const RSSI *>(details)->getRSSI();
		
  }
  else if(category == catIndication){
    mediumState = static_cast<const MediumIndication *>(details)->getState();
    if(mediumState == MediumIndication::BUSY) receiveDetected();
  }
}

void REDMacLayer::releaseAdcTask(){
	    
}

void REDMacLayer::requestAdc(){
	
}

void REDMacLayer::setRxMode() {
  if(radioState == RadioState::RECV) {
    rxModeDone();
    if(!rssiStableTimer->isScheduled()) rssiStable();
  } 
  else {
    setFlag(&flags, SWITCHING);
    clearFlag(&flags, RSSI_STABLE);
    if(rssiStableTimer->isScheduled()) cancelEvent(rssiStableTimer);
    EV << " RX " << endl;
    checkCounter = 0;
    radio->switchToRecv();
  }
  if(RadioState::SLEEP){
    EV << "error, radio couldn't be switched!\n";
  }
}    
    
void REDMacLayer::setSleepMode() {
  clearFlag(&flags, RSSI_STABLE);
  if(rssiStableTimer->isScheduled()) cancelEvent(rssiStableTimer);
  
  if(radioState == RadioState::SLEEP) {
    sleepModeDone();
  }
  else {
    EV << " setSleepMode " << endl;
    clearFlag(&flags, RSSI_STABLE);
    releaseAdcTask();
    setFlag(&flags, SWITCHING);
    radio->switchToSleep();
  }
}  


void REDMacLayer::setTxMode() {
  clearFlag(&flags, RSSI_STABLE);
  if(rssiStableTimer->isScheduled()) cancelEvent(rssiStableTimer);
  
  if(radioState == RadioState::SEND) {
    txModeDone();
  }
  else {
    releaseAdcTask();
    EV << " TX " << endl;
    clearFlag(&flags, RSSI_STABLE);
    setFlag(&flags, SWITCHING);
    radio->switchToSend();
  }
  if(RadioState::SLEEP){
    EV << "error, radio couldn't be switched!\n";
  }
}        
      

void REDMacLayer::setSleepTime(int sT){ 
  sleepTime = sT;
  scheduleAt(simTime() + startOneShot(sleepTime), sampleTimer);
  for(MIN_BACKOFF_MASK = 1; MIN_BACKOFF_MASK < sleepTime; ) {	
    MIN_BACKOFF_MASK = ((MIN_BACKOFF_MASK << 1) + 1);
  }
  MIN_BACKOFF_MASK = MIN_BACKOFF_MASK >>= 3; 
}

int REDMacLayer::getSleepTime() { //handleUpperControl
  int st;
  st = sleepTime;
  return st;    	
}

void REDMacLayer::ageMsgsTask(){
  unsigned i;
  for(i = 0; i < MSG_TABLE_ENTRIES; i++){
    if(knownMsgTable[i].age <= MAX_AGE) ++knownMsgTable[i].age;	
  }	
}
	

void REDMacLayer::startDoneTask() {
  REDMacPkt *pkt = new REDMacPkt();
  EV << " start the sampleTimer " << endl;
  scheduleAt(simTime() + startOneShot(sleepTime), sampleTimer);
  macState = SLEEP;
  setFlag(&flags, TEAMGEIST_ACTIVE);
  teamgeistType = observedAMType(pkt);
  //startDone(SUCCESS);        
}

void REDMacLayer::stopDoneTask(){

}
    
    
REDMacLayer::Error_t REDMacLayer::splitControlStart() {
  macState = INIT;
  return SUCCESS;
}

REDMacLayer::Error_t REDMacLayer::splitControlStop() {
  cancelEvent(timer);
  cancelEvent(sampleTimer);
  if((macState == SLEEP) && (isFlagSet(&flags, SWITCHING))) {
    macState = STOP;
    EV << "  inside the function splitcontrolStop STOP " << endl;
  }
  else {
    macState = STOP;
    setSleepMode();
    EV << "  inside the function splitcontrolStop STOP and switch to SLEEP " << endl;
  }
  return SUCCESS;
}


void REDMacLayer::rssiStable(){
  setFlag(&flags, RSSI_STABLE);
  if((macState == RX) || (macState == CCA)) {
    scheduleAt(simTime() + startOneShot(DATA_DETECT_TIME), timer);
    EV << " start of the timer " << endl;
  }     
  else if(macState == INIT) {
    EV << " INIT " << endl;
    updateNoiseFloor();
  }        
}

void REDMacLayer::rxModeDone(){
	
  clearFlag(&flags, SWITCHING);
  if((macState == RX) || (macState == RX_ACK) || (macState == CCA) ||
     (macState == INIT) || (macState == STOP)) {
    EV << "" << endl;
    if(macState != RX_ACK) requestAdc();
  }
  else {
    EV << " another macState " << endl;
  }	
}

void REDMacLayer::txModeDone(){
	
  EV << " the tx-Mode is done " << endl;
  clearFlag(&flags, SWITCHING);
  if(macState == TX) {
    setFlag(&flags, ACTION_DETECTED);
    if(packetSend(txBufPtr) == SUCCESS) {    
      EV << " packetSend " << endl;
    }
    else {
      EV << " no packetSend " << endl;
    }
  }
  else if(macState == TX_ACK) {
    if(packetSend(ackMsg) == SUCCESS) {    
      EV << " packetSend " << endl;
    } else {
      EV << " no packetSend " << endl;
    }
  }
  else {
    EV << " macState is not TX_ACK " << endl;
  }
}
	
void REDMacLayer::sleepModeDone(){
  EV << " the sleep-Mode is done " << endl;
  clearFlag(&flags, SWITCHING);
  if(isFlagSet(&flags, ACTION_DETECTED)) {
    if(congestionLevel < 5) congestionLevel++;
  } 
  else {
    if(congestionLevel > 0) congestionLevel--;
  }
  // if(congestionLevel > 3) EV << "congestionLevel: " << congestionLevel << endl);
  if(macState == SLEEP) {
    EV << " SLEEP " << endl;
    if(!timer->isScheduled()) {
      EV << " no timer is scheduled " << endl;
      if(isFlagSet(&flags, RESUME_BACKOFF)) {
	EV << " RESUME_BACKOFF " << endl;
	clearFlag(&flags, RESUME_BACKOFF);
	scheduleAt(simTime() + startOneShot(restLaufzeit), timer);
	restLaufzeit = 0;
      }
      else {
	EV << " no RESUME_BACKOFF " << endl;
	checkSend();
      }
    }
  }
  else if(macState == INIT) {
    EV << " INIT " << endl;
  }
  else if(macState == STOP) {
    EV << " STOP " << endl;
    stopDoneTask();
  }
  congestionEvent(congestionLevel);	
}	


void REDMacLayer::checkSend(){
  EV << "checksend" << endl;
  if((shortRetryCounter) && (txBufPtr != NULL) && (isFlagSet(&flags, MESSAGE_PREPARED)) && 
     (macState == SLEEP) && (!isFlagSet(&flags, RESUME_BACKOFF)) && (!timer->isScheduled())){
    EV << " macState switch to CCA " << endl;
    macState = CCA;
    checkCounter = 0;
    setRxMode();
  }
  else {
    if(macState != SLEEP) EV << " macState != SLEEP " << endl;
    if(txBufPtr) EV << " txBufrBtr " << endl;
    if(shortRetryCounter) EV << " shortRetryCounter " << endl;
    if(isFlagSet(&flags, MESSAGE_PREPARED)) EV << " MESSAGE_PREPARED " << endl;
    if(txBufPtr) {
      if(macState == SLEEP)EV << "macState is sleep" << endl;
      if(!isFlagSet(&flags, RESUME_BACKOFF)) EV << " RESUME_Backoff " << endl;
      if(!timer->isScheduled()) EV << "timer is  not scheduled" << endl;
    }
  }
}

int REDMacLayer::backoff(int counter) {
  int rVal = intrand(0xFFFF) & MIN_BACKOFF_MASK;
  return (rVal << counter) + ZERO_BACKOFF_MASK;
}

bool REDMacLayer::needsAckTx(REDMacPkt *pkt){
	 
  cInfo = new REDMacControlInfo(); 
  bool rVal = false;
  if(pkt->getDestAddr() < AM_BROADCAST_ADDR) {
    if(cInfo->getAck() != NO_ACK_REQUESTED) {
      rVal = true;
    }
  }
  return rVal;	
}

bool REDMacLayer::needsAckRx(REDMacPkt *pkt){
	
  bool rVal = false;
  int dest = pkt->getDestAddr();
  int token;
  double snr = 1;
  if(dest < AM_BROADCAST_ADDR) {
    if(dest < RELIABLE_MCAST_MIN_ADDR) {
      token = pkt->getToken();
      if(isFlagSet(&flags, ACK_REQUESTED)) { 
	rVal = true;
      }
    }
    else {
      if((isFlagSet(&flags, TEAMGEIST_ACTIVE)) &&
	 (pkt->getType() == teamgeistType)) { 
	snr = rssi;
	rVal = tgNeedsAck(pkt, pkt->getSrcAddr(), pkt->getDestAddr(), snr);
      }
    }
  }
  return rVal;
}


void REDMacLayer::prepareMsgTask(){
  int length = txLen;
  int sT = sleepTime;
  REDMacPkt *pkt = txBufPtr;
  int dest = pkt->getDestAddr();
    
  pkt->setRepetitionCounter(sT/(length * BYTE_TIME + SUB_HEADER_TIME + SUB_FOOTER_TIME + TX_GAP_TIME) + 1);
  if((longRetryCounter > 1) &&
     (isFlagSet(&flags, TEAMGEIST_ACTIVE) &&
      (pkt->getType() == teamgeistType))) {
    dest = getDestination(pkt, longRetryCounter - 1);
  }
  pkt->setToken(seqNo);
  if(needsAckTx(pkt))
    (pkt->setToken(pkt->getToken() | ACK_REQUESTED));  
  setFlag(&flags, MESSAGE_PREPARED);
  if((macState == SLEEP) && (!timer->isScheduled()) && (!isFlagSet(&flags, RESUME_BACKOFF))) {   
    if((longRetryCounter == 1) &&
       (dest != AM_BROADCAST_ADDR)) {
      scheduleAt(simTime() + startOneShot((intrand(0xFFFF) >> 3) & (ZERO_BACKOFF_MASK)), timer);
    }
    else {
      scheduleAt(simTime() + startOneShot(backoff(longRetryCounter)), timer);
    }
  }
}


bool REDMacLayer::prepareRepetition(){ //TODO
  int repeat;                    //txBufr == txMacHdr
  if(isFlagSet(&flags, CANCEL_SEND)){
    repeat = txBufPtr->getRepetitionCounter(); 
    txBufPtr->setRepetitionCounter(0);
  }
  else {  
    repeat = txBufPtr->getRepetitionCounter();
    txBufPtr->setRepetitionCounter(repeat - 1);
  }
  return repeat != 0;//Vergleich != 0
}

void REDMacLayer::signalSendDone(Error_t error){
    
  REDMacPkt *pkt = new REDMacPkt();
  cInfo = new REDMacControlInfo();
  EV << " signalSendDone " << endl;
  txBufPtr = NULL;
  longRetryCounter = 0;
  shortRetryCounter = 0;
    
  cInfo->setStrength(FWMath::dBm2mW(rssi));
  pkt->setControlInfo(cInfo);
  delete cInfo;

  if(isFlagSet(&flags, CANCEL_SEND)) {       
    error = XCANCEL;
  }
  clearFlag(&flags, MESSAGE_PREPARED);
  clearFlag(&flags, CANCEL_SEND);
  
  EV << "CANCEL_SEND:" << error << endl;
  EV << "CANCEL_SEND:" << pkt->getType() << endl;
  macSendDone(pkt, error);
	
}

void REDMacLayer::updateRetryCounters(){
  shortRetryCounter++;
  if(shortRetryCounter > MAX_SHORT_RETRY){
    longRetryCounter++;
    shortRetryCounter = 1;
    if(longRetryCounter > MAX_LONG_RETRY){
      EV << " longRetryCounter greater than MAX_LONG_RETRY " << endl;
      signalSendDone(FAIL);
    }
  }
}

void REDMacLayer::updateLongRetryCounters(){
  clearFlag(&flags, MESSAGE_PREPARED);
  longRetryCounter++;
  shortRetryCounter = 1;
  if(longRetryCounter > MAX_LONG_RETRY){
    EV << " longRetryCounter greater than MAX_LONG_RETRY " << endl;
    signalSendDone(FAIL);
  } else {
    prepareMsgTask();
  }
	
}

bool REDMacLayer::ackIsForMe(REDMacPkt *pkt){
	
  int dest = pkt->getDestAddr();
  int localToken = seqNo;
  setFlag(&flags, TOKEN_ACK_FLAG);
  if((dest == myMacAddr) && (localToken == pkt->getToken())) return true;
  return false;
	
}

void REDMacLayer::interruptBackoffTimer() {
  if(timer->isScheduled()) {	
    restLaufzeit = static_cast<int>((timer->arrivalTime() - simTime())*32768);
    cancelEvent(timer);
    setFlag(&flags, RESUME_BACKOFF);
  }
}

bool REDMacLayer::msgIsForMe(REDMacPkt *pkt){
  if(pkt->getDestAddr() == AM_BROADCAST_ADDR) return true;
  if(pkt->getDestAddr() == myMacAddr) return true;
  if(pkt->getDestAddr() >= RELIABLE_MCAST_MIN_ADDR) return true;
  return false;

}

bool REDMacLayer::isControl(REDMacPkt *pkt){
  int token = pkt->getToken(); 
  return isFlagSet(&token, TOKEN_ACK_FLAG);
}

void REDMacLayer::computeBackoff() {
  if(!isFlagSet(&flags, RESUME_BACKOFF)){
    setFlag(&flags, RESUME_BACKOFF);
    restLaufzeit = backoff(longRetryCounter);
    updateRetryCounters();
  }
}


bool REDMacLayer::isNewMsg(REDMacPkt *pkt){
	
  bool rVal = true;
  for(unsigned int i=0;i < MSG_TABLE_ENTRIES; i++){
    if((pkt->getSrcAddr() == knownMsgTable[i].src) &&
       (((pkt->getToken()) & TOKEN_ACK_MASK) == knownMsgTable[i].token) &&
       (knownMsgTable[i].age < MAX_AGE)) {
      knownMsgTable[i].age = 0;
      rVal = false;
      break;
    }
  }
  return rVal;
}


unsigned REDMacLayer::findOldest(){
  unsigned i;
  unsigned oldIndex = 0;
  unsigned age = knownMsgTable[oldIndex].age;
  for(i = 1; i < MSG_TABLE_ENTRIES; i++) {
    if(age < knownMsgTable[i].age) {
      oldIndex = i;
      age = knownMsgTable[i].age;
    }
  }
  return oldIndex;
}

void REDMacLayer::rememberMsg(REDMacPkt *pkt){
	
  unsigned oldest = findOldest();
  knownMsgTable[oldest].src = pkt->getSrcAddr();
  knownMsgTable[oldest].token = (pkt->getToken()) & TOKEN_ACK_MASK;
  knownMsgTable[oldest].age = 0;
}

void REDMacLayer::prepareAck(REDMacPkt *pkt){
	
  REDMacPkt *ackMac = new REDMacPkt("ackMac");
  int rToken = ackMac->getToken() & TOKEN_ACK_MASK;
  setFlag(&flags, TOKEN_ACK_FLAG);
  ackMac->setToken(rToken);
  ackMac->setSrcAddr(myMacAddr);
  ackMac->setDestAddr(pkt->getSrcAddr());
  ackMac->setType(pkt->getType());

  repCounter = pkt->getRepetitionCounter();
  packetSend(ackMac); 
	
}

double REDMacLayer::calcGeneratedTime(REDMacPkt *pkt){
  return rxTime - (pkt->getTime()) - startOneShot(TIME_CORRECTION);
}

void REDMacLayer::channelBusy(){
  EV << " channel is BUSY " << endl;
  checkOnBusy();
}

void REDMacLayer::channelIdle(){
  EV << " channel is IDLE " << endl;
  checkOnIdle();
}

void REDMacLayer::checkOnBusy() {
  setFlag(&flags, ACTION_DETECTED);
  EV << " checkOnBusy " << endl;
  if((macState == RX) || (macState == CCA) || (macState == CCA_ACK)) {
    if(macState == CCA) {
      computeBackoff();
    }
    requestAdc();
    macState = RX;
    checkCounter = 0;
    scheduleAt(simTime() + startOneShot(TX_GAP_TIME>>1), timer);
  }
}
    
void REDMacLayer::checkOnIdle()  {
  EV << " checkOnIdle " << endl;
  if(macState == RX) {
    checkCounter++;
    if(checkCounter >= 3) {
      EV << " after 3 times, the macState is switched to SLEEP " << endl;
      macState = SLEEP;
      setSleepMode();
    }
    else {
      EV << " the timer is started in RX-state " << endl;
      scheduleAt(simTime() + startOneShot(TX_GAP_TIME>> 1), timer);
      requestAdc();
    }
  }
  else if(macState == CCA) {
    checkCounter++;
    if(checkCounter < 3) {
      EV << " the timer is started in CCA-state " << endl;                
      scheduleAt(simTime() + startOneShot(TX_GAP_TIME>> 1), timer);
      requestAdc();
    }
    else {
      EV << " switch macState to TX " << endl;
      macState = TX;
      setTxMode();
    }
  }
  else if(macState == CCA_ACK) {
    EV << " switch macstate to TX_ACK " << endl;
    macState = TX_ACK;
    setTxMode();
  }
}
    

void REDMacLayer::timerFired(){
  EV << " timer fired " << endl;
  if((macState == RX) || (macState == CCA) || (macState == CCA_ACK)) {
    channelMonitorStart();
  }
  else if(macState == RX_ACK) {
    if(prepareRepetition()) {
      EV << " macState is changed to TX " << endl;
      macState = TX;
      setTxMode();
    }
    else {
      if(needsAckTx(txBufPtr)) {
	EV << " needsAckTx " << endl;
	updateLongRetryCounters();
      }
      else {
	EV << " no needsAckTx " << endl;
	signalSendDone(SUCCESS);
      }
      macState = SLEEP;
      setSleepMode();
    }
  }
  else if(macState == TX_ACK) {
    setTxMode();
    EV << " setTxMode " << endl;
  }
  else if(macState == SLEEP) {
    if(isFlagSet(&flags, SWITCHING)) {
      EV << " start of the timer " << endl;
      scheduleAt(simTime() + startOneShot((intrand(0xFFFF) & 0x0f)), timer);            
    }
    else {
      if(isFlagSet(&flags, RESUME_BACKOFF)) {
	EV << " RESUME_BACKOFF " << endl;
	clearFlag(&flags, RESUME_BACKOFF);
	scheduleAt(simTime() + startOneShot(restLaufzeit), timer);
	restLaufzeit = 0;
      }
      else {
	EV << " no RESUME_BACKOFF " << endl;
	checkSend();
      }
    }
  }
  else if((macState == RX_ACK_P) || (macState == RX_P)) {
    EV << " macState is RX_ACK_P or RX_P " << endl;
  }
  else if(macState == INIT) {
    EV << endl << endl;
    startDoneTask();
  }
  else {
    EV << " another macState " << endl;
  }
}

void REDMacLayer::sampleTimerFired(){
  scheduleAt(simTime() + startOneShot(sleepTime), sampleTimer);
  EV << " start of sampleTimer " << endl;
  if((macState == SLEEP) && (!isFlagSet(&flags, SWITCHING))) {
    clearFlag(&flags, ACTION_DETECTED);
    interruptBackoffTimer();
    macState = RX;
    EV << " macState changed to RX " << endl;
    setRxMode();
    cancelEvent(timer);
  }
  else {
    if(macState != SLEEP) EV << "macState != SLEEP, it is" << macState << endl;
    if(isFlagSet(&flags, SWITCHING)) EV << "SWITCHING" << endl;
  }
  ageMsgsTask();
}

void REDMacLayer::macReceiveDone(REDMacPkt *pkt){
	
  int dest = pkt->getDestAddr();
  cInfo = new REDMacControlInfo();
  // decaps +  sendup, controlinfo ausfüllen + verschieben
  // KEIN delete 
  if(dest == myMacAddr || dest == L2BROADCAST){
    //pkt->setLength(headerLength/(8.0*1.5));
    pkt->setControlInfo(new REDMacControlInfo(pkt->getSrcAddr()));
    sendUp(decapsMsg(pkt));
  }
}


double REDMacLayer::macSend(cMessage *msg, int len){
  error = SUCCESS;
  EV << " got a message from above" << endl;   
  if((shortRetryCounter == 0) && (txBufPtr == NULL)) {
    clearFlag(&flags, MESSAGE_PREPARED);
    EV << " the message is prepared " << endl;
    shortRetryCounter = 1;
    longRetryCounter = 1;
    txBufPtr = REDMacLayer::encapsMsg(msg); 
    txLen = txBufPtr->byteLength(); 
    seqNo++;
    if(seqNo >= TOKEN_ACK_FLAG) seqNo = 1;
  }
  else {
    EV << " XBUSY " << endl;
    error = XBUSY;
  }
  if(error == SUCCESS) {
    prepareMsgTask();
  }
  else {
    if(shortRetryCounter) EV << " short retry counter != 0" << endl;   
    if(txBufPtr) EV << " txBufPtr != 0" << endl;   
  }
  return error;
}

void REDMacLayer::macSendDone(REDMacPkt *pkt, Error_t error){
	
}

double REDMacLayer::macCancel(REDMacPkt *pkt){
  error = FAIL;
  if(pkt == txBufPtr) { 
    EV << " CANCEL_SEND " << endl;
    setFlag(&flags, CANCEL_SEND);
    shortRetryCounter = MAX_SHORT_RETRY + 2;
    longRetryCounter  = MAX_LONG_RETRY + 2;
    if(macState == SLEEP) {
      EV << " SLEEP " << endl;
      signalSendDone(XCANCEL);
    }
    else {
      EV << " macState is not SLEEP " << endl;
    }
    EV << " macState: " << macState << endl;
    error = SUCCESS;
  }
  else {
    EV << " macState: " << macState << endl;
  }
  return error;
        
}

void REDMacLayer::updateNoiseFloor(){
  updateNoiseFloorDone(); 
}

void REDMacLayer::updateNoiseFloorDone(){
  if(macState == INIT) {
    scheduleAt(simTime() + startOneShot(intrand(0xFFFF) % DEFAULT_SLEEP_TIME), timer);
    setSleepMode();
  } 
  else {
    EV << " macState is not INIT and the sampleTimer is not scheduled " << endl;
  }
	  
}

bool REDMacLayer::isOwner(){
  return true;
}

void REDMacLayer::setFlag(int *which, int pos) { 
  (*which) |= pos;
}
 
void REDMacLayer::clearFlag(int *which, int pos){
  (*which) = (*which) & (~pos);
}

bool REDMacLayer::isFlagSet(const int *which, int pos){
  return (*which) & pos;	
}

void REDMacLayer::channelMonitorStart(){
  if(rssi < busyRSSI) {
    channelIdle();
  } 
  else {
    channelBusy();
  }
}

int REDMacLayer::observedAMType(REDMacPkt *pkt){
  int x = 0;
  return x;
}

bool REDMacLayer::tgNeedsAck(REDMacPkt *pkt, int src, int dest, double snr){
  bool needsAck = true;
  return needsAck;
}

double REDMacLayer::estimateForwarders(REDMacPkt *pkt){
  double estimate = 0;
  return estimate;
}

int REDMacLayer::getDestination(REDMacPkt *pkt, int retryCounter){
  return retryCounter;
	
}

void REDMacLayer::gotAck(REDMacPkt *pkt, int acksender , double snrValue){
}

void REDMacLayer::congestionEvent(int level){
}

void REDMacLayer::finish(){
  if (!timer->isScheduled()) delete timer;
  if (!sampleTimer->isScheduled()) delete sampleTimer;
  if (!rssiStableTimer->isScheduled()) delete rssiStableTimer;
  if (!timeOut->isScheduled()) delete timeOut;
}

double REDMacLayer::startOneShot(int t){ //changes tic-values into seconds
  return (double)t/32768.0;
}

/**double REDMacLayer::milliToDB(double db){
   return 10*log(db);
   }**/

REDMacPkt* REDMacLayer::encapsMsg(cMessage *msg)
{  
  REDMacPkt *pkt = new REDMacPkt(msg->name(), msg->kind());
  pkt->setLength(headerLength);

  // copy dest address from the Control Info attached to the network
  // mesage by the network layer
  MacControlInfo* cInfo = static_cast<MacControlInfo*>(msg->removeControlInfo());

  EV <<"CInfo removed, mac addr="<< cInfo->getNextHopMac()<<endl;
  pkt->setDestAddr(cInfo->getNextHopMac());

  //delete the control info
  delete cInfo;

  //set the src address to own mac address (nic module id())
  pkt->setSrcAddr(myMacAddr);
  
  //encapsulate the network packet
  pkt->encapsulate(msg);
  EV <<"pkt encapsulated\n";
  
  return pkt;
}
const int REDMacLayer::BYTE_TIME =  9;                // byte at 23405 kBit/s, 4b6b encoded, 0.00064087 s.
const int REDMacLayer::PREAMBLE_BYTE_TIME = 6;        // byte at 23405 kBit/s, no coding, 0.00042725 s.
const int REDMacLayer::PHY_HEADER_TIME = 36;           // 6 Phy Preamble at 23405 bits/s, 0.00256347 s.

const int REDMacLayer::TIME_CORRECTION = 7;           // difference between txSFD andrxSFD: 475us, 0.00048828 s.
const int REDMacLayer::SUB_HEADER_TIME = PHY_HEADER_TIME + 7*BYTE_TIME;
const int REDMacLayer::SUB_FOOTER_TIME = 2*BYTE_TIME;         // 2 bytes crc
/*
  const int REDMacLayer::DEFAULT_SLEEP_TIME = 1625;        // 0.04959106 s.
  const int REDMacLayer::DEFAULT_SLEEP_TIME = 3250;        // 0.09918213 s.
  const int REDMacLayer::DEFAULT_SLEEP_TIME = 6500;        // 0.19836426 s.
  const int REDMacLayer::DEFAULT_SLEEP_TIME = 9750;        // 0.29754639 s.
**/
const int REDMacLayer::DEFAULT_SLEEP_TIME = 16384;               // 0.5 s.
//const int REDMacLayer::DEFAULT_SLEEP_TIME = 32768;             // 1.0 s.
const int REDMacLayer::DATA_DETECT_TIME = 17;          // 0.00051880 s.
const int REDMacLayer::RX_SETUP_TIME = 102;             // time to set up receiver, 0.00311279 s.
const int REDMacLayer::TX_SETUP_TIME = 58;             // time to set up transmitter, 0.00177001 s.
const int REDMacLayer::RECEIVE_DONE_TIME = 16;         // 0.00048828 s.  
const int REDMacLayer::ADDED_DELAY = 30;               // 0.00091552 s.
const int REDMacLayer::RX_ACK_TIMEOUT = (RX_SETUP_TIME + PHY_HEADER_TIME + ADDED_DELAY + 30);
const int REDMacLayer::TX_GAP_TIME = (RX_ACK_TIMEOUT + TX_SETUP_TIME + 33);
const int REDMacLayer::ACK_DURATION = (SUB_HEADER_TIME + SUB_FOOTER_TIME);
const int REDMacLayer::MAX_SHORT_RETRY = 9;
const int REDMacLayer::MAX_LONG_RETRY = 1;
const unsigned int REDMacLayer::MAX_AGE = (2*MAX_LONG_RETRY*MAX_SHORT_RETRY);
const unsigned int REDMacLayer::MSG_TABLE_ENTRIES = 20;
const int REDMacLayer::TOKEN_ACK_FLAG = 64;
const int REDMacLayer::TOKEN_ACK_MASK = 0x3F;


//const int REDMacLayer::PREAMBLE_LONG = 5;
//const int REDMacLayer::PREAMBLE_SHORT = 2;
        
const int REDMacLayer::ZERO_BACKOFF_MASK = 0xFF;
 
