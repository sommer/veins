/*
 * SampledAntenna1D.cc
 *
 *  Created on: Jun 19, 2016
 *      Author: Alexander Brummer
 */

#include "veins/modules/phy/SampledAntenna1D.h"
#include "veins/base/utils/FWMath.h"


SampledAntenna1D::SampledAntenna1D(std::vector<double>& values, std::string offsetType, std::vector<double>& offsetParams, std::string rotationType, std::vector<double>& rotationParams, cRNG* rng){
    // use Mapping to store the samples (interpolates automatically)
    samples = MappingUtils::createMapping();
    Argument pos;
    double dist = (2*M_PI)/values.size();

    // instantiate a random number generator for sample offsets if one is specified
    cRandom* offsetGen = 0;
    if (offsetType == "uniform") {
        if (!FWMath::close(offsetParams[0], -offsetParams[1])) {
            throw cRuntimeError("SampledAntenna1D::SampledAntenna1D(): The mean of the random distribution for the samples' offsets has to be 0.");
        }
        offsetGen = new cUniform(rng, offsetParams[0], offsetParams[1]);
    } else if (offsetType == "normal") {
        if (!FWMath::close(offsetParams[0], 0)) {
            throw cRuntimeError("SampledAntenna1D::SampledAntenna1D(): The mean of the random distribution for the samples' offsets has to be 0.");
        }
        offsetGen = new cNormal(rng, offsetParams[0], offsetParams[1]);
    } else if (offsetType == "triang") {
        if (!FWMath::close((offsetParams[0] + offsetParams[1] + offsetParams[2])/3, 0)) {
            throw cRuntimeError("SampledAntenna1D::SampledAntenna1D(): The mean of the random distribution for the samples' offsets has to be 0.");
        }
        offsetGen = new cTriang(rng, offsetParams[0], offsetParams[1], offsetParams[2]);
    }

    // determine random rotation of the antenna if specified
    cRandom* rotationGen = 0;
    if (rotationType == "uniform") {
        rotationGen = new cUniform(rng, rotationParams[0], rotationParams[1]);
    } else if (rotationType == "normal") {
        rotationGen = new cNormal(rng, rotationParams[0], rotationParams[1]);
    } else if (rotationType == "triang") {
        rotationGen = new cTriang(rng, rotationParams[0], rotationParams[1], rotationParams[2]);
    }
    rotation = (rotationGen == 0) ? 0 : rotationGen->draw();
    if (rotationGen != 0)
        delete rotationGen;

    // transform to rad
    rotation *= (M_PI/180);

    // populate the mapping
    for (unsigned int i = 0; i < values.size(); i++) {
        pos.setTime(dist*i);
        double offset = 0;
        if (offsetGen != 0) {
            offset = offsetGen->draw();
            // transform to rad
            offset *= (M_PI/180);
        }
        samples->setValue(pos, (values[i] + offset));
    }
    if (offsetGen != 0)
        delete offsetGen;

    // assign the value of 0 degrees to 360 degrees as well to assure correct interpolation
    pos.setTime(0);
    double value0 = samples->getValue(pos);
    pos.setTime(2*M_PI);
    samples->setValue(pos, value0);
}

SampledAntenna1D::~SampledAntenna1D() {
    delete samples;
}

double SampledAntenna1D::getGain(Coord ownPos, Coord ownOrient, Coord otherPos) {
    // get the line of sight vector
    Coord los = otherPos - ownPos;
    // calculate angle using atan2
    double angle = atan2(los.y, los.x) - atan2(ownOrient.y, ownOrient.x);

    // apply possible rotation
    angle -= rotation;

    // make sure angle is within [0, 2*M_PI)
    angle = fmod(angle, 2*M_PI);
    if (angle < 0)
        angle += 2*M_PI;

    // return value at the calculated angle
    Argument pos(angle);
    lastAngle = angle;
    return FWMath::dBm2mW(samples->getValue(pos));
}

double SampledAntenna1D::getLastAzi() {
    return lastAngle/M_PI*180.0;;
}

double SampledAntenna1D::getLastEle() {
    return -1.0;
}
