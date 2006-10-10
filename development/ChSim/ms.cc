/* 
 * Author: Thorsten Pawlak 
 *		Universität Paderborn
 * 
 * Original by: Randy Vu
 *		Technische Universitaet Berlin
 */

#include "omnetpp.h"
#include "channelStateMsg_m.h"

// module class declaration:
class ms:public cSimpleModule {
      public:
	Module_Class_Members(ms, cSimpleModule, 0)
	virtual void initialize();
	virtual void handleMessage(cMessage * msg);
	virtual void finish();

      private:
	void initCir();
	void updatePositionCir();
	void initRWP();
	void updatePositionRWP();
	void initMHG();
	void updatePositionMHG();
	void CalculateDistance();
	void Display();

	int J;
	int PLAYGROUNDSIZEX;
	int PLAYGROUNDSIZEY;
	int GRIDSPACING;
	
	double currentXpos, currentYpos, destXpos, destYpos, startXpos, startYpos;
	double currentSpeed, xSpeed, ySpeed;
	double currentDistance;
	simtime_t startWalking;
	simtime_t arivalTime;
	
	int state; //0 waiting, 1 walking
};

// module type registration:
Define_Module(ms);

void ms::initialize()
{
	J = par("qty_ms");
	PLAYGROUNDSIZEX = par("playgroundSizeX");
	PLAYGROUNDSIZEY = par("playgroundSizeY");
	GRIDSPACING = par("gridSpacing");

	if (int(par("mobilityModel")) == 0) initCir();	// Circular
	if (int(par("mobilityModel")) == 1) initRWP();	// RandomWayPoint
	if (int(par("mobilityModel")) == 2) initMHG();	// ManhattanGrid

	startWalking = simTime() + (double)par("waitTime");
	currentSpeed = 0;
	state = 0;
	
	CalculateDistance();
	Display();
}

void ms::initCir()
{
	currentXpos = uniform(0,PLAYGROUNDSIZEX);
	currentYpos = uniform(0,PLAYGROUNDSIZEY);
}

void ms::initRWP()
{
	currentXpos = uniform(0,PLAYGROUNDSIZEX);
	currentYpos = uniform(0,PLAYGROUNDSIZEY);
}

void ms::initMHG()
{
	currentXpos = intuniform(0,PLAYGROUNDSIZEX / GRIDSPACING) * GRIDSPACING;
	currentYpos = intuniform(0,PLAYGROUNDSIZEY / GRIDSPACING) * GRIDSPACING;
}

// implementation of the module class:
void ms::handleMessage(cMessage * msg)
{

	channelStateMsg *channelStatemsg = (channelStateMsg *) msg;
	
	if (int(par("mobilityModel")) == 0) updatePositionCir();	// Circular
	if (int(par("mobilityModel")) == 1) updatePositionRWP();	// RandomWayPoint
	if (int(par("mobilityModel")) == 2) updatePositionMHG();	// ManhattanGrid
	
//	ev << "WT: " << channelStatemsg->getMsId() << " x: " << currentXpos << " y: " << currentYpos << " distance: " << currentDistance << " speed: " << currentSpeed << endl;

	channelStatemsg->setDistance(currentDistance);
	channelStatemsg->setSpeed(currentSpeed);
	channelStatemsg->setXPos(currentXpos);
	channelStatemsg->setYPos(currentYpos);
	send(channelStatemsg, "out", channelStatemsg->getMsChannel());
}

/* implements circular movement with constant distance to the basestation
 * with optional pause and speed change after each round
 */
void ms::updatePositionCir()
{
	switch (state)
	{
		case 0 : // waiting
			if (simTime() >= startWalking)
			{
				startXpos = currentXpos;
				startYpos = currentYpos;
				currentSpeed = par("speed");
				// t = s/v; s = 2*pi*r
				arivalTime = startWalking + (2 * M_PI * currentDistance) / currentSpeed;
				// v = s/t
				state = 1;
			}
			break;
		case 1 : // walking
			double phi;
			if ((startXpos - PLAYGROUNDSIZEX / 2) > 0) phi = atan((startYpos - PLAYGROUNDSIZEY / 2) / (startXpos - PLAYGROUNDSIZEX / 2));
			if ((startXpos - PLAYGROUNDSIZEX / 2) < 0) phi = atan((startYpos - PLAYGROUNDSIZEY / 2) / (startXpos - PLAYGROUNDSIZEX / 2)) + M_PI;
			if ((startXpos - PLAYGROUNDSIZEX / 2) == 0 && (startYpos - PLAYGROUNDSIZEY / 2) > 0) phi = 0.5 * M_PI;
			if ((startXpos - PLAYGROUNDSIZEX / 2) == 0 && (startYpos - PLAYGROUNDSIZEY / 2) < 0) phi = -0.5 * M_PI;
			if ((startXpos - PLAYGROUNDSIZEX / 2) == 0 && (startYpos - PLAYGROUNDSIZEY / 2) == 0) phi = 0;	//undefined r=0

			// delta_phi = 2 * pi / T * t
			double delta_phi = 2 * M_PI / (arivalTime - startWalking) * (simTime() - startWalking);

			// x = r * cos (phi)
			currentXpos=PLAYGROUNDSIZEX/2 + currentDistance * cos(phi + delta_phi);
			// y = r * sin (phi)
			currentYpos=PLAYGROUNDSIZEY/2 + currentDistance * sin(phi + delta_phi);
			CalculateDistance();
			Display();
			if (simTime() >= arivalTime)
			{	
				startWalking = arivalTime + (double)par("waitTime");
				currentSpeed = 0;
				state = 0;
			}
			break;
	}
}

// implements Random Wapoint Mobility
void ms::updatePositionRWP()
{
	switch (state)
	{
		case 0 : // waiting
			if (simTime() >= startWalking)
			{
				// get next position
				destXpos = uniform(0,PLAYGROUNDSIZEX);
				destYpos = uniform(0,PLAYGROUNDSIZEY);
				startXpos = currentXpos;
				startYpos = currentYpos;
				currentSpeed = par("speed");
				// t = s/v;
				arivalTime = startWalking + fabs(destXpos - startXpos) / currentSpeed;
				// v = s/t
				xSpeed = (destXpos - startXpos) / (arivalTime - startWalking);
				ySpeed = (destYpos - startYpos) / (arivalTime - startWalking);
				state = 1;
			}
			break;
		case 1 : // walking
			// s = v*t
			currentXpos=startXpos + xSpeed * (simTime() - startWalking);
			currentYpos=startYpos + ySpeed * (simTime() - startWalking);
			CalculateDistance();
			Display();
			if (simTime() >= arivalTime)
			{	
				startWalking = arivalTime + (double)par("waitTime");
				currentSpeed = 0;
				state = 0;
			}
			break;
	}
}

// implements Manhattan Grid
void ms::updatePositionMHG()
{
	switch (state)
	{
		case 0 : // waiting
			if (simTime() >= startWalking)
			{
				startXpos = currentXpos;
				startYpos = currentYpos;

				currentSpeed = par("speed");
				// t = s/v;
				arivalTime = startWalking + GRIDSPACING / currentSpeed;

				// get next position
				do {
                    int direction = intuniform(1,4);
                    switch (direction) {
                        case 1:	// north
                            destXpos = currentXpos;
                            destYpos = currentYpos + GRIDSPACING;
                            xSpeed = 0;
                            ySpeed = currentSpeed;
                            break;
                        case 2:	// east
                            destXpos = currentXpos + GRIDSPACING;
                            destYpos = currentYpos;
                            xSpeed = currentSpeed;
                            ySpeed = 0;
                            break;
                        case 3:	// south
                            destXpos = currentXpos;
                            destYpos = currentYpos - GRIDSPACING;
                            xSpeed = 0;
                            ySpeed = - currentSpeed;
                            break;
                        case 4:	// west
                            destXpos = currentXpos - GRIDSPACING;
                            destYpos = currentYpos;
                            xSpeed = - currentSpeed;
                            ySpeed = 0;
                            break;
                    }
				} while ((destXpos > PLAYGROUNDSIZEX) | (destXpos < 0) | (destYpos > PLAYGROUNDSIZEY) | (destYpos < 0));
				
				state = 1;
			}
			break;
		case 1 : // walking
			// s = v*t
			currentXpos=startXpos + xSpeed * (simTime() - startWalking);
			currentYpos=startYpos + ySpeed * (simTime() - startWalking);
			CalculateDistance();
			Display();
			if (simTime() >= arivalTime)
			{	
				startWalking = arivalTime + (double)par("waitTime");
				currentSpeed = 0;
				state = 0;
			}
			break;
	}
}

void ms::CalculateDistance()
{
	// c=sqrt(a^2+b^2), basestation is in the middle;
	currentDistance=sqrt(pow(currentXpos-PLAYGROUNDSIZEX/2, 2)+pow(currentYpos-PLAYGROUNDSIZEY/2, 2));
}	

void ms::Display()
{
	char xStr[32], yStr[32];
	sprintf(xStr, "%d", (int)currentXpos);
	sprintf(yStr, "%d", (int)currentYpos);
	parentModule()->displayString().setTagArg("p", 0, xStr);
	parentModule()->displayString().setTagArg("p", 1, yStr);
}

void ms::finish()
{
}

