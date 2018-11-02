//
// Copyright (C) 2018 Fabian Bronner <fabian.bronner@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// TODO: Use fixed size std::vector instead of c-style array

#include "veins/base/toolbox/Signal.h"
#include "veins/base/phyLayer/AnalogueModel.h"

namespace Veins {

Signal::Signal()
    : spectrum(0)
    , values(0)
    , numAbsoluteValues(0)
    , numRelativeValues(0)
    , numDataValues(0)
    , relativeOffset(0)
    , dataOffset(0)
    , centerFrequencyIndex(0)
    , timingUsed(false)
    , sendingStart(0)
    , duration(0)
    , propagationDelay(0)
    , analogueModelList(0)
    , numAnalogueModelsApplied(0)
    , senderPos(0, 0)
    , receiverPos(0, 0)
    , bitrate(0)
    , senderModuleID(-1)
    , senderFromGateID(-1)
    , receiverModuleID(-1)
    , receiverToGateID(-1)
{
    //
}

Signal::Signal(const Signal& other)
    : spectrum(other.spectrum)
    , numAbsoluteValues(other.numAbsoluteValues)
    , numRelativeValues(other.numRelativeValues)
    , numDataValues(other.numDataValues)
    , relativeOffset(other.relativeOffset)
    , dataOffset(other.dataOffset)
    , centerFrequencyIndex(other.centerFrequencyIndex)
    , timingUsed(other.timingUsed)
    , sendingStart(other.sendingStart)
    , duration(other.duration)
    , propagationDelay(other.propagationDelay)
    , analogueModelList(other.analogueModelList)
    , numAnalogueModelsApplied(other.numAnalogueModelsApplied)
    , senderPos(other.senderPos)
    , receiverPos(other.receiverPos)
    , bitrate(other.bitrate)
    , senderModuleID(other.senderModuleID)
    , senderFromGateID(other.senderFromGateID)
    , receiverModuleID(other.receiverModuleID)
    , receiverToGateID(other.receiverToGateID)
{
    values = new double[spectrum->getNumFreqs()]{0};
    std::copy(other.getAbsoluteValues(), other.getAbsoluteValues() + numAbsoluteValues, values);
}

Signal::Signal(SpectrumPtr spec)
    : spectrum(spec)
    , numAbsoluteValues(spec->getNumFreqs())
    , numRelativeValues(0)
    , numDataValues(0)
    , relativeOffset(0)
    , dataOffset(0)
    , centerFrequencyIndex(0)
    , timingUsed(false)
    , sendingStart(0)
    , duration(0)
    , propagationDelay(0)
    , analogueModelList(0)
    , numAnalogueModelsApplied(0)
    , senderPos(0, 0)
    , receiverPos(0, 0)
    , bitrate(0)
    , senderModuleID(-1)
    , senderFromGateID(-1)
    , receiverModuleID(-1)
    , receiverToGateID(-1)
{
    values = new double[numAbsoluteValues]{0};
}

Signal::Signal(SpectrumPtr spec, simtime_t start, simtime_t dur)
    : spectrum(spec)
    , numAbsoluteValues(spec->getNumFreqs())
    , numRelativeValues(0)
    , numDataValues(0)
    , relativeOffset(0)
    , dataOffset(0)
    , centerFrequencyIndex(0)
    , timingUsed(true)
    , sendingStart(start)
    , duration(dur)
    , propagationDelay(0)
    , analogueModelList(0)
    , numAnalogueModelsApplied(0)
    , senderPos(0, 0)
    , receiverPos(0, 0)
    , bitrate(0)
    , senderModuleID(-1)
    , senderFromGateID(-1)
    , receiverModuleID(-1)
    , receiverToGateID(-1)
{
    values = new double[numAbsoluteValues]{0};
}

Signal::~Signal()
{
    if (values) delete[] values;
}

double& Signal::operator[](size_t index)
{
#ifndef PREFER_VECTORIZATION
    includeAbsoluteIndex(index);
#endif

    return values[index];
}

const double& Signal::operator[](size_t index) const
{
    return values[index];
}

double& Signal::operator()(double freq)
{
    size_t index = spectrum->indexOf(freq);

    includeAbsoluteIndex(index);

    return values[index];
}

double Signal::operator()(double freq) const
{
    size_t index = spectrum->indexOf(freq);

    return values[index];
}

SpectrumPtr Signal::getSpectrum() const
{
    return spectrum;
}

double Signal::getAbsoluteFreqAt(size_t freqIndex) const
{
    return spectrum->freqAt(freqIndex);
}

double Signal::getRelativeFreqAt(size_t freqIndex) const
{
    return spectrum->freqAt(freqIndex + relativeOffset);
}

double Signal::getDataFreqAt(size_t freqIndex) const
{
    return spectrum->freqAt(freqIndex + dataOffset);
}

size_t Signal::getRelativeStart() const
{
    return relativeOffset;
}

size_t Signal::getDataStart() const
{
    return dataOffset;
}

size_t Signal::getRelativeEnd() const
{
    return relativeOffset + numRelativeValues;
}

size_t Signal::getDataEnd() const
{
    return dataOffset + numDataValues;
}

void Signal::setDataStart(size_t index)
{
    dataOffset = index;
}

void Signal::setDataEnd(size_t index)
{
    numDataValues = 1 + index - dataOffset;
}

void Signal::setDataNumValues(size_t num)
{
    numDataValues = num;
}

double* Signal::getAbsoluteValues() const
{
    return values;
}

double* Signal::getRelativeValues() const
{
    return values + relativeOffset;
}

double* Signal::getDataValues() const
{
    return values + dataOffset;
}

size_t Signal::getNumAbsoluteValues() const
{
    return numAbsoluteValues;
}

size_t Signal::getNumRelativeValues() const
{
    return numRelativeValues;
}

size_t Signal::getNumDataValues() const
{
    return numDataValues;
}

size_t Signal::getRelativeOffset() const
{
    return relativeOffset;
}

size_t Signal::getDataOffset() const
{
    return dataOffset;
}

void Signal::setAbsolute(size_t index, double value)
{
    includeAbsoluteIndex(index);

    values[index] = value;
}

void Signal::setRelative(size_t index, double value)
{
    values[index + relativeOffset] = value;
}

void Signal::setData(size_t index, double value)
{
    values[index + dataOffset] = value;
}

void Signal::setAtFreq(double freq, double value)
{
    size_t index = spectrum->indexOf(freq);

    includeAbsoluteIndex(index);

    values[index] = value;
}

double Signal::getAbsolute(size_t index) const
{
    return values[index];
}

double Signal::getRelative(size_t index) const
{
    return values[index + relativeOffset];
}

double Signal::getData(size_t index) const
{
    return values[index + dataOffset];
}

double Signal::getAtFreq(double freq) const
{
    size_t index = spectrum->indexOf(freq);
    return values[index];
}

void Signal::setCenterFrequencyIndex(size_t index)
{
    centerFrequencyIndex = index;
}

size_t Signal::getCenterFrequencyIndex() const
{
    return centerFrequencyIndex;
}

double Signal::getAtCenterFrequency() const
{
    return values[centerFrequencyIndex];
}

bool Signal::greaterAtCenterFrequency(double threshold)
{
    if (values[centerFrequencyIndex] < threshold) return false;

    uint16_t maxAnalogueModels = analogueModelList->size();

    while (numAnalogueModelsApplied < maxAnalogueModels) {
        // Apply filter here
        (*analogueModelList)[numAnalogueModelsApplied]->filterSignal(this, senderPos, receiverPos);
        numAnalogueModelsApplied++;

        if (values[centerFrequencyIndex] < threshold) return false;
    }
    return true;
}

bool Signal::smallerAtCenterFrequency(double threshold)
{
    if (values[centerFrequencyIndex] < threshold) return true;

    uint16_t maxAnalogueModels = analogueModelList->size();

    while (numAnalogueModelsApplied < maxAnalogueModels) {
        // Apply filter here
        (*analogueModelList)[numAnalogueModelsApplied]->filterSignal(this, senderPos, receiverPos);
        numAnalogueModelsApplied++;

        if (values[centerFrequencyIndex] < threshold) return true;
    }
    return false;
}

uint16_t Signal::getNumAnalogueModelsApplied() const
{
    return numAnalogueModelsApplied;
}

void Signal::applyAnalogueModel(uint16_t index)
{
    uint16_t maxAnalogueModels = analogueModelList->size();

    if (index >= maxAnalogueModels || index < numAnalogueModelsApplied) return;

    (*analogueModelList)[index]->filterSignal(this, senderPos, receiverPos);
    numAnalogueModelsApplied++;
}

void Signal::applyAllAnalogueModels()
{
    uint16_t maxAnalogueModels = analogueModelList->size();
    while (numAnalogueModelsApplied < maxAnalogueModels) {
        (*analogueModelList)[numAnalogueModelsApplied]->filterSignal(this, senderPos, receiverPos);

        numAnalogueModelsApplied++;
    }
}

void Signal::setAnalogueModelList(AnalogueModelList* list)
{
    analogueModelList = list;
}

void Signal::setSenderPos(Coord pos)
{
    senderPos = pos;
}

void Signal::setReceiverPos(Coord pos)
{
    receiverPos = pos;
}

AnalogueModelList* Signal::getAnalogueModelList() const
{
    return analogueModelList;
}

size_t Signal::getNumAnalogueModels() const
{
    return analogueModelList->size();
}

Coord Signal::getSenderPos() const
{
    return senderPos;
}

Coord Signal::getReceiverPos() const
{
    return receiverPos;
}

void Signal::setSendingStart(simtime_t start)
{
    sendingStart = start;
    timingUsed = true;
}

void Signal::setDuration(simtime_t dur)
{
    duration = dur;
    timingUsed = true;
}

void Signal::setPropagationDelay(simtime_t_cref delay)
{
    propagationDelay = delay;
}

void Signal::setTiming(simtime_t start, simtime_t dur)
{
    sendingStart = start;
    duration = dur;
    timingUsed = true;
}

simtime_t_cref Signal::getSendingStart() const
{
    return sendingStart;
}

simtime_t Signal::getSendingEnd() const
{
    return sendingStart + duration;
}

simtime_t Signal::getReceptionStart() const
{
    return sendingStart + propagationDelay;
}

simtime_t Signal::getReceptionEnd() const
{
    return sendingStart + propagationDelay + duration;
}

simtime_t_cref Signal::getDuration() const
{
    return duration;
}

simtime_t_cref Signal::getPropagationDelay() const
{
    return propagationDelay;
}

bool Signal::hasTiming() const
{
    return timingUsed;
}

double Signal::getRelativeMin() const
{
    return *(std::min_element(values + relativeOffset, values + relativeOffset + numRelativeValues));
}

double Signal::getDataMin() const
{
    return *(std::min_element(values + dataOffset, values + dataOffset + numDataValues));
}

double Signal::getMinInRange(size_t freqIndexLow, size_t freqIndexHigh) const
{
    return *(std::min_element(values + freqIndexLow, values + freqIndexHigh));
}

double Signal::getRelativeMax() const
{
    return *(std::max_element(values + relativeOffset, values + relativeOffset + numRelativeValues));
}

double Signal::getDataMax() const
{
    return *(std::max_element(values + dataOffset, values + dataOffset + numDataValues));
}

double Signal::getMaxInRange(size_t freqIndexLow, size_t freqIndexHigh) const
{
    return *(std::max_element(values + freqIndexLow, values + freqIndexHigh));
}

void Signal::print() const
{
    if (timingUsed) std::cout << "Range: " << getReceptionStart() << " - " << getReceptionEnd() << " (" << duration << ")" << std::endl;

    for (uint16_t i = getRelativeStart(); i < getRelativeEnd(); i++) {
        std::cout << spectrum->freqAt(i) << ":\t " << values[i] << std::endl;
    }

    std::cout << "-----------------------------------------------" << std::endl;
}

void Signal::printAbsolute() const
{
    std::cout << "-----------------------------------------------" << std::endl;

    if (timingUsed) std::cout << "Range: " << getReceptionStart() << " - " << getReceptionEnd() << " (" << duration << ")" << std::endl;

    for (uint16_t i = 0; i < numAbsoluteValues; i++) {
        std::cout << spectrum->freqAt(i) << ":\t " << values[i] << std::endl;
    }
}

Signal& Signal::operator=(const double value)
{
    for (size_t i = 0; i < numAbsoluteValues; i++) {
        values[i] = value;
    }
    return *this;
}

Signal& Signal::operator=(const Signal& other)
{
    if (this == &other) return *this;

    senderModuleID = other.senderModuleID;
    senderFromGateID = other.senderFromGateID;
    receiverModuleID = other.receiverModuleID;
    receiverToGateID = other.receiverToGateID;

    spectrum = other.getSpectrum();

    relativeOffset = other.getRelativeOffset();
    dataOffset = other.getDataOffset();

    centerFrequencyIndex = other.getCenterFrequencyIndex();

    numAbsoluteValues = other.getNumAbsoluteValues();
    numRelativeValues = other.getNumRelativeValues();
    numDataValues = other.getNumDataValues();

    // TODO: Check if new spectrum has changed and new one is larger
    if (values) delete[] values;
    values = new double[spectrum->getNumFreqs()]{0};

    std::copy(other.getAbsoluteValues(), other.getAbsoluteValues() + numAbsoluteValues, values);

    analogueModelList = other.getAnalogueModelList();
    numAnalogueModelsApplied = other.getNumAnalogueModelsApplied();
    senderPos = other.getSenderPos();
    receiverPos = other.getReceiverPos();

    timingUsed = other.hasTiming();
    sendingStart = other.getSendingStart();
    duration = other.getDuration();
    propagationDelay = other.getPropagationDelay();

    bitrate = other.getBitrate();

    return *this;
}

Signal& Signal::operator+=(const Signal& other)
{
    assert(this->getSpectrum() == other.getSpectrum());

    size_t lowIndex = std::min(this->getRelativeStart(), other.getRelativeStart());
    size_t highIndex = std::max(this->getRelativeEnd(), other.getRelativeEnd());

    for (size_t i = lowIndex; i < highIndex; i++) {
        (*this)[i] += other[i];
    }
    return *this;
}

Signal& Signal::operator+=(const double value)
{
    for (size_t i = 0; i < numAbsoluteValues; i++) {
        (*this)[i] += value;
    }
    return *this;
}

Signal& Signal::operator-=(const Signal& other)
{
    assert(this->getSpectrum() == other.getSpectrum());

    size_t lowIndex = std::min(this->getRelativeStart(), other.getRelativeStart());
    size_t highIndex = std::max(this->getRelativeEnd(), other.getRelativeEnd());

    for (size_t i = lowIndex; i < highIndex; i++) {
        (*this)[i] -= other[i];
    }
    return *this;
}

Signal& Signal::operator-=(const double value)
{
    for (size_t i = 0; i < numAbsoluteValues; i++) {
        (*this)[i] -= value;
    }
    return *this;
}

Signal& Signal::operator*=(const Signal& other)
{
    assert(this->getSpectrum() == other.getSpectrum());

    size_t lowIndex = std::min(this->getRelativeStart(), other.getRelativeStart());
    size_t highIndex = std::max(this->getRelativeEnd(), other.getRelativeEnd());

    for (size_t i = lowIndex; i < highIndex; i++) {
        (*this)[i] *= other[i];
    }
    return *this;
}

Signal& Signal::operator*=(const double value)
{
    for (size_t i = 0; i < numAbsoluteValues; i++) {
        (*this)[i] *= value;
    }
    return *this;
}

Signal& Signal::operator/=(const Signal& other)
{
    assert(this->getSpectrum() == other.getSpectrum());

    size_t lowIndex = std::min(this->getRelativeStart(), other.getRelativeStart());
    size_t highIndex = std::max(this->getRelativeEnd(), other.getRelativeEnd());

    for (size_t i = lowIndex; i < highIndex; i++) {
        (*this)[i] /= other[i];
    }
    return *this;
}

Signal& Signal::operator/=(const double value)
{
    for (size_t i = 0; i < numAbsoluteValues; i++) {
        (*this)[i] /= value;
    }
    return *this;
}

Signal& Signal::operator<<=(uint16_t n)
{
    if (n > relativeOffset || n == 0) return *this;

    size_t i = relativeOffset;
    while (i < relativeOffset + numRelativeValues) {
        values[i - n] = values[i];
        values[i] = 0; // Fill with 0s
        i++;
    }

    relativeOffset -= n;
    dataOffset -= n;

    return *this;
}

Signal& Signal::operator>>=(uint16_t n)
{
    if (n > numAbsoluteValues - (relativeOffset + numRelativeValues) || n == 0) return *this;

    size_t i = relativeOffset + numRelativeValues;
    while (i > 0) {
        i--;
        values[i + n] = values[i];
        values[i] = 0; // Fill with 0s
    }

    relativeOffset += n;
    dataOffset += n;

    return *this;
}

Signal operator+(const Signal& lhs, const Signal& rhs)
{
    assert(lhs.getSpectrum() == rhs.getSpectrum());

    Signal temp(lhs.getSpectrum());

#ifndef PREFER_VECTORIZATION
    // Default version
    size_t lowIndex = std::min(lhs.getRelativeStart(), rhs.getRelativeStart());
    size_t highIndex = std::max(lhs.getRelativeEnd(), rhs.getRelativeEnd());

    for (size_t i = lowIndex; i < highIndex; i++) {
        temp[i] = lhs[i] + rhs[i];
    }
#else
    // Optimized for vectorizing
    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs[i] + rhs[i];
    }
#endif

    return temp;
}

Signal operator+(const Signal& lhs, double rhs)
{
    Signal temp(lhs.getSpectrum());

    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs[i] + rhs;
    }

    return temp;
}

Signal operator+(double lhs, const Signal& rhs)
{
    Signal temp(rhs.getSpectrum());

    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs + rhs[i];
    }

    return temp;
}

Signal operator-(const Signal& lhs, const Signal& rhs)
{
    assert(lhs.getSpectrum() == rhs.getSpectrum());

    Signal temp(lhs.getSpectrum());

#ifndef PREFER_VECTORIZATION
    // Default version
    size_t lowIndex = std::min(lhs.getRelativeStart(), rhs.getRelativeStart());
    size_t highIndex = std::max(lhs.getRelativeEnd(), rhs.getRelativeEnd());

    for (size_t i = lowIndex; i < highIndex; i++) {
        temp[i] = lhs[i] - rhs[i];
    }
#else
    // Optimized for vectorizing
    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs[i] - rhs[i];
    }
#endif

    return temp;
}

Signal operator-(const Signal& lhs, double rhs)
{
    Signal temp(lhs.getSpectrum());

    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs[i] - rhs;
    }

    return temp;
}

Signal operator-(double lhs, const Signal& rhs)
{
    Signal temp(rhs.getSpectrum());

    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs - rhs[i];
    }

    return temp;
}

Signal operator*(const Signal& lhs, const Signal& rhs)
{
    assert(lhs.getSpectrum() == rhs.getSpectrum());

    Signal temp(lhs.getSpectrum());

#ifndef PREFER_VECTORIZATION
    // Default version
    size_t lowIndex = std::min(lhs.getRelativeStart(), rhs.getRelativeStart());
    size_t highIndex = std::max(lhs.getRelativeEnd(), rhs.getRelativeEnd());

    for (size_t i = lowIndex; i < highIndex; i++) {
        temp[i] = lhs[i] * rhs[i];
    }
#else
    // Optimized for vectorizing
    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs[i] * rhs[i];
    }
#endif

    return temp;
}

Signal operator*(const Signal& lhs, double rhs)
{
    Signal temp(lhs.getSpectrum());

    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs[i] * rhs;
    }

    return temp;
}

Signal operator*(double lhs, const Signal& rhs)
{
    Signal temp(rhs.getSpectrum());

    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs * rhs[i];
    }

    return temp;
}

Signal operator/(const Signal& lhs, const Signal& rhs)
{
    assert(lhs.getSpectrum() == rhs.getSpectrum());

    Signal temp(lhs.getSpectrum());

#ifndef PREFER_VECTORIZATION
    // Default version
    size_t lowIndex = std::min(lhs.getRelativeStart(), rhs.getRelativeStart());
    size_t highIndex = std::max(lhs.getRelativeEnd(), rhs.getRelativeEnd());

    for (size_t i = lowIndex; i < highIndex; i++) {
        temp[i] = lhs[i] / rhs[i];
    }
#else
    // Optimized for vectorizing
    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs[i] / rhs[i];
    }
#endif

    return temp;
}

Signal operator/(const Signal& lhs, double rhs)
{
    Signal temp(lhs.getSpectrum());

    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs[i] / rhs;
    }

    return temp;
}

Signal operator/(double lhs, const Signal& rhs)
{
    Signal temp(rhs.getSpectrum());

    for (size_t i = 0; i < temp.numAbsoluteValues; i++) {
        temp[i] = lhs / rhs[i];
    }

    return temp;
}

Signal Signal::operator<<(uint16_t n)
{
    Signal temp = *(this);
    return temp <<= n;
}

Signal Signal::operator>>(uint16_t n)
{
    Signal temp = *(this);
    return temp >>= n;
}

void Signal::toFile(std::string path) const
{
    std::fstream file;
    file.open(path.c_str(), std::ios::out | std::ios::app);

    if (!file.good()) return;

    file << "signal:" << getReceptionStart() << "," << duration << ":";

    size_t last = getRelativeEnd();

    for (size_t i = getRelativeStart(); i < last; i++) {
        file << values[i];
        if (i != last - 1) file << ",";
    }
    file << std::endl;

    file.close();
}

uint64_t Signal::getBitrate() const
{
    return bitrate;
}

void Signal::setBitrate(uint64_t rate)
{
    bitrate = rate;
}

void Signal::includeAbsoluteIndex(size_t freqIndex)
{
    // No value so far
    if (numRelativeValues == 0) {
        numRelativeValues = 1;
        relativeOffset = freqIndex;
    }

    // value right outside of values
    if (freqIndex >= relativeOffset + numRelativeValues) {
        numRelativeValues += freqIndex + 1 - (relativeOffset + numRelativeValues);
    }
    // value left outside of values
    else if (freqIndex < relativeOffset) {
        numRelativeValues += relativeOffset - freqIndex;
        relativeOffset = freqIndex;
    }
}

cModule* Signal::getReceptionModule() const
{
    return receiverModuleID < 0 ? NULL : getSimulation()->getModule(receiverModuleID);
}

cGate* Signal::getReceptionGate() const
{
    if (receiverToGateID < 0) return NULL;

    cModule* const mod = getReceptionModule();
    return !mod ? NULL : mod->gate(receiverToGateID);
}

cModule* Signal::getSendingModule() const
{
    return senderModuleID < 0 ? NULL : getSimulation()->getModule(senderModuleID);
}

cGate* Signal::getSendingGate() const
{
    if (senderFromGateID < 0) return NULL;

    cModule* const mod = getSendingModule();
    return !mod ? NULL : mod->gate(senderFromGateID);
}

void Signal::setReceptionSenderInfo(const cMessage* const pMsg)
{
    if (!pMsg) return;

    assert(senderModuleID < 0);

    senderModuleID = pMsg->getSenderModuleId();
    senderFromGateID = pMsg->getSenderGateId();

    receiverModuleID = pMsg->getArrivalModuleId();
    receiverToGateID = pMsg->getArrivalGateId();
}

void Signal::addAttenuation(uint16_t freqIndex, double factor)
{
    values[freqIndex] *= factor;
}

void Signal::addUniformAttenuation(double factor)
{
    for (uint16_t i = relativeOffset; i < relativeOffset + numRelativeValues; i++) {
        values[i] *= factor;
    }
}

/***********************/

simtime_t calculateStart(const Signal& lhs, const Signal& rhs)
{
    return std::max(lhs.sendingStart, rhs.sendingStart);
}

simtime_t calculateDuration(const Signal& lhs, const Signal& rhs)
{
    simtime_t temp = std::min(lhs.sendingStart + lhs.duration, rhs.sendingStart + rhs.duration) - std::max(lhs.sendingStart, rhs.sendingStart);
    return (temp > 0) ? temp : 0;
}

} // namespace Veins
