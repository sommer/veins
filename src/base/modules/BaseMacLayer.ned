package org.mixim.base.modules;

// Base module for every mac layer module
simple BaseMacLayer extends BaseLayer
{
    parameters:
        @class(BaseMacLayer);
        bool coreDebug; // debug switch
        double headerLength @unit(bit); // length of the MAC packet header (in bits)
}

