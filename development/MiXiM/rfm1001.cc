#include "rfm1001.h"

Define_Module_Like(RFM1001, RadioClass);

void RFM1001::initialize() {
	Radio::initialize();
	printfNoInfo(PRINT_INIT, "\t\tInitializing RFM1001 radio...");
	//transmitters_count = 0;
/*	radio_state = RADIO_LISTEN;
	lp_mode = 1;
	time_since_clear = time_since_listen = simTime();
	time_since_clear = last_stat_time = simTime();
	stat_time_sleep = stat_time_tx = stat_time_rx = stat_time_collision = 0.0;
	stat_time_lb_rx = stat_time_lb_tx = 0.0;

	rx_state = RX_IDLE;
	scheduled_rx = scheduled_hdr_rx = NULL;*/
}

void RFM1001::finish() {
	Radio::finish();
	printfNoInfo(PRINT_INIT, "\t\tEnding RFM1001 radio...");
	// record last part
	//~ record_stats();
	//~ printf(PRINT_STATS, "stats: sleep=%.02lf tx=%.02lf rx=%.02lf collision=%.02lf",
	//~ stat_time_sleep, stat_time_tx,
	//~ stat_time_rx, stat_time_collision);
	//~ printf(PRINT_STATS, "stats: sleep=%.04lf tx=%.04lf rx=%.04lf tx_lb=%.04f rx_lb=%.04f collision=%.04lf", stat_time_sleep, stat_time_tx, stat_time_rx, stat_time_lb_tx, stat_time_lb_rx, stat_time_collision);
}

