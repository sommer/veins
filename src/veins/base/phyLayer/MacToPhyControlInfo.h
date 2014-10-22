#ifndef MACTOPHYCONTROLINFO_H_
#define MACTOPHYCONTROLINFO_H_

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/phyLayer/Signal_.h"

/**
 * @brief Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */
class MIXIM_API MacToPhyControlInfo: public cObject {
protected:
	/** @brief A pointer to the signal representing the transmission.*/
	Signal* signal;

public:
	/**
	 * @brief Initialize the MacToPhyControlInfo with the passed
	 * signal or null if signal is ommited.
	 *
	 * NOTE: Once a signal is passed to the MacToPhyControlInfo,
	 * 		 MacToPhyControlInfo takes the ownership of the Signal.
	 */
	MacToPhyControlInfo(Signal* signal = NULL):
		signal(signal) {}

	/**
	 * @brief Delete the signal if it is still in our ownership.
	 */
	virtual ~MacToPhyControlInfo() {
		if(signal)
			delete signal;
	}

	/**
	 * @brief Sets the signal of this MacToPhyControlInfo.
	 *
	 * NOTE: Once a signal is passed to the MacToPhyControlInfo,
	 * 		 MacToPhyControlInfo takes the ownership of the Signal.
	 */
	void setSignal(Signal* s) {
		if(signal)
			delete signal;

		signal = s;
	}

	/**
	 * @brief Returns a pointer to the Signal of this ControlInfo.
	 *
	 * NOTE: The ownership of the Signal is passed together with
	 * 		 the Signal itself. This means the caller of this
	 * 		 method is responsible for deletion of the Signal.
	 * 		 MacToPhyControlInfo also loses its pointer to the
	 * 		 Signal when this Method is called so following
	 * 		 calls of this method will return null!
	 */
	Signal* retrieveSignal() {
		Signal* tmp = signal;
		signal = 0;
		return tmp;
	}

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
    static cObject *const setControlInfo(cMessage *const pMsg, Signal *const pSignal) {
    	MacToPhyControlInfo *const cCtrlInfo = new MacToPhyControlInfo(pSignal);
    	pMsg->setControlInfo(cCtrlInfo);

    	return cCtrlInfo;
    }
    /**
     * @brief extracts the signal from message "control info".
     */
    static Signal *const getSignal(cMessage *const pMsg) {
    	return getSignalFromControlInfo(pMsg->getControlInfo());
    }
    /**
     * @brief extracts the signal from "control info".
     */
    static Signal *const getSignalFromControlInfo(cObject *const pCtrlInfo) {
    	MacToPhyControlInfo *const cCtrlInfo = dynamic_cast<MacToPhyControlInfo *const>(pCtrlInfo);

    	if (cCtrlInfo)
    		return cCtrlInfo->retrieveSignal();
    	return NULL;
    }
};

#endif /*MACTOPHYCONTROLINFO_H_*/
