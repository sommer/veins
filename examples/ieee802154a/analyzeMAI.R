#!/usr/bin/R
#
# (C) 2009 Jerome Rousselot <jerome.rousselot@csem.ch>, CSEM.
#
# This script plots BER and PSR as a function of the number of interferers
# for various channel models.
#

# Parameters
channels <- c("ghassemzadeh-los",  "ghassemzadeh-nlos", "cm1", "cm5")
packetSizes <- c("7 bytes", "31 bytes")
interferers <- seq(0, 3, 1)
#channels <- c("ghassemzadeh-los", "ghassemzadeh-nlos", "cm1", "cm2")
#channels <- c("ghassemzadeh-los", "ghassemzadeh-nlos")
#distances <- c(1:10, seq(10, 20, 2), seq(25, 50, 5))
#packetSizes <- c("8 bytes", "16 bytes", "32 bytes", "64 bytes", "128 bytes") # , 128
#channels <- c("ghassemzadeh-los", "ghassemzadeh-nlos", "cm1", "cm2", "cm5") #, "cm1", "cm2", "cm5")
#distances <- c(seq(1, 5, 1), 7.5, seq(10, 30, 5))
#packetSizes <- c("7 bytes", "31 bytes", "127 bytes") 


measures <- c("Packet Success Rate (R-S)", "Bit error rate")

latticeFile <- "MAICircle_Lattice-%d.png"

textSize <- 2.4
titleSize = 1.4
axisSize = 2.5

ylims <- list(c(0, 1.1), c(0, 1.1), c(1E-6, 1.1)) # Plot limits 
yticks <- list(c(0, 0.2, 0.4, 0.6, 0.8, 1), 	  # Where to place tick marks
		c(0, 0.2, 0.4, 0.6, 0.8, 1),
		c(1E-6, 1E-3, 1E-2, 1E-1, 0.5))

# Line styles
symbolSize = 3
lineWidth=2
styles <- list() 
cols <- list()
symbs <- list()
cols[channels[1]]="red"  # colors is a reserved keyword
cols[channels[2]]="blue"
cols[channels[3]]="green"
cols[channels[4]]="brown"
cols[channels[5]]="orange"
styles[channels[1]]=1  # 1 (solid) for LOS
styles[channels[2]]=5  # 5 (long-dashed) for non-LOS
styles[channels[3]]=1
styles[channels[4]]=5
styles[channels[5]]=1
symbs[channels[1]]=15  # symbols is a reserved keyword
symbs[channels[2]]=15  # 1 for Ghassemzadeh
symbs[channels[3]]=16  # 2 for IEEE 802.15.4A
symbs[channels[4]]=16
symbs[channels[5]]=16


library(lattice)



loadData <- function(config, param) {
  fileName <- paste(sep="", paste(sep="-", config, param), ".csv")
  data <- read.table(fileName, header=TRUE)
  data
}


# utility function to tell lattice where to mark axes
# from http://lmdvr.r-forge.r-project.org/figures/figures.html Figure 8.4
yscale.components.log10 <- function(lim, ...) {
    ans <- yscale.components.default(lim = lim, ...)
    tick.at.major <- c(0.01, 0.1, 1) # seq(0.1, 1, 0.1) # c(0.25, 0.5, 0.75, 1) # seq(0.2, 1, 0.2)
    ans$left$labels$labels <- c("0.01", "0.1", "1")	# tick.at.major
    ans$left$labels$at <- log(tick.at.major, 10)
    ans$left$ticks$tck <- 1.5
    ans$left$ticks$at  <- log(tick.at.major, 10)
    ans
}


gridPanel <- function(...) {
  panel.grid(h=-1, v=-1, col="grey")       
  panel.xyplot(...,)
}

latticeMAICirclePERPlot <- function(data) {
#  lattice.options(default.theme = col.whitebg)#canonical.theme(color = TRUE), new=TRUE)
  trellis.device(device=png, theme=col.whitebg)
  png(filename=latticeFile, width=2048, height=1536) #, bg="transparent")
  theFigure <- xyplot(
    per + ber ~ interferers | channels*packetSizes, data=data, 
    type="b",
    xlab = list(label="Interferers", cex=textSize),
    ylab = list(label="Error Rates", cex=textSize),
	#    ylim= c(0, 1),
#    cex = textSize,
    par.settings = simpleTheme(pch=c(21,22), cex=4, lwd=2),
    par.strip.text=list(cex=textSize),
#    prepanel=prepanel.loess,
	#    panel=panel.loess,
	#    panel=panel.grid(h=-1, v=-1),
    panel = gridPanel,
	#    aspect ="xy", # change aspect ratio to make lines appear as 45 degrees oriented
    as.table = TRUE, # start drawing from top left to bottom right
	#    layout = c(0, 10), # put 10 figures per page as it pleases you
    layout = c(2, 2, 2), # 2 columns, 2 lines, 2 pages
    auto.key = list(space="bottom", text=c("Packet error rate", "Bit error rate"), type="b", cex=textSize),
	#    key = list(space="bottom", 
	#		text=c("with Reed-Solomon error correction", 
	#			"without Reed-Solomon error correction"),		
	#		cex=textSize, size=14, pch=16, type="b"),
    scales = list(y=list(log = 10), cex=textSize), 
    yscale.components = yscale.components.log10
   )
  print(theFigure)
  dev.off()
}


plotAll <-function() {
  psrFrame <- expand.grid(interferers=interferers, 
			  channels=channels, 
			  packetSizes=packetSizes) 
  psrFrame$per <- 1-loadData(config="maicirclen",param=1)$Estimate
  psrFrame$ber <- loadData(config="maicirclen", param=2)$Estimate
  latticeMAICirclePERPlot(psrFrame)
}

