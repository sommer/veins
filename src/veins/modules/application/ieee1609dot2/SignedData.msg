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
#include "SignerIdentifier_m.h"
#include "ToBeSignedData_m.h"
typedef int HashAlgorithm;
typedef int Signature;
}}


class SignerIdentifier;
class ToBeSignedData;


class noncobject HashAlgorithm;
class noncobject Signature;

packet SignedData {
	HashAlgorithm hashId;
	ToBeSignedData tbsData; //modify
	SignerIdentifier signer;
	Signature signature;	
}
