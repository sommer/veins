
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2.h"
#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2Message_m.h"

#include "veins/modules/application/ieee1609dot2/ContentUnsecuredData_m.h"
#include "veins/modules/application/ieee1609dot2/ContentSignedData_m.h"
#include "veins/modules/application/ieee1609dot2/ContentEncryptedData_m.h"
#include "veins/modules/application/ieee1609dot2/ContentSignedCertificateRequest_m.h"

#include "veins/modules/application/ieee1609dot2/ContentChoiceType_m.h"


using namespace veins;

std::string Ieee1609Dot2::processSPDU(Ieee1609Dot2Message* spdu)
{
    if(spdu->getData().getProtocolVersion() != 3){
        delete(spdu);
        return nullptr;
    } else {
        int checkType = spdu->getData().getContent().getContentType();

        switch (checkType) {
        case ContentChoiceType::UNSECURE_DATA:
        {
            /*unsecuredData indicates that the content is an OCTET STRING to be consumed outside the
            SDS.*/

            ContentUnsecuredData unsecuredData = spdu->getData().getContent().getUnsecuredData();
            return unsecuredData.getUnsecuredData();
        }
        case ContentChoiceType::SIGNED_DATA:
        {
            /*signedData indicates that the content has been signed according to this standard.*/

            ContentEncryptedData encryptedData = spdu->getData().getContent().getEncryptedData();
            return "signed"; //TODO
        }
        case ContentChoiceType::ENCRYPTED_DATA:
        {
            /*encryptedData indicates that the content has been encrypted according to this standard.*/

            ContentEncryptedData encryptedData = spdu->getData().getContent().getEncryptedData();
            return "encrypted"; //TODO
        }
        case ContentChoiceType::SIGNED_CERTIFICATE_REQUEST:
        {
            /*signedCertificateRequest indicates that the content is a certificate request. Further
            specification of certificate requests is not provided in this version of this standard.*/

            ContentSignedCertificateRequest signedCertificateRequest = spdu->getData().getContent().getSignedCertificateRequest();
            delete(spdu);
            return "sigendCertificateRequest";
        }

        }
    }
}

Ieee1609Dot2Data* Ieee1609Dot2::createSPDU(int type, const char * msg)
{

    Ieee1609Dot2Data* data = new Ieee1609Dot2Data();
    data->setProtocolVersion(3);

    Ieee1609Dot2Content* content = new Ieee1609Dot2Content();

    switch(type){
    case ContentChoiceType::UNSECURE_DATA:
        ContentUnsecuredData* contentUnsecuredData = new ContentUnsecuredData();
        contentUnsecuredData->setUnsecuredData(msg);
        content->setUnsecuredData(*contentUnsecuredData);
        break;
    }

    data->setContent(*content);

    return data;
}

EncryptedData* Ieee1609Dot2::SecEncryptedDataRequest(
            Ieee1609Dot2Data* data,
            int dataType,
            int dataEncryptionKeyType,
            int symmetricCHM,
            CertificateBase* recipientCertificates,
            std::string signedDataRecipientInfo,
            std::string responseEncryptionKey,
            int ecPointFormat
            )
{
    EncryptedData* encryptedData = new EncryptedData();
    encryptedData->setCiphertext(data->getContent().getUnsecuredData().getUnsecuredData());
    encryptedData->setRecipients("recipients");
    return encryptedData;
}

