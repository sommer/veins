

#ifndef MACTOUWBIRPHYCONTROLINFO_
#define MACTOUWBIRPHYCONTROLINFO_

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
class MacToUWBIRPhyControlInfo: public MacToPhyControlInfo {
protected:
	IEEE802154A::config cfg;
	public:
	  MacToUWBIRPhyControlInfo(Signal* signal = 0, IEEE802154A::config cfg = IEEE802154A::cfg_mandatory_16M):
		  MacToPhyControlInfo(signal), cfg(cfg) { };

	  IEEE802154A::config getConfig() { return cfg; };

};

#endif
