module MacClass
	parameters:
		lpl: bool,
		lpl_on: numeric,
		lpl_off: numeric,
		siftNodes: numeric;

    gates:
        in: fromRadio;
        in: fromRouting;
        out: toRadio;
        out: toRouting;
endmodule

