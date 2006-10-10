#include <omnetpp.h>

// module class declaration:
class triggerGen:public cSimpleModule {
      public:
	Module_Class_Members(triggerGen, cSimpleModule, 0)
	virtual void initialize();
	virtual void handleMessage(cMessage * msg);
	virtual void finish();

      private:
    double MAX_TIME;		// time length (in seconds) for the simulation
	int SAMPLE_PER_SEC;		// how often channelState samples should be generated per second, e.g. 500 means every 2ms		
    
	// simulation timer - calculates the simulation real-time
	double start_time;	
	int sample_counter;
};

// module type registration:
Define_Module(triggerGen);

void triggerGen::initialize()
{
	ev << "triggerGen->initialize()" << endl;
	
	start_time = time(NULL);
	sample_counter = 0;
	
	MAX_TIME = par("max_time");
	SAMPLE_PER_SEC = par("sample_per_sec");
	
	scheduleAt(simTime() + (double) 1 / SAMPLE_PER_SEC, new cMessage("timer"));	// self triggering first time
}

// implementation of the module class:
void triggerGen::handleMessage(cMessage * msg)
{
	if(msg->isName("timer")) {	// msg is timer event

		fprintf(stderr,"simTime [s]: t=%.2f\r",simTime()), fflush(stderr);

		cMessage *msgout = new cMessage("trigger");
		send(msgout, "trigger");
		sample_counter++;
		
		// schedule next call, runs until MAX_TIME * SAMPLE_PER_SEC are sent
		if(sample_counter < MAX_TIME * SAMPLE_PER_SEC){
			scheduleAt(simTime() + (double) 1 / SAMPLE_PER_SEC, msg);
		}
	}
}

void triggerGen::finish()
{
	ev << "LENGTH OF SIMULATION: " << time(NULL) - start_time << endl;
}
