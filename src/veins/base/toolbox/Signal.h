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

#pragma once

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"
#include "veins/base/toolbox/Spectrum.h"
#include "veins/base/phyLayer/AnalogueModel.h"

namespace Veins {

class Signal {
public:
    Signal() = default;
    Signal(const Signal& other);
    explicit Signal(Spectrum spec);
    Signal(Spectrum spec, simtime_t start, simtime_t duration);
    ~Signal();

    double& operator[](size_t index);
    const double& operator[](size_t index) const;
    double& operator()(double freq);
    double operator()(double freq) const;

    Spectrum getSpectrum() const;

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

    friend std::ostream& operator<<(std::ostream& os, const Signal& s);

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

    Spectrum spectrum;

    double* values = nullptr;

    size_t numAbsoluteValues = 0;
    size_t numRelativeValues = 0;
    size_t numDataValues = 0;

    size_t relativeOffset = 0;
    size_t dataOffset = 0;

    size_t centerFrequencyIndex = 0;

    bool timingUsed = false;
    /** @brief The start of the signal transmission at the sender module.*/
    simtime_t sendingStart = 0;
    /** @brief The duration of the signal transmission.*/
    simtime_t duration = 0;
    /** @brief The propagation delay of the transmission. */
    simtime_t propagationDelay = 0;

    AnalogueModelList* analogueModelList = nullptr;
    uint16_t numAnalogueModelsApplied = 0;
    Coord senderPos = Coord(0, 0);
    Coord receiverPos = Coord(0, 0);

    /** @brief Stores the function which describes the bitrate of the signal*/
    uint64_t bitrate = 0;

protected:
    /** @brief Sender module id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
    int senderModuleID = -1;
    /** @brief Sender gate id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
    int senderFromGateID = -1;
    /** @brief Receiver module id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
    int receiverModuleID = -1;
    /** @brief Receiver gate id, additional definition here because BasePhyLayer will do some selfMessages with AirFrame. */
    int receiverToGateID = -1;
};

Signal operator+(const Signal& lhs, const Signal& rhs);
Signal operator+(const Signal& lhs, double rhs);
Signal operator+(double lhs, const Signal& rhs);

Signal operator-(const Signal& lhs, const Signal& rhs);
Signal operator-(const Signal& lhs, double rhs);
Signal operator-(double lhs, const Signal& rhs);

Signal operator*(const Signal& lhs, const Signal& rhs);
Signal operator*(const Signal& lhs, double rhs);
Signal operator*(double lhs, const Signal& rhs);

Signal operator/(const Signal& lhs, const Signal& rhs);
Signal operator/(const Signal& lhs, double rhs);
Signal operator/(double lhs, const Signal& rhs);

} // namespace Veins
