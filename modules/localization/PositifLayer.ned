// The localization module for the BaseLocalization.
//
// This module has to be extended in order to implement a certain localization
// algorithm.
//
// @author Peterpaul Klein Haneveld

// changes PKH: Thu Feb  8 09:26:10 CET 2007
// - added switch to specify a node as an anchor node

simple PositifLayer
{
    parameters:
        bool debug; // debug switch
        bool isAnchor;
        double headerLength; // length of the network packet header (in bits)
        double accuracy;
        //	nr_dims: numeric const,
        //	x_dim: numeric const,
        //	y_dim: numeric const,
        //	z_dim: numeric const,
        //	num_nodes: numeric const,
        double anchor_frac;
        //	range: numeric const,
        double range_variance;
        double pos_variance;
        bool do_2nd_phase;
        double phase1_min_anchors;
        double phase1_max_anchors;
        double flood_limit;
        double refine_limit;
        double tri_alg;
        double var0;
        double var1;
        double var2;
        double repeats;
        bool use_log;
        bool msg_buffering;
        bool use_confs;
        string terrain;
        bool multihop;
        bool equal_pairs;
        string outputfile;
    gates:
        input uppergateIn; // from application layer
        output uppergateOut; // to application layer
        input upperControlIn; // control from application layer
        output upperControlOut; // control to application layer
        input lowergateIn; // from network layer
        output lowergateOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer
}

