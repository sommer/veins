#ifndef __PATTERN_H__
#define __PATTERN_H__

#include "mixim.h"
#include "appselector.h"
#include <vector>


class Pattern : public MiximBaseModule {
	Module_Class_Members(Pattern, MiximBaseModule, 0);

private:
	cMessage * timeout1_msg, * timeout2_msg;
	void handleMessage(cMessage * msg);

protected:
	int turned_on;	// active pattern on active nod
	int node_id;
	int stat_tx, stat_rx;
	int stat_delay_count;
	double stat_delay_total;
	virtual void rx(Packet * msg);
	virtual void activated();
	virtual void deActivated();
	virtual void init();
	virtual void timeout();
	virtual void timeout2();
	virtual void rxDelay(cMessage * msg) {}

	void tx(Packet * msg);
	void setTimeout(double time);
	void setTimeout2(double time);
	void cancelTimeout();
	void cancelTimeout2();

	void initialize();
	void finish();
		
	vector<int> *getNeighbours();
	vector<int> *getNeighbours(int id);
	vector<int> *getNeighbours(Node *node);

	virtual ~Pattern();

public:
	int is_active; // idle or active pattern
	double msginterval;
	int msglength;

};

#endif
