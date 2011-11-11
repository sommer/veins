package org.mixim.examples.CSMAMac;

import org.mixim.base.modules.BaseNetwLayer;

simple SimpleNetwLayer extends BaseNetwLayer
{
    parameters:
        @class(SimpleNetwLayer);
        
        headerLength = 12 bit;
        double ip;
        bool   isSwitch;
        double maxTtl;
        double boredTime;
}
