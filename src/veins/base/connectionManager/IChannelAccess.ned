package org.car2x.veins.base.connectionManager;

// Common parameters for modules based on ChannelAccess class
moduleinterface IChannelAccess
{
    parameters:
        double antennaOffsetX @unit("m"); // Offset of antenna position (x direction) with respect to what a BaseMobility module will tell us
        double antennaOffsetY @unit("m"); // Offset of antenna position (y direction) with respect to what a BaseMobility module will tell us
        double antennaOffsetZ @unit("m"); // Offset of antenna position (z direction) with respect to what a BaseMobility module will tell us
        double antennaOffsetYaw @unit("rad"); // Offset of antenna orientation (yaw) with respect to what a BaseMobility module will tell us
}

