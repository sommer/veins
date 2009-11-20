package org.mixim.modules.mac;



simple AlohaMacLayer extends UWBIRMac
{
    parameters:
        double maxRetries;
        int minBE = default(3); // min backoff exponent
        int maxBE = default(8); // max backoff exponent
        @class(AlohaMacLayer);
}

