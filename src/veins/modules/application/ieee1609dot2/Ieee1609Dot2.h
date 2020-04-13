

#pragma once

#include <omnetpp.h>

#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2Message_m.h"
#include "veins/modules/application/ieee1609dot2/EncryptedData_m.h"
#include "veins/modules/application/ieee1609dot2/CertificateBase_m.h"


namespace veins {


class VEINS_API Ieee1609Dot2{
public:
    Ieee1609Dot2() { }
    Ieee1609Dot2Data* createSPDU(int type, const char * content);
    std::string processSPDU(Ieee1609Dot2Message* spdu);

    EncryptedData* SecEncryptedDataRequest(
            Ieee1609Dot2Data* data,
            int dataType,
            int dataEncryptionKeyType,
            int symmetricCHM,
            std::vector<CertificateBase> recipientCertificates,
            std::string signedDataRecipientInfo,
            std::string responseEncryptionKey,
            int ecPointFormat
            );

    EncryptedData* SecEncryptedDataConfirm(
                int resultCode,
                Ieee1609Dot2Data* data,
                std::string signedDataRecipientInfo
                );


};
}
