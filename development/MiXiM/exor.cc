#include "exor.h"
#include <omnetpp.h>

#define ATTEMPT_LIMIT 2

#define MSG_TYPE(msg) ((h->true_dest==FLOOD_ADDRESS || h->true_dest==BROADCAST)?EXOR_MSG_BCAST:EXOR_MSG_DATA)
#define REPLY_TYPE(msg) ((h->true_dest==FLOOD_ADDRESS || h->true_dest==BROADCAST)?EXOR_BCAST_REPLY:EXOR_MSG_REPLY)

void ExOR::setSaved(ExORTimer t, Packet *pkt, exor *ex)
{
	saved[t].p = pkt;
	saved[t].e = ex;
	if (saved[t].p==NULL && saved[t].e == NULL)
		opp_error("didn't store anything for %d\n",t);
	printf(PRINT_ROUTING,"Stored %p and %p for %d\n",pkt,ex,t);
}

void ExOR::getSaved(unsigned int t, Packet **pkt, exor **ex)
{
	/*if (saved[t].p==NULL && saved[t].e == NULL)
		opp_error("didn't have anything stored for %d\n",t);*/
	*pkt = saved[t].p;
	*ex = saved[t].e;
	memset(&saved[t],0,sizeof(timer_data));
}

list<NeighbourInfo*>* ExOR::getNeighList(const int *neighs, unsigned int nc)
{
	list<NeighbourInfo*>* ret = new list<NeighbourInfo*>();
	for (unsigned int i=0;i<nc;i++)
	{
		NeighbourInfo *n = (NeighbourInfo*)malloc(sizeof(NeighbourInfo));
		n->idx = neighs[i];
		ret->push_back(n);
	}
	return ret;
}

void ExOR::genHeaderNeighs(const list<NeighbourInfo*> *neighs, Header *h)
{
	unsigned int i = 0;
	for (list<NeighbourInfo*>::const_iterator iter = neighs->begin(); iter!=neighs->end(); iter++)
	{
		h->neighbours[i] = ((NeighbourInfo*)*iter)->idx;
		i++;
	}
	h->nc = neighs->size();
}

void ExOR::forceNoSleep()
{
	Packet *s = new Packet("Node_ForceNoSleep",FORCE_AWAKE_REQ);
	sendToMac(s);
}

void ExOR::endForce()
{
	Packet *s = new Packet("Node_EndForce",FORCE_END);
	sendToMac(s);
}

void ExOR::handleTimer(unsigned int index)
{
	Packet *msg=NULL;
	exor *e=NULL;
	if (index!=EXOR_ENDFORCE)
		getSaved(index,&msg,&e);
	Header *h = NULL;
	unsigned int id = 0;
	if (msg!=NULL)
	{
		h = (Header*)msg->getData(ROUTING_DATA);
		id = h->exor_id;
	}

	printf(PRINT_ROUTING,"timer fired, msg=%p",msg);
	switch ((ExORTimer)index)
	{
		case EXOR_REP_TIMER:
			{
				Packet *rep = new Packet("EXOR_REPLY",TX);
				Header h_out;
				rep->setLocalTo(BROADCAST);
				printf_nr(PRINT_ROUTING,"time to send a reply to exor_id %d using %p",id,rep);
				if (h->reply_to!=-1)
					printf_clr(PRINT_ROUTING,"in reply to %d",h->reply_to);
				printf_clr(PRINT_ROUTING,"\n");
				h_out.src = node->getNodeId();
				//assert((int)rep->par("sinkid")==0);
				h_out.metric = h->metric;
				h_out.exor_id = h->exor_id;
				//rep->addPar("orig_kind") = msg->kind();
				h_out.reply_to = h->reply_to;
				h_out.sent_as = e->send_as;	
				h_out.type = EXOR_REPLY;
				rep->setData(ROUTING_DATA,&h_out,sizeof(Header),EXOR_HEADER_SIZE);
				sendToMac(rep);
				delete msg;
				//delete e;
			}
			break;
		
		case EXOR_ONWARDS:
			{
				assert(e!=NULL);
				if (e->attempts>ATTEMPT_LIMIT)
				{
					printf(PRINT_ROUTING,"ran out of attempts");
					if (h->true_dest!= BROADCAST && h->true_dest!=FLOOD_ADDRESS)
						exor_func(e,msg,EXOR_FAIL);
					assert(e->tosend == msg);
					e->tosend = NULL;
					delete msg;
					//delete e;
					break;
				}
				exor_func(e,msg,MSG_TYPE(msg));
				printf_nr(PRINT_ROUTING,"time to send onwards exor=%d, metric = %d, attempt=%d",e->msg_id,e->metric,e->attempts);
				if (h->lastfrom!=-1)
					printf_clr(PRINT_ROUTING,"last loc was %d",h->lastfrom);
				printf_clr(PRINT_ROUTING,"\n");
				printf_nr(PRINT_ROUTING,"missing = nc=%d",neighbours->size());
				list<NeighbourInfo*> *miss;
				if (e->invert)
				{
					printf_clr(PRINT_ROUTING," not missed=");
					miss = e->notMissed(neighbours);
				}
				else
				{
					printf_clr(PRINT_ROUTING," missed=");
					miss = e->missed(neighbours);
				}
				for (list<NeighbourInfo*>::const_iterator iter = miss->begin(); iter!=miss->end(); iter++)
				{
					NeighbourInfo *m = *iter;
					printf_clr(PRINT_ROUTING,"%d",m->idx);
					if (iter!=miss->end())
						printf_clr(PRINT_ROUTING,", ");
				}
				printf_clr(PRINT_ROUTING,"\n");
				
				printf(PRINT_ROUTING, "metric = %d missed=%d, invert=%d",h->metric,miss->size(),e->invert);
				assert(e!=NULL);
				assert(e->tosend == msg);
				forceNoSleep(); // stay awake until we've got the replies
				waiting.e = e;
				waiting.miss = miss;
				if (waiting.inv)
					delete waiting.inv;
				waiting.inv = (Packet*)(msg->dup());
				delete msg;
			}
			break;

		case EXOR_ENDFORCE:
			endForce();
			break;

	}
	return;
}

void ExOR::initialize()
{
	NeighbourRouting::initialize();
	memset(&waiting,0,sizeof(waiting));
	next_id = 1;
}

ExOR::~ExOR()
{
	for(list<bcast_pkt*>::const_iterator iter = bcasts.begin();iter!=bcasts.end();iter++)
	{
		bcast_pkt*b = (bcast_pkt*)*iter;
		free(b->neighbours);
		free(b);
	}
	for(list<exor*>::const_iterator iter = msgs.begin();iter!=msgs.end();iter++)
		delete *iter;
	if (waiting.inv)
		delete waiting.inv;
}

void ExOR::txDone(Packet *msg)
{
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	if (h->type !=EXOR_REPLY)
		sendToApp(msg);
	else
	{
		delete msg;
	}
}

void ExOR::rx(Packet * msg)	
{
	/*bool ret = PositifNode::handleMessage(msg,newNeighbour);
	if (ret)
		return true;*/
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	ExORPktType kind = (ExORPktType)h->type;
	exor* test=NULL;
	list<NeighbourInfo *>*neighs;
	int seq;
	unsigned int id = h->exor_id;
	switch(kind)
	{	
		case EXOR_REPLY:
		{
			printf(PRINT_ROUTING,"got exor reply msg_id = %d",id);
			list<exor*>::const_iterator iter = msgs.begin();
			for(;iter!=msgs.end();iter++)
			{
				test = *iter;
				//printf(PRINT_ROUTING," stored msg_id = "<< test->msg_id <<"\n";
				if (test->msg_id == id)
				{
					//sprintf(pointer,"%p",test->tosend);
					//ev << node->getNodeId() <<": test->tosend = " << pointer << " send="<<test->send<<"\n";
					/*if (!test->send && test->tosend!=NULL)
						test->tosend = NULL;*/
					
					h->true_dest = test->dest;
					h->sinkid = test->origin;
					this->exor_func(test,msg,REPLY_TYPE(msg));
					h->type = EXOR_REPLY;
					
					//assert(1==0);
					test->addChecked(msg->from);
					printf(PRINT_ROUTING,"got exor reply %d from %d, incoming metric = %d, metric=%d, send=%d",id,msg->from,h->metric,test->metric,test->send);
					if (h->reply_to!=-1)
						printf(PRINT_ROUTING,"in reply to %d",h->reply_to);
					list<NeighbourInfo*> *miss = test->missed(neighbours);
					printf_nr(PRINT_ROUTING,"missing = %d nc= %d missed=",miss->size(),neighbours->size());
					for (list<NeighbourInfo*>::const_iterator iter = miss->begin(); iter!=miss->end(); iter++)
					{
						NeighbourInfo *m = *iter;
						printf_clr(PRINT_ROUTING,"%d",m->idx);
						if (iter!=miss->end())
							printf_clr(PRINT_ROUTING,", ");
					}
					delete miss;
					printf_clr(PRINT_ROUTING,"\n");
					if (test->tosend!=NULL)
					{
						//sprintf(pointer,"%p",test);
						//printf(PRINT_ROUTING," testing tosend of "<<pointer<<"\n";
						if (!test->send)
						{
							printf(PRINT_ROUTING,"cancelling %p",test->tosend);
							//removeTimer((timer_info *)(void *)test->tosend->par("timer"));
							cancelTimer(EXOR_ONWARDS);
							printf(PRINT_ROUTING,"canceled %p,",test->tosend);
							delete test->tosend;
							test->tosend = NULL;
						}
						else
						{
							if (test->metric!=0)
							{
								Header *ts = (Header*)test->tosend->getData(ROUTING_DATA);
								assert(ts!=NULL);
								ts->metric = test->metric;
								printf(PRINT_ROUTING,"have active tosend, kind=%d, exor=%d pointer=%p",ts->type,ts->metric,test->tosend);
							}
						}
					}
					break;
				}
			}
			if (iter == msgs.end())
			{
				list<bcast_pkt*>::const_iterator iter = bcasts.begin();
				bcast_pkt* pkt;
				for(;iter!=bcasts.end();iter++)
				{
					pkt = *iter;
					if (pkt->exor == id)
					{
						printf(PRINT_ROUTING," caught returning bcast\n");
					}
				}
			}
			break;
		}

		default: // normal msg marked with exor == need to reply
			printf(PRINT_ROUTING,"exor msg, id=%d, type = %s from %d, seq=%d, true_dest=%d",id,msg->name(),msg->from,h->seqno,h->true_dest);
			bool dup = false;
			for(list<exor*>::const_iterator iter = msgs.begin();iter!=msgs.end();iter++)
			{
				test = *iter;
				if (test->msg_id == id)
				{
					printf(PRINT_ROUTING,"have already seen %d, origin=%d",id,h->origin);
					test->addChecked(msg->from);
					dup = true;
					break;
				}
			}
			neighs = getNeighList(h->neighbours,h->nc);
			seq = -1;
			if (!dup)
			{
				test = new exor(id,msg,node->getNodeId());
				test->addChecked(msg->from);
				test->metric = h->metric;
			}
			//printf(PRINT_ROUTING,"ev << node->getNodeId() <<": count = "<<neighs->size()<<"\n";
			unsigned int i=0;
			for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
			{
				int id = ((NeighbourInfo*)*iter)->idx;
				//printf(PRINT_ROUTING," n_id = "<<neighs[i] << "\n"; 
				if (id == node->getNodeId())
				{
					seq = i;
					break;
				}
				i++;
			}
			if (!h->invert)
			{
				if (seq == -1)
				{
					printf(PRINT_ROUTING," not in neighbour list from %d (nc=%d)",msg->from,neighs->size());
					break;
				}
			}
			else
				if (seq != -1)
				{
					printf(PRINT_ROUTING," in neighbour list from %d and invert is %d",msg->from,h->invert);
					break;
				}
			this->exor_func(test,msg,MSG_TYPE(msg));
			if (!dup)
				msgs.push_back(test);
			printf_nr(PRINT_ROUTING,"got exor_id %d, src=%d, dest=%d, incoming metric=%d, calc'ed metric=%d, sequence = %d send=%d send_as =%d",id,msg->from,msg->to,h->metric,test->metric,seq,test->send,test->send_as);
			//assert (msg->from!=13 || msg->kind()!=EXOR_MSG_NEXT+1);
			if (h->true_dest == node->getNodeId())
				test->send = false;
			if (test->send && !dup)
			{
				test->tosend = (Packet*)msg->dup();
				if (test->metric !=0 || msg->to==FLOOD_ADDRESS)
				{
					/*if(!test->tosend->hasPar("lastfrom"))
						test->tosend->addPar("lastfrom");
					test->tosend->par("lastfrom") = msg->from;*/
					//printf(PRINT_ROUTING," info.src = "<<test->tosend->par("lastfrom")<<"("<<info.src<<")\n";
					//assert((unsigned int)test->tosend->par("lastfrom")==info.src);
					addFeatures(h->true_dest,h->sinkid,test->tosend,test->metric,neighs,false);
					printf_clr(PRINT_ROUTING,"\n");
					//test->tosend->addPar("oldkind") = msg->kind();
					//test->tosend->setKind(EXOR_ONWARDS);
					/*test->tosend->addPar("data");
					test->tosend->par("data").setPointerValue((void*)test);*/
					//test->tosend->par("data").configPointer(NULL,NULL,sizeof(exor));
				}
				else if (msg->to !=BROADCAST)
				{
					printf(PRINT_ROUTING,"root node!");
					//test->tosend->delPar("metric");
				}
				//test->tosend->setSrc(node->getNodeId());
				simtime_t when;
				if (h->invert==0)
					when = (neighs->size()+2)*DELAY_PER_NEIGH;
				else
					when = (NEIGH_WAIT+1)*DELAY_PER_NEIGH;
				setSaved(EXOR_ONWARDS, test->tosend, test);
				setTimer(EXOR_ONWARDS, when);
				test->attempts = 0;
				//scheduleAt(when,test->tosend);
				printf(PRINT_ROUTING,"tosend=%p at %d",test->tosend,when);
			}
			else
				printf_clr(PRINT_ROUTING,"\n");
			
			Packet *reply = new Packet("EXOR_reply_temp", msg->kind());
			reply->setLocalTo(msg->from);
			{
				Header h_out;
				h_out.reply_to = msg->from;
				h_out.exor_id = id;
				//dPar("data").setPointerValue((void*)test);
				reply->setData(ROUTING_DATA,&h_out,sizeof(Header),EXOR_HEADER_SIZE);
			}
			setSaved(EXOR_REP_TIMER, reply , test);
			if (h->invert)
			{
				setTimer(EXOR_REP_TIMER,(seq+1)*DELAY_PER_NEIGH);
			}
			else
			{
				seq = intuniform(1,NEIGH_WAIT);
				printf(PRINT_ROUTING,"on invert, so chose sequence %d",seq);
				setTimer(EXOR_REP_TIMER,seq*DELAY_PER_NEIGH);
			}
			/*if (h->true_dest == node->getNodeId())
				return false;*/
			delete neighs;	
			break;
	}
	delete msg;
	return;
}

unsigned int ExOR::sendExOR(int dest, int sink_id, Packet *msg, unsigned int metric,const list<NeighbourInfo*> *neigh, bool invertlist, int nid)
{
	unsigned int old_id=0xFFFF;
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	h->tx_count = 1; // FIXME: update this properly
	if (nid!=-1)
	{
		old_id = next_id;
		next_id = nid;
	}
	addFeatures(dest, sink_id,msg,metric,neigh,invertlist);
	printf_clr(PRINT_ROUTING,"\n");
	if (sink_id!=BROADCAST && sink_id!=FLOOD_ADDRESS) // i.e. a non-bcast, non-flood msg. So set the next hop to a broadcast
	{
		h->sinkid = sink_id;
	}
	msg->setTo(BROADCAST);
	msg->setLocalTo(BROADCAST);
		
	h->origin = node->getNodeId();
	if (nid!=-1)
		next_id = old_id;
	printf(PRINT_ROUTING,"sending exor %d, type=%d, pointer=%p sink_id=%d",h->exor_id,h->type,msg,sink_id);
	//msg->addPar("oldkind") = msg->kind();
	//msg->setKind(EXOR_ONWARDS);
	exor *e = new exor(h->exor_id,msg,node->getNodeId());
	this->exor_func(e,msg,MSG_TYPE(msg));
	msgs.push_back(e);
	//msg->addPar("data").setPointerValue((void*)e);
	e->tosend = msg;
	setSaved(EXOR_ONWARDS, msg, e);
	setTimer(EXOR_ONWARDS,0.01);
	return h->exor_id;
}

void ExOR::sendBcast(Packet *msg, unsigned int retries, int exor_id)
{
	bcast_pkt *pkt = (bcast_pkt*)calloc(1,sizeof(struct _bcast_pkt));
	pkt->exor = sendExOR(BROADCAST,-1,msg,msg->hasPar("metric")?msg->par("metric"):0,neighbours,true,exor_id);
	pkt->nc = neighbours->size();
	pkt->neighbours = (unsigned int *)malloc(pkt->nc*sizeof(unsigned int));
	int i=0;
	for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
	{
		pkt->neighbours[i] = ((NeighbourInfo*)*iter)->idx;
		//printf(PRINT_ROUTING," adding neigh " << ids[i] << "\n";
		i++;
	}
	bcasts.push_back(pkt);
}

void ExOR::sendFlood(Packet *msg)
{
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	bcast_pkt *pkt = (bcast_pkt*)calloc(1,sizeof(struct _bcast_pkt));
	pkt->exor = sendExOR(FLOOD_ADDRESS,h->sinkid,msg,msg->hasPar("metric")?msg->par("metric"):0,neighbours,true);
	pkt->nc = this->neighbours->size();
	pkt->neighbours = (unsigned int *)malloc(pkt->nc*sizeof(unsigned int));
	int i=0;
	for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
	{
		pkt->neighbours[i] = ((NeighbourInfo*)*iter)->idx;
		//printf(PRINT_ROUTING," adding neigh " << ids[i] << "\n";
		i++;
	}
	bcasts.push_back(pkt);
}

void ExOR::addFeatures(int dest,int sink_id, Packet *msg, unsigned int metric,const list<NeighbourInfo*> *neigh, bool invert)
{
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	h->metric = metric;
	h->exor_id = next_id;
	h->sinkid = sink_id;
	h->true_dest = dest;
	h->invert = invert;
	h->lastfrom = BROADCAST;
	msg->setFrom(node->getNodeId());
	genHeaderNeighs(neigh,h);
	printf_clr(PRINT_ROUTING,"(dest =%d) nc = %d\n",dest,h->nc);
	next_id+=1;
}

exor::exor(unsigned int id,Packet *msg, unsigned int node_id)
{
	Header *h = (Header*)msg->getData(ROUTING_DATA);
	msg_id = id;
	tosend = NULL;
	send = false;
	dest = msg->from;
	checked = NULL;
	addChecked(msg->from);
	attempts = 0;
	send_as = node_id;
	origin = h->origin;
	//origin = 0xFFFC;
}

void exor::addChecked(int idx)
{
	if (checked == NULL)
		checked = new list<NeighbourInfo *>();
	NeighbourInfo *n = (NeighbourInfo*)malloc(sizeof(NeighbourInfo));
	n->idx = idx;
	checked->push_back(n);
}

exor::~exor()
{
	if (checked)
	{
		for (list<NeighbourInfo*>::const_iterator iter = checked->begin(); iter!=checked->end(); iter++)
			free (*iter);
		delete checked;
	}
}

list<NeighbourInfo*> * exor::missed(list<NeighbourInfo*>* neighbours)
{
	list<NeighbourInfo*> *ret = new list<NeighbourInfo*>();
	for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
	{
		bool found = false;
		NeighbourInfo *n = *iter;
		for (list<NeighbourInfo*>::const_iterator chiter = checked->begin(); chiter!=checked->end(); chiter++)
		{
			if ((*chiter)->idx == n->idx)
			{
				found = true;
				break;
			}
		}
		if (!found)
			ret->push_back(n);
	}
	return ret;
}

list<NeighbourInfo*> * exor::notMissed(list<NeighbourInfo*>* neighbours)
{
	list<NeighbourInfo*> *ret = new list<NeighbourInfo*>();
	for (list<NeighbourInfo*>::const_iterator iter = neighbours->begin(); iter!=neighbours->end(); iter++)
	{
		NeighbourInfo *n = *iter;
		for (list<NeighbourInfo*>::const_iterator chiter = checked->begin(); chiter!=checked->end(); chiter++)
		{
			if ((*chiter)->idx == n->idx)
			{
				ret->push_back(n);
				break;
			}
		}
	}
	return ret;
}

bool exor::alreadySeen(int id)
{
	bool found = false;
	for (list<NeighbourInfo*>::const_iterator chiter = checked->begin(); chiter!=checked->end(); chiter++)
	{
		if ((*chiter)->idx == id)
		{
			found = true;
			break;
		}
	}
	return found;
}

void ExOR::exor_func(exor*, Packet*,const ExORAction)
{
	assert(0); // should have been overridden
}
				
void ExOR::forceGrant(bool success)
{
	exor *e = waiting.e;
	Packet *inv = waiting.inv;
	list<NeighbourInfo*> *miss = waiting.miss;
	Header *h = (Header*)(inv->getData(ROUTING_DATA));
	memset(&waiting,0,sizeof(waiting));

	if (!success)
	{
		printf(PRINT_CRIT,"forceGrant failure");
		assert(0);
		return;
	}
	//e->send = false;
	printf_nr(PRINT_ROUTING,"destroyed tosend of %p = %p\n", e, e->tosend);
	e->tosend = NULL;
	inv->setKind(TX);
	setSaved(EXOR_ONWARDS,inv,e);
	//setTimer(EXOR_ONWARDS,(miss->size()+2)*DELAY_PER_NEIGH);

	printf_nr(PRINT_ROUTING, "created repeat timer, msg id=%p",inv);
	setTimer(EXOR_ENDFORCE,(miss->size()+1)*DELAY_PER_NEIGH);
	e->tosend = inv;

	Packet *rep = (Packet*)inv->dup();
	unsigned int old_id = next_id;
	next_id = e->msg_id;
	addFeatures(h->true_dest,h->sinkid,rep,e->metric,miss,e->invert);
	next_id = old_id;
	//rep->setSrc(node->getNodeId());
	//rep->setKind(msg->par("oldkind"));
	e->attempts +=1;
	//rep->par("metric") = e->metric;
	//rep->setLocalTo(BROADCAST);
	rep->setKind(TX);
	sendToMac(rep);

	delete miss;
}
