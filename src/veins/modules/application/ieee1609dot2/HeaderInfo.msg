//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

//
// TODO generated message class
//

cplusplus
{{
    #include "MissingCrlIdentifier_m.h"
    #include "EncryptionKey_m.h"
    #include "ThreeDLocation_m.h"
    typedef int Psid;
    typedef uint64_t Time64;
}}

class noncobject Psid;
class noncobject Time64;
class ThreeDLocation;

class MissingCrlIdentifier;
class EncryptionKey;

packet HeaderInfo {
    Psid psid;
    Time64 generationTime;
    Time64 expiryTime;
    ThreeDLocation generationLocation;
    char p2pcdLearningRequest[3]; //HashedId3
    MissingCrlIdentifier missingCrlIdentifier;
    EncryptionKey encryptionKey;
}
