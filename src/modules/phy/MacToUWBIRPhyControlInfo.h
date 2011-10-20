

#ifndef MACTOUWBIRPHYCONTROLINFO_
#define MACTOUWBIRPHYCONTROLINFO_

#include <cassert>

#include "MiXiMDefs.h"
#include "MacToPhyControlInfo.h"
#include "IEEE802154A.h"

/**
 * @brief This control info allows to store the IEEE802154A config parameters
 * that were used to generate the accompanying signal.
 *
 * @ingroup ieee802154a
 * @ingroup phyLayer
 * @ingroup macLayer
 */
class MIXIM_API MacToUWBIRPhyControlInfo: public MacToPhyControlInfo {
protected:
	IEEE802154A::config cfg;
	public:
	  MacToUWBIRPhyControlInfo(Signal* signal = 0, const IEEE802154A::config& cfg = IEEE802154A::cfg_mandatory_16M):
		  MacToPhyControlInfo(signal), cfg(cfg) { };

	  const IEEE802154A::config& getConfig() const { return cfg; };

	    /**
	     * @brief Attaches a "control info" structure (object) to the message pMsg.
	     *
	     * This is most useful when passing packets between protocol layers
	     * of a protocol stack, the control info will contain the signal.
	     *
	     * The "control info" object will be deleted when the message is deleted.
	     * Only one "control info" structure can be attached (the second
	     * setL3ToL2ControlInfo() call throws an error).
	     *
	     * @param pMsg		The message where the "control info" shall be attached.
	     * @param pSignal	The signal which should be send.
	     */
	    static cObject *const setControlInfo(cMessage *const pMsg, Signal *const pSignal, const IEEE802154A::config& pConfig = IEEE802154A::cfg_mandatory_16M ) {
	    	MacToUWBIRPhyControlInfo *const cCtrlInfo = new MacToUWBIRPhyControlInfo(pSignal, pConfig);
	    	pMsg->setControlInfo(cCtrlInfo);

	    	return cCtrlInfo;
	    }
	    /**
	     * @brief extracts the signal from "control info".
	     */
	    static Signal *const getSignalFromControlInfo(cObject *const pCtrlInfo) {
	    	MacToUWBIRPhyControlInfo *const cCtrlInfo = dynamic_cast<MacToUWBIRPhyControlInfo *const>(pCtrlInfo);

	    	if (cCtrlInfo)
	    		return cCtrlInfo->retrieveSignal();
	    	return NULL;
	    }
	    /**
	     * @brief extracts the signal from "control info".
	     */
	    static const IEEE802154A::config& getConfigFromControlInfo(cObject *const pCtrlInfo) {
	    	MacToUWBIRPhyControlInfo *const cCtrlInfo = dynamic_cast<MacToUWBIRPhyControlInfo *const>(pCtrlInfo);

	    	assert(cCtrlInfo);
	    	return cCtrlInfo->getConfig();
	    }
};

#endif
