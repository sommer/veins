#include "BaseUtility.h"
#include "BaseWorldUtility.h"
#include <assert.h>

Define_Module(BaseUtility);

//BB start

#ifndef coreEV
#define coreEV (ev.disabled()||!coreDebug) ? std::cout : ev <<parentModule()->name()<<"["<<parentModule()->index()<<"]::Blackboard: "
#endif

std::ostream& operator<<(std::ostream& os, const BaseUtility::SubscriberVector& v)
{
    os << v.size() << " client(s) ";
    BaseUtility::SubscriberVector::const_iterator it;
    for (it = v.begin(); it != v.end(); ++it)
    {
        os << (it == v.begin() ? ": " : ", ");
        if (dynamic_cast<cModule*>((*it).client))
        {
            cModule *mod = dynamic_cast<cModule*>((*it).client);
            os << "mod (" << mod->className() << ") " << mod->fullName() << " id=" << mod->id();
        }
        else if (dynamic_cast<cPolymorphic*>((*it).client))
        {
            cPolymorphic *obj = dynamic_cast<cPolymorphic*>((*it).client);
            os << "a " << obj->className();
        }
        else
        {
            os << "a " << opp_typename(typeid((*it).client));
        }
        os << " scope: " << (*it).scopeModuleId;
    }
    return os;
}



BaseUtility::~BaseUtility()
{
}

//BB end

void BaseUtility::initialize(int stage) {
	BaseModule::initialize(stage);
	if (stage == 0) {
		hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
		// reading the out from omnetpp.ini makes predefined scenarios a lot easier
		if (hasPar("x") && hasPar("y") && hasPar("z")){
			// pos.x = par("x");
			// pos.y = par("y");
			// pos.z = par("z");
			pos.setX(par("x"));
			pos.setY(par("y"));
			pos.setZ(par("z"));
		} else{
			// Start at a random position
			// pos.x = pos.y = pos.z = -1;
			pos.setX(-1);
			pos.setY(-1);
			pos.setZ(-1);
		}
		coreEV << "pos: " << pos.info() << endl;
		
//BB start
		WATCH_VECTOR(clientVector);
	    WATCH_VECTOR(categoryDescriptions);
	    
	    clientVector.clear();
	    categoryDescriptions.clear();
	    parentVector.clear();
	    nextCategory = 0;	
//BB end 
	} else if (stage == 1) {
		BaseWorldUtility *world = dynamic_cast<BaseWorldUtility*>(getGlobalModule("BaseWorldUtility"));
		assert(world!=NULL);
		Coord pgs =  world->getPgs();

		coreEV << "pos: " << pos.info() << endl;

		// -1 indicates start at random position
		if (pos.getX() == -1 || pos.getY() == -1) // consideration of 3D?
		{
			pos = world->getRandomPosition();
			coreEV << "pos: " << pos.info() << endl;
		}
		//we do not have negative positions
		//also checks whether position is within the playground
		else if (	pos.getX() < 0 || pos.getY() < 0 || pos.getZ() < 0 ||
				pos.getX() > pgs.getX() || pos.getY() > pgs.getY() || pos.getZ() > pgs.getZ())
			error("node position specified in omnetpp.ini exceeds playgroundsize");
	}
}

void BaseUtility::handleMessage(cMessage *msg) {
	error("This module does not handle any messages yet");
}

void BaseUtility::setPos(Coord* newCoord) {
  coreEV << "Setting position to pos: " << pos.info() << endl;
	pos.setX(newCoord->getX());
	pos.setY(newCoord->getY());
	pos.setZ(newCoord->getZ());
}

//BB start


const char* BaseUtility::categoryName(int category) 
{
    if(categoryDescriptions.size() < static_cast<unsigned>(category)) {
        error("BaseUtility::categoryName called with unknown category (%i)", category);
    }
    return categoryDescriptions[category];
}

int BaseUtility::findAndCreateDescription(bool *isNewEntry, const BBItem *category) 
{

    CategoryDescriptions::size_type it;
    std::string desc = category->className();
    std::string cName;
    
    for(it = 0; it < categoryDescriptions.size(); ++it)
    {
        cName = categoryDescriptions[it];
        if(cName == desc) {
            (*isNewEntry) = false;
            break;
        }
    }
    if(it == categoryDescriptions.size()) {
        (*isNewEntry) = true;
        categoryDescriptions.push_back(category->className());
        it = categoryDescriptions.size() - 1;
        clientVector.push_back(SubscriberVector());
        if(it != clientVector.size()-1)
            error("BaseUtility::findAndCreateDescription SubscriberVector creation failed");
        parentVector.push_back(-1);
        if(it != parentVector.size()-1)
            error("BaseUtility::findAndCreateDescription ParentVector creation failed");
        nextCategory = categoryDescriptions.size();
    }
    return static_cast<int>(it);
}

void BaseUtility::fillParentVector(const BBItem *category, int cat) 
{
    bool isNewEntry = true;
    int it;
    
    BBItem *parentObject;
    BBItem base;
    
    parentObject = category->parentObject();

    if(typeid(*parentObject) != typeid(base)) {
        it = findAndCreateDescription(&isNewEntry, parentObject);
        parentVector[cat] = it;
        fillParentVector(parentObject, it);
    }
    delete parentObject;
}



int BaseUtility::subscribe(ImNotifiable *client, int category, int scopeModuleId)
{
    if(coreDebug) {
        Enter_Method("subscribe(%s, %i)", categoryName(category), scopeModuleId);
    } else {
        Enter_Method_Silent();
    }
    
    // find or create entry for this category
    SubscriberVector& clients = clientVector[category];
    SubscriberVector::const_iterator it;
    for(it = clients.begin(); it != clients.end(); ++it) {
        if((*it).client == client) {
            std::string cname("unkown");
            cModule *cm = dynamic_cast<cModule *>(client);
            if(cm) cname = cm->fullPath();
            error("BaseUtility::subscribe called twice for item %s, by client %s. \nThis probably means that a class from which you derived yours has \nalready subscribed to this item. This may result in conflicts, please check.\n ", categoryName(category), cname.c_str());
            break;
        }
    }
    // add client if not already there
    if (it == clients.end()) {
        clients.push_back(Subscriber(client, scopeModuleId));
    }
    return category;
}

int BaseUtility::subscribe(ImNotifiable *client, const BBItem *category, int scopeModuleId) 
{
    if(coreDebug) {
        Enter_Method("subscribe(%s, %i)", category?category->className() : "n/a",
                     scopeModuleId);
    } else {
        Enter_Method_Silent();
    }
    CategoryDescriptions::size_type it = 0;
    bool isNewEntry;
    
    if(category) {
        it = findAndCreateDescription(&isNewEntry, category);
        if(isNewEntry) fillParentVector(category, static_cast<int>(it));
        subscribe(client, static_cast<int>(it), scopeModuleId);
    }
    else {
        error("BaseUtility::subscribe called without category item");
    }
    return static_cast<int>(it);
}

void BaseUtility::unsubscribe(ImNotifiable *client, int category)
{
    if(coreDebug) {
        Enter_Method("unsubscribe(%s)", categoryName(category));
    } else {
        Enter_Method_Silent();
    }
    // find (or create) entry for this category
    SubscriberVector& clients = clientVector[category];

    // remove client if there
    SubscriberVector::iterator it;
    for(it = clients.begin(); it != clients.end(); ++it) {
        if((*it).client == client) clients.erase(it);
        break;
    }
}

void BaseUtility::publishBBItem(int category, const BBItem *details, int scopeModuleId)
{
    if(coreDebug) {
        Enter_Method("publish(%s, %s, %i)", categoryName(category),
                     details?details->info().c_str() : "n/a", scopeModuleId);
    } else {
        Enter_Method_Silent();
    }
    
    int pCat;
    if(clientVector.size() > 0) {
        SubscriberVector::const_iterator j;
        for (j=clientVector[category].begin(); j!=clientVector[category].end(); ++j)
        {
            if(((*j).scopeModuleId == -1) || ((*j).scopeModuleId == scopeModuleId))
            {
                (*j).client->receiveBBItem(category, details, scopeModuleId);
            }
        }
        pCat = parentVector[category];
        if(pCat >= 0) publishBBItem(pCat, details, scopeModuleId);
    }
}

int BaseUtility::getCategory(const BBItem *details) 
{
    if(coreDebug) {
        Enter_Method("getCategory(%s)", details?details->className():"n/a");
    } else {
        Enter_Method_Silent();
    }
    int category = -1;
    bool isNewEntry;
    
    if(details) {
        category = findAndCreateDescription(&isNewEntry, details);
        if(isNewEntry) fillParentVector(details, category);
    } else {
        error("BaseUtility::getCategory called without category item");
    }
    return category;
}

//BB end 

