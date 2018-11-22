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

#include "catch/catch.hpp"

#include "veins/base/phyLayer/DeciderToPhyInterface.h"
#include "veins/base/toolbox/Spectrum.h"
#include "veins/base/toolbox/Signal.h"
#include "veins/base/toolbox/SignalUtils.h"
#include "veins/base/messages/AirFrame_m.h"
#include "testutils/Simulation.h"
#include "DummyAnalogueModel.h"

using namespace Veins;
using AirFrameVector = DeciderToPhyInterface::AirFrameVector;

SCENARIO("Spectrum", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A vector of frequencies (1,1,5,3,4,2,6,4)")
    {
        Freqs freqs = {1, 1, 5, 3, 4, 2, 6, 4};

        WHEN("the spectrum is constructed")
        {
            Spectrum spectrum(freqs);

            THEN("duplicates are removed and frequencies are sorted ascending")
            {
                REQUIRE(spectrum[0] == 1);
                REQUIRE(spectrum[1] == 2);
                REQUIRE(spectrum[2] == 3);
                REQUIRE(spectrum[3] == 4);
                REQUIRE(spectrum[4] == 5);
                REQUIRE(spectrum[5] == 6);
            }
            THEN("the number of frequencies is four")
            {
                REQUIRE(spectrum.getNumFreqs() == 6);
            }
            THEN("accessing via freqAt-method returns correct frequency")
            {
                REQUIRE(spectrum.freqAt(0) == 1);
                REQUIRE(spectrum.freqAt(1) == 2);
                REQUIRE(spectrum.freqAt(2) == 3);
                REQUIRE(spectrum.freqAt(3) == 4);
                REQUIRE(spectrum.freqAt(4) == 5);
                REQUIRE(spectrum.freqAt(5) == 6);
            }
            THEN("accessing by frequency returns correct index")
            {
                REQUIRE(spectrum.indexOf(1) == 0);
                REQUIRE(spectrum.indexOf(2) == 1);
                REQUIRE(spectrum.indexOf(3) == 2);
                REQUIRE(spectrum.indexOf(4) == 3);
                REQUIRE(spectrum.indexOf(5) == 4);
                REQUIRE(spectrum.indexOf(6) == 5);
            }
            WHEN("another spectrum is created")
            {
                Spectrum spectrumClone(freqs);
                THEN("the singleton pattern just returns the same shared pointer")
                {
                    REQUIRE(spectrum == spectrumClone);
                }
            }
            WHEN("another spectrum is created with the frequencies in a different order")
            {
                auto reversedFrecs = freqs;
                std::reverse(reversedFrecs.begin(), reversedFrecs.end());
                Spectrum spectrumClone(reversedFrecs);
                THEN("the singleton pattern just returns the same shared pointer")
                {
                    REQUIRE(freqs != reversedFrecs);
                    REQUIRE(spectrum == spectrumClone);
                }
            }
            WHEN("another spectrum is created with some frequencies duplicated")
            {
                auto freqsWithDuplicates = freqs;
                freqsWithDuplicates.push_back(freqs.back());
                freqsWithDuplicates.push_back(freqs.front());
                Spectrum spectrumClone(freqsWithDuplicates);
                THEN("the singleton pattern just returns the same shared pointer")
                {
                    REQUIRE(spectrum == spectrumClone);
                }
            }
        }
    }
}

SCENARIO("Signal Constructors and Assignment", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4, 5, 6)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};

        Spectrum spectrum(freqs);

        WHEN("a signal is constructed without parameters")
        {
            Signal signal1;
            THEN("all values have their default values")
            {
                REQUIRE(signal1.getSpectrum() == Spectrum());
                REQUIRE(signal1.getAbsoluteValues() == 0);
                REQUIRE(signal1.getNumAbsoluteValues() == 0);
                REQUIRE(signal1.getNumRelativeValues() == 0);
                REQUIRE(signal1.getNumDataValues() == 0);
                REQUIRE(signal1.getRelativeOffset() == 0);
                REQUIRE(signal1.getDataOffset() == 0);
                REQUIRE(signal1.getCenterFrequencyIndex() == 0);

                // Checked in timing as well
                REQUIRE(signal1.hasTiming() == false);
                REQUIRE(signal1.getSendingStart() == 0);
                REQUIRE(signal1.getDuration() == 0);
                REQUIRE(signal1.getPropagationDelay() == 0);

                REQUIRE(signal1.getNumAnalogueModelsApplied() == 0);
                REQUIRE(signal1.getBitrate() == 0);

                WHEN("another signal is created with spectrum as parameter")
                {
                    Signal signal2(spectrum);
                    THEN("all values have their default values and numAbsoluteValues and values match to spectrum")
                    {
                        REQUIRE(signal2.getSpectrum() == spectrum);
                        REQUIRE(signal2.getAbsoluteValues() != 0);
                        REQUIRE(signal2.getNumAbsoluteValues() == 6);
                        REQUIRE(signal2.getNumRelativeValues() == 0);
                        REQUIRE(signal2.getNumDataValues() == 0);
                        REQUIRE(signal2.getRelativeOffset() == 0);
                        REQUIRE(signal2.getDataOffset() == 0);
                        REQUIRE(signal2.getCenterFrequencyIndex() == 0);

                        // Checked in timing as well
                        REQUIRE(signal2.hasTiming() == false);
                        REQUIRE(signal2.getSendingStart() == 0);
                        REQUIRE(signal2.getDuration() == 0);
                        REQUIRE(signal2.getPropagationDelay() == 0);

                        REQUIRE(signal2.getNumAnalogueModelsApplied() == 0);
                        REQUIRE(signal2.getBitrate() == 0);
                    }
                }
                WHEN("another signal is created with spectrum and timing as parameters")
                {
                    Signal signal2(spectrum, 10, 5);
                    THEN("all values have their default values and numAbsoluteValues, values match to spectrum and timing is applied properly")
                    {
                        REQUIRE(signal2.getSpectrum() == spectrum);
                        REQUIRE(signal2.getAbsoluteValues() != 0);
                        REQUIRE(signal2.getNumAbsoluteValues() == 6);
                        REQUIRE(signal2.getNumRelativeValues() == 0);
                        REQUIRE(signal2.getNumDataValues() == 0);
                        REQUIRE(signal2.getRelativeOffset() == 0);
                        REQUIRE(signal2.getDataOffset() == 0);
                        REQUIRE(signal2.getCenterFrequencyIndex() == 0);

                        // Checked in timing as well
                        REQUIRE(signal2.hasTiming() == true);
                        REQUIRE(signal2.getSendingStart() == 10);
                        REQUIRE(signal2.getDuration() == 5);
                        REQUIRE(signal2.getPropagationDelay() == 0);

                        REQUIRE(signal2.getNumAnalogueModelsApplied() == 0);
                        REQUIRE(signal2.getBitrate() == 0);

                        WHEN("another signal is created by using the copy constructor with the previous signal")
                        {
                            Signal signal3(signal2);
                            THEN("both signals match")
                            {
                                REQUIRE(signal2.getSpectrum() == signal3.getSpectrum());

                                // Each signal has its own values
                                REQUIRE(signal2.getAbsoluteValues() != signal3.getAbsoluteValues());
                                REQUIRE(signal2.getAbsoluteValues() != 0);
                                REQUIRE(signal3.getAbsoluteValues() != 0);

                                REQUIRE(signal2.getNumAbsoluteValues() == signal3.getNumAbsoluteValues());
                                REQUIRE(signal2.getNumRelativeValues() == signal3.getNumRelativeValues());
                                REQUIRE(signal2.getNumDataValues() == signal3.getNumDataValues());
                                REQUIRE(signal2.getRelativeOffset() == signal3.getRelativeOffset());
                                REQUIRE(signal2.getDataOffset() == signal3.getDataOffset());
                                REQUIRE(signal2.getCenterFrequencyIndex() == signal3.getCenterFrequencyIndex());

                                // Checked in timing as well
                                REQUIRE(signal2.hasTiming() == signal3.hasTiming());
                                REQUIRE(signal2.getSendingStart() == signal3.getSendingStart());
                                REQUIRE(signal2.getDuration() == signal3.getDuration());
                                REQUIRE(signal2.getPropagationDelay() == signal3.getPropagationDelay());

                                REQUIRE(signal2.getNumAnalogueModelsApplied() == signal3.getNumAnalogueModelsApplied());
                                REQUIRE(signal2.getBitrate() == signal3.getBitrate());
                            }
                        }
                        WHEN("another signal is created by using the assignment operator with the previous signal")
                        {
                            Signal signal3 = signal2;
                            THEN("both signals match")
                            {
                                REQUIRE(signal2.getSpectrum() == signal3.getSpectrum());

                                // Each signal has its own values
                                REQUIRE(signal2.getAbsoluteValues() != signal3.getAbsoluteValues());
                                REQUIRE(signal2.getAbsoluteValues() != 0);
                                REQUIRE(signal3.getAbsoluteValues() != 0);

                                REQUIRE(signal2.getNumAbsoluteValues() == signal3.getNumAbsoluteValues());
                                REQUIRE(signal2.getNumRelativeValues() == signal3.getNumRelativeValues());
                                REQUIRE(signal2.getNumDataValues() == signal3.getNumDataValues());
                                REQUIRE(signal2.getRelativeOffset() == signal3.getRelativeOffset());
                                REQUIRE(signal2.getDataOffset() == signal3.getDataOffset());
                                REQUIRE(signal2.getCenterFrequencyIndex() == signal3.getCenterFrequencyIndex());

                                // Checked in timing as well
                                REQUIRE(signal2.hasTiming() == signal3.hasTiming());
                                REQUIRE(signal2.getSendingStart() == signal3.getSendingStart());
                                REQUIRE(signal2.getDuration() == signal3.getDuration());
                                REQUIRE(signal2.getPropagationDelay() == signal3.getPropagationDelay());

                                REQUIRE(signal2.getNumAnalogueModelsApplied() == signal3.getNumAnalogueModelsApplied());
                                REQUIRE(signal2.getBitrate() == signal3.getBitrate());
                            }
                        }
                    }
                }
            }
        }
    }
}

SCENARIO("Signal Value Access", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};

        Spectrum spectrum(freqs);

        WHEN("a signal is constructed and values assigned to frequencies")
        {
            Signal signal(spectrum);
            signal[1] = 4;
            signal[2] = 3;
            signal[3] = 2;
            signal[4] = 1;

            // Access via operators
            THEN("relative interval is as expected")
            {
                REQUIRE(signal.getNumAbsoluteValues() == 6);

                REQUIRE(signal.getRelativeOffset() == 1);
                REQUIRE(signal.getNumRelativeValues() == 4);

                REQUIRE(signal.getRelativeStart() == 1);
                REQUIRE(signal.getRelativeEnd() == 5); // Note: This is not 4 as it should support iterations (what about <= in for-loop?)
            }
            THEN("frequencies can be addressed by index")
            {
                REQUIRE(signal[1] == 4);
                REQUIRE(signal[2] == 3);
                REQUIRE(signal[3] == 2);
                REQUIRE(signal[4] == 1);
            }
            THEN("frequencies can be addressed by frequency")
            {
                REQUIRE(signal(2) == 4);
                REQUIRE(signal(3) == 3);
                REQUIRE(signal(4) == 2);
                REQUIRE(signal(5) == 1);
            }

            WHEN("data interval defined")
            {
                signal.setDataStart(2);
                signal.setDataEnd(3);

                THEN("iterating over data interval returns correct values")
                {
                    for (uint16_t i = signal.getDataStart(); i < signal.getDataEnd(); i++)
                        REQUIRE(signal[i] == 5 - i);
                }
            }
        }
    }
}

#ifndef NDEBUG
SCENARIO("Invalid Signal Index Access", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3) and values for a signal with more frequencies (4,3,2,1)")
    {
        Freqs freqs = {1, 2, 3};
        std::vector<double> values = {4, 3, 2, 1};
        Spectrum spectrum(freqs);
        Signal signal(spectrum);
        WHEN("An invalid frequency index of the signal gets read")
        {
            THEN("An error should be raised (when in DEBUG mode)")
            {
                REQUIRE_THROWS(signal[3]);
            }
        }
        WHEN("The 4th signal value gets written to the signal with 3 frequencies")
        {
            signal[0] = values[0];
            signal[1] = values[1];
            signal[2] = values[2];
            THEN("An error should be raised (when in DEBUG mode)")
            {
                REQUIRE_THROWS(signal[3] = values[3]); // only throws when compiled in debug mode
            }
        }
    }
}
#endif

SCENARIO("Signal Timing", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4) and a signal (4,3,2,1)")
    {
        Freqs freqs = {1, 2, 3, 4};

        Spectrum spectrum(freqs);

        Signal signal(spectrum);
        signal[0] = 4;
        signal[1] = 3;
        signal[2] = 2;
        signal[3] = 1;

        WHEN("nothing done regarding timing")
        {
            THEN("there is no timing defined")
            {
                REQUIRE(signal.hasTiming() == false);
                REQUIRE(signal.getReceptionStart() == 0);
                REQUIRE(signal.getReceptionEnd() == 0);
                REQUIRE(signal.getDuration() == 0);
                REQUIRE(signal.getPropagationDelay() == 0);
            }

            WHEN("timing (start and duration) assigned")
            {
                signal.setTiming(1, 2);

                THEN("start is 1, duration 2, end 3, propagation delay still 0")
                {
                    REQUIRE(signal.hasTiming() == true);
                    REQUIRE(signal.getReceptionStart() == 1);
                    REQUIRE(signal.getReceptionEnd() == 3);
                    REQUIRE(signal.getDuration() == 2);
                    REQUIRE(signal.getPropagationDelay() == 0);
                }

                WHEN("additional propagation delay is assigned")
                {
                    signal.setPropagationDelay(10);

                    THEN("start is 11, duration still 2, end 13 and propagation delay 10")
                    {
                        REQUIRE(signal.hasTiming() == true);
                        REQUIRE(signal.getReceptionStart() == 11);
                        REQUIRE(signal.getReceptionEnd() == 13);
                        REQUIRE(signal.getDuration() == 2);
                        REQUIRE(signal.getPropagationDelay() == 10);
                    }

                    WHEN("start and duration is changed independently")
                    {
                        signal.setSendingStart(100);
                        signal.setDuration(1);

                        THEN("start is 110, duration 1, end 111 and propagation delay 10")
                        {
                            REQUIRE(signal.hasTiming() == true);
                            REQUIRE(signal.getReceptionStart() == 110);
                            REQUIRE(signal.getReceptionEnd() == 111);
                            REQUIRE(signal.getDuration() == 1);
                            REQUIRE(signal.getPropagationDelay() == 10);
                        }
                    }
                }
            }
        }
    }
}

SCENARIO("Signal Arithmetic Operators (Signal and Constant)", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (0,1,2,3,0,0) and a constant 2")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};

        Spectrum spectrum(freqs);

        Signal signal(spectrum);
        signal[1] = 1;
        signal[2] = 2;
        signal[3] = 3;

        double c = 2;

        WHEN("constant is added to signal or vice-versa")
        {
            Signal sum1 = signal + c;
            Signal sum2 = c + signal;
            THEN("result is (2,3,4,5,2,2)")
            {
                REQUIRE(sum1.getNumRelativeValues() == 6);
                REQUIRE(sum1.getRelativeStart() == 0);
                REQUIRE(sum1.getRelativeEnd() == 6);
                REQUIRE(sum1[0] == 2);
                REQUIRE(sum1[1] == 3);
                REQUIRE(sum1[2] == 4);
                REQUIRE(sum1[3] == 5);
                REQUIRE(sum1[4] == 2);
                REQUIRE(sum1[5] == 2);

                REQUIRE(sum2.getNumRelativeValues() == 6);
                REQUIRE(sum2.getRelativeStart() == 0);
                REQUIRE(sum2.getRelativeEnd() == 6);
                REQUIRE(sum2[0] == 2);
                REQUIRE(sum2[1] == 3);
                REQUIRE(sum2[2] == 4);
                REQUIRE(sum2[3] == 5);
                REQUIRE(sum2[4] == 2);
                REQUIRE(sum2[5] == 2);
            }
        }
        WHEN("constant is substracted from signal")
        {
            Signal difference1 = signal - c;
            Signal difference2 = c - signal;
            THEN("result is (-2,-1,0,1,-2,-2) rsp. (2,1,0,-1,2,2)")
            {
                REQUIRE(difference1.getNumRelativeValues() == 6);
                REQUIRE(difference1.getRelativeStart() == 0);
                REQUIRE(difference1.getRelativeEnd() == 6);
                REQUIRE(difference1[0] == -2);
                REQUIRE(difference1[1] == -1);
                REQUIRE(difference1[2] == 0);
                REQUIRE(difference1[3] == 1);
                REQUIRE(difference1[4] == -2);
                REQUIRE(difference1[5] == -2);

                REQUIRE(difference2.getNumRelativeValues() == 6);
                REQUIRE(difference2.getRelativeStart() == 0);
                REQUIRE(difference2.getRelativeEnd() == 6);
                REQUIRE(difference2[0] == 2);
                REQUIRE(difference2[1] == 1);
                REQUIRE(difference2[2] == 0);
                REQUIRE(difference2[3] == -1);
                REQUIRE(difference2[4] == 2);
                REQUIRE(difference2[5] == 2);
            }
        }
        WHEN("signal1 is multiplicated with constant or vice-versa")
        {
            Signal product1 = signal * c;
            Signal product2 = c * signal;
            THEN("result is (0,2,4,6,0,0)")
            {
                REQUIRE(product1.getNumRelativeValues() == 6);
                REQUIRE(product1.getRelativeStart() == 0);
                REQUIRE(product1.getRelativeEnd() == 6);
                REQUIRE(product1[0] == 0);
                REQUIRE(product1[1] == 2);
                REQUIRE(product1[2] == 4);
                REQUIRE(product1[3] == 6);
                REQUIRE(product1[4] == 0);
                REQUIRE(product1[5] == 0);

                REQUIRE(product2.getNumRelativeValues() == 6);
                REQUIRE(product2.getRelativeStart() == 0);
                REQUIRE(product2.getRelativeEnd() == 6);
                REQUIRE(product2[0] == 0);
                REQUIRE(product2[1] == 2);
                REQUIRE(product2[2] == 4);
                REQUIRE(product2[3] == 6);
                REQUIRE(product2[4] == 0);
                REQUIRE(product2[5] == 0);
            }
        }
        WHEN("signal1 is divided by constant or vice-versa")
        {
            Signal quotient1 = signal / c;
            Signal quotient2 = c / signal;
            THEN("result is (0,0.5,1,0.6..7,0,0) rsp. (inf,2,1,0.6..7,inf,inf)")
            {
                REQUIRE(quotient1.getNumRelativeValues() == 6);
                REQUIRE(quotient1.getRelativeStart() == 0);
                REQUIRE(quotient1.getRelativeEnd() == 6);
                REQUIRE(quotient1[0] == 0);
                REQUIRE(quotient1[1] == 0.5);
                REQUIRE(quotient1[2] == 1);
                REQUIRE(quotient1[3] == 1.5);
                REQUIRE(quotient1[4] == 0);
                REQUIRE(quotient1[5] == 0);

                REQUIRE(quotient2.getNumRelativeValues() == 6);
                REQUIRE(quotient2.getRelativeStart() == 0);
                REQUIRE(quotient2.getRelativeEnd() == 6);
                REQUIRE(quotient2[0] == INFINITY);
                REQUIRE(quotient2[1] == 2);
                REQUIRE(quotient2[2] == 1);
                REQUIRE(quotient2[3] == double(2.0 / 3.0));
                REQUIRE(quotient2[4] == INFINITY);
                REQUIRE(quotient2[5] == INFINITY);
            }
        }
        WHEN("signal1 is left shifted by 1")
        {
            Signal leftShift = signal << uint16_t(1);
            THEN("result is (1,2,3,0,0,0)")
            {
                REQUIRE(leftShift.getRelativeStart() == 0);
                REQUIRE(leftShift.getRelativeEnd() == 3);
                REQUIRE(leftShift[0] == 1);
                REQUIRE(leftShift[1] == 2);
                REQUIRE(leftShift[2] == 3);
                REQUIRE(leftShift[3] == 0);
                REQUIRE(leftShift[4] == 0);
                REQUIRE(leftShift[5] == 0);
            }
        }
        WHEN("signal1 is left shifted by 2")
        {
            Signal leftShift = signal << uint16_t(2);
            THEN("shifting is illegal, so original signal is returned (0,1,2,3,0,0)")
            {
                REQUIRE(leftShift.getRelativeStart() == 1);
                REQUIRE(leftShift.getRelativeEnd() == 4);
                REQUIRE(leftShift[0] == 0);
                REQUIRE(leftShift[1] == 1);
                REQUIRE(leftShift[2] == 2);
                REQUIRE(leftShift[3] == 3);
                REQUIRE(leftShift[4] == 0);
                REQUIRE(leftShift[5] == 0);
            }
        }
        WHEN("signal1 is right shifted by 2")
        {
            Signal rightShift = signal >> uint16_t(2);
            THEN("result is (0,0,0,1,2,3)")
            {
                REQUIRE(rightShift.getRelativeStart() == 3);
                REQUIRE(rightShift.getRelativeEnd() == 6);
                REQUIRE(rightShift[0] == 0);
                REQUIRE(rightShift[1] == 0);
                REQUIRE(rightShift[2] == 0);
                REQUIRE(rightShift[3] == 1);
                REQUIRE(rightShift[4] == 2);
                REQUIRE(rightShift[5] == 3);
            }
        }
        WHEN("signal1 is right shifted by 3")
        {
            Signal rightShift = signal >> uint16_t(3);
            THEN("shifting is illegal, so original signal is returned (0,1,2,3,0,0)")
            {
                REQUIRE(rightShift.getRelativeStart() == 1);
                REQUIRE(rightShift.getRelativeEnd() == 4);
                REQUIRE(rightShift[0] == 0);
                REQUIRE(rightShift[1] == 1);
                REQUIRE(rightShift[2] == 2);
                REQUIRE(rightShift[3] == 3);
                REQUIRE(rightShift[4] == 0);
                REQUIRE(rightShift[5] == 0);
            }
        }
    }
}

SCENARIO("Signal Arithmetic Operators (Two Signals)", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6) and two signals (0,1,2,0,0,0), (0,0,3,4,0,0)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};

        Spectrum spectrum(freqs);

        Signal signal1(spectrum);
        signal1[1] = 1;
        signal1[2] = 2;

        Signal signal2(spectrum);
        signal2[2] = 3;
        signal2[3] = 4;

        WHEN("both signals are summed up")
        {
            Signal sum = signal1 + signal2;
            THEN("result is (0,1,5,4,0,0)")
            {
                REQUIRE(sum.getNumRelativeValues() == 3);
                REQUIRE(sum.getRelativeStart() == 1);
                REQUIRE(sum.getRelativeEnd() == 4);
                REQUIRE(sum[1] == 1);
                REQUIRE(sum[2] == 5);
                REQUIRE(sum[3] == 4);
            }
        }
        WHEN("signal1 is substracted from signal2")
        {
            Signal difference = signal2 - signal1;
            THEN("result is (0,1,5,4,0,0)")
            {
                REQUIRE(difference.getNumRelativeValues() == 3);
                REQUIRE(difference.getRelativeStart() == 1);
                REQUIRE(difference.getRelativeEnd() == 4);
                REQUIRE(difference[1] == -1);
                REQUIRE(difference[2] == 1);
                REQUIRE(difference[3] == 4);
            }
        }
        WHEN("signal1 is multiplicated with signal2")
        {
            Signal product = signal1 * signal2;
            THEN("result is (0,1,5,4,0,0)")
            {
                REQUIRE(product.getNumRelativeValues() == 3);
                REQUIRE(product.getRelativeStart() == 1);
                REQUIRE(product.getRelativeEnd() == 4);
                REQUIRE(product[1] == 0);
                REQUIRE(product[2] == 6);
                REQUIRE(product[3] == 0);
            }
        }
        WHEN("signal1 is divided by signal2")
        {
            Signal quotient = signal1 / signal2;
            THEN("result is (0,1,5,4,0,0)")
            {
                REQUIRE(quotient.getNumRelativeValues() == 3);
                REQUIRE(quotient.getRelativeStart() == 1);
                REQUIRE(quotient.getRelativeEnd() == 4);
                REQUIRE(quotient[1] == INFINITY);
                REQUIRE(quotient[2] == double(2.0 / 3.0));
                REQUIRE(quotient[3] == 0);
            }
        }
    }
}

SCENARIO("Signal Compound Assignment Operators (Signal and Constant)", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (0,1,2,3,0,0) and a constant 2")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};

        Spectrum spectrum(freqs);

        Signal signal(spectrum);
        signal[1] = 1;
        signal[2] = 2;
        signal[3] = 3;

        double c = 2;

        WHEN("constant is added to signal or vice-versa")
        {
            Signal sum = signal;
            sum += c;
            THEN("result is (2,3,4,5,2,2)")
            {
                REQUIRE(sum.getNumRelativeValues() == 6);
                REQUIRE(sum.getRelativeStart() == 0);
                REQUIRE(sum.getRelativeEnd() == 6);
                REQUIRE(sum[0] == 2);
                REQUIRE(sum[1] == 3);
                REQUIRE(sum[2] == 4);
                REQUIRE(sum[3] == 5);
                REQUIRE(sum[4] == 2);
                REQUIRE(sum[5] == 2);
            }
        }
        WHEN("constant is substracted from signal")
        {
            Signal difference = signal;
            difference -= c;
            THEN("result is (-2,-1,0,1,-2,-2) rsp. (2,1,0,-1,2,2)")
            {
                REQUIRE(difference.getNumRelativeValues() == 6);
                REQUIRE(difference.getRelativeStart() == 0);
                REQUIRE(difference.getRelativeEnd() == 6);
                REQUIRE(difference[0] == -2);
                REQUIRE(difference[1] == -1);
                REQUIRE(difference[2] == 0);
                REQUIRE(difference[3] == 1);
                REQUIRE(difference[4] == -2);
                REQUIRE(difference[5] == -2);
            }
        }
        WHEN("signal1 is multiplicated with constant or vice-versa")
        {
            Signal product = signal;
            product *= c;
            THEN("result is (0,2,4,6,0,0)")
            {
                REQUIRE(product.getNumRelativeValues() == 6);
                REQUIRE(product.getRelativeStart() == 0);
                REQUIRE(product.getRelativeEnd() == 6);
                REQUIRE(product[0] == 0);
                REQUIRE(product[1] == 2);
                REQUIRE(product[2] == 4);
                REQUIRE(product[3] == 6);
                REQUIRE(product[4] == 0);
                REQUIRE(product[5] == 0);
            }
        }
        WHEN("signal1 is divided by constant or vice-versa")
        {
            Signal quotient = signal;
            quotient /= c;
            THEN("result is (0,0.5,1,0.6..7,0,0) rsp. (inf,2,1,0.6..7,inf,inf)")
            {
                REQUIRE(quotient.getNumRelativeValues() == 6);
                REQUIRE(quotient.getRelativeStart() == 0);
                REQUIRE(quotient.getRelativeEnd() == 6);
                REQUIRE(quotient[0] == 0);
                REQUIRE(quotient[1] == 0.5);
                REQUIRE(quotient[2] == 1);
                REQUIRE(quotient[3] == 1.5);
                REQUIRE(quotient[4] == 0);
                REQUIRE(quotient[5] == 0);
            }
        }
    }
}

SCENARIO("Signal Compound Assignment Operators (Two Signals)", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6) and two signals (0,1,2,0,0,0), (0,0,3,4,0,0)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};

        Spectrum spectrum(freqs);

        Signal signal1(spectrum);
        signal1[1] = 1;
        signal1[2] = 2;

        Signal signal2(spectrum);
        signal2[2] = 3;
        signal2[3] = 4;

        WHEN("both signals are summed up")
        {
            Signal sum = signal1;
            sum += signal2;
            THEN("result is (0,1,5,4,0,0)")
            {
                REQUIRE(sum.getNumRelativeValues() == 3);
                REQUIRE(sum.getRelativeStart() == 1);
                REQUIRE(sum.getRelativeEnd() == 4);
                REQUIRE(sum[1] == 1);
                REQUIRE(sum[2] == 5);
                REQUIRE(sum[3] == 4);
            }
        }
        WHEN("signal1 is substracted from signal2")
        {
            Signal difference = signal2;
            difference -= signal1;
            THEN("result is (0,1,5,4,0,0)")
            {
                REQUIRE(difference.getNumRelativeValues() == 3);
                REQUIRE(difference.getRelativeStart() == 1);
                REQUIRE(difference.getRelativeEnd() == 4);
                REQUIRE(difference[1] == -1);
                REQUIRE(difference[2] == 1);
                REQUIRE(difference[3] == 4);
            }
        }
        WHEN("signal1 is multiplicated with signal2")
        {
            Signal product = signal1;
            product *= signal2;
            THEN("result is (0,1,5,4,0,0)")
            {
                REQUIRE(product.getNumRelativeValues() == 3);
                REQUIRE(product.getRelativeStart() == 1);
                REQUIRE(product.getRelativeEnd() == 4);
                REQUIRE(product[1] == 0);
                REQUIRE(product[2] == 6);
                REQUIRE(product[3] == 0);
            }
        }
        WHEN("signal1 is divided by signal2")
        {
            Signal quotient = signal1;
            quotient /= signal2;
            THEN("result is (0,1,5,4,0,0)")
            {
                REQUIRE(quotient.getNumRelativeValues() == 3);
                REQUIRE(quotient.getRelativeStart() == 1);
                REQUIRE(quotient.getRelativeEnd() == 4);
                REQUIRE(quotient[1] == INFINITY);
                REQUIRE(quotient[2] == double(2.0 / 3.0));
                REQUIRE(quotient[3] == 0);
            }
        }
    }
}

SCENARIO("Signal Thresholding (smaller)", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6) and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};

        Spectrum spectrum(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.5);

        AnalogueModelList analogueModels;
        analogueModels.push_back(&am1);
        analogueModels.push_back(&am2);

        Signal signal(spectrum);
        signal[0] = 10;
        signal[1] = 20;
        signal[2] = 30;
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);

        WHEN("checked if a given power already above current value is smaller than current value")
        {
            bool belowThreshold = signal.smallerAtCenterFrequency(40);
            THEN("true returned and no AM applied")
            {
                REQUIRE(belowThreshold == true);
                REQUIRE(signal.getAtCenterFrequency() == 30);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 0);
            }
        }
        WHEN("checked if a given power that requires only first AM to be applied is smaller than current value")
        {
            bool belowThreshold = signal.smallerAtCenterFrequency(25);
            THEN("true returned and first AM applied")
            {
                REQUIRE(belowThreshold == true);
                REQUIRE(signal.getAtCenterFrequency() == 3);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 1);
            }
        }
        WHEN("checked if a given power that requires only first AM to be applied is smaller than current value again")
        {
            bool belowThreshold = signal.smallerAtCenterFrequency(25);
            THEN("true directly returned and no further AM applied (only first still applied)")
            {
                REQUIRE(belowThreshold == true);
                REQUIRE(signal.getAtCenterFrequency() == 3);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 1);
            }
        }
        WHEN("checked if a given power that requires both AMs to be applied is smaller than current value")
        {
            bool belowThreshold = signal.smallerAtCenterFrequency(2.5);
            THEN("true directly returned and no further AM applied (only first still applied)")
            {
                REQUIRE(belowThreshold == true);
                REQUIRE(signal.getAtCenterFrequency() == 1.5);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 2);
            }
        }
        WHEN("checked if a given power that is smaller than signal when all AMs applied")
        {
            bool belowThreshold = signal.smallerAtCenterFrequency(1);
            THEN("false directly returned and no further AM applied (only first still applied)")
            {
                REQUIRE(belowThreshold == false);
                REQUIRE(signal.getAtCenterFrequency() == 1.5);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 2);
            }
        }
    }
}

SCENARIO("Signal Thresholding (greater)", "[toolbox]") // Not used in Veins, but supported
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0) and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};

        Spectrum spectrum(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.5);

        AnalogueModelList analogueModels;
        analogueModels.push_back(&am1);
        analogueModels.push_back(&am2);

        Signal signal(spectrum);
        signal[0] = 10;
        signal[1] = 20;
        signal[2] = 30;
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);

        WHEN("checked if a given power already below current value is smaller than current value")
        {
            bool aboveThreshold = signal.greaterAtCenterFrequency(40);
            THEN("false returned and no AM applied")
            {
                REQUIRE(aboveThreshold == false);
                REQUIRE(signal.getAtCenterFrequency() == 30);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 0);
            }
        }
        WHEN("checked if a given power that requires only first AM to be applied is smaller than current value")
        {
            bool aboveThreshold = signal.greaterAtCenterFrequency(25);
            THEN("false returned and first AM applied")
            {
                REQUIRE(aboveThreshold == false);
                REQUIRE(signal.getAtCenterFrequency() == 3);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 1);
            }
        }
        WHEN("checked if a given power that requires only first AM to be applied is smaller than current value again")
        {
            bool aboveThreshold = signal.greaterAtCenterFrequency(25);
            THEN("false directly returned and no further AM applied (only first still applied)")
            {
                REQUIRE(aboveThreshold == false);
                REQUIRE(signal.getAtCenterFrequency() == 3);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 1);
            }
        }
        WHEN("checked if a given power that requires both AMs to be applied is smaller than current value")
        {
            bool aboveThreshold = signal.greaterAtCenterFrequency(2.5);
            THEN("false directly returned and no further AM applied (only first still applied)")
            {
                REQUIRE(aboveThreshold == false);
                REQUIRE(signal.getAtCenterFrequency() == 1.5);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 2);
            }
        }
        WHEN("checked if a given power that is smaller than signal when all AMs applied")
        {
            bool aboveThreshold = signal.greaterAtCenterFrequency(1);
            THEN("true directly returned and no further AM applied (only first still applied)")
            {
                REQUIRE(aboveThreshold == true);
                REQUIRE(signal.getAtCenterFrequency() == 1.5);
                REQUIRE(signal.getNumAnalogueModelsApplied() == 2);
            }
        }
    }
}

SCENARIO("SignalUtils minimum Value at Frequency and Timestamp", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0),(), another signal (),() and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};
        Spectrum spectrum(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.1);
        AnalogueModelList analogueModels{&am1, &am2};

        Signal signal(spectrum);
        signal[0] = 100;
        signal[1] = 200;
        signal[2] = 300;
        signal.setDataStart(0);
        signal.setDataEnd(2);
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);
        signal.setTiming(5, 10);

        AirFrameVector airFrames;
        AirFrame signalFrame;
        signalFrame.setSignal(signal);
        airFrames.push_back(&signalFrame);

        WHEN("check for already higher threshold at a time-stamp in the middle of the signal")
        {
            bool belowThreshold = SignalUtils::smallerAtFreqIndex(10, 10, airFrames, 2, 500.0);
            THEN("no AnalogueModel applied and true returned")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 0);
                REQUIRE(belowThreshold == true);
            }
        }
        WHEN("check for already higher threshold at a time-stamp at the start of the signal")
        {
            bool belowThreshold = SignalUtils::smallerAtFreqIndex(5, 5, airFrames, 2, 500.0);
            THEN("no AnalogueModel applied and true returned")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 0);
                REQUIRE(belowThreshold == true);
            }
        }
        WHEN("check for already higher threshold at a time-stamp at the end of the signal")
        {
            bool belowThreshold = SignalUtils::smallerAtFreqIndex(15, 15, airFrames, 2, 500.0);
            THEN("no AnalogueModel applied and true returned")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 0);
                REQUIRE(belowThreshold == true);
            }
        }
        WHEN("check for a too low threshold at a time-stamp at the end of the signal")
        {
            bool belowThreshold = SignalUtils::smallerAtFreqIndex(10, 10, airFrames, 2, 1.0);
            THEN("all AnalogueModels applied and false returned")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(belowThreshold == false);
            }
        }
        WHEN("check for a too low threshold at a time-stamp at the end of the signal")
        {
            bool belowThreshold = SignalUtils::smallerAtFreqIndex(5, 5, airFrames, 2, 1.0);
            THEN("all AnalogueModels applied and false returned")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(belowThreshold == false);
            }
        }
        WHEN("check for a too low threshold at a time-stamp at the end of the signal")
        {
            bool belowThreshold = SignalUtils::smallerAtFreqIndex(15, 15, airFrames, 2, 1.0);
            THEN("no AnalogueModel applied (as signal not within intervall) and true returned (as signal does not contribute to power)")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 0);
                REQUIRE(belowThreshold == true);
            }
        }
    }
}

SCENARIO("SignalUtils Get Min SINR Simple Test Cases", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0), another (interferer) signal, and a list with two DummyAnalogueModels (0.1, 0.1)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};
        Spectrum spectrum(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.1);
        AnalogueModelList analogueModels = {&am1, &am2};

        Signal signal(spectrum);
        signal[0] = 100;
        signal[1] = 200;
        signal[2] = 300;
        signal.setDataStart(0);
        signal.setDataEnd(2);
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);
        signal.setTiming(5, 10);

        AirFrameVector airFrames;
        AirFrame signalFrame;
        signalFrame.setSignal(signal);
        airFrames.push_back(&signalFrame);

        Signal interferer(signal);
        AirFrameVector interfererFrames;
        AirFrame interfererFrame;
        interfererFrame.setSignal(interferer);
        interfererFrames.push_back(&interfererFrame);

        WHEN("searching for global min SINR over the whole time and signals perfectly overlap")
        {
            interfererFrame.getSignal().setTiming(5, 10);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 2.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searching for global min over the whole time and signals only overlap for half of the time")
        {
            interfererFrame.getSignal().setTiming(0, 5);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 1.0);
            }
        }
        WHEN("searching for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame.getSignal().setTiming(15, 5);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 1.0);
            }
        }
        WHEN("searching for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame.getSignal().setTiming(7.5, 5);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searching for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame.getSignal().setTiming(5, 5);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searching for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame.getSignal().setTiming(10, 5);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searching for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame.getSignal().setTiming(0, 7.5);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searching for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame.getSignal().setTiming(12.5, 7.5);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searching for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame.getSignal().setTiming(0, 0);
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 1.0);
            }
        }
    }
}

SCENARIO("SignalUtils Get Min SINR Complex Test Case", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0), eight other signals and a list with two DummyAnalogueModels (0.1, 0.1)")
    {
        Freqs freqs = {1, 2, 3, 4, 5, 6};
        Spectrum spectrum(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.1);
        AnalogueModelList analogueModels = {&am1, &am2};

        Signal signal(spectrum);
        signal[0] = 100;
        signal[1] = 200;
        signal[2] = 300;
        signal.setDataStart(0);
        signal.setDataEnd(2);
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);
        signal.setTiming(5, 10);

        AirFrameVector airFrames;
        AirFrame signalFrame;
        signalFrame.setSignal(signal);
        airFrames.push_back(&signalFrame);

        const std::vector<std::pair<double, double>> timings = {
            {5, 10},
            {0, 5},
            {0, 7.5},
            {5, 5},
            {7.5, 5},
            {10, 5},
            {12.5, 7.5},
            {15, 5},
        };
        std::vector<std::unique_ptr<AirFrame>> interfererFrameOwner; // for automatic deletion (RAII)
        AirFrameVector interfererFrames;
        for (auto& timing : timings) {
            Signal interferer(signal);
            interferer.setTiming(timing.first, timing.second);
            interfererFrameOwner.emplace_back(new AirFrame());
            AirFrame* interfererFrame = interfererFrameOwner.back().get();
            interfererFrame->setSignal(interferer);
            interfererFrames.push_back(interfererFrame);
        }

        WHEN("searching for global min SINR over the whole time and signals perfectly overlap")
        {
            double min = SignalUtils::getMinSINR(5, 15, &signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels are applied and min is equal to 2.0")
            {
                REQUIRE(signalFrame.getSignal().getNumAnalogueModelsApplied() == 2);
                for (auto& interfererFrame : interfererFrames) {
                    REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                }
                REQUIRE(min == 0.25);
            }
        }
    }
}

SCENARIO("SignalUtils::smallerAtFreqIndex treats signal start/end as inclusive/exclusive", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("Signals from 1-2 and 2-3 with power 10")
    {
        Freqs freqs = {1, 2, 3, 4, 5};
        Spectrum spectrum(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.1);
        AnalogueModelList analogueModels = {&am1, &am2};

        Signal signal(spectrum);
        signal[1] = 1000;
        signal[2] = 1000; // after both analogue models, this should have dropped to 10
        signal[3] = 1000;
        signal.setDataStart(1);
        signal.setDataEnd(3);
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);

        const std::vector<std::pair<double, double>> timings = {
            {1, 1},
            {2, 1},
        };
        std::vector<std::unique_ptr<AirFrame>> airFrameOwner; // for automatic deletion (RAII)
        AirFrameVector airFrames;
        for (auto& timing : timings) {
            Signal s(signal);
            s.setTiming(timing.first, timing.second);
            airFrameOwner.emplace_back(new AirFrame());
            AirFrame* airFrame = airFrameOwner.back().get();
            airFrame->setSignal(s);
            airFrames.push_back(airFrame);
        }

        const std::vector<std::tuple<double, double, bool>> checks = {
            std::make_tuple(0, 0, true),
            std::make_tuple(0.5, 0.5, true),
            std::make_tuple(1, 1, false),
            std::make_tuple(1.5, 1.5, false),
            std::make_tuple(2, 2, false),
            std::make_tuple(2.5, 2.5, false),
            std::make_tuple(3, 3, true),
            std::make_tuple(3.5, 3.5, true),
        };
        for (auto& check : checks) {
            auto begin = std::get<0>(check);
            auto end = std::get<1>(check);
            bool res = std::get<2>(check);
            double threshold = 5;

            INFO("getting smallerAtFreqIndex(2) for " << begin << " to " << end << " and threshold " << threshold << " should return " << res);

            bool belowThreshold = SignalUtils::smallerAtFreqIndex(begin, end, airFrames, 2, threshold);

            REQUIRE(belowThreshold == res);
        }
    }
}

SCENARIO("SignalUtils::getMinSINR treats signal start/end as inclusive/exclusive", "[toolbox]")
{
    DummySimulation ds(new cNullEnvir(0, nullptr, nullptr)); // necessary so simtime_t works
    GIVEN("Signals from 1-2 and 2-3 with power 10")
    {
        Freqs freqs = {1, 2, 3, 4, 5};
        Spectrum spectrum(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.1);
        AnalogueModelList analogueModels = {&am1, &am2};

        Signal signal(spectrum);
        signal[1] = 1000;
        signal[2] = 1000; // after both analogue models, this should have dropped to 10
        signal[3] = 1000;
        signal.setDataStart(1);
        signal.setDataEnd(3);
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);

        const std::vector<std::pair<double, double>> timings = {
            {1, 1},
            {2, 1},
        };
        std::vector<std::unique_ptr<AirFrame>> airFrameOwner; // for automatic deletion (RAII)
        AirFrameVector airFrames;
        for (auto& timing : timings) {
            Signal s(signal);
            s.setTiming(timing.first, timing.second);
            airFrameOwner.emplace_back(new AirFrame());
            AirFrame* airFrame = airFrameOwner.back().get();
            airFrame->setSignal(s);
            airFrames.push_back(airFrame);
        }

        AirFrame signalFrame;
        signalFrame.setSignal(signal);
        airFrames.push_back(&signalFrame);

        const std::vector<std::tuple<double, double, double>> checks = {
            std::make_tuple(0, 0.5, INFINITY),
            std::make_tuple(0.5, 1.5, 1),
            std::make_tuple(1, 2, 1),
            std::make_tuple(1.5, 2.5, 1),
            std::make_tuple(2, 3, 1),
            std::make_tuple(2.5, 3.5, 1),
            std::make_tuple(3.5, 4.5, INFINITY),
        };
        for (auto& check : checks) {
            auto begin = std::get<0>(check);
            auto end = std::get<1>(check);
            double res = std::get<2>(check);

            INFO("getting getMinSINR for signal from " << begin << " to " << end << " should return " << res);

            signalFrame.getSignal().setTiming(begin, end - begin);
            double min = SignalUtils::getMinSINR(begin, end, &signalFrame, airFrames, 0);

            REQUIRE(min == res);
        }
    }
}
