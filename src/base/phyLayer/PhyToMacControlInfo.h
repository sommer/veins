#ifndef PHYTOMACCONTROLINFO_H_
#define PHYTOMACCONTROLINFO_H_

#include <omnetpp.h>

/**
 * Controlinfo for packets which are send from Physical
 * layer to the MAC layer. The ControlInfo contains the
 * the DeciderResult of the Decider.
 */
class PhyToMacControlInfo: public cObject {
protected:
	/** The result of the decider evaluation.*/
	DeciderResult result;
	
public:
	/**
	 * Initializes the PhyToMacControlInfo with the passed DeciderResult.
	 */
	PhyToMacControlInfo(const DeciderResult& result):
		result(result) {}
	
	/**
	 * Returns the result of the evaluation of the Decider.
	 */
	DeciderResult getDeciderResult() const {
		return result;
	}	
};

#endif /*PHYTOMACCONTROLINFO_H_*/
