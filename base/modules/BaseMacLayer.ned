package org.mixim.base.modules;

simple BaseMacLayer extends BaseLayer
{
    parameters:
        @class(BaseMacLayer);
        bool coreDebug; // debug switch
        double headerLength; // length of the MAC packet header (in bits)
}

