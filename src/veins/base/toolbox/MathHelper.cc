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

#include "veins/base/toolbox/MathHelper.h"

#include "veins/base/messages/AirFrame_m.h"

using namespace Veins;
using Veins::AirFrame;

namespace {
bool compareByTime(const SignalChange& lhs, const SignalChange& rhs)
{
    return (lhs.time < rhs.time);
}
} // namespace

double MathHelper::getGlobalMax(simtime_t start, simtime_t end, const AirFrameVector& airFrames)
{
    if (airFrames.size() == 0) return 0;

    std::vector<SignalChange> changes;
    calculateChanges(start, end, airFrames, &changes);

    // Works fine so far, as there is at least one AirFrame
    if (changes.size() == 0) return 0;

    SpectrumPtr spectrum = airFrames.front()->getSignal().getSpectrum();

    Signal interference = Signal(spectrum);

    // Calculate interference at beginning
    std::vector<SignalChange>::iterator it = changes.begin();

    while (it != changes.end()) {
        if (it->time > start) break;

        interference += *(it->signal);

        it++;
    }

    // Make sure to calculate at beginning
    double maximum = interference.getRelativeMax();

    // Calculate all chunks
    while (it != changes.end()) {
        if (it->type == SIGNAL_STARTS) {
            interference += *(it->signal);
        }
        else if (it->type == SIGNAL_ENDS) {
            interference -= *(it->signal);
        }

        double tmpMax = interference.getRelativeMax();
        if (tmpMax > maximum) maximum = tmpMax;

        it++;
    }

    return maximum;
}

double MathHelper::getGlobalMin(simtime_t start, simtime_t end, const AirFrameVector& airFrames)
{
    if (airFrames.size() == 0) return 0;

    std::vector<SignalChange> changes;
    calculateChanges(start, end, airFrames, &changes);

    // Works fine so far, as there is at least one AirFrame
    if (changes.size() == 0) return 0;

    SpectrumPtr spectrum = airFrames.front()->getSignal().getSpectrum();

    Signal interference = Signal(spectrum);

    // Calculate interference at beginning
    std::vector<SignalChange>::iterator it = changes.begin();

    while (it != changes.end()) {
        if (it->time > start) break;

        interference += *(it->signal);

        it++;
    }

    // Make sure to calculate at beginning
    double minimum = interference.getRelativeMin();

    // Calculate all chunks
    while (it != changes.end()) {
        if (it->type == SIGNAL_STARTS) {
            interference += *(it->signal);
        }
        else if (it->type == SIGNAL_ENDS) {
            interference -= *(it->signal);
        }

        double tmpMin = interference.getRelativeMin();
        if (tmpMin < minimum) minimum = tmpMin;

        it++;
    }

    return minimum;
}

// TODO: Check influence of exclude -> suspicion that parameter has no influence, although it is not NULL
double MathHelper::getMinAtFreqIndex(simtime_t start, simtime_t end, const AirFrameVector& airFrames, size_t freqIndex, AirFrame* exclude)
{
    if (airFrames.size() == 0) return 0;

    std::vector<SignalChange> changes;
    calculateChanges(start, end, airFrames, &changes, exclude);

    // Works fine so far, as there is at least one AirFrame
    if (changes.size() == 0) return 0;

    // Calculate interference at beginning
    double interference = 0;

    std::vector<SignalChange>::iterator it = changes.begin();

    while (it != changes.end()) {
        if (it->time > start) break;

        interference += (*(it->signal))[freqIndex];

        it++;
    }

    // Make sure to calculate at beginning
    double minimum = interference;

    // Calculate all chunks
    while (it != changes.end()) {
        if (it->type == SIGNAL_STARTS) {
            interference += (*(it->signal))[freqIndex];
        }
        else if (it->type == SIGNAL_ENDS) {
            interference -= (*(it->signal))[freqIndex];
        }

        if (interference < minimum) minimum = interference;

        it++;
    }

    return minimum;
}

/*
 * Assume everything is at maximum value at the beginning (like no AMs applied)
 * Then apply AMs one after the other
 * If value is already below threshold before all AMs are applied, abort and return true
 */
bool MathHelper::smallerAtFreqIndex(simtime_t start, simtime_t end, AirFrameVector& airFrames, size_t freqIndex, double threshold, AirFrame* exlude)
{
    // Assume that threshold is >0 -> if there is no other AirFrame, this is 0 -> return true (0 < threshold)
    if (airFrames.size() == 0) return true;

    std::vector<SignalChange> changes;
    calculateChanges(start, end, airFrames, &changes);

    // Works fine so far, as there is at least one AirFrame
    if (changes.size() == 0) return true;

    // Assumption: There is no AM with an attenuation > 1
    uint16_t maxAnalogueModels = airFrames.front()->getSignal().getNumAnalogueModels();
    for (uint16_t i = 0; i <= maxAnalogueModels; i++) {
        std::vector<SignalChange>::iterator it = changes.begin();

        double channelLoad = 0;

        while (it != changes.end()) {
            if (it->time > start) break;

            channelLoad += (*(it->signal))[freqIndex];

            it++;
        }

        double minimum = channelLoad;

        // Calculate all chunks
        while (it != changes.end()) {
            if (it->type == SIGNAL_STARTS) {
                channelLoad += (*(it->signal))[freqIndex];
            }
            else if (it->type == SIGNAL_ENDS) {
                channelLoad -= (*(it->signal))[freqIndex];
            }

            if (channelLoad < minimum) minimum = channelLoad;

            it++;
        }

        // Check if we are already below threshold with the current AMs
        if (minimum < threshold) return true;

        // Apply filters for next iteration here
        if (i == maxAnalogueModels) break;

        for (it = changes.begin(); it != changes.end(); ++it) {
            it->signal->applyAnalogueModel(i);
        }
    }

    return false;
}

double MathHelper::getMinSINR(simtime_t start, simtime_t end, AirFrame* signalFrame, AirFrameVector& interfererFrames, double noise)
{
    // Make sure all filters are applied
    signalFrame->getSignal().applyAllAnalogueModels();
    for (AirFrameVector::iterator it = interfererFrames.begin(); it != interfererFrames.end(); ++it) {
        (*it)->getSignal().applyAllAnalogueModels();
    }

    Signal& signal = signalFrame->getSignal();
    SpectrumPtr spectrum = signal.getSpectrum();

    Signal interference_noise = Signal(spectrum);
    interference_noise = noise;

    Signal sinr = Signal(spectrum);

    // Method will "filter out" the signalFrame
    std::vector<SignalChange> changes;
    calculateChanges(start, end, interfererFrames, &changes, signalFrame);

    // Prepare I+N at the beginning
    std::vector<SignalChange>::iterator changesIt = changes.begin();
    while (changesIt != changes.end()) {
        if (changesIt->time <= start) {
            interference_noise += *(changesIt->signal);
        }
        else {
            break;
        }
        changesIt++;
    }

    // Make sure to calculate SINR at the beginning
    double minSINR = INFINITY;

    for (uint16_t i = signal.getDataStart(); i < signal.getDataEnd(); i++) {
        double sinr = signal[i] / interference_noise[i];
        if (sinr < minSINR) minSINR = sinr;
    }

    // Calculate all chunks
    while (changesIt != changes.end()) {
        if (changesIt->type == SIGNAL_STARTS) {
            interference_noise += *(changesIt->signal);
        }
        else if (changesIt->type == SIGNAL_ENDS) {
            interference_noise -= *(changesIt->signal);
        }

        for (uint16_t i = signal.getDataStart(); i < signal.getDataEnd(); i++) {
            double sinr = signal[i] / interference_noise[i];
            if (sinr < minSINR) minSINR = sinr;
        }

        changesIt++;
    }

    return minSINR;
}

void MathHelper::calculateChanges(simtime_t_cref start, simtime_t_cref end, const AirFrameVector& airFrames, std::vector<SignalChange>* changes, const AirFrame* exclude)
{
    for (AirFrameVector::const_iterator it = airFrames.begin(); it != airFrames.end(); ++it) {
        if ((*it) == exclude) {
            continue;
        }

        SignalChange temp;
        temp.signal = &((*it)->getSignal());

        // In case of looking at a time-stamp (start=end) also take starting signals into account
        if (start == end && temp.signal->getReceptionStart() == start) {
            temp.type = SIGNAL_STARTS;
            temp.time = temp.signal->getReceptionStart();
            changes->push_back(temp);

            continue;
        }

        // Already filter changes outside the region of interest
        // End already outside region of interest (should be filtered out before)
        // Signal has to start before(!) end and must not end before the start of the interval
        if (temp.signal->getReceptionStart() < end && temp.signal->getReceptionEnd() > start) {
            temp.type = SIGNAL_STARTS;
            temp.time = temp.signal->getReceptionStart();
            changes->push_back(temp);

            // Already filter changes outside the region of interest
            if (temp.signal->getReceptionEnd() <= end) {
                temp.type = SIGNAL_ENDS;
                temp.time = temp.signal->getReceptionEnd();
                changes->push_back(temp);
            }
        }
    }

    // Bring changes (signals start and end) into an order
    std::sort(changes->begin(), changes->end(), compareByTime);
}
