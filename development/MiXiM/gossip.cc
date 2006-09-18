#include "node.h"
#include "neighbourrouting.h"

#include <vector>
using namespace std;

// Gossip Node subclass

typedef enum {GOSSIP_DISCOVERY=0} GossipTimer;
#define GOSSIP_TIMERS 1

typedef enum {GOSSIP_MSG_DISC, GOSSIP_MSG_BCAST} GossipType;

#define SECS_BETWEEN_RETX 30

#define MAX_TTL 20
#define FANOUT_LIMIT 3

typedef struct _data_pkt
{
	int src;
	unsigned int seq;
	int ttl;
} data_pkt;

typedef struct 
{
	GossipType type;
	data_pkt d;
	int nextHop;
} GossipPacket;

class Gossip : public NeighbourRouting
{
	private:
		unsigned int seq;
		unsigned int sends;
		int sink;
		int max_ttl;
		int fanout_limit;
		bool broadcast(Packet *,GossipPacket*);
	public:
		Module_Class_Members(Gossip, NeighbourRouting, 0) 
		// Implement Node's abstract functions.
		virtual void initialize(void);
		virtual void handleTimer(unsigned int timer);
		virtual void rx(Packet *msg);
		virtual void tx(Packet *msg);
		virtual void txDone(Packet *msg);
	
		virtual void sendToMac(Packet *msg);
};

Define_Module_Like(Gossip, Routing);

void Gossip::initialize()
{
	NeighbourRouting::initialize();
	sink = getLongParameter("destination", 0);
	seq = 1;
	max_ttl = getLongParameter("max_ttl",MAX_TTL);
	fanout_limit = getLongParameter("fanout_limit",FANOUT_LIMIT);
	initTimers(GOSSIP_TIMERS);
	setTimer(GOSSIP_DISCOVERY,5+intuniform(1,5*10)*0.1);
}

// Dispatch this timer to the proper handler function.
void Gossip::handleTimer(unsigned int count)
{
	GossipTimer idx = (GossipTimer)count;
	switch (idx)
	{
		case GOSSIP_DISCOVERY:
		{
			Packet *disc = new Packet("NeighDiscovery",TX);
			GossipPacket g;
			disc->from = node->getNodeId();
			g.type = GOSSIP_MSG_DISC;
			printf(PRINT_ROUTING,": gossip disc");
			disc->setLocalTo(BROADCAST);
			disc->setData(ROUTING_DATA,&g,sizeof(GossipPacket),1);
			sendToMac(disc);
			setTimer(GOSSIP_DISCOVERY,SECS_BETWEEN_RETX+intuniform(1,5)*0.1);
			break;
		}
	}
}

void Gossip::rx(Packet *msg) {
	GossipPacket *g;
	g = (GossipPacket*) msg->getData(ROUTING_DATA);
	data_pkt *d = &(g->d);
	addNeighbour(msg);
	switch (g->type)
	{
		case GOSSIP_MSG_BCAST:
			if (g->nextHop!=node->getNodeId())
			{
				/*if (newNeighbour)
					addTimer(timer(1,GOSSIP_DISCOVERY,intuniform(1,5)*0.1,NULL));*/
				printf(PRINT_ROUTING,"nextHop =%d, and we're %d",g->nextHop,node->getNodeId());	
				break;
			}
			printf(PRINT_ROUTING,"recv'ed a msg from %d seq=%d via %d",d->src,d->seq,msg->from);
			if (node->getNodeId()==sink)
			{
				printf(PRINT_ROUTING,"got a message to a sink!");
				sendToApp(msg);
				break;
			}
			if (!broadcast(msg,g))
			{
				stat_tx_drop++;
				delete msg;
			}
				
			break;

		case GOSSIP_MSG_DISC: // only there to give a neighbour list
			delete msg;
			//printf(PRINT_ROUTING,"message is from %d %d",g->lastHop,g->d.src);
			break;
		
	}
}

void Gossip::tx(Packet *msg) {
	GossipPacket *g;
	msg->from = node->getNodeId();
	g = (GossipPacket*) calloc(1,sizeof(GossipPacket));
	g->d.src= node->getNodeId();
	g->d.seq = seq;
	g->d.ttl = max_ttl;
	seq++;
	printf(PRINT_ROUTING,"Sending from app");
	if (!broadcast(msg,g))
	{
		msg->setKind(TX_FAILED);
		sendToApp(msg);
	}
	free(g);
}

void Gossip::txDone(Packet *msg) {
	if (!msgQueue->empty())
		sendToMac((Packet *) msgQueue->pop());

	GossipPacket *gossipPacket = (GossipPacket *) msg->getData(ROUTING_DATA);
	if (msg->from == node->getNodeId() && gossipPacket->type != GOSSIP_MSG_DISC)
		sendToApp(msg);
	else
		//FIXME: stats?
		delete msg;
}

bool Gossip::broadcast(Packet *msg,GossipPacket *g) {
	data_pkt *d = &g->d;
	if (neighbours->size()==0 || d->ttl <= 0) {
		if (neighbours->size()==0)
			printf(PRINT_ROUTING,"We have no neighbours! Discard app message");
		return false;
	}
	int dest = 0;
	unsigned int i=0;
	printf(PRINT_ROUTING,"nc = %d",neighbours->size());
	for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
	{
		NeighbourInfo *ni = *iter;
		if (ni->idx == sink)
		{
			printf(PRINT_ROUTING,"Broadcasting data src=%d seq=%d dest=%d",d->src,d->seq,sink);
			msg->setLocalTo(sink);
			g->nextHop = 0;
			g->type = GOSSIP_MSG_BCAST;
			msg->setData(ROUTING_DATA,g,sizeof(GossipPacket),sizeof(GossipPacket));
			msg->setKind(TX);
			sendToMac(msg);
			return true;
		}
	}
	if (i!=neighbours->size())
	{
		vector <int> sent;
		int split = (d->ttl>max_ttl-fanout_limit?2:1);
		msg->setKind(TX);
		for (int j=0;j<split;j++) // fanout of 2 up to fanout_limit depth
		{
			while(true)
			{
				dest = intuniform(0,neighbours->size()-1);
				if (sent.size()==0 || sent.at(0) != dest || neighbours->size()==1)
					break;
			}
			sent.insert(sent.begin(),dest);
			i=0;
			for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
			{
				if ((int)i == dest)
				{
					NeighbourInfo *ni = *iter;
					dest = ni->idx;
					printf(PRINT_ROUTING,"assigned dest %d",dest);
					//assert(dest!=this->me);
					printf(PRINT_ROUTING,"Broadcasting data src=%d seq=%d dest=%d, ttl=%d",d->src,d->seq,dest,d->ttl);
					g->nextHop = dest;
					g->type = GOSSIP_MSG_BCAST;
					g->d.ttl--;
					msg->setData(ROUTING_DATA,g,sizeof(GossipPacket),sizeof(GossipPacket));
					msg->setLocalTo(dest);
					Packet *out = (Packet*)msg->dup();
					sendToMac(out);
					break;
				}
				i++;
			}
			assert(i!=neighbours->size());
		}
		delete msg;
	}
	return true;
}

void Gossip::sendToMac(Packet *msg) {
	if (macBusy)
		addToQueue(msg);
	else
		NeighbourRouting::sendToMac(msg);
}
