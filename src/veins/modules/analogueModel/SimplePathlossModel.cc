#include "veins/modules/analogueModel/SimplePathlossModel.h"

#include "veins/base/messages/AirFrame_m.h"

using namespace Veins;

using Veins::AirFrame;

void SimplePathlossModel::filterSignal(Signal* signal, const Coord& senderPos, const Coord& receiverPos)
{
    /** Calculate the distance factor */
    double sqrDistance = useTorus ? receiverPos.sqrTorusDist(senderPos, playgroundSize) : receiverPos.sqrdist(senderPos);

    EV_TRACE << "sqrdistance is: " << sqrDistance << endl;

    if (sqrDistance <= 1.0) {
        // attenuation is negligible
        return;
    }

    // wavelength in meters
    double wavelength = (BaseWorldUtility::speedOfLight() / carrierFrequency);
    EV_TRACE << "wavelength is: " << wavelength << endl;

    // the part of the attenuation only depending on the distance
    double distFactor = pow(sqrDistance, -pathLossAlphaHalf) / (16.0 * M_PI * M_PI);
    EV_TRACE << "distance factor is: " << distFactor << endl;

    for (uint16_t i = signal->getRelativeStart(); i < signal->getRelativeEnd(); i++) {
        double wavelength = BaseWorldUtility::speedOfLight() / signal->getAbsoluteFreqAt(i);
        double attenuation = (wavelength * wavelength) * distFactor;

        signal->addAttenuation(i, attenuation);
    }
}

double SimplePathlossModel::calcPathloss(const Coord& receiverPos, const Coord& senderPos)
{
    /*
     * maybe we can reuse an already calculated value for the square-distance
     * at this point.
     *
     */
    double sqrdistance = 0.0;

    if (useTorus) {
        sqrdistance = receiverPos.sqrTorusDist(senderPos, playgroundSize);
    }
    else {
        sqrdistance = receiverPos.sqrdist(senderPos);
    }

    EV_TRACE << "sqrdistance is: " << sqrdistance << endl;

    double attenuation = 1.0;
    // wavelength in metres
    double wavelength = (BaseWorldUtility::speedOfLight() / carrierFrequency);

    EV_TRACE << "wavelength is: " << wavelength << endl;

    if (sqrdistance > 1.0) {
        attenuation = (wavelength * wavelength) / (16.0 * M_PI * M_PI) * (pow(sqrdistance, -1.0 * pathLossAlphaHalf));
    }

    EV_TRACE << "attenuation is: " << attenuation << endl;

    return attenuation;
}
