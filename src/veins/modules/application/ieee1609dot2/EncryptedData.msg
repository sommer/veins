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

//cplusplus 
//{{
//    typedef char SequenceOfRecipientInfo[10];
//    typedef char SymmetricCiphertext[10];
//}}

//class noncobject SequenceOfRecipientInfo;
//class noncobject SymmetricCiphertext;

packet EncryptedData {
    string recipients; //should be SequenceOfRecipientInfo
    string ciphertext; //SymmetricCiphertext
}
