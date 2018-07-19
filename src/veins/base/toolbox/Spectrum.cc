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

#include "veins/base/toolbox/Spectrum.h"

using namespace Veins;

Spectrum::Spectrum(Freqs freqs)
{
    // std::cout << "Spectrum constructed" << std::endl;

    frequencies = freqs;

    std::sort(frequencies.begin(), frequencies.end());
    frequencies.erase(std::unique(frequencies.begin(), frequencies.end()), frequencies.end());
}

Spectrum::~Spectrum()
{
    // std::cout << "Spectrum destructed" << std::endl;

    frequencies.clear();
}

const double& Spectrum::operator[](size_t index) const
{
    return frequencies.at(index);
}

size_t Spectrum::indexOf(double freq) const
{
    // Binary search
    auto it = std::lower_bound(frequencies.begin(), frequencies.end(), freq);
    bool found = it != frequencies.end() && (*it) == freq;

    assert(found == true);

    return std::distance(frequencies.begin(), it);

    // Linear search
    // return std::distance(frequencies.begin(), std::find(frequencies.begin(), frequencies.end(), freq));
}

size_t Spectrum::indexNearLow(double freq)
{
    size_t index = 0;
    for (size_t i = 0; i < frequencies.size(); i++) {
        if (frequencies[i] < freq) index = i;
    }
    return index;
}

size_t Spectrum::indexNearUp(double freq)
{
    size_t index = frequencies.size() - 1;
    for (size_t i = frequencies.size() - 1; i > 0; i--) {
        if (frequencies[i] > freq) index = i;
    }
    return index;
}

double Spectrum::freqAt(size_t freqIndex) const
{
    return frequencies[freqIndex];
}

size_t Spectrum::getNumFreqs() const
{
    return frequencies.size();
}

void Spectrum::print()
{
    for (uint16_t i = 0; i < frequencies.size(); i++) {
        std::cout << frequencies[i] << std::endl;
    }
}

void Spectrum::toFile(std::string path)
{
    std::fstream file;
    file.open(path.c_str(), std::ios::out | std::ios::app);

    if (!file.good()) return;

    file << "spectrum:";
    for (Freqs::iterator it = frequencies.begin(); it != frequencies.end(); ++it) {
        file << *it;
        if (it != frequencies.end() - 1) file << ",";
    }
    file << std::endl;

    file.close();
}
