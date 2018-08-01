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

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include "veins/base/utils/Coord.h"
#include "veins/base/utils/MiXiMDefs.h"

#include "veins/base/toolbox/Spectrum.h"

namespace Veins {

// Forward declaration
class AnalogueModel;
typedef std::vector<AnalogueModel*> AnalogueModelList;

class Signal {
public:
    Signal();
    Signal(const Signal& other);
    Signal(SpectrumPtr spec);
    Signal(SpectrumPtr spec, simtime_t start, simtime_t duration);
    ~Signal();

    double& operator[](size_t index);
    const double& operator[](size_t index) const;
    double& operator()(double freq);
    double operator()(double freq) const;

    SpectrumPtr getSpectrum() const;

    double getAbsoluteFreqAt(size_t freqIndex) const;
    double getRelativeFreqAt(size_t freqIndex) const;
    double getDataFreqAt(size_t freqIndex) const;

    size_t getRelativeStart() const;
    size_t getDataStart() const;

    size_t getRelativeEnd() const;
    size_t getDataEnd() const;

    void setDataStart(size_t index);
    void setDataEnd(size_t index);
    void setDataNumValues(size_t num);

    double* getAbsoluteValues() const;
    double* getRelativeValues() const;
    double* getDataValues() const;

    size_t getNumAbsoluteValues() const;
    size_t getNumRelativeValues() const;
    size_t getNumDataValues() const;

    size_t getRelativeOffset() const;
    size_t getDataOffset() const;

    void setAbsolute(size_t index, double value);
    void setRelative(size_t index, double value);
    void setData(size_t index, double value);
    void setAtFreq(double freq, double value);

    double getAbsolute(size_t index) const;
    double getRelative(size_t index) const;
    double getData(size_t index) const;
    double getAtFreq(double freq) const;

    void setCenterFrequencyIndex(size_t index);
    size_t getCenterFrequencyIndex() const;
    double getAtCenterFrequency() const;

    bool greaterAtCenterFrequency(double threshold);
    bool smallerAtCenterFrequency(double threshold);

    uint16_t getNumAnalogueModelsApplied() const;
    void applyAnalogueModel(uint16_t index);
    void applyAllAnalogueModels();

    void setAnalogueModelList(AnalogueModelList* list);
    void setSenderPos(Coord pos);
    void setReceiverPos(Coord pos);

    AnalogueModelList* getAnalogueModelList() const;
    size_t getNumAnalogueModels() const;
    Coord getSenderPos() const;
    Coord getReceiverPos() const;

    void setSendingStart(simtime_t start);
    void setDuration(simtime_t duration);
    void setPropagationDelay(simtime_t_cref delay);
    void setTiming(simtime_t start, simtime_t duration);

    simtime_t_cref getSendingStart() const;
    simtime_t getSendingEnd() const;
    simtime_t getReceptionStart() const;
    simtime_t getReceptionEnd() const;
    simtime_t_cref getDuration() const;
    simtime_t_cref getPropagationDelay() const;

    bool hasTiming() const;

    double getRelativeMin() const;
    double getDataMin() const;
    double getMinInRange(size_t freqIndexLow, size_t freqIndexHigh) const;

    double getRelativeMax() const;
    double getDataMax() const;
    double getMaxInRange(size_t freqIndexLow, size_t freqIndexHigh) const;

    void print() const;
    void printAbsolute() const;
    void toFile(std::string path) const;

    Signal& operator=(const double value);
    Signal& operator=(const Signal& other);

    Signal& operator+=(const Signal& other);
    Signal& operator+=(const double value);

    Signal& operator-=(const Signal& other);
    Signal& operator-=(const double value);

    Signal& operator*=(const Signal& other);
    Signal& operator*=(const double value);

    Signal& operator/=(const Signal& other);
    Signal& operator/=(const double value);

    Signal& operator<<=(uint16_t n);
    Signal& operator>>=(uint16_t n);

    friend Signal operator+(const Signal& lhs, const Signal& rhs);
    friend Signal operator+(const Signal& lhs, double rhs);
    friend Signal operator+(double lhs, const Signal& rhs);

    friend Signal operator-(const Signal& lhs, const Signal& rhs);
    friend Signal operator-(const Signal& lhs, double rhs);
    friend Signal operator-(double lhs, const Signal& rhs);

    friend Signal operator*(const Signal& lhs, const Signal& rhs);
    friend Signal operator*(const Signal& lhs, double rhs);
    friend Signal operator*(double lhs, const Signal& rhs);

    friend Signal operator/(const Signal& lhs, const Signal& rhs);
    friend Signal operator/(const Signal& lhs, double rhs);
    friend Signal operator/(double lhs, const Signal& rhs);

    Signal operator<<(uint16_t n);
    Signal operator>>(uint16_t n);

    friend inline simtime_t calculateStart(const Signal& lhs, const Signal& rhs);
    friend inline simtime_t calculateDuration(const Signal& lhs, const Signal& rhs);

    uint64_t getBitrate() const;
    void setBitrate(uint64_t rate);

    /**
     * Returns a pointer to the arrival module. It returns NULL if the signal
     * has not been sent/received yet, or if the module was deleted
     * in the meantime.
     */
    cModule* getReceptionModule() const;

    /**
     * Returns pointers to the gate from which the signal was sent and
     * on which gate it arrived. A NULL pointer is returned
     * for new (unsent) signal.
     */
    cGate* getReceptionGate() const;

    /**
     * Returns a pointer to the sender module. It returns NULL if the signal
     * has not been sent/received yet, or if the sender module got deleted
     * in the meantime.
     */
    cModule* getSendingModule() const;

    /**
     * Returns pointers to the gate from which the signal was sent and
     * on which gate it arrived. A NULL pointer is returned
     * for new (unsent) signal.
     */
    cGate* getSendingGate() const;

    /** @brief Saves the arrival sender module information form message. */
    void setReceptionSenderInfo(const cMessage* const pMsg);

    void addAttenuation(uint16_t freqIndex, double factor);
    void addUniformAttenuation(double factor);

private:
    void includeAbsoluteIndex(size_t freqIndex);

    SpectrumPtr spectrum;

    double* values;

    size_t numAbsoluteValues;
    size_t numRelativeValues;
    size_t numDataValues;

    size_t relativeOffset;
    size_t dataOffset;

    size_t centerFrequencyIndex;

    bool timingUsed;
    /** @brief The start of the signal transmission at the sender module.*/
    simtime_t sendingStart;
    /** @brief The duration of the signal transmission.*/
    simtime_t duration;
    /** @brief The propagation delay of the transmission. */
    simtime_t propagationDelay;

    AnalogueModelList* analogueModelList;
    uint16_t numAnalogueModelsApplied;
    Coord senderPos;
    Coord receiverPos;

    /** @brief Stores the function which describes the bitrate of the signal*/
    uint64_t bitrate;

protected:
    /** @brief Sender module id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
    int senderModuleID;
    /** @brief Sender gate id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
    int senderFromGateID;
    /** @brief Receiver module id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
    int receiverModuleID;
    /** @brief Receiver gate id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
    int receiverToGateID;
};

} // namespace Veins

#endif /* SIGNAL_H_ */
