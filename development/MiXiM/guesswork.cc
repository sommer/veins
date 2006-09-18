#include "exor.h"
#include <string.h>

// Guesswork Node subclass

// Define timer types
enum GuessworkTimer {TIMER_SND_SINK=EXOR_TIMER_COUNT, TIMER_START_DELAY};
#define GUESSWORK_TIMER_COUNT 2
enum GuessworkPktType {MSG_SINK_BCAST=EXOR_PKT_COUNT, MSG_SOURCE_XMIT};

typedef struct {
	unsigned int id;
	unsigned int etx;
} sink_info;

// flag in the incoming metric for "this other node will in fact send stuff onwards"
#define USEFUL_SENDER 0x8000

class Guesswork : public ExOR
{
	double accuracy;

	cLinkedList sinks;

	void startDelay();
	void sinkBroadcast();
	sink_info * sendSink(int id, int etx, sink_info *oldsink);
	sink_info * addSink(int id, int etx, sink_info *oldsink);

	int app_seqno;

	sink_info *sink_send;

	int sink;

	bool sendSource(Packet *msg,int sequence);

	void update_neighbor(Packet * msg);

	void exor_func(exor*, Packet *,const ExORAction);
	sink_info * const getSink(const unsigned int sinkid);

  public:
	~Guesswork();
	Module_Class_Members(Guesswork, ExOR, 0)
	// Implement Node's abstract functions.
	virtual void initialize();
	virtual void handleTimer(unsigned int idx);
	virtual void rx(Packet *msg);
	virtual void tx(Packet *msg);
	virtual void txDone(Packet *msg);
};

Define_Module_Like(Guesswork, Routing);

void Guesswork::initialize()
{
	ExOR::initialize();
	initTimers(EXOR_TIMER_COUNT+GUESSWORK_TIMER_COUNT);
	sink = getLongParameter("destination", 0);
	app_seqno  = 1;
	setTimer(TIMER_START_DELAY,5.0);
}

Guesswork::~Guesswork()
{
	for (cLinkedListIterator iter(sinks); !iter.end(); iter++)
		delete (sink_info *) iter();
}

// Dispatch this timer to the proper handler function.
void Guesswork::handleTimer(unsigned int idx)
{
	if (idx<EXOR_TIMER_COUNT)
	{
		ExOR::handleTimer(idx);
		return;
	}
	switch ((GuessworkTimer)idx) {
		case TIMER_SND_SINK:
			sinkBroadcast();
			break;
	
		case TIMER_START_DELAY:
			startDelay();
			break;
	}
	return;
}

void Guesswork::tx(Packet *msg)
{
	if (!sendSource(msg,app_seqno))
	{
		msg->setKind(TX_FAILED);
		sendToApp(msg);
	}
	else
		app_seqno++;
}

void Guesswork::rx(Packet * msg)
{
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	if (h->type<=EXOR_PKT_COUNT)
	{
		ExOR::rx(msg);
		return;
	}
	//printf(PRINT_ROUTING," Guesswork recv'ed a msg "<<msg->kind()<<"\n";
	switch ((GuessworkPktType)h->type)
	{
		case MSG_SINK_BCAST:
			update_neighbor(msg);
			delete msg;
			break;

		case MSG_SOURCE_XMIT:
		{
			/*Header h_out;
			h_out.seqno = h->seqno;
			h_out.periodsend = h->periodsend;
			msg->setData(ROUTING_DATA,&h_out,sizeof(Header),EXOR_HEADER_SIZE);*/
			if (h->true_dest == node->getNodeId())
				sendToApp(msg);
			else
				delete msg;
			//assert(1==0);
		}
			break;

	}
	return;
}

sink_info * Guesswork::sendSink(int id, int etx, sink_info *oldsink)
{
	sink_info *snk = addSink(id,etx,oldsink);
	setTimer(TIMER_SND_SINK,1.0);
	sink_send = snk;
	return snk;
}

sink_info* Guesswork::addSink(int id,int etx, sink_info *oldsink)
{
	sink_info *snk;
	if (oldsink == NULL)
	{
		printf(PRINT_ROUTING,"addSink(): new sink id = %d",id);		
		snk = new sink_info;
		snk->id = id;
		sinks.insert(snk);
	}
	else
		snk = oldsink;
	snk->etx = etx & ~USEFUL_SENDER;
	//printf(PRINT_ROUTING," storing "<<snk->etx<<" "<<etx<<"\n";
	return snk;
}

void Guesswork::startDelay()
{
	if (node->getNodeId()==sink) {
		// Sink nodes need to broadcast location to neighbours.
		printf(PRINT_ROUTING,"I'm a sink!");
		sendSink(node->getNodeId(),0,NULL);
	}
}

void Guesswork::update_neighbor(Packet * msg)
{
	sink_info *snk = NULL;
	//int src = msg->from;

	bool found = false;
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	unsigned int sinkid = h->sinkid;
	unsigned int new_cost = h->etx + h->tx_count;
	for (cLinkedListIterator iter(sinks); !iter.end(); iter++)
	{
		snk = (sink_info *) iter();

		if (snk->id == sinkid) {
			if (new_cost >= snk->etx)
				found = true;
			break;
		}
		snk = NULL;
	}

	if (!found)
	{
		printf(PRINT_ROUTING," from %d improved sink %d with etx=%d",msg->from,sinkid,new_cost);
		//sendSink(sinkid,new_cost,snk);
	}
}

void Guesswork::sinkBroadcast()
{
	Packet *msg = new Packet("SINK", TX);
	Header h;
	//msg->setDest(BCAST_DEST);
	sink_info *sink = sink_send;
	printf_nr(PRINT_ROUTING,"Broadcasting sink id=%d, etx=%d",sink->id,sink->etx);
	h.sinkid = sink->id;
	h.metric = sink->etx;
	h.seqno = app_seqno++;
	h.type = MSG_SINK_BCAST;
	h.sent_as = node->getNodeId();
	msg->setData(ROUTING_DATA,&h,sizeof(Header),EXOR_HEADER_SIZE);
	sendFlood(msg);
}

bool Guesswork::sendSource(Packet *msg, int sequence)
{
	if (sinks.length()==0)
	{
		printf(PRINT_ROUTING,"sendSource() failed due to lack of sinks");
		return false;
	}
	Header h;
	unsigned int sentto=0;
	h.seqno = sequence;
	h.type = MSG_SOURCE_XMIT;
	h.metric = 0;
	msg->setData(ROUTING_DATA,&h,sizeof(Header),EXOR_HEADER_SIZE);
	printf(PRINT_ROUTING,"sendSource");
	for (cLinkedListIterator iter(sinks); !iter.end(); iter++) {
		sink_info* snk = (sink_info *) iter();
		printf(PRINT_ROUTING,"sending exor msg to %d", snk->id);
		sendExOR(snk->id,snk->id,msg,snk->etx|USEFUL_SENDER,neighbours);
		sentto++;
	}
	return sentto!=0;
}

sink_info * const Guesswork::getSink(const unsigned int sinkid)
{
	sink_info *snk = NULL;
	printf(PRINT_ROUTING,"getSink(): sink id = %d",sinkid);
	for (cLinkedListIterator iter(sinks); !iter.end(); iter++)
	{
		snk = (sink_info *) iter();
		if (snk->id == sinkid)
			return snk;
	}
	return NULL;
}

void Guesswork::exor_func(exor *e, Packet *info, const ExORAction action)
{
	unsigned int sinkid=0;
	Header *h = (Header*)info->getData(ROUTING_DATA);
	unsigned int metric = h->metric&~USEFUL_SENDER;
	bool sender_will_forward = (h->metric&USEFUL_SENDER)>0;
	//if (action == EXOR_MSG_BCAST || action == EXOR_BCAST_REPLY)
		sinkid = h->sinkid;
	/*else
		sinkid = h->true_dest;*/
	//printf(PRINT_ROUTING,"sinkid = %d",sinkid);	
	sink_info *snk = getSink(sinkid);

	switch(action)
	{
		case EXOR_MSG_BCAST:
			printf(PRINT_ROUTING,"incoming sink message to %d",h->true_dest);
			if (snk == NULL)
			{
				printf(PRINT_ROUTING,"new sink %d, etx= %d tx %d",sinkid,h->metric,h->tx_count);
				assert(sinkid==0);
				assert(h->metric<100000);
				snk = addSink(sinkid, h->metric+h->tx_count, NULL);
			}
			e->origin = sinkid;
			if (h->true_dest!=FLOOD_ADDRESS)
				e->send = false;
			else if(e->alreadySeen(e->msg_id))
				printf(PRINT_ROUTING,"already seen %d",e->msg_id);
			else
				e->send = true;
			e->metric = snk->etx;
			if (info->hasPar("miss") && e->countSeen() == (unsigned int)info->par("miss"))
			{
				printf(PRINT_ROUTING,"no new neighbours heard about stuff (countseen=%d, nc=%d), therefore send out an invert. pulse attempts = %d",e->countSeen(),neighbours->size(),e->attempts);
				e->invert = true;
			}
			else
				e->invert = false;
			{
				list<NeighbourInfo*> *miss = e->missed(neighbours);
				if (miss->size()==0)
					e->invert = true;
				delete miss;
			}
			break;

		case EXOR_MSG_DATA:
			if (snk!=NULL)
			{
				printf(PRINT_ROUTING,"my etx for %d is %d transmitter node(%d) is=%d",sinkid,snk->etx,info->from, metric);
				if (metric > snk->etx || (info->from==node->getNodeId() && metric == snk->etx))
				{
					printf(PRINT_ROUTING,"I'm a sender");
					e->metric = snk->etx;
					e->send = true;
				}
				else {
					e->metric = metric;
					if (metric< snk->etx)
					{
						e->send = false;
						e->metric |= USEFUL_SENDER; // i'm not one, but i've seen one
						e->send_as = info->from;
					}
					if (sender_will_forward && info->from > node->getNodeId())
					{
						e->send_as = info->from;
						e->send = false;
						printf(PRINT_ROUTING,"dropping msg due to other candidate");
					}
				}
				e->invert = false;
			}
			break;

		case EXOR_MSG_REPLY:
			assert (snk!=NULL);
			printf(PRINT_ROUTING,"my etx for %d is %d reply node(actual=%d, claimed=%d) is=%d", sinkid, snk->etx, info->from, h->sent_as, metric);
			if (e->send_as==node->getNodeId() && metric == snk->etx)
			{
				printf(PRINT_ROUTING,"node says it's me, so don't cancel yet!");
				e->metric = snk->etx;
				break;
			}
			if (metric > snk->etx)
			{
				e->metric = snk->etx;
				e->send = true;
			}
			else {
				e->metric = metric;
				if (sender_will_forward && (metric < snk->etx || (h->sent_as>node->getNodeId()))) // if it's a better node, *or* it's not claiming to be me (i.e. has found a better node)
				{
					e->send = false;
					e->metric |= USEFUL_SENDER; // i'm not one, but i've seen one
					printf(PRINT_ROUTING, "have found another node to send this on");
					e->send_as = h->sent_as;
				}
			}
			e->invert = false;
			break;
			
		case EXOR_BCAST_REPLY:
			//assert (snk!=NULL);
			if (snk!=NULL)
			{
				printf(PRINT_ROUTING,"my etx for %d is %d reply bcast node(actual=%d, claimed=%d) is=%d",sinkid, snk->etx, info->from, h->sent_as, metric);
			}
			e->send = true;
			e->invert = false;
			break;

		case EXOR_FAIL:
			printf(PRINT_ROUTING,"msg failed, dropping");
			//sendExOR(snk->id,msg,snk->etx|USEFUL_SENDER,&this->neighbours);
			break;
	}
	if (e->send)
		e->metric |= USEFUL_SENDER;
	printf(PRINT_ROUTING,"e->send = %d, e->metric=%d, e->send_as=%d",e->send,e->metric,e->send_as);
	assert(e->metric<65536);
}

void Guesswork::txDone(Packet *msg)
{
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	if (h->type == MSG_SOURCE_XMIT)
		sendToApp(msg);
	else
		delete msg;
}


