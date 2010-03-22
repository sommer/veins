package org.mixim.tests.basePhyLayer;
import org.mixim.base.phyLayer.BasePhyLayer;

simple TestPhyLayer extends BasePhyLayer
{
    parameters:
    	@class(TestPhyLayer);
    	
        bool testBaseDecider; 			//shall a TestBaseDecider be used as Decider?
}

