#!/usr/bin/perl

# mA values derived from RFM1000 documentation
# Alter to model other radios

$sleep_cost = 0.02;
$rx_cost = 4.0;
$tx_cost = 10.0;

while(<>) {
	($r_listen) = m/radio_rx=([\d.]*) / or die "radio_rx";
	($r_transmit) = m/radio_tx=([\d.]*) / or die "radio_tx";
	($r_sleep) = m/radio_sleep=([\d.]*) / or die "radio_sleep";
	($r_collision) = m/radio_collision=([\d.]*) / or die "radio_collision";
	($rx_data) = m/mac_rx_data=([\d.]*) / or die "mac_rx_data";
	($rx_overhead) = m/mac_rx_overhead=([\d.]*) / or die "mac_rx_overhead";
	($rx_overhear) = m/mac_rx_overhear=([\d.]*) / or die "mac_rx_overhear";
	($tx_data) = m/mac_tx_data=([\d.]*) / or die "mac_tx_data";
	($tx_overhead) = m/mac_tx_overhead=([\d.]*)/ or die "mac_tx_overhead";
	($mac) = m/mac=(.*?) / or die "mac";
	($msglen) = m/msglen=(\d*) / or die "msglen";
	($msginterval) = m/msginterval=([\d.]*)/ or die "msginterval";

	($time) = m/time=([\d.]*) / or die "time";
	($nodes) = m/nodes=(\d*) / or die "nodes";
	
	$tn = $time * (1.0*$nodes);

	$idle = $r_listen - $rx_data - $rx_overhead - $rx_overhear - $r_collision;

	#$_ = $_. " mac=$mac msglen=$msglen msginterval=$msginterval ";
	print "ek=idle e=", $rx_cost*$idle/$tn, " ", $_;
	print "ek=collision e=", $rx_cost*$r_collision/$tn, " ", $_;
	print "ek=rx_data e=", $rx_cost*$rx_data/$tn, " ", $_;
	print "ek=rx_overhead e=", $rx_cost*$rx_overhead/$tn, " ", $_;
	print "ek=rx_overhear e=", $rx_cost*$rx_overhear/$tn, " ", $_;
	print "ek=tx_data e=", $tx_cost*$tx_data/$tn, " ", $_;
	print "ek=tx_overhead e=", $tx_cost*$tx_overhead/$tn, " ", $_;
	print "ek=sleep e=", $sleep_cost*$r_sleep/$tn, " ", $_;
}


