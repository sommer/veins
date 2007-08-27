// The localization module for the BaseLocalization.
//
// This module has to be extended in order to implement a certain localization
// algorithm.
//
// @author Peterpaul Klein Haneveld

// changes PKH: Thu Feb  8 09:26:10 CET 2007
// - added switch to specify a node as an anchor node

simple PositifLayer
    parameters:
	debug: numeric const, // debug switch
	accuracy: numeric const,
//	nr_dims: numeric const,
//	x_dim: numeric const,
//	y_dim: numeric const,
//	z_dim: numeric const,
//	num_nodes: numeric const,
	anchor_frac: numeric const,
//	range: numeric const,
	range_variance: numeric const,
	pos_variance: numeric const,
	do_2nd_phase: bool,
	phase1_min_anchors: numeric const,
	phase1_max_anchors: numeric const,
	flood_limit: numeric const,
	refine_limit: numeric const,
	tri_alg: numeric const,
	var0: numeric const,
	var1: numeric const,
	var2: numeric const,
	repeats: numeric const,
	use_log: bool,
	msg_buffering: bool,
	use_confs: bool,
	terrain: string,
	multihop: bool,
	equal_pairs: bool,
	anchor: bool,
	outputfile: string,
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

