package org.mixim.base.modules;

moduleinterface IBaseArp {
}

// Module which is responsible for Addressresolution
simple BaseArp like IBaseArp
{
    parameters:
        bool notAffectedByHostState = default(true);
        bool coreDebug = default(false); // debug switch for the core framework
}

