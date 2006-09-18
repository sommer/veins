#ifndef __EXOR_H__
#define __EXOR_H__

#include "neighbourrouting.h"
#include <list>

using namespace std;

enum ExORTimer {EXOR_REP_TIMER=1, EXOR_ONWARDS, EXOR_ENDFORCE};
#define EXOR_TIMER_COUNT 4 

enum ExORPktType {EXOR_REPLY};
#define EXOR_PKT_COUNT 1

#define DELAY_PER_NEIGH 0.1
#define NEIGH_WAIT 6
#define FLOOD_ADDRESS 0xfffe

typedef struct _bcast_pkt
{
	unsigned int nc;
	unsigned int *neighbours;
	unsigned int exor;
} bcast_pkt;


typedef enum {EXOR_MSG_BCAST, EXOR_BCAST_REPLY, EXOR_MSG_DATA, EXOR_MSG_REPLY, EXOR_FAIL} ExORAction;

struct Header
{
	unsigned int type;
	int reply_to;	
	int metric;
	int exor_id;
	int src;
	int sent_as;
	int lastfrom;
	int sinkid;
	int true_dest;
	int seqno;
	int origin;
	int neighbours[100];
	unsigned int nc;
	int periodsend;
	bool invert;
	unsigned tx_count;
	int etx;
};

#define EXOR_HEADER_SIZE sizeof(Header)

class exor
{
		list<NeighbourInfo*> *checked; // already tested neighbours
	public:
		exor(unsigned int id,Packet *msg, unsigned int node_id);
		~exor();
		unsigned int metric; // output metric value
		bool send; // we should send
		bool invert; // should send as invert
		int send_as; // default is our own id, but can also enable 'lying bastard mode'
		unsigned int dest; // destination sink
		unsigned int origin; // origin node
		unsigned int msg_id;
		unsigned int attempts;
		Packet *tosend;
		//Packet *reply;

		void addChecked(int idx);
		list<NeighbourInfo*>* missed(list<NeighbourInfo*>* neighbours);
		list<NeighbourInfo*>* notMissed(list<NeighbourInfo*>* neighbours);
		unsigned int countSeen() { return checked->size();}
		bool alreadySeen(int);
};

typedef struct _force_data
{
	exor *e;
	Packet *inv;
	list<NeighbourInfo*> *miss;
} force_data;

class ExOR : public NeighbourRouting
{
	private:
		struct timer_data
		{
			Packet *p;
			exor *e;
		};

		list<exor*> msgs;
		list<bcast_pkt*> bcasts;
		force_data waiting;
		unsigned int next_id;
		void addFeatures(int dest, int sink_id, Packet *msg, unsigned int metric, const list<NeighbourInfo*> *neigh, bool invert);
		void forceNoSleep();
		void endForce();
		static list<NeighbourInfo*>* getNeighList(const int *neighs, unsigned int nc);
		static void genHeaderNeighs(const list<NeighbourInfo*> *neighs, Header *h);
		void setSaved(ExORTimer t, Packet *pkt, exor *ex);
		void getSaved(unsigned int t, Packet **pkt, exor **ex);
		timer_data saved[EXOR_TIMER_COUNT];
	
	public:
		~ExOR();
		Module_Class_Members(ExOR, NeighbourRouting, 0)
		virtual void initialize();
		virtual void rx(Packet *msg);
		virtual void tx(Packet *msg)=0;
		virtual void forceGrant(bool success);

		virtual unsigned int sendExOR(int dest, int sink_id, Packet *msg, unsigned int metric,const list<NeighbourInfo*> *neigh,bool invertlist=false,int nid=-1);
		virtual void sendBcast(Packet *msg, unsigned int retries=3, int exor_id = -1);
		virtual void sendFlood(Packet *msg);
		
		virtual void handleTimer(unsigned int index);

		virtual void exor_func(exor*, Packet*,const ExORAction);
	
};

#endif
