#include "veins/modules/analogueModel/SimplePathlossModel.h"

#include "veins/base/messages/AirFrame_m.h"

using namespace Veins;

using Veins::AirFrame;

#define splmEV EV << "PhyLayer(SimplePathlossModel): "

void SimplePathlossModel::filterSignal(Signal* signal, const Coord& sendersPos, const Coord& receiverPos)
{
    /** Calculate the distance factor */
    double sqrDistance = useTorus ? receiverPos.sqrTorusDist(sendersPos, playgroundSize) : receiverPos.sqrdist(sendersPos);

    splmEV << "sqrdistance is: " << sqrDistance << endl;

    if (sqrDistance <= 1.0) {
        // attenuation is negligible
        return;
    }

    // wavelength in meters
    double wavelength = (BaseWorldUtility::speedOfLight() / carrierFrequency);
    splmEV << "wavelength is: " << wavelength << endl;

    // the part of the attenuation only depending on the distance
    double distFactor = pow(sqrDistance, -pathLossAlphaHalf) / (16.0 * M_PI * M_PI);
    splmEV << "distance factor is: " << distFactor << endl;

    for (uint16_t i = signal->getRelativeStart(); i < signal->getRelativeEnd(); i++) {
        double wavelength = BaseWorldUtility::speedOfLight() / signal->getAbsoluteFreqAt(i);
        double attenuation = (wavelength * wavelength) * distFactor;

        signal->addAttenuation(i, attenuation);
    }
}

double SimplePathlossModel::calcPathloss(const Coord& receiverPos, const Coord& sendersPos)
{
    /*
     * maybe we can reuse an already calculated value for the square-distance
     * at this point.
     *
     */
    double sqrdistance = 0.0;

    if (useTorus) {
        sqrdistance = receiverPos.sqrTorusDist(sendersPos, playgroundSize);
    }
    else {
        sqrdistance = receiverPos.sqrdist(sendersPos);
    }

    splmEV << "sqrdistance is: " << sqrdistance << endl;

    double attenuation = 1.0;
    // wavelength in metres
    double wavelength = (BaseWorldUtility::speedOfLight() / carrierFrequency);

    splmEV << "wavelength is: " << wavelength << endl;

    if (sqrdistance > 1.0) {
        attenuation = (wavelength * wavelength) / (16.0 * M_PI * M_PI) * (pow(sqrdistance, -1.0 * pathLossAlphaHalf));
    }

    splmEV << "attenuation is: " << attenuation << endl;

    return attenuation;
}
