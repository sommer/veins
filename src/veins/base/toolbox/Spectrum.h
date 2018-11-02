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

#ifndef SPECTRUM_H_
#define SPECTRUM_H_

#include <stdint.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <memory>
#include <fstream>
#include <assert.h>

namespace Veins {

typedef std::vector<double> Freqs;

class Spectrum;
typedef std::shared_ptr<Spectrum> SpectrumPtr;

class Spectrum {
public:
    static SpectrumPtr getInstance(Freqs freqs)
    {
        static std::shared_ptr<Spectrum> instance(new Spectrum(freqs));
        return instance;
    }
    ~Spectrum();

private:
    Spectrum(Freqs freqs);
    Spectrum(Spectrum const&) = delete;
    void operator=(Spectrum const&) = delete;

public:
    const double& operator[](size_t index) const;

    size_t getNumFreqs() const;

    size_t indexOf(double freq) const;
    size_t indexNearLow(double freq);
    size_t indexNearUp(double freq);

    double freqAt(size_t freqIndex) const;

    void print();
    void toFile(std::string path);

private:
    Freqs frequencies;
};

} // namespace Veins

#endif /* SPECTRUM_H_ */
