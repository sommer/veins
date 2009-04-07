package org.mixim.base.modules;

simple BaseNetwLayer extends BaseLayer like IBaseNetwLayer
{
    parameters:
        @class(BaseNetwLayer);
        
        bool debug; // debug switch
        bool stats; // stats switch
        double headerLength @unit(bit); // length of the network packet header (in bits)
}

