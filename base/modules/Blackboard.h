// -*- mode:c++ -*-
// Copyright (C) 2005 Andras Varga
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


#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <omnetpp.h>
#include <vector>
#include <string>

#include "ModuleAccess.h"

class ImNotifiable;
class BBItem;


/**
 * In protocol simulations, one often has to evaluate the performance
 * of a protocol. This implies that not only the protocol has to be
 * developed, but also the code used for the performance evaluation.
 * Often, this leads to ugly implementations: real protocol code is
 * mixed with debug and performance evaluation code. This mix becomes
 * annoying if the code is made public to the community.
 *
 * Another, somewhat similar, problem appears if programmers want to
 * implement new protocols that integrate information from other
 * protocol layers. Usually, researchers have to reimplement at least
 * parts of the protocol tp expose the necessary information -- making
 * it less useful for other researchers as this usually increases the
 * coupling of the protocols.
 *
 * One way around both problems is a black board. On the black board,
 * protocols publish their state and possible state changes. It is now
 * possible to separate performance monitors from the protocol
 * code. Also, cross-layer information exchange also boils down to
 * publishing and subscribing information -- without introducing tight
 * coupling. The only commonly known entity is the blackboard.
 *
 * The interaction with the blackboard is simple:
 *
 * publisher -publish(BBItem)-> Blackboard -notify(BBItem)--> subscriber
 *
 * The publisher can be anything that knows how to call a Blackboard
 * and how to construct a proper BBItem. It remains the logical owner
 * of the published item. The BB neither stores it, nor keeps any
 * reference to it.
 *
 * The subscriber must implement the ImNotifiable
 * interface. It can keep a copy of the published item (cache it), but
 * it should not keep a reference to it. Otherwise, things become
 * really messy. (Some code written for older versions of the BB
 * speaks for itself).
 *
 * 
 *
 * 
 * @see ImNotifiable
 * @ingroup blackboard
 *
 * @author Andras Varga
 * @author Andreas Koepke
 */
class Blackboard : public cSimpleModule
{
  protected:
    bool coreDebug;

    class Subscriber 
    {
    public:
        ImNotifiable *client;
        int scopeModuleId;
    public:
        Subscriber(ImNotifiable *c=0, int b=-1) :
            client(c), scopeModuleId(b) {};
    };
    

    typedef std::vector<Subscriber> SubscriberVector;
    typedef std::vector<SubscriberVector> ClientVector;
    
    typedef std::vector<const char* > CategoryDescriptions;

    typedef std::vector<int> ParentVector;
    
    typedef ClientVector::iterator ClientVectorIter;
    typedef CategoryDescriptions::iterator DescriptionsIter;
    typedef ParentVector::iterator ParentVectorIter;
    
    ClientVector clientVector;
    CategoryDescriptions categoryDescriptions;
    ParentVector parentVector;
    
    int nextCategory;
    
    friend std::ostream& operator<<(std::ostream&, const SubscriberVector&);

 protected:
    /**
     * find the description of a category
     */
    const char* categoryName(int category);
    /**
     * find or create a category, returns iterator in map and sets
     * isNewEntry to true if the entry was created new.
     */
    int findAndCreateDescription(bool *isNewEntry, const BBItem *category);

    /**
     * traverse inheritance diagramm and make sure that children of
     * category are also delivered to modules that susbcribed to one
     * of its parent classes.
     */
    void fillParentVector(const BBItem *category, int cat);
    
 public:
    Module_Class_Members(Blackboard, cSimpleModule, 0);
    virtual ~Blackboard();

    /**
     * Initialize.
     */
    virtual void initialize();

    /**
     * Does nothing.
     */
    virtual void handleMessage(cMessage *msg);

    /** @name Methods for consumers of change notifications */
    //@{
    /**
     * Subscribe to changes of a specific category. The category is
     * defined by the BBItem class to which this object belongs.
     * Returns the id of this category.
     *
     * Both subscribe functions subscribe also to children of this
     * category class.
     * 
     */
    int subscribe(ImNotifiable *client, const BBItem *category, int scopeModuleId=-1);
    /**
     * Subscribe to changes of a specific category. This time using
     * the numeric id. This implies that you have previously called
     * subscribe with the apprpriate category class.
     *
     * Both subscribe functions subscribe also to children of this
     * category class.
     *
     * Subscribe in stage 0 of initialize -- this way you get all
     * publishes that happen at stage 1.
     */
    int subscribe(ImNotifiable *client, int category, int scopeModuleId=-1);
    /**
     * Unsubscribe from notifications
     */
    void unsubscribe(ImNotifiable *client, int category);
    //@}

    /** @name Methods for producers of change notifications */
    //@{
    /**
     * Tells Blackboard that a change of the given category has taken
     * place. The optional details object (@see BBItem) may carry more
     * specific information about the change (e.g. exact location,
     * specific attribute that changed, old value, new value, etc).
     *
     * This function is very central once you start working with the
     * blackboard. You should heed the following advices:
     * 
     *     - Publish in stage 1 of initialize after everbody was able
     *       to subscribe.
     *     - This function should be the last thing you call in a method.
     *       There is a chance that a certain race condition occurs,
     *       as in the following case, for simplicity the module is subscribed
     *       to the value that it publishes: 
     *          -- Module::myMethod: lock module; publishBBItem; unlock module
     *          -- Module::receiveBBItem: if module not locked, do something
     *       Since receiveBBItem is called from within publishBBItem, the module
     *       will always be locked and the code in receiveBBItem is never
     *       executed. 
     *           
     */
    void publishBBItem(int category, const BBItem* details, int scopeModuleId);

    /**
     * Get the category from the BB -- if you don't want to subscribe but rather
     * publish
     */
    int  getCategory(const BBItem* details);
    //@}
};

/**
 * Clients can receive change notifications from the Blackboard via
 * this interface. Clients must "implement" (subclass from) this class.
 *
 * @see Blackboard
 * @ingroup blackboard
 *
 * @author Andras Varga
 * @author Andreas Koepke
 * 
 */
class  ImNotifiable
{
  public:
    /**
     * Called by the Blackboard whenever a change of a category
     * occurs to which this client has subscribed.
     *
     * If your class is derived from a class that also is notifiable,
     * make sure that you call it first thing you do.
     * E.g.
     * 
     * BaseClass : public ImNotifiable ...
     * 
     * YourClass : public BaseClass ...
     *
     * YourClass::receiveBBItem(category, details, scopeModuleId) {
     *    BaseClass::receiveBBItem(category, details, scopeModuleId);
     *    switch(category) {
     *      ...
     *    }
     * }
     *
     * This also implies that you should handle unknown categories gracefully --
     * maybe a subclass subscribed to them.
     *
     * This function is called from within publishBBItem, please pay
     * attention to race conditions that can appear. If you want to
     * schedule messages when this function of your module is called,
     * you have to use the Enter_Method or the Enter_Method_Silent
     * macro.
     */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId) = 0;

    /** @brief Prevent compiler complaints, but its best not to use the destructor. */
    virtual ~ImNotifiable() { }
};

/**
 * Gives access to the Blackboard instance within the host/router.
 *
 * @ingroup blackboard
 */
class  BlackboardAccess : public ModuleAccess<Blackboard>
{
    public:
        BlackboardAccess() : ModuleAccess<Blackboard>("blackboard") {}
};

/**
 * Contains detailed information about the items published using the
 * BB.
 * 
 * To understand this class, consider the case of routing
 * protocols. The minimal entry in a routing table consists of the
 * tuple <target, neighbor, cost>. However, some more specific routing
 * protocol may wqant to store more information in a table
 * entry. Hence, it subclasses the original entry and adds its own
 * fields. How do other parties that are interested in topology
 * changes still receive information about changes in the routing
 * table? The only way is to make the inheritance structure a bit
 * accessible. This enables the BB to further deliver notifications to
 * subscribers of the original entry, but also to those of the new one
 * -- and all that without the need to recompile the framework due to
 * a newly introduced constant.
 *
 * The info function of cPolymorphic should be implemented -- you can
 * use it to disinguish between items of the same category (like two
 * MAC addresses if the host has two network interfaces).
 * 
 * @see BBITEM_METAINFO
 * @see RadioState
 * @ingroup blackboard
 * @author Andreas Koepke
 */
class  BBItem : public cPolymorphic
{
 public:
    virtual BBItem *parentObject() const {
        return new BBItem();
    }
};

/**
 * Helper macro to define a minimum of necessary fields for siblings
 * of BBItem. @see RadioState for an implementation example.
 *
 * @author Andreas Koepke
 * @see BBItem
 * @see RadioState
 */
#define BBITEM_METAINFO(BASECLASS) \
    public: \
      virtual BBItem* parentObject() const { \
      return new BASECLASS(); } 
      
#endif

