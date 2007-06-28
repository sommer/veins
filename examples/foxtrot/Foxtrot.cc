/* -*- mode:c++ -*- ********************************************************
 * file:        Foxtrot.cc
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     wsn-specific modules
 * description: aggregation layer: basic core. subclass to build your own
 *              aggregation protocol
 ***************************************************************************/


#include "Foxtrot.h"
#include "NetwControlInfo.h"
#include <assert.h>

#include <SinkAddress.h>
#include "bits.h"

Define_Module_Like(Foxtrot, BaseAggLayer);

seconds_t nextTimeout = 0;
uint8_t Foxtrot::pts = 0;

typedef enum {TIMER_SEND=0, TIMER_PROCESS} FoxtrotTimer;

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

void Foxtrot::initialize(int stage)
{
	BaseAggLayer::initialize(stage);
	if (stage ==0)
	{
		FrameTimer::init(this);
		Timer::init(this);
		items = 0;
		pts = 1;
		limits[0] = 1.0;

		wait_data = false;
		maxLatency = par("maxLatency");
		setFrameTimer(10);
		local = new FoxtrotPacket();
		local->setLocalPos(this);
	}
}

Foxtrot::~Foxtrot()
{
	delete local;
	for (unsigned int i=0;i<storage.size();i++)
		delete storage[i];
}

/**
 * Redefine this function if you want to process messages from lower
 * layers before they are forwarded to upper layers
 *
 *
 * If you want to forward the message to upper layers please use
 * @ref sendUp which will take care of decapsulation and thelike
 **/
void Foxtrot::handleLowerMsg(cMessage * msg)
{
	FoxtrotPacket *m = check_and_cast < FoxtrotPacket * >(msg);
	try_merge(m, true);
	delete m;
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * For the Foxtrot we just use the destAddr of the Aggpkt
 * message as a nextHop
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void Foxtrot::handleUpperMsg(cMessage * msg)
{
	AggPkt *m = check_and_cast < AggPkt * >(msg);
	//m->setControlInfo(new NetwControlInfo(SINK_ADDRESS));
	pts = m->getDataArraySize();
	foxtrot_point data[pts];
	for (unsigned int i=0;i<pts;i++)
		data[i] = m->getData(i);
	newPoint(data);
	delete m;
}

void Foxtrot::try_merge(FoxtrotPacket *newData, bool keep_new) /* keep_new indicates whether to keep new data if it doesn't merge */
{
	uint8_t mcount;
	uint8_t subset[BYTES_USED(storage.size())];

	bool ret=false;

	print_data("New data into foxtrot",newData);

	if (storage.size()>0)
	{
		uint8_t delete_count=0;
		uint8_t i = 0;
		for (;i<storage.size();i++)
		{
			if (storage[i]->isEqual(newData))
			{
				DBG("Found dupe at %d\n",i);
				ret = true;
				break;
			}
		}
		if (!ret)
		{
			storage.push_back(new FoxtrotPacket(*newData));
			storage[storage.size()-1]->setWhen(getGlobalSeconds());
			keep_new = false;
			mcount = mergeableSubset(&storage,subset);
			DBG("Mcount = %d\n",mcount);
			if (mcount>1)
			{
				ret = merge(subset, &delete_count);
				if (delete_count == 0)
					ret = false;
			}
			/*if (delete_count>1)
			{
				DBG("storage size is %d, delete_count is %d\n",storage.size(),delete_count);
				for (uint8_t i=0;i<delete_count-1;i++)
				{
					assert(storage.size()>1);
					storage.erase(storage.end()-1);
				}
			}*/
		}
	}
	if (!ret && keep_new)
	{
		EV << "Wasn't able to merge data. Adding entry (time index="<<newData->getWhen()<<")\n";
		FoxtrotPacket *fp = new FoxtrotPacket(*newData);
		storage.push_back(fp);
	}

	for (uint8_t i=0;i<storage.size();i++)
		print_data("Data Point (try)",storage[i]);
	if (isSink)
		processPackets();
}

inline bool Foxtrot::merge(uint8_t *subset, uint8_t *delete_count)
{
	uint8_t i,j,l;
	int k;
	FoxtrotPacket *midrange = new FoxtrotPacket();
	bool timeset = false;
	const uint8_t count = storage.size();

	midrange->setDataArraySize(pts);

	uint8_t most_inside = 0, in_boxes[1];

	uint8_t change[1];
	uint8_t remove[1];

	*delete_count = 0;
	remove[0] = 0;
	change[0] = 0;
	in_boxes[0] = 0;

	for (i=0;i<count;i++)
		print_data("Data Point (merge)",storage[i]);

	for (i=0;i<count;i++)
	{
		uint8_t temp[1], inbox=0;
		temp[0]=0;
		if (!ANY_CHK(i,subset))
			continue;
		for (j=0;j<count;j++)
		{
			if (i==j || !ANY_CHK(j,subset))
				continue;
			if (inside(storage[i],storage[j]))
			{
				ANY_SET(j,temp);
				DBG("%d is inside %d\n",j,i);
				inbox++;
			}
		}
		if (inbox>0)
		{
			ANY_SET(i,temp);
			inbox++;
			if (inbox>most_inside)
			{
				most_inside = inbox;
				in_boxes[0] = temp[0];
				midrange->setAllLoc(storage[i]);
			}
		}
	}
	if (most_inside!=0)
	{
		remove[0] = in_boxes[0];
		change[0] = in_boxes[0];
	}
	else
	{
		for (j=0;j<AXES;j++)
		{
			foxtrot_point temp=0;
			uint8_t used = 0;
			for (i=0;i<count;i++)
			{
				if (!ANY_CHK(i,subset))
					continue;
				temp+=(storage[i]->getLoc(j).min+storage[i]->getLoc(j).max);
				used+=2;
			}
			midrange->setMinLoc(j,(foxtrot_point)(temp/(used*1.0)));
			midrange->setMaxLoc(j,midrange->getLoc(j).min);
		}
		print_data("midrange",midrange);
		for (i=0;i<AXES*2;i++) // pts axis * 2 directions (smaller and larger)
		{
			uint8_t curr = 1<<(i/2);
			foxtrot_point val[2];
			uint8_t setval = 1;
			val[0] = MAX_FOXTROT_VALUE;
			val[1] = MAX_FOXTROT_VALUE;
			DBG("i=%d, curr=%d\n",i,curr);

			for (j=0;j<(1<<AXES);j++) // in a given direction, we're looking for 2^(pts-1) points that create a hypersurface. we also have the other 2^pts-2^(pts-1) points, but they get ignored
			{
				uint8_t better = 0;
				if (curr & j) // don't handle the extra ones where the current tested direction being compared
				{
					DBG("Skipped %d\n",j);
					continue;
				}
				if (setval==1)
				{
					val[0] = (i%2==0)?midrange->getLoc(i/2).min:midrange->getLoc(i/2).max;
					val[1] = val[0];
				}
				setval = (setval==0)?1:0;	
				for (k=0;k<count;k++) // check all the known boxes
				{
					if (!ANY_CHK(k,subset)) // only use the subset
						continue;
					for (l=0;l<AXES;l++) // for a particular box, does it have a better co-ordinate?
					{
						bool max;
						if (l==i/2)
							max = i%2;
						else	
							max = (1<<l)&j;
						if (max)
						{
							if (storage[k]->getLoc(l).max >= midrange->getLoc(l).max)
							{
								DBG("box %d exceeds max for pt %d\n",k,l);
								continue;
							}
							DBG("box %d fails max limits (" FP_PRINTF " < " FP_PRINTF") on pt %d\n",k,storage[k]->getLoc(l).max,midrange->getLoc(l).max,l);
						}
						else
						{
							if (storage[k]->getLoc(l).min <= midrange->getLoc(l).min)
							{
								DBG("box %d is smaller than min for pt %d\n",k,l);
								continue;
							}
							DBG("box %d fails min limits (" FP_PRINTF " > " FP_PRINTF") on pt %d\n",k,storage[k]->getLoc(l).min,midrange->getLoc(l).min,l);
						}
						break;
					}
					if (l==AXES)
					{
						uint8_t m=0,jc=j;
						DBG("box %d was good for axis %d and other locs",k,i/2);
						for (m=0;m<AXES;m++)
						{
							if (m==i/2)
							{
								DBG_clear(" (n/a)");
							}
							else if (jc%2==0)
							{
								DBG_clear(" neg");
							}
							else
							{
								DBG_clear(" pos");
							}
							jc = jc>>1;
						}
						DBG_clear("\n");
						better |= 1<<k;
					}
				}
				if (better==0)
				{
					DBG("failed to find anything for coord %d in the %s direction\n",i,i%2==0?"negative":"positive");
					delete midrange;
					assert(0);
					return false;
				}
				DBG("better for axis %d in the %s direction is",i/2,i%2==0?"negative":"positive");
				for (k=0;k<count;k++)
					if (better&(1<<k))
					{
						if (i%2==0)
						{
							if (val[setval]>storage[k]->getLoc(i/2).min)
								val[setval] = storage[k]->getLoc(i/2).min;
						}
						else
						{
							if (val[setval]<storage[k]->getLoc(i/2).max)
								val[setval] = storage[k]->getLoc(i/2).max;
						}
						DBG_clear(" %d",k);
					}
				DBG_clear(" val[%d]=" FP_PRINTF "\n",setval,val[setval]);
				if (setval==1)
				{
					if (i%2==0)
						midrange->getLoc(i/2).min = MAX(val[0],val[1]);
					else	
						midrange->getLoc(i/2).max = MIN(val[0],val[1]);
				}
			}
		}
		print_data("midrange",midrange);
		for (k=0;k<count;k++)
		{
			if (!ANY_CHK(k,subset))
				continue;
			if (midrange->isEqual(storage[k]))
			{	
				ANY_SET(k,remove);
				EV << "Wiping box "<<k<<", identical\n";
				continue;
			}
			if (inside(midrange,storage[k]))
			{
				EV << "Wiping box "<<k<<", complete inside\n";
				ANY_SET(k,remove);
				ANY_SET(k,change);
				continue;
			}
		}
	}
	if (remove==0)
	{
		EV << "wasn't able to get rid of any old data, so don't store\n";
		delete midrange;
		assert(0);
		return false;
	}
	for (k=0;k<count;k++)
	{
		uint8_t diff = AXES;
		if (!ANY_CHK(k,subset) || ANY_CHK(k,remove))
			continue;
		for (i=0;i<AXES;i++)
		{
			if (!all_in_range(&midrange->getLoc(i),&storage[k]->getLoc(i)))
			{
				if (diff!=AXES)
					break;
				diff = i;
			}	
		}
		if (i!=AXES) /* either zero-overlap, or multiple non-intersecting. Either way, it's unusable */
			continue;
		if (val_in_range(&midrange->getLoc(diff),storage[k]->getLoc(diff).min))
			storage[k]->getLoc(diff).min = midrange->getLoc(diff).max;
		else if (val_in_range(&midrange->getLoc(diff),storage[k]->getLoc(diff).max))
			storage[k]->getLoc(diff).max = midrange->getLoc(diff).min;
		else /* not this block */
			continue;
		EV << "box "<<k<<"%d is partially inside\n";	
		ANY_SET(k,change);	
	}

	if (change == 0)
	{
		EV << "Got the same blocks\n";
		delete midrange;
		return false;
	}

	assert(remove!=0);
	for (k=0;k<count;k++)
	{
		if (!ANY_CHK(k,change)) // only use changed blocks
			continue; 
		if (!timeset || storage[k]->getWhen()<midrange->getWhen())
		{
			midrange->setWhen(storage[k]->getWhen());
			timeset = true;
		}
	}
	for (k=count-1;k>=0;k--)
	{
		if (ANY_CHK(k,remove))
		{
			EV << "deleting block "<<k<<"\n";
			midrange->expandData(storage[k]);
			if (!timeset || storage[k]->getWhen()<midrange->getWhen())
			{
				midrange->setWhen(storage[k]->getWhen());
				timeset = true;
			}
			storage.erase(k);
			(*delete_count)++;
		}
	}
	EV << "Using new box to store data\n";
	storage.push_back(midrange);
	assert(midrange->getWhen()!=0);
	assert(midrange->getData(0).min!=MAX_FOXTROT_VALUE);
	DBG("wiped %d blocks\n",*delete_count);
	return (*delete_count)>1; /* if we used the last index, return false (as it's the new item) */
}

inline void Foxtrot::gen_data(foxtrot_data *d, const foxtrot_point *p)
{
	for (uint8_t i=0;i<pts;i++)
		d[i].min = d[i].max = p[i];
}

void Foxtrot::newPoint(const foxtrot_point *data)
{
	foxtrot_data *pkt = (foxtrot_data*)malloc(sizeof(foxtrot_data)*pts);
	
	gen_data(pkt,data);
	newData(pkt);
	free(pkt);
}

seconds_t Foxtrot::getGlobalSeconds()
{
	return (seconds_t)simTime();
}

void Foxtrot::handleTimer(unsigned int index)
{
	switch((FoxtrotTimer)index)
	{
		case TIMER_SEND:
		{
			FoxtrotPacket *sender = new FoxtrotPacket(*(storage[storage.size()-1]));
			sender->setControlInfo(new NetwControlInfo(SINK_ADDRESS));
			sender->setName("foxtrot packet");
			sender->setWhen(getGlobalSeconds());
			sendDown(sender);
			break;
		}
		case TIMER_PROCESS:
			if (getGlobalSeconds()>=nextTimeout)
				processPackets();
			break;
	}
}

void Foxtrot::doSend()
{
	assert(!isSink);
	if (storage.size()>0)
	{
		// 512 == 1024 (1s) /2 i.e. 50% of the maxlatency
		double rnd = genk_uniform(0,0,maxLatency/2.0);
		//storage[storage.size()-1]->setWhen(getGlobalSeconds());
		setTimer(TIMER_SEND,rnd);
	}
}

void Foxtrot::newData(const foxtrot_data *data)
{
	local->setDataArraySize(pts);
	local->setWhen(NO_TIME);
	//print_data("local before",local);
	local->setAllData(this,data);
	print_data("local",local);
	try_merge(local, true);
	if (!isSink)
	{
		if (wait_data)	
		{
			doSend();
			wait_data = false;
		}
	}
	return;
}


uint8_t Foxtrot::mergeableSubset(FoxtrotPacketStorage *list, uint8_t *subset)
{
	uint8_t i,j;
	uint8_t best_count = 0;
	uint8_t count = list->size();
	DBG("Count is %d\n",count);
	//foxtrot_data range[MAX_PTS];

	if (pts == 0)
	{
		for (i=0;i<count;i++)
		{	
			uint8_t curr_count = 0;
			uint8_t curr[FOX_QUEUE_LEN_BYTES];
			memset(&curr,0,FOX_QUEUE_LEN_BYTES);
			for (j=0;j<count;j++)
			{
				if (abs((*list)[j]->getWhen()-(*list)[i]->getWhen())>MAX_SECS_DIFF)
					continue;
				ANY_SET(j,curr);
				curr_count++;
			}
			if (curr_count>best_count)
			{
				uint8_t k;
				best_count = curr_count;
				memcpy(subset,&curr,FOX_QUEUE_LEN_BYTES);
				EV << "Best match is "<<curr_count<<endl;
				EV << "Matched: ";
				for (k=0;k<count;k++)
					if (ANY_CHK(k,curr))
						EV << k;
				EV << "\n";
				if (curr_count == best_count);
					break;
			}
		}
		return best_count;	
	}

	for (i=0;i<count;i++)
	{
	 	DBG("Data point (ms) %d: " FP_PRINTF " - " FP_PRINTF " (box " FP_PRINTF "-" FP_PRINTF "," FP_PRINTF "-" FP_PRINTF "). Time=%d\n",i, (*list)[i]->getData(0).min,(*list)[i]->getData(0).max,(*list)[i]->getLoc(X_DIR).min,(*list)[i]->getLoc(X_DIR).max,(*list)[i]->getLoc(Y_DIR).min,(*list)[i]->getLoc(Y_DIR).max,(*list)[i]->getWhen());
	}
	
	/*for (i=0;i<pts;i++)
	{
		memcpy(&range[i],&list[0].getData(i),sizeof(foxtrot_data));
		for (j=1;j<count;j++)
		{
			if (range[i].min>list[j].getData(i).min)
				range[i].min = list[j].getData(i).min;
			if (range[i].max<list[j].getData(i).max)
				range[i].max = list[j].getData(i).max;
		}
	}*/
	for (j=0;j<count;j++)
	{
		DBG("TESTING: %d\n",j);
		for (i=0;i<pts*2;i++)
		{
			uint8_t k;	
			foxtrot_data check[MAX_PTS];
			uint8_t curr_count = 0;
			uint8_t curr[FOX_QUEUE_LEN_BYTES];
			memset(&curr,0,FOX_QUEUE_LEN_BYTES);
			for (k=0;k<pts;k++)
			{
				bool upper;
				if (i==0 || k==0)
					upper = (i%2==1);
				else
					upper = (((i/k)%2)==1);
				if (upper)
				{
					check[k].max = MIN((*list)[j]->getData(k).max+limits[k],MAX_FOXTROT_VALUE);
					check[k].min = (*list)[j]->getData(k).min;	
				}
				else	
				{
					check[k].max = (*list)[j]->getData(k).max;
					check[k].min = ((*list)[j]->getData(k).min<=limits[k])?0:(*list)[j]->getData(k).min-limits[k];	
				}
				if (check[k].min == check[k].max)
				{
					DBG("check[%d].{min,max} = " FP_PRINTF "\n",k,check[k].min);
					//break;
				}
				else
				{
					DBG("check[%d].min = " FP_PRINTF "\n",k,check[k].min);
					DBG("check[%d].max = " FP_PRINTF "\n",k,check[k].max);
				}
			}
			if (k!=pts) // we broke
			{
				DBG("We broke with %d\n",k);
				continue;
			}
			for (k=0;k<count;k++)
			{
				uint8_t l;
				if (k==j)
				{
					ANY_SET(k,curr);
					DBG("k==j\n");
					curr_count++;
					continue;
				}
				if ((*list)[k]->getWhen()!=NO_TIME && (*list)[k]->getWhen()!=NO_TIME && abs((*list)[k]->getWhen()-(*list)[j]->getWhen())>MAX_SECS_DIFF)
				{
					DBG("Time diff is too big\n");
					continue;
				}

				for (l=0;l<pts;l++)
				{
					if ((*list)[k]->getData(l).min<check[l].min || (*list)[k]->getData(l).max>check[l].max)
					{
						DBG("%d fails on pt %d against %d\n",k,l,j);
						break;
					}
				}
				if (l==pts) // no break
				{
					ANY_SET(k,curr);
					DBG("%d matches with %d\n",k,j);
					curr_count++;
				}
				else
					DBG("Broke l on %d\n",l);
			}
			if (curr_count>best_count)
			{
				DBG("Best match is %d\n",curr_count);
				DBG("Matched: ");
				for (k=0;k<count;k++)
					if (ANY_CHK(k,curr))
						DBG_clear("%d ",k);
				DBG_clear("\n");
				memcpy(subset,&curr,FOX_QUEUE_LEN_BYTES);
				best_count = curr_count;
				if (best_count == count)
					break;
			}
		}
		if (i!=pts*2) // we broke
			break;
	}
	//assert(best_count<2);
	return best_count;
}

void Foxtrot::processPackets()
{
	seconds_t now = getGlobalSeconds();
	seconds_t when = MAX_SECONDS;
	for (uint8_t i=0;i<storage.size();i++)
	{
		seconds_t expire = storage[i]->getWhen()+maxLatency;
		//print_data("Data Point (pp)",storage[i]);
		if (expire<=now)
		{
			FoxtrotPacket *sender = new FoxtrotPacket(*(storage[i]));
			storage.erase(i);
			i--;
			DBG("expiry exceeded (now=%d, when=%d, maxlatency=%d)\n",now,sender->getWhen(),maxLatency);
			sender->setKind(0xDEAD);
			assert(isSink);
			sendUp(sender);
		}
		else if (expire-now<when)
		{
			DBG("expiry *not* exceeded, extra wait is %d (now=%d, when=%d, maxlatency=%d)\n",expire-now,now,storage[i]->getWhen(),maxLatency);
			when = expire-now;
		}
		else
			DBG("expiry *not* exceeded (now=%d, when=%d, maxlatency=%d)\n",now,storage[i]->getWhen(),maxLatency);
	}
	if (when == MAX_SECONDS)
	{
		DBG("No later events, killing clock\n");
		//call Clock.disable();
	}
	else
	{	
		DBG("Waiting %u seconds for next event\n",when);
		nextTimeout = now+when;
		setTimer(TIMER_PROCESS,when);
		//call Clock.enable();
		//call Clock.BigWait(when<<10); // shift left by 10 to go from 1024 msec units to 1 msec units
	}
}

inline bool Foxtrot::val_in_range(const foxtrot_data *a, const foxtrot_point pt)
{
	return (a->min<=pt && pt<=a->max);
}

inline bool Foxtrot::all_in_range(const foxtrot_data *a, const foxtrot_data *b)
{
	return val_in_range(a,b->min) && val_in_range(a,b->max);
}

inline bool Foxtrot::inside(const FoxtrotPacket *outer, const FoxtrotPacket *inner)
{
	uint8_t i;
	for (i=0;i<AXES;i++)
		if (outer->getLoc(i).min>inner->getLoc(i).min || outer->getLoc(i).max<inner->getLoc(i).max)
			return false;
	return true;
}

void Foxtrot::handleFrameTimer(unsigned int index)
{
	//bool wd =  Foxtrot.dataNow();
	bool wd = false;
	if (!wd)
	{
		DBG("(foxtrot) not waiting for data, send now\n");
		if (storage.size()>0 && !isSink)
			doSend();
		else
			DBG("storage.size = %d, sink = %d\n",storage.size(),isSink);
	}
	else
	{
		DBG("(foxtrot) waiting for data\n");
		wait_data = false;
	}
	items++;
	if (items>1)
	{
		DBG("Cancelling frame timer\n");
		cancelFrameTimer(0);
	}
}
