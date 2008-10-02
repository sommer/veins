#ifndef IMNOTIFIABLE_H_
#define IMNOTIFIABLE_H_

#include <omnetpp.h>


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
 * The info function of cObject should be implemented -- you can
 * use it to disinguish between items of the same category (like two
 * MAC addresses if the host has two network interfaces).
 * 
 * @see BBITEM_METAINFO
 * @see RadioState
 * @ingroup blackboard
 * @author Andreas Koepke
 */
class  BBItem : public cObject
{
 public:
    virtual BBItem *parentObject() const {
        return new BBItem();
    }
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
      

#endif /*IMNOTIFIABLE_H_*/
