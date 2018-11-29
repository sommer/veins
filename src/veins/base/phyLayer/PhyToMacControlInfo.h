#pragma once

#include "veins/veins.h"

#include "veins/base/phyLayer/Decider.h"

namespace Veins {

/**
 * @brief Controlinfo for packets which are send from Physical
 * layer to the MAC layer.
 *
 * The ControlInfo contains the the DeciderResult of the Decider.
 * @ingroup phyLayer
 * @ingroup macLayer
 */
class VEINS_API PhyToMacControlInfo : public cObject {
protected:
    /** The result of the decider evaluation.*/
    DeciderResult* result;

public:
    /**
     * @brief Initializes the PhyToMacControlInfo with the passed DeciderResult.
     *
     * NOTE: PhyToMacControlInfo takes ownership of the passed DeciderResult!
     */
    PhyToMacControlInfo(DeciderResult* result)
        : result(result)
    {
    }

    /**
     * @brief Clean up the DeciderResult.
     */
    ~PhyToMacControlInfo() override
    {
        if (result) delete result;
    }

    /**
     * @brief Returns the result of the evaluation of the Decider.
     */
    DeciderResult* getDeciderResult() const
    {
        return result;
    }

    /**
     * @brief Attaches a "control info" structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the decider result.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg                The message where the "control info" shall be attached.
     * @param pDeciderResult    The decider results.
     */
    static cObject* const setControlInfo(cMessage* const pMsg, DeciderResult* const pDeciderResult)
    {
        PhyToMacControlInfo* const cCtrlInfo = new PhyToMacControlInfo(pDeciderResult);
        pMsg->setControlInfo(cCtrlInfo);

        return cCtrlInfo;
    }
    /**
     * @brief extracts the decider result from message "control info".
     */
    static DeciderResult* const getDeciderResult(cMessage* const pMsg)
    {
        return getDeciderResultFromControlInfo(pMsg->getControlInfo());
    }
    /**
     * @brief extracts the decider result from message "control info".
     */
    static DeciderResult* const getDeciderResultFromControlInfo(cObject* const pCtrlInfo)
    {
        PhyToMacControlInfo* const cCtrlInfo = dynamic_cast<PhyToMacControlInfo* const>(pCtrlInfo);

        if (cCtrlInfo) return cCtrlInfo->getDeciderResult();
        return nullptr;
    }
};

} // namespace Veins
