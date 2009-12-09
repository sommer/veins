png("ebn0_per.png", width=1024, height=768)

lineWidth = 3
symbolSize = 3

# We add a first point at -5 dB, 100% PER for each line

psrRSValues <- c(0.97061, 0.97061, 0.97061, 0.97061, 0.97061, 
		0.97061, 0.97061, 0.969377, 0.963361, 0.907644,
		0.698511, 0.359492, 0.103714, 0.0218919, 0,
		0, 0)

psrNoRSValues <- c(0.964188, 0.965016, 0.969944, 0.941074, 0.866947,
		0.837347,0.698542, 0.511625, 0.282612,  0.0915956,
		0.0334867, 0.00547897, 0.000303711, 0, 0,
		0, 0)
pathlosses <- c(4.6664e-15, 4.2618e-15, 3.89914e-15, 3.5734e-15, 3.28021e-15,
	3.01581e-15, 2.77692e-15, 2.56069e-15, 2.36464e-15, 2.18656e-15,
	2.02457e-15, 1.87696e-15, 1.74225e-15, 1.61914e-15, 1.50647e-15,
	1.40321e-15)
noiseValues <- c(3.26942e-21, 3.26933e-21, 3.26948e-21, 3.26947e-21,  3.26945e-21,
	3.26942e-21, 3.26954e-21, 3.26945e-21, 3.26948e-21, 3.26942e-21,
	3.26961e-21, 3.26987e-21, 3.27003e-21, 3.26944e-21, 3.26952e-21,
	3.26948e-21)

epflPER_EDoptNoDrift <- rev(c(0, 0, 5E-4, 8E-3, 1.5E-1, 7E-1, 1, 1))
epflPER_EDfixDrift <- c(1, 1, 1, 1, 1, 5E-1, 2.5E-2, 2E-3)
epflEbN0 <- c(-5, 8, 9, 10, 11, 12, 13, 14) # dB

Epulse <- 1E-3

EbValues <- pathlosses*Epulse

EbN0Values <- 10*log10( EbValues / noiseValues )
EbN0Values <- c(EbN0Values, -5)

# binSize = 2
perRSValues <- c( 1, 1,  1,  1,  1, 0.325, 0.01, 0.01)
EbN0Values  <- c(-6, 0, 10, 12, 14,    16,   18, 20)
# binSize = 1
perRSValues <- c( 1, 1,  1, 1,   1,  1,  1, 0.64, 0.001, 0.001, 0.001, 0.001)
EbN0Values  <- c(-6, 0, 10, 11, 12, 13, 14,   15,  16,    18,   19, 21)
par(mar= c(5, 6, 4, 2) + 0.1)

plot(c(0, 20),  c(1, 1E-3), type="n", las=1, xlab="Eb/N0 (dB)", ylab="", 
	cex.lab=2, axes=FALSE, cex=2, log="y", lwd=lineWidth) # setup figure
#lines(EbN0Values, 1-psrNoRSValues, type="b", col="darkblue", pch=21, cex=symbolSize, lwd=lineWidth)
#lines(EbN0Values, 1-psrRSValues, type="b", col="darkgreen", pch=22, cex=symbolSize, lwd=lineWidth)

grid(col="grey")
lines(EbN0Values, perRSValues, type="b", col="darkgreen", pch=15, cex=symbolSize, lwd=lineWidth)
lines(epflEbN0, epflPER_EDoptNoDrift, type="l", col="black",  cex=symbolSize, lwd=lineWidth)
lines(epflEbN0, epflPER_EDfixDrift,  type="b", pch=16, col="darkorange", cex=symbolSize, lwd=lineWidth) #23

lines(c(17, 17), c(1.5, 1E-4), lty=5, type="l", col="darkred", cex=symbolSize, lwd=lineWidth)
mtext("Packet Error Rate", side=3, line=1, adj=0, cex=2, lwd=lineWidth)
legend(x="bottomright",legend=c("CSEM MiXiM MPAE",
				"CEA Analytical SNR Limit",
				"EPFL MATLAB EDfix",
				"EPFL MATLAB EDOpt"
				),
	lty=c(1, 5, 1, 1), 
	col=c("darkgreen", "darkred", "darkorange", "black"),
	pch=c(15, NA_integer_, 16, NA_integer_), # no symbol for third and last ones
	cex=2, pt.cex=symbolSize, lwd=lineWidth)
axis(1, at=seq(0,20,5), cex.axis=2)
axis(2, las=1, at=c(1, 0.1, 0.01, 0.001), cex.axis=2, lab=c("1", "10⁻¹", "10⁻²", "10⁻³"))
box()
dev.off()

