#!/usr/bin/R
#
# (C) 2009 Jérôme Rousselot, CSEM SA Neuchâtel, Switzerland.
# This script plots data representing the impact of
# MAI on UWB-IR networks.
#
library(lattice)
# Parameters
channels <- c("ghassemzadeh-los",  "ghassemzadeh-nlos", "cm1", "cm5")
interferers <- 1:4
packetSizes <- c("7 bytes", "31 bytes")
packetSizeValues <- c(7, 31) 
nbPackets <- 2500
textSize <- 2.5
repetitions <- 0:4

datareps <- expand.grid(repetitions=repetitions, interferers=interferers, channels=channels, packetSizes=packetSizes)
datareps$ber <- read.table("MAICircleN-Average BER.csv")$V1
datareps$perRS <- read.table("MAICircleN-nbReceivedPacketsRS.csv")$V1  #/(nbPackets*data$interferers)
datareps$perNoRS <- read.table("MAICircleN-nbReceivedPacketsnoRS.csv")$V1 #/(nbPackets*data$interferers)
data <- aggregate(x=list(perRS=datareps$perRS,
			 perNoRS=datareps$perNoRS),
		by = list(interferers=datareps$interferers,
			 channels=datareps$channels, 
			 packetSizes=datareps$packetSizes),
		FUN=sum
		)
data$perRS <- 1 - data$perRS/(nbPackets*data$interferers)
data$perNoRS <- 1 - data$perNoRS/(nbPackets*data$interferers)
data$ber <- aggregate(x=list(ber=datareps$ber),
			by=list(interferers=datareps$interferers,
			 channels=datareps$channels, 
			 packetSizes=datareps$packetSizes),
			FUN=mean
			)$ber


gridPanel <- function(...) {
  panel.grid(h=-1, v=-1, col="grey")       
  panel.xyplot(...,)
}
yscale.components.log10 <- function(lim, ...) {
    ans <- yscale.components.default(lim = lim, ...)
    tick.at.major <- c(0.001, 0.01, 0.1, 1) # seq(0.1, 1, 0.1) # c(0.25, 0.5, 0.75, 1) # seq(0.2, 1, 0.2)
    ans$left$labels$labels <- c("10⁻³", "10⁻²", "10⁻¹", "1")#tick.at.major
    ans$left$labels$at <- log(tick.at.major, 10)
    ans$left$ticks$tck <- 1.5
    ans$left$ticks$at  <- log(tick.at.major, 10)
    ans
}

png(filename="MAICircleN-Interferers_Lattice-%d.png", width=1024, height=768) #, bg="transparent")

linePanel <- function(...) {
  panel.grid(h=-1, v=0, col="grey")       
  panel.barchart(...,)
}

figure <- barchart(perRS+perNoRS ~ interferers | channels*packetSizes, data=data, type='o',
    xlab = list(label="Traffic sources", cex=textSize),
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

