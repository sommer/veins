// The localization Layer for Savvides.
//
// This module has to be extended in order to implement a 
// localization algorithm.
//
// @author Peterpaul Klein Haneveld
simple Savvides
    parameters:
        debug: numeric const, // debug switch
	isAnchor: bool,
	refine_limit: numeric const,
	flood_limit: numeric const,
	repeats: numeric const,
	tri_alg: numeric const,
	accuracy: numeric const,
	phase1_min_anchors: numeric const,
	phase1_max_anchors: numeric const,
        headerLength: numeric const; // length of the network packet header (in bits)
    gates:
        in: uppergateIn; // from application layer
        out: uppergateOut; // to application layer
        in: upperControlIn; // control from application layer
        out: upperControlOut; // control to application layer
        in: lowergateIn; // from network layer
        out: lowergateOut; // to network layer
        in: lowerControlIn; // control from network layer
        out: lowerControlOut; // control to network layer
endsimple

