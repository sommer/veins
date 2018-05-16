/*
 * SampledAntenna2D.cc
 *
 *  Created on: Apr 16, 2017
 *      Author: Alexander Brummer
 */

#include "veins/modules/phy/SampledAntenna2D.h"
#include "veins/base/utils/FWMath.h"


SampledAntenna2D::SampledAntenna2D(vector<double>& aziValues, string aziOffsetType, vector<double>& aziOffsetParams, string aziRotationType, vector<double>& aziRotationParams,
        vector<double>& eleValues, string eleOffsetType, vector<double>& eleOffsetParams, string eleRotationType, vector<double>& eleRotationParams, cRNG* rng){
    // use Mapping to store the azimuth samples (interpolates automatically)
    aziSamples = MappingUtils::createMapping();
    Argument pos;
    double dist = 360.0/aziValues.size();

    // instantiate a random number generator for azimuth sample offsets if one is specified
    cRandom* aziOffsetGen = 0;
    if (aziOffsetType == "uniform") {
        if (!FWMath::close(aziOffsetParams[0], -aziOffsetParams[1])) {
            throw cRuntimeError("SampledAntenna2D::SampledAntenna2D(): The mean of the random distribution for the azimuth samples' offsets has to be 0.");
        }
        aziOffsetGen = new cUniform(rng, aziOffsetParams[0], aziOffsetParams[1]);
    } else if (aziOffsetType == "normal") {
        if (!FWMath::close(aziOffsetParams[0], 0)) {
            throw cRuntimeError("SampledAntenna2D::SampledAntenna2D(): The mean of the random distribution for the azimuth samples' offsets has to be 0.");
        }
        aziOffsetGen = new cNormal(rng, aziOffsetParams[0], aziOffsetParams[1]);
    } else if (aziOffsetType == "triang") {
        if (!FWMath::close((aziOffsetParams[0] + aziOffsetParams[1] + aziOffsetParams[2])/3, 0)) {
            throw cRuntimeError("SampledAntenna2D::SampledAntenna2D(): The mean of the random distribution for the azimuth samples' offsets has to be 0.");
        }
        aziOffsetGen = new cTriang(rng, aziOffsetParams[0], aziOffsetParams[1], aziOffsetParams[2]);
    }

    // determine random rotation of the azimuth plane pattern if specified
    cRandom* aziRotationGen = 0;
    if (aziRotationType == "uniform") {
        aziRotationGen = new cUniform(rng, aziRotationParams[0], aziRotationParams[1]);
    } else if (aziRotationType == "normal") {
        aziRotationGen = new cNormal(rng, aziRotationParams[0], aziRotationParams[1]);
    } else if (aziRotationType == "triang") {
        aziRotationGen = new cTriang(rng, aziRotationParams[0], aziRotationParams[1], aziRotationParams[2]);
    }
    aziRotation = (aziRotationGen == 0) ? 0 : aziRotationGen->draw();
    if (aziRotationGen != 0)
        delete aziRotationGen;

    // populate the azimuth plane mapping
    for (unsigned int i = 0; i < aziValues.size(); i++) {
        pos.setTime(dist*i);
        double offset = 0;
        if (aziOffsetGen != 0) {
            offset = aziOffsetGen->draw();
        }
        aziSamples->setValue(pos, (aziValues[i] + offset));
    }
    if (aziOffsetGen != 0)
        delete aziOffsetGen;

    // assign the value of 0 degrees to 360 degrees as well to assure correct interpolation
    pos.setTime(0);
    double value0 = aziSamples->getValue(pos);
    pos.setTime(360);
    aziSamples->setValue(pos, value0);


    // now do the same procedure for the elevation plane pattern
    eleSamples = MappingUtils::createMapping();
    dist = 360.0/eleValues.size();

    cRandom* eleOffsetGen = 0;
    if (eleOffsetType == "uniform") {
        if (!FWMath::close(eleOffsetParams[0], -eleOffsetParams[1])) {
            throw cRuntimeError("SampledAntenna2D::SampledAntenna2D(): The mean of the random distribution for the elevation samples' offsets has to be 0.");
        }
        eleOffsetGen = new cUniform(rng, eleOffsetParams[0], eleOffsetParams[1]);
    } else if (eleOffsetType == "normal") {
        if (!FWMath::close(eleOffsetParams[0], 0)) {
            throw cRuntimeError("SampledAntenna2D::SampledAntenna2D(): The mean of the random distribution for the elevation samples' offsets has to be 0.");
        }
        eleOffsetGen = new cNormal(rng, eleOffsetParams[0], eleOffsetParams[1]);
    } else if (eleOffsetType == "triang") {
        if (!FWMath::close((eleOffsetParams[0] + eleOffsetParams[1] + eleOffsetParams[2])/3, 0)) {
            throw cRuntimeError("SampledAntenna2D::SampledAntenna2D(): The mean of the random distribution for the elevation samples' offsets has to be 0.");
        }
        eleOffsetGen = new cTriang(rng, eleOffsetParams[0], eleOffsetParams[1], eleOffsetParams[2]);
    }

    cRandom* eleRotationGen = 0;
    if (eleRotationType == "uniform") {
        eleRotationGen = new cUniform(rng, eleRotationParams[0], eleRotationParams[1]);
    } else if (eleRotationType == "normal") {
        eleRotationGen = new cNormal(rng, eleRotationParams[0], eleRotationParams[1]);
    } else if (eleRotationType == "triang") {
        eleRotationGen = new cTriang(rng, eleRotationParams[0], eleRotationParams[1], eleRotationParams[2]);
    }
    eleRotation = (eleRotationGen == 0) ? 0 : eleRotationGen->draw();
    if (eleRotationGen != 0)
        delete eleRotationGen;

    for (unsigned int i = 0; i < eleValues.size(); i++) {
        pos.setTime(dist*i);
        double offset = 0;
        if (eleOffsetGen != 0) {
            offset = eleOffsetGen->draw();
        }
        eleSamples->setValue(pos, (eleValues[i] + offset));
    }
    if (eleOffsetGen != 0)
        delete eleOffsetGen;

    pos.setTime(0);
    value0 = eleSamples->getValue(pos);
    pos.setTime(360);
    eleSamples->setValue(pos, value0);
}

SampledAntenna2D::~SampledAntenna2D() {
    delete aziSamples;
    delete eleSamples;
}

double SampledAntenna2D::getGain(Coord ownPos, Coord ownOrient, Coord otherPos) {
    // get the line of sight vector
    Coord los = otherPos - ownPos;

    // calculate azimuth angle using scalar product
    double numerator = ownOrient.x*los.x + ownOrient.y*los.y;
    double denominator = sqrt(ownOrient.x*ownOrient.x + ownOrient.y*ownOrient.y)*sqrt(los.x*los.x + los.y*los.y);
    double azimuth = acos(numerator/denominator)/M_PI*180.0;
    // check if angle is mathematically negative
    if (los.y*ownOrient.x - los.x*ownOrient.y < 0) {
        azimuth = 360 - azimuth;
    }

    // apply possible rotation and check if angle is still within [0, 360]
    azimuth -= aziRotation;
    if (azimuth < 0)
        azimuth = azimuth + (floor(-azimuth/360) + 1)*360;
    else if (azimuth > 360)
        azimuth = azimuth - floor(azimuth/360)*360;
    // now calculate elevation angle (within [-90, 90]), which is made of the
    // los elevation and the elevation of our own orientation
    double losEle = asin(los.z/los.length())/M_PI*180.0;
    double orientEle = asin(ownOrient.z/ownOrient.length())/M_PI*180.0;
    if (azimuth > 90.0 && azimuth < 270.0)
        orientEle = -orientEle;
    double elevation = losEle - orientEle;

    // apply possible rotation and check if angle is within [-90, 360]
    if (azimuth > 90.0 && azimuth < 270.0)
        elevation += eleRotation;
    else
        elevation -= eleRotation;
    if (elevation < -90.0)
        elevation = elevation + (floor(-elevation/360) + 1)*360;
    else if (elevation > 360.0)
        elevation = elevation - floor(elevation/360)*360;
    // transform angle back to interval [-90, 90], which can also influence azimuth angle
    if (elevation >= 270.0)
        elevation -= 360.0;
    else if (elevation > 90.0) {
        elevation = 180 - elevation;
        azimuth = 180.0 - azimuth;
        if (azimuth < 0.0)
            azimuth += 360.0;
    }

    // based on azimuth and elevation angle, interpolate 3D antenna gain using method
    // presented in "A Three-Dimensional Directive Antenna Pattern Interpolation Method"
    Argument pos;
    pos.setTime((elevation >= 0) ? 0.0 : 180.0);
    double g_theta1 = eleSamples->getValue(pos);
    pos.setTime(azimuth);
    double g_theta2 = aziSamples->getValue(pos);
    pos.setTime(90.0 - elevation);
    double g_phi1 = eleSamples->getValue(pos);
    pos.setTime(270.0 + elevation);
    double g_phi2 = eleSamples->getValue(pos);

    double w1 = fabs(elevation)/90.0;
    double aziTransformed = (azimuth > 180.0) ? (azimuth - 360.0) : azimuth; // transform azimuth to [-180, 180]
    double w2 = 1 - fabs(aziTransformed)/180.0;
    double g_hm = g_theta1*w1 + g_theta2*(1 - w1);
    double g_vm = g_phi1*w2 + g_phi2*(1 - w2);

    double w3 = 2*fabs(45.0 - (90.0 - fabs(elevation)) + 90.0 - fabs(90 - fabs(aziTransformed)))/180.0;
    double g_final = g_hm*w3 + g_vm*(1 - w3);

    lastAzimuth = azimuth;
    lastElevation = elevation;

    return FWMath::dBm2mW(g_final);
}

double SampledAntenna2D::getLastAzi() {
    return lastAzimuth;
}

double SampledAntenna2D::getLastEle() {
    return lastElevation;
}

