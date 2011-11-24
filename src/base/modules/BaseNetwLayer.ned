package org.mixim.base.modules;

// Base module for every network layer module
simple BaseNetwLayer extends BaseLayer like IBaseNetwLayer
{
    parameters:
        @class(BaseNetwLayer);
        
        bool coreDebug = default(false); // debug switch for core framework
        bool stats = default(false); // stats switch
        int  headerLength @unit(bit); // length of the network packet header (in bits)
}

