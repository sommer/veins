//
// Copyright (C) 2016 Julian Heinovski <julian.heinovski@ccs-labs.org>
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
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef SYNCHRONIZATIONSTATUS_H_
#define SYNCHRONIZATIONSTATUS_H_

/**
 * Represents a Roadside-to-vehicle period information table synchronization status. (ORT.SYN.STA)
 *
 * @author Julian Heinovski
 */
enum SynchronizationStatus {
    // unsynchronized
    // the sender was a mobile station but it is not synchronized
    // 000b, 001b, 010b, 011b,
    UNSYNCHRONIZED = 0,

    // the sender is a base station
    // Use if and only if the sender was synced directly by a base station
    // 100b
    SYNCHRONIZED_DIRECT = 4,

    // The sender was synced indirectly by another mobile station which was synced directly by a base station
    // 101b
    SYNCHRONIZED_ONCE = 5,

    // The sender was synced indirectly by another mobile station which was synced indirectly by another mobile station which was synced directly by a base station
    // 110b
    SYNCHRONIZED_TWICE = 6,

    // The sender was synced indirectly by another mobile station which was synced indirectly by another mobile station which was synced indirectly by another mobile station which was synced directly by a base station
    // 111b
    SYNCHRONIZED_THRICE = 7,
};

inline SynchronizationStatus incrementStatus(SynchronizationStatus s) {
    switch (s) {
    case SYNCHRONIZED_DIRECT:
        return SYNCHRONIZED_ONCE;
    case SYNCHRONIZED_ONCE:
        return SYNCHRONIZED_TWICE;
    case SYNCHRONIZED_TWICE:
        return SYNCHRONIZED_THRICE;
    default:
        // case SYNCHRONIZED_THRICE is not needed because than we determine the ivc control field invalid
        return UNSYNCHRONIZED;
    }
}

inline SynchronizationStatus decrementStatus(SynchronizationStatus s) {
    switch (s) {
    case SYNCHRONIZED_ONCE:
        return SYNCHRONIZED_DIRECT;
    case SYNCHRONIZED_TWICE:
        return SYNCHRONIZED_ONCE;
    case SYNCHRONIZED_THRICE:
        return SYNCHRONIZED_TWICE;
    default:
        // case DIRECT is not needed because than we determine the ivc control field invalid
        return UNSYNCHRONIZED;
    }
}

inline std::ostream& operator<<(std::ostream &out, SynchronizationStatus s) {
    switch (s) {
    case 4:
        return (out << "DIRECT");
    case 5:
        return (out << "ONCE");
    case 6:
        return (out << "TWICE");
    case 7:
        return (out << "THRICE");
    default:
        return (out << "UNSYNCHRONIZED");
    }
}

#endif /* SYNCHRONIZATIONSTATUS_H_ */
