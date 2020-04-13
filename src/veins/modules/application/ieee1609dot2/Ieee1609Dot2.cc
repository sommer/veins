
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
            return unsecuredData.getUnsecuredData().getUnsecuredData();
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

Ieee1609Dot2Data* Ieee1609Dot2::createSPDU(int type, Ieee1609Dot2Data* data)
{

    Ieee1609Dot2Data* spdu = new Ieee1609Dot2Data();
    spdu->setProtocolVersion(3);

    switch(type){
    case ContentChoiceType::UNSECURE_DATA:
    {
        spdu->setContent(data->getContent());
        break;
    }
    case ContentChoiceType::ENCRYPTED_DATA:
        {
            Ieee1609Dot2Content* content = new Ieee1609Dot2Content();
            content->setContentType(ContentChoiceType::ENCRYPTED_DATA);
            ContentEncryptedData* contentEncryptedData = new ContentEncryptedData();

            EncryptedData* encryptedData = SecEncryptedDataRequest(
                    data,
                    data->getContent().getContentType(),
                    0,
                    0,
                    std::vector<CertificateBase>(),
                    "",
                    "",
                    0
                    );

            contentEncryptedData->setEncryptedData(*encryptedData);
            content->setEncryptedData(*contentEncryptedData);
            spdu->setContent(*content);

            break;
        }
    case ContentChoiceType::SIGNED_CERTIFICATE_REQUEST:
    {
        Ieee1609Dot2Content* content = new Ieee1609Dot2Content();
        content->setContentType(ContentChoiceType::SIGNED_CERTIFICATE_REQUEST);

        ContentSignedCertificateRequest* contentSigendCertificateRequest = new ContentSignedCertificateRequest();
        contentSigendCertificateRequest->setSignedCertificateRequest("signedCertificateRequest");

        content->setSignedCertificateRequest(*contentSigendCertificateRequest);
        spdu->setContent(*content);
    }
    }

    //data->setContent(*content);

    return spdu;
}

EncryptedData* Ieee1609Dot2::SecEncryptedDataRequest(
            Ieee1609Dot2Data* data,
            int dataType,
            int dataEncryptionKeyType,
            int symmetricCHM,
            std::vector<CertificateBase> recipientCertificates,
            std::string signedDataRecipientInfo,
            std::string responseEncryptionKey,
            int ecPointFormat
            )
{
    EncryptedData* encryptedData = new EncryptedData();
    encryptedData->setCiphertext(data->getContent().getUnsecuredData().getUnsecuredData().getUnsecuredData());
    encryptedData->setRecipients("recipients");
    return encryptedData;
}

EncryptedData* Ieee1609Dot2::SecEncryptedDataConfirm(
            int resultCode,
            Ieee1609Dot2Data* data,
            std::string signedDataRecipientInfo
            )
{

}


