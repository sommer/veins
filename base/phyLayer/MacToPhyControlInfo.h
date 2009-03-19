#ifndef MACTOPHYCONTROLINFO_H_
#define MACTOPHYCONTROLINFO_H_

#include "Signal_.h"

/**
 * @brief Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */
class MacToPhyControlInfo: public cObject {
protected:
	Signal* signal;

public:
	/**
	 * @brief Initialize the MacToPhyControlInfo with the passed
	 * signal or null if signal is ommited.
	 *
	 * NOTE: Once a signal is passed to the MacToPhyControlInfo,
	 * 		 MacToPhyControlInfo takes the ownership of the Signal.
	 */
	MacToPhyControlInfo(Signal* signal = 0):
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
};

#endif /*MACTOPHYCONTROLINFO_H_*/
