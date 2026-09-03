package veins.modules.application.ieee80211p;

//
// Bridge module between the Veins mobility layer and the Vanetza library.
// Provides vehicle kinematic data (position, speed, heading, acceleration)
// and ITS station ID for CAM and DENM message construction.
//
simple ApplicationToVanetzaConverter
{
    parameters:
        @display("i=block/network2");
        @class(veins::ApplicationToVanetzaConverter);
}