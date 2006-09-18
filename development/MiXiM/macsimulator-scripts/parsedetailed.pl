#!/usr/bin/perl
#
while(<>) {
	# which graph is this?

	#print "line",$_;
	($msglen) = m/msglen=(\d+)/ or die "msglen";
	($freq) = m/msginterval=([\d.]*)/ or die "freq";
	$graph = $msglen . "-" . $freq;

	# what is the x value?
	($mac) = m/mac=(.*?) / or die "mac";
	$xval = $mac;

	# what is the y value?
	#print $_;
	($ek) = m/ek=(.*?) / or die "ek";
	$yval = $ek;
	
	# what is the pivot value
	($e) = m/ e=([\de\-.]*) / or die "e";
	$pval = $e;

	# remember all this
	$desc = $graph . "-" . $xval . "-" . $yval;
	$counts{$desc} ++;
	$totals{$desc} += $pval;
	$graphs{$graph} = 1;
	$xvals{$xval} = 1;
	$yvals{$yval} = 1;
}

# now calculate the averages
foreach $i (keys(%counts)) {
	$avgs{$i} = (1.0*$totals{$i})  / (1.0*$counts{$i});
}

#generate output files
#
foreach $graph (keys(%graphs)) {
	print "processing $graph\n";
	open(OUT, ">" . $graph . ".csv") or die "unable to open file";

#print header
foreach $yval (keys(%yvals)) {
	print OUT ",$yval";
}
print OUT "\n";

#print rows
foreach $xval (sort {$a <=> $b} keys(%xvals)) {
	print OUT $xval;
	foreach $yval (keys(%yvals)) {
		$desc = $graph. "-" . $xval . "-". $yval;
		print OUT ",", $avgs{$desc};
	}
	print OUT "\n";
}

close(OUT);

}
