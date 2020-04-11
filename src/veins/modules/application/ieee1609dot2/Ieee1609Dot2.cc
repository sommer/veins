
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2.h"
#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2Message_m.h"

#include "veins/modules/application/ieee1609dot2/ContentUnsecuredData_m.h"

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
            ContentUnsecuredData unsecuredData = spdu->getData().getContent().getUnsecuredData();
            //EV << "DATA: " << unsecuredData.getUnsecuredData() << "\n";
            //findHost()->getDisplayString().setTagArg("i", 1, "green");
            return unsecuredData.getUnsecuredData();

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

