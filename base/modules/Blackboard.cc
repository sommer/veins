// -*- mode:c++ -*-
// Copyright (C) 2004 Andras Varga, Andreas Koepke
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "Blackboard.h"

Define_Module(Blackboard);

#ifndef coreEV
#define coreEV (ev.disabled()||!coreDebug) ? std::cout : ev <<parentModule()->name()<<"["<<parentModule()->index()<<"]::Blackboard: "
#endif

std::ostream& operator<<(std::ostream& os, const Blackboard::SubscriberVector& v)
{
    os << v.size() << " client(s) ";
    Blackboard::SubscriberVector::const_iterator it;
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

Blackboard::~Blackboard()
{
}

void Blackboard::initialize()
{
    if(hasPar("coreDebug"))
      coreDebug = par("coreDebug").boolValue();
    else
      coreDebug = false;

    WATCH_VECTOR(clientVector);
    WATCH_VECTOR(categoryDescriptions);
    
    clientVector.clear();
    categoryDescriptions.clear();
    parentVector.clear();
    nextCategory = 0;
}

void Blackboard::handleMessage(cMessage *msg)
{
    error("Blackboard doesn't handle messages, it can be accessed via direct method calls");
}

const char* Blackboard::categoryName(int category) 
{
    if(categoryDescriptions.size() < static_cast<unsigned>(category)) {
        error("Blackboard::categoryName called with unknown category (%i)", category);
    }
    return categoryDescriptions[category];
}

int Blackboard::findAndCreateDescription(bool *isNewEntry, const BBItem *category) 
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
            error("Blackboard::findAndCreateDescription SubscriberVector creation failed");
        parentVector.push_back(-1);
        if(it != parentVector.size()-1)
            error("Blackboard::findAndCreateDescription ParentVector creation failed");
        nextCategory = categoryDescriptions.size();
    }
    return static_cast<int>(it);
}

void Blackboard::fillParentVector(const BBItem *category, int cat) 
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

int Blackboard::subscribe(ImNotifiable *client, int category, int scopeModuleId)
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
            error("Blackboard::subscribe called twice for item %s, by client %s. \nThis probably means that a class from which you derived yours has \nalready subscribed to this item. This may result in conflicts, please check.\n ", categoryName(category), cname.c_str());
            break;
        }
    }
    // add client if not already there
    if (it == clients.end()) {
        clients.push_back(Subscriber(client, scopeModuleId));
    }
    return category;
}

int Blackboard::subscribe(ImNotifiable *client, const BBItem *category, int scopeModuleId) 
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
        error("Blackboard::subscribe called without category item");
    }
    return static_cast<int>(it);
}

void Blackboard::unsubscribe(ImNotifiable *client, int category)
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

void Blackboard::publishBBItem(int category, const BBItem *details, int scopeModuleId)
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

int Blackboard::getCategory(const BBItem *details) 
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
        error("Blackboard::getCategory called without category item");
    }
    return category;
}

