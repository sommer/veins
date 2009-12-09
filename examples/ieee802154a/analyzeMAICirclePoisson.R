#!/usr/bin/R
#
# (C) 2009 Jérôme Rousselot, CSEM SA Neuchâtel, Switzerland.
# This script plots data representing the impact of
# MAI on UWB-IR networks with Poisson traffic.
#
library(lattice)
# Parameters
packetSizes <- c("31 bytes")
channels <- c("ghassemzadeh-los", "cm1")
interferers <- 1:9
lambda <- c("lambda=10", "lambda=200")
packetSizeValues <- c(31) 
nbPackets <- 1000
textSize <- 2.5
cfg <- "MAICircleNPoisson"

data <- expand.grid(lambda=lambda, interferers=interferers, 
		packetSizes=packetSizes, channels=channels)
data$ber <- read.table(paste(cfg, "-Average BER.csv", sep=""))$V1
data$perRS <- 1 - read.table(paste(cfg, "-nbReceivedPacketsRS.csv", sep=""))$V1/(nbPackets*data$interferers)
data$perNoRS <- 1 - read.table(paste(cfg, "-nbReceivedPacketsnoRS.csv", sep=""))$V1/(nbPackets*data$interferers)

gridPanel <- function(...) {
  panel.grid(h=-1, v=-1, col="grey")       
  panel.xyplot(...,)
}
yscale.components.log10 <- function(lim, ...) {
    ans <- yscale.components.default(lim = lim, ...)
    tick.at.major <- c(0.001, 0.01, 0.1, 1) # seq(0.1, 1, 0.1) # c(0.25, 0.5, 0.75, 1) # seq(0.2, 1, 0.2)
    ans$left$labels$labels <- c("10⁻³", "10⁻²", "10⁻¹", "1")#tick.at.major
    ans$left$labels$at <- log(tick.at.major, 10)
#    ans$left$ticks$tck <- 1.5
    ans$left$ticks$at  <- log(tick.at.major, 10)
    ans
}

png(filename=paste(cfg, "-Interferers_Lattice-%d.png", sep=""), width=1024, height=768) #, bg="transparent")

# new code for barchart
linePanel <- function(...) {
  panel.grid(h=-1, v=0, col="grey")       
  panel.barchart(...,)
}

figure <- barchart(perRS+perNoRS ~ interferers | lambda*channels, 
    data=data, #type='o',
    xlab = list(label="Interferers", cex=textSize),
    ylab = list(label="Packet error rates", cex=textSize),
    horizontal=FALSE, as.table = TRUE, 
    panel=linePanel,
    par.settings = simpleTheme(pch=c(15,15), cex=4, lwd=4),
    par.strip.text=list(cex=textSize),
    auto.key = list(space="bottom", 
		    text=c("with Reed-Solomon error correction", 
			   "without Reed-Solomon error correction"), 
		    cex=textSize),
    scales = list(y=list(log = 10), cex=textSize),
    yscale.components = yscale.components.log10,
    layout = c(2, 2, 2) # 2 columns, 2 lines, 2 pages
)

print(figure)
dev.off()

