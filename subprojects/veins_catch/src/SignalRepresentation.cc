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

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include "../../../src/veins/base/toolbox/Spectrum.h"
#include "../../../src/veins/base/toolbox/Signal.h"
#include "../../../src/veins/base/toolbox/MathHelper.h"

#include "DummyAnalogueModel.h"

/*void deleteAirFrameVector(AirFrameVector& airFrames)
{
    uint16_t numFrames = airFrames.size();
    std::cout << "Delete airFrames: " << numFrames << " frames deleted" << std::endl;
    for(uint16_t i=0;i<numFrames;i++)
    {
        delete airFrames.front();
        airFrames.pop_front();
    }
}*/

SCENARIO("Spectrum", "[toolbox]")
{
    GIVEN("A vector of frequencies (1,1,5,3,4,2,6,4)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(1);
        freqs.push_back(5);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(2);
        freqs.push_back(6);
        freqs.push_back(4);

        WHEN("the spectrum is constructed")
        {
            SpectrumPtr spectrum = Spectrum::getInstance(freqs);

            THEN("duplicates are removed and frequencies are sorted ascending")
            {
                REQUIRE((*spectrum)[0] == 1);
                REQUIRE((*spectrum)[1] == 2);
                REQUIRE((*spectrum)[2] == 3);
                REQUIRE((*spectrum)[3] == 4);
                REQUIRE((*spectrum)[4] == 5);
                REQUIRE((*spectrum)[5] == 6);
            }
            THEN("the number of frequencies is four")
            {
                REQUIRE(spectrum->getNumFreqs() == 6);
            }
            THEN("accessing via freqAt-method returns correct frequency")
            {
                REQUIRE(spectrum->freqAt(0) == 1);
                REQUIRE(spectrum->freqAt(1) == 2);
                REQUIRE(spectrum->freqAt(2) == 3);
                REQUIRE(spectrum->freqAt(3) == 4);
                REQUIRE(spectrum->freqAt(4) == 5);
                REQUIRE(spectrum->freqAt(5) == 6);
            }
            THEN("accessing by frequency returns correct index")
            {
                REQUIRE(spectrum->indexOf(1) == 0);
                REQUIRE(spectrum->indexOf(2) == 1);
                REQUIRE(spectrum->indexOf(3) == 2);
                REQUIRE(spectrum->indexOf(4) == 3);
                REQUIRE(spectrum->indexOf(5) == 4);
                REQUIRE(spectrum->indexOf(6) == 5);
            }
            WHEN("another spectrum is created")
            {
                SpectrumPtr spectrumClone = Spectrum::getInstance(freqs);
                THEN("the singleton pattern just returns the same shared pointer")
                {
                    REQUIRE(spectrum==spectrumClone);
                }
            }
        }
    }
}

SCENARIO("Signal Constructors and Assignment", "[toolbox]")
{
    GIVEN("A spectrum with frequencies (1,2,3,4)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

        WHEN("a signal is constructed without parameters")
        {
            Signal signal1;
            THEN("all values have their default values")
            {
                REQUIRE(signal1.getSpectrum() == nullptr);
                REQUIRE(signal1.getAbsoluteValues() == 0);
                REQUIRE(signal1.getNumAbsoluteValues() == 0);
                REQUIRE(signal1.getNumRelativeValues() == 0);
                REQUIRE(signal1.getNumDataValues() == 0);
                REQUIRE(signal1.getRelativeOffset() == 0);
                REQUIRE(signal1.getDataOffset() == 0);
                REQUIRE(signal1.getCenterFrequencyIndex() == 0);

                //Checked in timing as well
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

                        //Checked in timing as well
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

                        //Checked in timing as well
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

                                //Each signal has its own values
                                REQUIRE(signal2.getAbsoluteValues() != signal3.getAbsoluteValues());
                                REQUIRE(signal2.getAbsoluteValues() != 0);
                                REQUIRE(signal3.getAbsoluteValues() != 0);

                                REQUIRE(signal2.getNumAbsoluteValues() == signal3.getNumAbsoluteValues());
                                REQUIRE(signal2.getNumRelativeValues() == signal3.getNumRelativeValues());
                                REQUIRE(signal2.getNumDataValues() == signal3.getNumDataValues());
                                REQUIRE(signal2.getRelativeOffset() == signal3.getRelativeOffset());
                                REQUIRE(signal2.getDataOffset() == signal3.getDataOffset());
                                REQUIRE(signal2.getCenterFrequencyIndex() == signal3.getCenterFrequencyIndex());

                                //Checked in timing as well
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

                                //Each signal has its own values
                                REQUIRE(signal2.getAbsoluteValues() != signal3.getAbsoluteValues());
                                REQUIRE(signal2.getAbsoluteValues() != 0);
                                REQUIRE(signal3.getAbsoluteValues() != 0);

                                REQUIRE(signal2.getNumAbsoluteValues() == signal3.getNumAbsoluteValues());
                                REQUIRE(signal2.getNumRelativeValues() == signal3.getNumRelativeValues());
                                REQUIRE(signal2.getNumDataValues() == signal3.getNumDataValues());
                                REQUIRE(signal2.getRelativeOffset() == signal3.getRelativeOffset());
                                REQUIRE(signal2.getDataOffset() == signal3.getDataOffset());
                                REQUIRE(signal2.getCenterFrequencyIndex() == signal3.getCenterFrequencyIndex());

                                //Checked in timing as well
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
    GIVEN("A spectrum with frequencies (1,2,3,4)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

        WHEN("a signal is constructed and values assigned to frequencies")
        {
            Signal signal(spectrum);
            signal[1] = 4;
            signal[2] = 3;
            signal[3] = 2;
            signal[4] = 1;

            //Access via operators
            THEN("relative interval is as expected")
            {
                REQUIRE(signal.getNumAbsoluteValues() == 6);

                REQUIRE(signal.getRelativeOffset() == 1);
                REQUIRE(signal.getNumRelativeValues() == 4);

                REQUIRE(signal.getRelativeStart() == 1);
                REQUIRE(signal.getRelativeEnd() == 5);  //Note: This is not 4 as it should support iterations (what about <= in for-loop?)
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
                    for(uint16_t i=signal.getDataStart();i<signal.getDataEnd();i++)
                        REQUIRE(signal[i] == 5-i);
                }
            }
        }
    }
}

SCENARIO("Signal Timing", "[toolbox]")
{
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6) and a signal (0,4,3,2,1,0)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

        Signal signal(spectrum);
        signal[1] = 4;
        signal[2] = 3;
        signal[3] = 2;
        signal[4] = 1;

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
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (0,1,2,3,0,0) and a constant 2")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

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
                REQUIRE(quotient2[3] == double(2.0d/3.0d));
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
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6) and two signals (0,1,2,0,0,0), (0,0,3,4,0,0)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

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
                REQUIRE(quotient[2] == double(2.0d/3.0d));
                REQUIRE(quotient[3] == 0);
            }
        }
    }
}

SCENARIO("Signal Compound Assignment Operators (Signal and Constant)", "[toolbox]")
{
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (0,1,2,3,0,0) and a constant 2")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

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
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6) and two signals (0,1,2,0,0,0), (0,0,3,4,0,0)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

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
                REQUIRE(quotient[2] == double(2.0d/3.0d));
                REQUIRE(quotient[3] == 0);
            }
        }
    }
}

SCENARIO("Signal Thresholding (smaller)", "[toolbox]")
{
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6) and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

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

SCENARIO("Signal Thresholding (greater)", "[toolbox]") //Not used in Veins, but supported
{
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0) and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

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

/*SCENARIO("MathHelper Get Min at Frequency", "[toolbox]") //Not used in Veins, but supported
{
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0),(), another signal (),() and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.5);

        AnalogueModelList analogueModels;
        analogueModels.push_back(&am1);
        analogueModels.push_back(&am2);

        Signal signal1(spectrum);
        signal1[0] = 10;
        signal1[1] = 20;
        signal1[2] = 30;
        signal1.setCenterFrequencyIndex(2);
        signal1.setAnalogueModelList(&analogueModels);

        Signal signal2 = signal1;

        AirFrameVector airFrames;
        AirFrame* frame1 = new AirFrame(signal1);
        AirFrame* frame2 = new AirFrame(signal2);
        airFrames.push_back(frame1);
        airFrames.push_back(frame2);

        WHEN("searched for global min over the whole time and signals perfectly overlap")
        {
            frame1->getSignal().setTiming(0, 10);
            frame2->getSignal().setTiming(0, 10);
            double min = MathHelper::getMinAtFreq(0, 10, airFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 2.0")
            {
                REQUIRE(frame1->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(frame2->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 2.0);
            }
        }
        WHEN("searched for global min over the whole time and signals only overlap for half of the time")
        {
            frame1->getSignal().setTiming(0, 10);
            frame2->getSignal().setTiming(5, 10);
            double min = MathHelper::getMinAtFreq(0, 15, airFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(frame1->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(frame2->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 1.0);
            }
        }
        WHEN("searched for global min over the whole time and signals do not overlap but are right behind each other")
        {
            frame1->getSignal().setTiming(0, 10);
            frame2->getSignal().setTiming(10, 10);
            double min = MathHelper::getMinAtFreq(0, 20, airFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(frame1->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(frame2->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 1.0);
            }
        }
        deleteAirFrameVector(airFrames);
    }
}*/

/*SCENARIO("MathHelper minimum Value at Frequency and Timestamp", "[toolbox]")
{
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0),(), another signal (),() and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.1);

        AnalogueModelList analogueModels;
        analogueModels.push_back(&am1);
        analogueModels.push_back(&am2);

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
        AirFrame* signalFrame = new AirFrame(signal);
        airFrames.push_back(signalFrame);

        WHEN("check for already higher threshold at a time-stamp in the middle of the signal")
        {
            bool belowThreshold = MathHelper::smallerAtFreqIndex(10, 10, airFrames, 2, 500.0);
            THEN("no AnalogueModel applied and true returned")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 0);
                REQUIRE(belowThreshold == true);
            }
        }
        WHEN("check for already higher threshold at a time-stamp at the start of the signal")
        {
            bool belowThreshold = MathHelper::smallerAtFreqIndex(5, 5, airFrames, 2, 500.0);
            THEN("no AnalogueModel applied and true returned")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 0);
                REQUIRE(belowThreshold == true);
            }
        }
        WHEN("check for already higher threshold at a time-stamp at the end of the signal")
        {
            bool belowThreshold = MathHelper::smallerAtFreqIndex(15, 15, airFrames, 2, 500.0);
            THEN("no AnalogueModel applied and true returned")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 0);
                REQUIRE(belowThreshold == true);
            }
        }
        WHEN("check for a too low threshold at a time-stamp at the end of the signal")
        {
            bool belowThreshold = MathHelper::smallerAtFreqIndex(10, 10, airFrames, 2, 1.0);
            THEN("all AnalogueModels applied and false returned")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(belowThreshold == false);
            }
        }
        WHEN("check for a too low threshold at a time-stamp at the end of the signal")
        {
            bool belowThreshold = MathHelper::smallerAtFreqIndex(5, 5, airFrames, 2, 1.0);
            THEN("all AnalogueModels applied and false returned")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(belowThreshold == false);
            }
        }
        WHEN("check for a too low threshold at a time-stamp at the end of the signal")
        {
            bool belowThreshold = MathHelper::smallerAtFreqIndex(15, 15, airFrames, 2, 1.0);
            THEN("no AnalogueModel applied (as signal not within intervall) and true returned (as signal does not contribute to power)")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 0);
                REQUIRE(belowThreshold == true);
            }
        }
        deleteAirFrameVector(airFrames);
    }
}

SCENARIO("MathHelper Get Min SINR Simple Test Cases", "[toolbox]")
{
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0),(), another signal (),() and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.1);

        AnalogueModelList analogueModels;
        analogueModels.push_back(&am1);
        analogueModels.push_back(&am2);

        Signal signal(spectrum);
        signal[0] = 100;
        signal[1] = 200;
        signal[2] = 300;
        signal.setDataStart(0);
        signal.setDataEnd(2);
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);
        signal.setTiming(5, 10);

        Signal interferer = signal;

        AirFrame* signalFrame = new AirFrame(signal);

        AirFrameVector interfererFrames;
        AirFrame* interfererFrame = new AirFrame(interferer);
        interfererFrames.push_back(interfererFrame);

        WHEN("searched for global min SINR over the whole time and signals perfectly overlap")
        {
            interfererFrame->getSignal().setTiming(5, 10);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 2.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searched for global min over the whole time and signals only overlap for half of the time")
        {
            interfererFrame->getSignal().setTiming(0, 5);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 1.0);
            }
        }
        WHEN("searched for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame->getSignal().setTiming(15, 5);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 1.0);
            }
        }
        WHEN("searched for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame->getSignal().setTiming(7.5, 5);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searched for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame->getSignal().setTiming(5, 5);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searched for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame->getSignal().setTiming(10, 5);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searched for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame->getSignal().setTiming(0, 7.5);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searched for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame->getSignal().setTiming(12.5, 7.5);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.5);
            }
        }
        WHEN("searched for global min over the whole time and signals do not overlap but are right behind each other")
        {
            interfererFrame->getSignal().setTiming(0, 0);
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 1.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 1.0);
            }
        }
        deleteAirFrameVector(interfererFrames);
    }
}

SCENARIO("MathHelper Get Min SINR Complex Test Case", "[toolbox]")
{
    GIVEN("A spectrum with frequencies (1,2,3,4,5,6), a signal (10,20,30,0,0,0),(), another signal (),() and a list with two DummyAnalogueModels (0.1, 0.9)")
    {
        Freqs freqs;
        freqs.push_back(1);
        freqs.push_back(2);
        freqs.push_back(3);
        freqs.push_back(4);
        freqs.push_back(5);
        freqs.push_back(6);

        SpectrumPtr spectrum = Spectrum::getInstance(freqs);

        DummyAnalogueModel am1(0.1);
        DummyAnalogueModel am2(0.1);

        AnalogueModelList analogueModels;
        analogueModels.push_back(&am1);
        analogueModels.push_back(&am2);

        Signal signal(spectrum);
        signal[0] = 100;
        signal[1] = 200;
        signal[2] = 300;
        signal.setDataStart(0);
        signal.setDataEnd(2);
        signal.setCenterFrequencyIndex(2);
        signal.setAnalogueModelList(&analogueModels);
        signal.setTiming(5, 10);

        Signal interferer1 = signal;
        interferer1.setTiming(5, 10);

        Signal interferer2 = signal;
        interferer2.setTiming(0, 5);

        Signal interferer3 = signal;
        interferer3.setTiming(0, 7.5);

        Signal interferer4 = signal;
        interferer4.setTiming(5, 5);

        Signal interferer5 = signal;
        interferer5.setTiming(7.5, 5);

        Signal interferer6 = signal;
        interferer6.setTiming(10, 5);

        Signal interferer7 = signal;
        interferer7.setTiming(12.5, 7.5);

        Signal interferer8 = signal;
        interferer8.setTiming(15, 5);

        AirFrame* signalFrame = new AirFrame(signal);

        AirFrameVector interfererFrames;
        AirFrame* interfererFrame1 = new AirFrame(interferer1);
        AirFrame* interfererFrame2 = new AirFrame(interferer2);
        AirFrame* interfererFrame3 = new AirFrame(interferer3);
        AirFrame* interfererFrame4 = new AirFrame(interferer4);
        AirFrame* interfererFrame5 = new AirFrame(interferer5);
        AirFrame* interfererFrame6 = new AirFrame(interferer6);
        AirFrame* interfererFrame7 = new AirFrame(interferer7);
        AirFrame* interfererFrame8 = new AirFrame(interferer8);

        interfererFrames.push_back(interfererFrame1);
        interfererFrames.push_back(interfererFrame2);
        interfererFrames.push_back(interfererFrame3);
        interfererFrames.push_back(interfererFrame4);
        interfererFrames.push_back(interfererFrame5);
        interfererFrames.push_back(interfererFrame6);
        interfererFrames.push_back(interfererFrame7);
        interfererFrames.push_back(interfererFrame8);

        WHEN("searched for global min SINR over the whole time and signals perfectly overlap")
        {
            double min = MathHelper::getMinSINR(5, 15, signalFrame, interfererFrames, 1);
            THEN("all AnalogueModels applied and min is equal to 2.0")
            {
                REQUIRE(signalFrame->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame1->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame2->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame3->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame4->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame5->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame6->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame7->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(interfererFrame8->getSignal().getNumAnalogueModelsApplied() == 2);
                REQUIRE(min == 0.25);
            }
        }
        deleteAirFrameVector(interfererFrames);
    }
}*/
