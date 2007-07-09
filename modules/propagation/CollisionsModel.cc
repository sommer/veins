#include "CollisionsModel.h"
#include "CorruptMessage_m.h"
#include "CollisionsPhy.h"
#include "StartMessage_m.h"

Define_Module_Like(CollisionsModel, BasePropagation);

void CollisionsModel::registerNic( BasePhyLayer* phy)
{
	CollisionsPhy *cp = dynamic_cast<CollisionsPhy*>(phy);
	if (cp==NULL)
		error("CollisionsModel needs CollisionsPhy subclasses for phy's, in order to handle start and corrupt packets");
	BasePropagation::registerNic(phy);	
}

void CollisionsModel::initialize(int stage)
{
	BasePropagation::initialize(stage);
	if (stage == 0)
	{
		Timer::init(this);
		active = new std::map<unsigned int,std::pair<int,AirFrame *>* >();
	}
}

CollisionsModel::~CollisionsModel()
{
	delete active;
}

typedef struct 
{
	BasePhyLayer *phy;
	AirFrame *msg;
} PropMsg;

void cleanup(void * data)
{
	PropMsg *p = (PropMsg*)data;
	delete p->msg;
	free(p);
}

void CollisionsModel::sendToChannel(BasePhyLayer *phy,AirFrame *msg)
{
	Enter_Method_Silent();
	coreEV << "node number "<<phy->getNode()->index()<<" sending a message in "<<msg->getDuration()<<" seconds"<<endl;
	NodeList *hear = canHear(phy);
	if (hear->begin() == hear->end())
		error("No nodes to talk to!");
	unsigned int timer = setTimer(msg->getDuration());
	PropMsg *p = (PropMsg*)malloc(sizeof(PropMsg));
	p->phy = phy;
	p->msg = msg;
	setContextPointer(timer,p);
	setContextDestructor(timer,cleanup);
	for (NodeList::iterator i = hear->begin();i!=hear->end();i++)
	{
		if (*i ==p->phy)
			continue;
		unsigned int index = (*i)->getNode()->index();
		if (active->find(index) == active->end())
		{
			coreEV << "Marking "<<index<<" as active"<<endl;
			(*active)[index] = new std::pair<int,AirFrame *>(1,msg);
			sendDirect(new StartMessage("Start"),0.0,*i,INGATE);
		}
		else
		{
			(*active)[index]->first ++;
			coreEV << "Incrementing "<<index<<" as active (count is "<<(*active)[index]->first<<")"<<endl;
			(*active)[index]->second = NULL;
		}
	}
	delete hear;
}

void CollisionsModel::handleTimer(unsigned int index)
{
	PropMsg *p = (PropMsg*)contextPointer(index);
	AirFrame *msg = p->msg;
	NodeList *hear = canHear(p->phy);
	for (NodeList::iterator i = hear->begin();i!=hear->end();i++)
	{
		if (*i ==p->phy)
			continue;
		int index = (*i)->getNode()->index();
		cMessage *n;
		if ((*active)[index]->second==msg)
		{
			n = static_cast<cMessage*>(msg->dup());
			coreEV << "sending message to "<<index<<endl;
		}
		else
		{
			coreEV << "Message to "<<index<<" blocked due to corruption! (count is "<<(*active)[index]->first<<")"<<endl;
			n = new CorruptMessage("Corrupt data");
		}
		sendDirect(n,0.0,*i,INGATE);
		(*active)[index]->first --;
		if ((*active)[index]->first == 0)
		{
			std::pair<int,AirFrame *> *p = (*active)[index];
			delete p;
			active->erase(index);
			coreEV << "Wiping "<<index<<" as end"<<endl;
		}
	}
	cleanup(p);
	deleteTimer(index);
	delete hear;
}

NodeList * CollisionsModel::canHear(BasePhyLayer* phy)
{
	return new NodeList(*nodes);
}


