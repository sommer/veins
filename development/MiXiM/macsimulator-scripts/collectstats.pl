#!/usr/bin/perl
#
# calculate the statistics from the simulator output

$app_tx = 0;
$app_rx = 0;
$rt_tx = 0;
$rt_rx = 0;
$rt_tx_drop = 0;
$rt_rx_dup = 0;
$mac_rx = 0;
$mac_tx = 0;
$mac_tx_drop = 0;
$mac_rx_data = 0.0;
$mac_rx_overhead = 0.0;
$mac_rx_overhear = 0.0;
$mac_tx_data = 0.0;
$mac_tx_overhead = 0.0;
$radio_sleep = 0.0;
$radio_rx = 0.0;
$radio_tx = 0.0;
$radio_collision = 0.0;
$queue_len = 0;
$own_sched = 0;

$mac = "";
$msglen = -1;
$msginterval = -1;

$max_node_id = -1;
$simtime = 0.0;

$scenarioparams = "";
$macparameters = "";

while(<>) {
	if (@data = m/(idle|active)\d+: stats: tx=(\d+) rx=(\d+)/ ) {
		($kind, $tx, $rx) = @data;
		$app_tx += $tx;
		$app_rx += $rx;
	}
	#~ if (@data = m/nodes\[(\d+)\]\.software\.application: stats: tx=(\d+) rx=(\d+) tx_drop=(\d+) queuelen=(\d+)/ ) {
		#~ ($id,$tx,$rx,$txdrop, $q) = @data;
		#~ if($id > $max_node_id) { $max_node_id = $id; }
		#~ $rt_tx+=$tx;
		#~ $rt_rx+=$rx;
		#~ $rt_tx_drop+=$txdrop;
		#~ $queue_len += $q;
	#~ }
	if (@data = m/nodes\[(\d+)\]\.software\.routing: stats: tx=(\d+) rx=(\d+) tx_drop=(\d+) rx_dup=(\d+) queuelen=(\d+)/ ) {
		($id, $tx, $rx, $txdrop, $rxdup, $q) = @data;
		if($id > $max_node_id) { $max_node_id = $id; }
		$rt_tx+=$tx;
		$rt_rx+=$rx;
		$rt_tx_drop+=$txdrop;
		$rt_rx_dup+=$rxdup;
		$queue_len += $q;
	}
	if(@data = m/mac: stats: rx=(\d+) tx=(\d+) tx_drop=(\d+) s_rx_data=([\d.]*) s_rx_overhead=([\d.]*) s_rx_overhear=([\d.]*) s_tx_data=([\d.]*) s_tx_overhead=([\d.]*)/ ) {
		($rx, $tx, $txdrop, $rxdata, $rxoverhead, $rxoverhear, $txdata, $txoverhead) = @data;
		$mac_rx += $rx;
		$mac_tx += $tx;
		$mac_tx_drop += $txdrop;
		$mac_rx_data += $rxdata;
		$mac_rx_overhead += $rxoverhead;
		$mac_rx_overhear += $rxoverhear;
		$mac_tx_data += $txdata;
		$mac_tx_overhead += $txoverhead;
	} 
	if(@data = m/radio: stats: sleep=([0-9.]+) tx=([0-9.]+) rx=([0-9.]+) tx_lb=([0-9.]+) rx_lb=([0-9.]+) collision=([0-9.]+)/ ) {
		($sleep,$tx,$rx,$tx_lb,$rx_lb,$coll) = @data;
		$radio_sleep += $sleep;
		$radio_tx += $tx;
		$radio_rx += $rx;
		$radio_collision += $coll;
		$radio_tx += $tx_lb;
		$radio_rx += $rx_lb;
	}
	if(@data = m/^ *([\d.]+) /) {
		($t) = @data;
		if($t > $simtime) {
			$simtime = $t;
		}
	}
	if(m/own sched/) {
		$own_sched++;
	}
	if (@data = m/\]\.software\.mac: mac=([A-Za-z\d]*) *( .*)/ ) {
		($mac, $macparameters) = @data;
		($macparameters) =~ s/(\S+)=/mac.\1=/g;
	}
	if (@data = m/net\.scenario: msglen=(\d+) msginterval=([\d.]+) *( .*)/ ) {
		$msglen == -1 or die "Scripts can only handle a single scenario";
		($msglen, $msginterval, $scenarioparams) = @data;
		($scenarioparams) =~ s/(\S+)=/scen.\1=/g;
	}
} # while(<>)

print "time=$simtime nodes=",$max_node_id+1,
	" app_tx=$app_tx app_rx=$app_rx",
	" rt_tx=$rt_tx rt_rx=$rt_rx rt_tx_drop=$rt_tx_drop rt_rx_dup=$rt_rx_dup",
	" mac_tx=$mac_tx",
	" mac_rx=$mac_rx",
	" radio_sleep=$radio_sleep radio_tx=$radio_tx radio_rx=$radio_rx",
	" radio_collision=$radio_collision",
	" in_queue=$queue_len",
	" mac_rx_data=$mac_rx_data mac_rx_overhead=$mac_rx_overhead mac_rx_overhear=$mac_rx_overhear",
	" mac_tx_data=$mac_tx_data mac_tx_overhead=$mac_tx_overhead",
	" own_sched=$own_sched",
	" msglen=$msglen msginterval=$msginterval$scenarioparams",
	" mac=$mac$macparameters",
	"\n";
