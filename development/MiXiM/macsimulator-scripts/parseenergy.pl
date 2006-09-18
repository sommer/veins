#!/usr/bin/perl

# mA values derived from RFM1000 documentation
# Alter to model other radios

$sleep_cost = 0.02;
$rx_cost = 4.0;
$tx_cost = 10.0;

while(<>) {
	# get used energy
	($time) = m/time=([\d.]*) / or die "time";
	($rsleep) = m/radio_sleep=([\d.]*) / or die "rsleep";
	($rtx) = m/radio_tx=([\d.]*) / or die "rtx";
	($rrx) = m/radio_rx=([\d.]*) / or die "rrx";
	($nodes) = m/nodes=(\d*) / or die "nodes";

	$etotal = $sleep_cost*$rsleep + $rx_cost*$rrx + $tx_cost*$rtx;
	$amp = $etotal / (1.0*$nodes * $time);
	$yval = $amp;

	# which graph is this?
	($mac) = m/mac=(.*?) / or die "mac";
	($msglen) = m/msglen=(\d*) / or die "msglen";

	if ($mac eq "TMac") {
		($flags) = m/\bmac\.flags=(\d*)/ or die "flags";
		if($flags != 0) {
			$suffix = "";
			$suffix .= "-oa" if $flags & 1;
			$suffix .= "-frts" if $flags & 2;
			$suffix .= "-irts" if $flags & 4;
		} else {
			$suffix = "-none";
		}
	} elsif ($mac eq "SMac") {
		($listenTime) = m/\bmac\.listenTime=(\d+)/ or die "listenTime";
		$suffix = "lt$listenTime";
	} else {
		$suffix = "";
	}

	if ( m/\blpl=true\b/ ) {
		$suffix .= "-lpl";
	}

	$graph = $mac . $msglen . $suffix . ".plot";

	# what is the x value?
	($interval) = m/msginterval=([\d.]*)/ or die "freq";
	$xval = 1.0/$interval;

	# remember all this
	$desc = $graph . "-" . $xval;
	$counts{$desc} ++;
	$totals{$desc} += $yval;
	$graphs{$graph} = 1;
	$xvals{$xval} = 1;
}

# now calculate the averages
foreach $i (keys(%counts)) {
	$avgs{$i} = (1.0*$totals{$i})  / (1.0*$counts{$i});
}

# create the output files
foreach $graph (keys(%graphs)) {
	print "processing $graph\n";
	open (OUT, ">$graph") or die "access denied";
	foreach $xval (sort {$a <=> $b} keys(%xvals)) {
		$desc = $graph . "-" . $xval;
		print OUT $xval, " ", $avgs{$desc}, "\n"
			if $avgs{$desc};
	}
	close(OUT);
}

