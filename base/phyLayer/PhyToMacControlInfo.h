#ifndef PHYTOMACCONTROLINFO_H_
#define PHYTOMACCONTROLINFO_H_

#include <omnetpp.h>
#include <Decider.h>

/**
 * @brief Controlinfo for packets which are send from Physical
 * layer to the MAC layer.
 *
 * The ControlInfo contains the the DeciderResult of the Decider.
 * @ingroup phyLayer
 * @ingroup macLayer
 */
class PhyToMacControlInfo: public cObject {
protected:
	/** The result of the decider evaluation.*/
	DeciderResult* result;

public:
	/**
	 * @brief Initializes the PhyToMacControlInfo with the passed DeciderResult.
	 *
	 * NOTE: PhyToMacControlInfo takes ownership of the passed DeciderResult!
	 */
	PhyToMacControlInfo(DeciderResult* result):
		result(result) {}

	/**
	 * @brief Clean up the DeciderResult.
	 */
	~PhyToMacControlInfo() {
		if(result)
			delete result;
	}

	/**
	 * @brief Returns the result of the evaluation of the Decider.
	 */
	DeciderResult* getDeciderResult() const {
		return result;
	}
};

#endif /*PHYTOMACCONTROLINFO_H_*/
