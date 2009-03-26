package org.mixim.base.modules;

simple BaseNetwLayer extends BaseLayer
{
    parameters:
        @class(BaseNetwLayer);
        
        bool debug; // debug switch
        bool stats; // stats switch
        double headerLength; // length of the network packet header (in bits)
}

