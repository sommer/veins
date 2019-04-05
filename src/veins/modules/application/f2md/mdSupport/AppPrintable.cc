/*******************************************************************************
 * @author  Joseph Kamel
 * @email   josephekamel@gmail.com
 * @date    28/11/2018
 * @version 2.0
 *
 * SCA (Secure Cooperative Autonomous systems)
 * Copyright (c) 2013, 2018 Institut de Recherche Technologique SystemX
 * All rights reserved.
 *******************************************************************************/

#include <veins/modules/application/f2md/mdSupport/AppPrintable.h>

AppPrintable::AppPrintable(const char* name) {
    strcpy(this->name, name);

    flagsRangePlausibility_1 = 0;
    flagsPositionPlausibility_1 = 0;
    flagsSpeedPlausibility_1 = 0;
    flagsPositionConsistancy_1 = 0;
    flagsPositionSpeedConsistancy_1 = 0;
    flagsPositionSpeedMaxConsistancy_1 = 0;
    flagsSpeedConsistancy_1 = 0;
    flagsBeaconFrequency_1 = 0;
    flagsIntersection_1 = 0;
    flagsSuddenAppearence_1 = 0;
    flagsSuddenAppearence_1 = 0;
    flagsPositionHeadingConsistancy_1 = 0;
    flagskalmanPSCP_1 = 0;
    flagskalmanPSCS_1 = 0;
    flagskalmanPSCSP_1 = 0;
    flagskalmanPSCSS_1 = 0;
    flagskalmanPCS_1 = 0;
    flagskalmanPACS_1 = 0;
    flagskalmanSCC_1 = 0;

    flagsRangePlausibility_2 = 0;
    flagsPositionPlausibility_2 = 0;
    flagsSpeedPlausibility_2 = 0;
    flagsPositionConsistancy_2 = 0;
    flagsPositionSpeedConsistancy_2 = 0;
    flagsPositionSpeedMaxConsistancy_2 = 0;
    flagsSpeedConsistancy_2 = 0;
    flagsBeaconFrequency_2 = 0;
    flagsIntersection_2 = 0;
    flagsSuddenAppearence_2 = 0;
    flagsPositionHeadingConsistancy_2 = 0;
    flagskalmanPSCP_2 = 0;
    flagskalmanPSCS_2 = 0;
    flagskalmanPSCSP_2 = 0;
    flagskalmanPSCSS_2 = 0;
    flagskalmanPCS_2 = 0;
    flagskalmanPACS_2 = 0;
    flagskalmanSCC_2 = 0;

    cumulFlags_1 = 0;
    cumulFlags_2 = 0;

    allTests_1 = 0;
    allTests_2 = 0;
}

AppPrintable::AppPrintable() {
    flagsRangePlausibility_1 = 0;
    flagsPositionPlausibility_1 = 0;
    flagsSpeedPlausibility_1 = 0;
    flagsPositionConsistancy_1 = 0;
    flagsPositionSpeedConsistancy_1 = 0;
    flagsPositionSpeedMaxConsistancy_1 = 0;
    flagsSpeedConsistancy_1 = 0;
    flagsBeaconFrequency_1 = 0;
    flagsIntersection_1 = 0;
    flagsSuddenAppearence_1 = 0;
    flagsSuddenAppearence_1 = 0;
    flagsPositionHeadingConsistancy_1 = 0;
    flagskalmanPSCP_1 = 0;
    flagskalmanPSCS_1 = 0;
    flagskalmanPSCSP_1 = 0;
    flagskalmanPSCSS_1 = 0;
    flagskalmanPCS_1 = 0;
    flagskalmanPACS_1 = 0;
    flagskalmanSCC_1 = 0;

    flagsRangePlausibility_2 = 0;
    flagsPositionPlausibility_2 = 0;
    flagsSpeedPlausibility_2 = 0;
    flagsPositionConsistancy_2 = 0;
    flagsPositionSpeedConsistancy_2 = 0;
    flagsPositionSpeedMaxConsistancy_2 = 0;
    flagsSpeedConsistancy_2 = 0;
    flagsBeaconFrequency_2 = 0;
    flagsIntersection_2 = 0;
    flagsSuddenAppearence_2 = 0;
    flagsPositionHeadingConsistancy_2 = 0;
    flagskalmanPSCP_2 = 0;
    flagskalmanPSCS_2 = 0;
    flagskalmanPSCSP_2 = 0;
    flagskalmanPSCSS_2 = 0;
    flagskalmanPCS_2 = 0;
    flagskalmanPACS_2 = 0;
    flagskalmanSCC_2 = 0;

    cumulFlags_1 = 0;
    cumulFlags_2 = 0;

    allTests_1 = 0;
    allTests_2 = 0;
}

void AppPrintable::setName(const char* name) {
    strcpy(this->name, name);
}

void AppPrintable::incAll(mbTypes::Mbs mbType) {
    switch (mbType) {
    case mbTypes::Genuine: {
        allTests_1++;
    }
        break;
    case mbTypes::GlobalAttacker: {
        allTests_1++;
    }
        break;
    case mbTypes::LocalAttacker: {
        allTests_2++;
    }
        break;
    }
}

void AppPrintable::incCumulFlags(mbTypes::Mbs mbType) {

    switch (mbType) {
    case mbTypes::Genuine: {
        cumulFlags_1++;
    }
        break;
    case mbTypes::GlobalAttacker: {
        cumulFlags_1++;
    }
        break;
    case mbTypes::LocalAttacker: {
        cumulFlags_2++;
    }
        break;
    }
}

void AppPrintable::printOutDebug() {

    std::cout << "flagsRangePlausibility_1 " << flagsRangePlausibility_1
            << "\n";
    std::cout << "flagsPositionPlausibility_1 " << flagsPositionPlausibility_1
            << "\n";
    std::cout << "flagsSpeedPlausibility_1 " << flagsSpeedPlausibility_1
            << "\n";
    std::cout << "flagsPositionConsistancy_1 " << flagsPositionConsistancy_1
            << "\n";
    std::cout << "flagsPositionSpeedConsistancy_1 "
            << flagsPositionSpeedConsistancy_1 << "\n";
    std::cout << "flagsPositionSpeedMaxConsistancy_1 "
            << flagsPositionSpeedMaxConsistancy_1 << "\n";
    std::cout << "flagsSpeedConsistancy_1 " << flagsSpeedConsistancy_1 << "\n";
    std::cout << "flagsBeaconFrequency_1 " << flagsBeaconFrequency_1 << "\n";
    std::cout << "flagsIntersection_1 " << flagsIntersection_1 << "\n";
    std::cout << "flagsRangePlausibility_1 " << flagsRangePlausibility_1
            << "\n";
    std::cout << "flagsPositionHeadingConsistancy_1 "
            << flagsPositionHeadingConsistancy_1 << "\n";

    std::cout << "flagskalmanPSCP_1 " << flagskalmanPSCP_1 << "\n";
    std::cout << "flagskalmanPSCS_1 " << flagskalmanPSCS_1 << "\n";
    std::cout << "flagskalmanPSCSP_1 " << flagskalmanPSCSP_1 << "\n";
    std::cout << "flagskalmanPSCSS_1 " << flagskalmanPSCSS_1 << "\n";
    std::cout << "flagskalmanPCS_1 " << flagskalmanPCS_1 << "\n";
    std::cout << "flagskalmanPACS_1 " << flagskalmanPACS_1 << "\n";
    std::cout << "flagskalmanSCC_1 " << flagskalmanSCC_1 << "\n";

    std::cout << "cumulFlags_1 " << cumulFlags_1 << "\n";
    std::cout << "allTests_1 " << allTests_1 << "\n";

    std::cout << "cumulFlags_2 " << cumulFlags_2 << "\n";
    std::cout << "allTests_2 " << allTests_2 << "\n";
}

void AppPrintable::incFlags(mdChecksTypes::Checks check, mbTypes::Mbs mbType) {

    switch (mbType) {
    case mbTypes::Genuine: {
        switch (check) {
        case mdChecksTypes::RangePlausibility:
            flagsRangePlausibility_1++;
            break;
        case mdChecksTypes::PositionPlausibility:
            flagsPositionPlausibility_1++;
            break;
        case mdChecksTypes::SpeedPlausibility:
            flagsSpeedPlausibility_1++;
            break;
        case mdChecksTypes::PositionConsistancy:
            flagsPositionConsistancy_1++;
            break;
        case mdChecksTypes::PositionSpeedConsistancy:
            flagsPositionSpeedConsistancy_1++;
            break;
        case mdChecksTypes::PositionSpeedMaxConsistancy:
            flagsPositionSpeedMaxConsistancy_1++;
            break;
        case mdChecksTypes::SpeedConsistancy:
            flagsSpeedConsistancy_1++;
            break;
        case mdChecksTypes::BeaconFrequency:
            flagsBeaconFrequency_1++;
            break;
        case mdChecksTypes::Intersection:
            flagsIntersection_1++;
            break;
        case mdChecksTypes::SuddenAppearence:
            flagsSuddenAppearence_1++;
            break;
        case mdChecksTypes::PositionHeadingConsistancy:
            flagsPositionHeadingConsistancy_1++;
            break;
        case mdChecksTypes::kalmanPSCP:
            flagskalmanPSCP_1++;
            break;
        case mdChecksTypes::kalmanPSCS:
            flagskalmanPSCS_1++;
            break;
        case mdChecksTypes::kalmanPSCSP:
            flagskalmanPSCSP_1++;
            break;
        case mdChecksTypes::kalmanPSCSS:
            flagskalmanPSCSS_1++;
            break;
        case mdChecksTypes::kalmanPCC:
            flagskalmanPCS_1++;
            break;
        case mdChecksTypes::kalmanPACS:
            flagskalmanPACS_1++;
            break;
        case mdChecksTypes::kalmanSCC:
            flagskalmanSCC_1++;
            break;

        }
    }
        break;
    case mbTypes::GlobalAttacker: {
        switch (check) {
        case mdChecksTypes::RangePlausibility:
            flagsRangePlausibility_1++;
            break;
        case mdChecksTypes::PositionPlausibility:
            flagsPositionPlausibility_1++;
            break;
        case mdChecksTypes::SpeedPlausibility:
            flagsSpeedPlausibility_1++;
            break;
        case mdChecksTypes::PositionConsistancy:
            flagsPositionConsistancy_1++;
            break;
        case mdChecksTypes::PositionSpeedConsistancy:
            flagsPositionSpeedConsistancy_1++;
            break;
        case mdChecksTypes::PositionSpeedMaxConsistancy:
            flagsPositionSpeedMaxConsistancy_1++;
            break;
        case mdChecksTypes::SpeedConsistancy:
            flagsSpeedConsistancy_1++;
            break;
        case mdChecksTypes::BeaconFrequency:
            flagsBeaconFrequency_1++;
            break;
        case mdChecksTypes::Intersection:
            flagsIntersection_1++;
            break;
        case mdChecksTypes::SuddenAppearence:
            flagsSuddenAppearence_1++;
            break;
        case mdChecksTypes::PositionHeadingConsistancy:
            flagsPositionHeadingConsistancy_1++;
            break;
        case mdChecksTypes::kalmanPSCP:
            flagskalmanPSCP_1++;
            break;
        case mdChecksTypes::kalmanPSCS:
            flagskalmanPSCS_1++;
            break;
        case mdChecksTypes::kalmanPSCSP:
            flagskalmanPSCSP_1++;
            break;
        case mdChecksTypes::kalmanPSCSS:
            flagskalmanPSCSS_1++;
            break;
        case mdChecksTypes::kalmanPCC:
            flagskalmanPCS_1++;
            break;
        case mdChecksTypes::kalmanPACS:
            flagskalmanPACS_1++;
            break;
        case mdChecksTypes::kalmanSCC:
            flagskalmanSCC_1++;
            break;
        }
    }
        break;
    case mbTypes::LocalAttacker: {
        switch (check) {
        case mdChecksTypes::RangePlausibility:
            flagsRangePlausibility_2++;
            break;
        case mdChecksTypes::PositionPlausibility:
            flagsPositionPlausibility_2++;
            break;
        case mdChecksTypes::SpeedPlausibility:
            flagsSpeedPlausibility_2++;
            break;
        case mdChecksTypes::PositionConsistancy:
            flagsPositionConsistancy_2++;
            break;
        case mdChecksTypes::PositionSpeedConsistancy:
            flagsPositionSpeedConsistancy_2++;
            break;
        case mdChecksTypes::PositionSpeedMaxConsistancy:
            flagsPositionSpeedMaxConsistancy_2++;
            break;
        case mdChecksTypes::SpeedConsistancy:
            flagsSpeedConsistancy_2++;
            break;
        case mdChecksTypes::BeaconFrequency:
            flagsBeaconFrequency_2++;
            break;
        case mdChecksTypes::Intersection:
            flagsIntersection_2++;
            break;
        case mdChecksTypes::SuddenAppearence:
            flagsSuddenAppearence_2++;
            break;
        case mdChecksTypes::PositionHeadingConsistancy:
            flagsPositionHeadingConsistancy_2++;
            break;
        case mdChecksTypes::kalmanPSCP:
            flagskalmanPSCP_2++;
            break;
        case mdChecksTypes::kalmanPSCS:
            flagskalmanPSCS_2++;
            break;
        case mdChecksTypes::kalmanPSCSP:
            flagskalmanPSCSP_2++;
            break;
        case mdChecksTypes::kalmanPSCSS:
            flagskalmanPSCSS_2++;
            break;
        case mdChecksTypes::kalmanPCC:
            flagskalmanPCS_2++;
            break;
        case mdChecksTypes::kalmanPACS:
            flagskalmanPACS_2++;
            break;
        case mdChecksTypes::kalmanSCC:
            flagskalmanSCC_2++;
            break;
        }
    }
        break;
    }
}

void AppPrintable::resetAll() {
    flagsRangePlausibility_1 = 0;
    flagsPositionPlausibility_1 = 0;
    flagsSpeedPlausibility_1 = 0;
    flagsPositionConsistancy_1 = 0;
    flagsPositionSpeedConsistancy_1 = 0;
    flagsPositionSpeedMaxConsistancy_1 = 0;
    flagsSpeedConsistancy_1 = 0;
    flagsBeaconFrequency_1 = 0;
    flagsIntersection_1 = 0;
    flagsSuddenAppearence_1 = 0;
    flagsPositionHeadingConsistancy_1 = 0;
    flagskalmanPSCP_1 = 0;
    flagskalmanPSCS_1 = 0;
    flagskalmanPSCSP_1 = 0;
    flagskalmanPSCSS_1 = 0;
    flagskalmanPCS_1 = 0;
    flagskalmanPACS_1 = 0;
    flagskalmanSCC_1 = 0;

    flagsRangePlausibility_2 = 0;
    flagsPositionPlausibility_2 = 0;
    flagsSpeedPlausibility_2 = 0;
    flagsPositionConsistancy_2 = 0;
    flagsPositionSpeedConsistancy_2 = 0;
    flagsPositionSpeedMaxConsistancy_2 = 0;
    flagsSpeedConsistancy_2 = 0;
    flagsBeaconFrequency_2 = 0;
    flagsIntersection_2 = 0;
    flagsSuddenAppearence_2 = 0;
    flagsPositionHeadingConsistancy_2 = 0;
    flagskalmanPSCP_2 = 0;
    flagskalmanPSCS_2 = 0;
    flagskalmanPSCSP_2 = 0;
    flagskalmanPSCSS_2 = 0;
    flagskalmanPCS_2 = 0;
    flagskalmanPACS_2 = 0;
    flagskalmanSCC_2 = 0;

    cumulFlags_1 = 0;
    allTests_1 = 0;

    cumulFlags_2 = 0;
    allTests_2 = 0;
}

void AppPrintable::writeFile(std::string path, char* printStr) {
    ofstream outFile;
    if (printInit) {
        outFile.open(path, std::ofstream::out);
        printInit = false;
    } else {
        outFile.open(path,
                std::ofstream::out | std::ofstream::app | std::ofstream::ate);
    }

    outFile.seekp(0, std::ios::end);
    outFile << printStr << "\n";
    outFile.close();
}

void AppPrintable::getPrintable(char* outStr, double density, double deltaT,
        bool printOut) {
    char line[1024] = "";
    char data[64] = "";

    strcat(line, name); //1
    strcat(line, " ");
    sprintf(data, "%f", deltaT);//2
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", density); //3
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", cumulFlags_1); //4
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", allTests_1); //5
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", cumulFlags_2); //6
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", allTests_2); //7
    strcat(line, data);
    for (int i = 0; i < 1024; ++i) {
        outStr[i] = line[i];
    }

    if (printOut) {
        std::cout << "+++++++++++ " << name << " +++++++++++" << " Genuine:"
                << cumulFlags_1 << "/" << allTests_1 << " "
                << cumulFlags_1 / allTests_1 * 100 << "% Attackers:"
                << cumulFlags_2 << "/" << allTests_2 << " "
                << cumulFlags_2 / allTests_2 * 100 << "%" << '\n';
    }
}

void AppPrintable::getFlagsPrintable(char* outStr, double density,
        double deltaT, bool printOut) {
    char line[1024] = "";
    char data[64] = "";

    strcat(line, name); //1
    strcat(line, " ");
    sprintf(data, "%f", flagsRangePlausibility_1); //2
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionPlausibility_1); //3
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsSpeedPlausibility_1); //4
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionConsistancy_1); //5
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionSpeedConsistancy_1); //6
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionSpeedMaxConsistancy_1); //7
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsSpeedConsistancy_1); //8
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsBeaconFrequency_1); //9
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsIntersection_1); //10
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsSuddenAppearence_1); //11
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionHeadingConsistancy_1); //12
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPSCP_1); //13
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPSCS_1); //14
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPSCSP_1); //15
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPSCSS_1); //16
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPCS_1); //17
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPACS_1); //18
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanSCC_1); //19
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsRangePlausibility_2); //20
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionPlausibility_2); //21
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsSpeedPlausibility_2); //22
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionConsistancy_2); //23
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionSpeedConsistancy_2); //24
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionSpeedMaxConsistancy_2); //25
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsSpeedConsistancy_2); //26
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsBeaconFrequency_2); //27
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsIntersection_2); //28
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsSuddenAppearence_2); //29
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagsPositionHeadingConsistancy_2); //30
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPSCP_2); //31
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPSCS_2); //32
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPSCSP_2); //33
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPSCSS_2); //34
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPCS_2); //35
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanPACS_2); //36
    strcat(line, data);
    strcat(line, " ");
    sprintf(data, "%f", flagskalmanSCC_2); //37
    strcat(line, data);

    for (int i = 0; i < 1024; ++i) {
        outStr[i] = line[i];
    }

    if (printOut) {

//    std::cout << "+++++++++++ " << name << " +++++++++++" << " Factor_1:"
//            << cumulFlags_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsRangePlausibility_1:"
//            << flagsRangePlausibility_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsPositionPlausibility_1:"
//            << flagsPositionPlausibility_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsSpeedPlausibility_1:"
//            << flagsSpeedPlausibility_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsPositionConsistancy_1:"
//            << flagsPositionConsistancy_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsPositionSpeedConsistancy_1:"
//            << flagsPositionSpeedConsistancy_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsPositionHeadingConsistancy_1:"
//            << flagsPositionHeadingConsistancy_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsSpeedConsistancy_1:"
//            << flagsSpeedConsistancy_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsBeaconFrequency_1:" << flagsBeaconFrequency_1 / allTests_1 * 100
//            << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++" << " flagsIntersection_1:"
//            << flagsIntersection_1 / allTests_1 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++" << " flagsSuddenAppearence_1:"
//            << flagsSuddenAppearence_1 / allTests_1 * 100 << "%" << '\n';

//    std::cout << "+++++++++++ " << name << " +++++++++++" << " Factor_2:"
//            << cumulFlags_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsRangePlausibility_2:"
//            << flagsRangePlausibility_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsPositionPlausibility_2:"
//            << flagsPositionPlausibility_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsSpeedPlausibility_2:"
//            << flagsSpeedPlausibility_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsPositionConsistancy_2:"
//            << flagsPositionConsistancy_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsPositionSpeedConsistancy_2:"
//            << flagsPositionSpeedConsistancy_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsSpeedConsistancy_2:"
//            << flagsSpeedConsistancy_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsPositionHeadingConsistancy_2:"
//            << flagsPositionHeadingConsistancy_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++"
//            << " flagsBeaconFrequency_2:" << flagsBeaconFrequency_2 / allTests_2 * 100
//            << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++" << " flagsIntersection_2:"
//            << flagsIntersection_2 / allTests_2 * 100 << "%" << '\n';
//    std::cout << "++++++++++ " << name << " ++++++++++" << " flagsSuddenAppearence_2:"
//            << flagsSuddenAppearence_2 / allTests_2 * 100 << "%" << '\n';
    }
}

