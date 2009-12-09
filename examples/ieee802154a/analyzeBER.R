#!/usr/bin/R
#
# (C) 2009 Jerome Rousselot <jerome.rousselot@csem.ch>, CSEM.
#
# This script plots BER and PSR as a function of distance
# for various channel models.
# Packet Success Rate is plotted with and without Reed-Solomon error correction.
#

# Parameters
#channels <- c("ghassemzadeh-los",  "ghassemzadeh-nlos", "cm1", "cm2", "cm5")
#channels <- c("ghassemzadeh-los", "ghassemzadeh-nlos", "cm1", "cm2")
#channels <- c("ghassemzadeh-los", "ghassemzadeh-nlos")
#distances <- c(1:10, seq(10, 20, 2), seq(25, 50, 5))
#packetSizes <- c("8 bytes", "16 bytes", "32 bytes", "64 bytes", "128 bytes") # , 128
channels <- c("ghassemzadeh-los", "ghassemzadeh-nlos", "cm1", "cm2") #, "cm1", "cm2", "cm5")
distances <- c(seq(1, 5, 1), 7.5, seq(10, 30, 5))
packetSizes <- c("7 bytes", "31 bytes", "127 bytes") 
#packetSizes <- c("7 bytes", "31 bytes")
noAkaFields <- c("Average BER", "nbReceivedPacketsRS", "nbReceivedPacketsnoRS")
measures <- c("Packet Success Rate (R-S)", "Packet Success Rate (No R-S)", "Bit error rate")

outputFiles <- c("PSR_Distance_RS.png", "PSR_Distance_NoRS.png", "BER_Distance.png")
latticeFile <- "Distance_Lattice.png"

textSize <- 3.4

ylabels <- c("Packet delivery success rate\n(with Reed-Solomon error correction)",
	     "Packet delivery success rate\n(without Reed-Solomon error correction)",
	     "Bit error rate")
xlabel <- "Distance (meters)"
ylims <- list(c(0, 1.1), c(0, 1.1), c(1E-6, 1.1)) # Plot limits 
yticks <- list(c(0, 0.2, 0.4, 0.6, 0.8, 1), 	  # Where to place tick marks
		c(0, 0.2, 0.4, 0.6, 0.8, 1),
		c(1E-6, 1E-3, 1E-2, 1E-1, 0.5))




# annotations
#title="Robustness of IEEE 802.15.4A UWB-IR to Multiple Access Interference"
title=""
xtitle="Number of simultaneous transmissions"
ytitle="Packet Reception Success Rate"
titleSize = 1.4
axisSize = 2.5

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



loadData <- function(param) {
  fileName <- paste(sep="", paste(sep="-", "berdistance", param), ".csv")
  data <- read.table(fileName, header=TRUE)
  data
}

loadDataNoAkaroa <- function(param) {
  fileName <- paste(sep="", paste(sep="-", "BERDistance", param), ".csv")
  data <- read.table(fileName, header=FALSE)
  data$V1
}

plotPSR <- function(psr, line) {
  par(las=1)
  #par("cex.main"=titleSize)
  par("cex.sub"=axisSize) # unused
  par("cex.axis"=axisSize)
  par("cex.lab"=axisSize)
  i <- 1
  for (channel in channels) {
    indices <- seq(1+(i-1)*length(distances), i*length(distances), 1)
    if(i == 1) {
      plot(distances, psr$Estimate[indices], type="b", 
	xlab=xlabel, ylab="", las = 1, log="x",
	ylim = ylims[[1]], 
    #	xaxp = c(),
	yaxt = "n",		# do not add a vertical axis yet
	 lty=styles[[i]],  pch=symbs[[i]], col= cols[[i]], # style
         cex=symbolSize, lwd=lineWidth)
    } else {
      lines(distances, psr$Estimate[indices], type="b", 
	#xlab=xlabel, ylab="", las = 1,
	# ylim = ylims[[1]], 
    #	xaxp = c(),
	#yaxt = "n",		# do not add a vertical axis yet
	 lty=styles[[i]],  pch=symbs[[i]], col= cols[[i]], # style
         cex=symbolSize, lwd=lineWidth)
     }
     i <- i + 1
  }
    # Add no-RS results

    # y axis tick positions and axis label
    axis(side=2, at=yticks[[1]] )
    mtext(ylabels[1], side=3,  outer=TRUE, adj=0.02, 
 	line =0, padj=1, cex=axisSize)
    # Add a grid
    grid(col="grey")
    dev.off()
}

# utility function to tell lattice where to mark axes
# from http://lmdvr.r-forge.r-project.org/figures/figures.html Figure 8.4
yscale.components.log10 <- function(lim, ...) {
    ans <- yscale.components.default(lim = lim, ...)
    tick.at.major <- c(0.001, 0.01, 0.1, 1) # seq(0.1, 1, 0.1) # c(0.25, 0.5, 0.75, 1) # seq(0.2, 1, 0.2)
    ans$left$labels$labels <- c("10⁻³", "10⁻²", "10⁻¹", "1") #tick.at.major
    ans$left$labels$at <- log(tick.at.major, 10)
    ans$left$ticks$tck <- 1.5
    ans$left$ticks$at  <- log(tick.at.major, 10)
    ans
}

gridPanel <- function(...) {
  panel.grid(h=-1, v=-1, col="grey")       
  panel.xyplot(...,)
}

latticePSRPlot <- function(data) {
  textSize <- 3
#  lattice.options(default.theme = col.whitebg)#canonical.theme(color = TRUE), new=TRUE)
  trellis.device(device=png, theme=col.whitebg)
  png(filename="PSR-Distance_Lattice-%d.png", width=1280, height=1024) # width=2048, height=1536) #, bg="transparent")
  theFigure <- xyplot(
    perRS + perNoRS ~ distances | packetSizes*channels, data=data, 
    type="b",
    xlab = list(label="Distance (meters)", cex=textSize),
    ylab = list(label="Packet Error Rate", cex=textSize),
	#    ylim= c(0, 1),
#    cex = textSize,
    par.settings = simpleTheme(pch=c(15, 16), cex=3, lwd=4),
    par.strip.text=list(cex=textSize),
#    prepanel=prepanel.loess,
	#    panel=panel.loess,
	#    panel=panel.grid(h=-1, v=-1),
    panel = gridPanel,
	#    aspect ="xy", # change aspect ratio to make lines appear as 45 degrees oriented
    as.table = TRUE, # start drawing from top left to bottom right
	#    layout = c(0, 10), # put 10 figures per page as it pleases you
    layout = c(3, 2, 3), # 3 columns, 2 lines, 3 pages
    auto.key = list(space="bottom", text=c("with Reed-Solomon error correction", "without Reed-Solomon error correction"), cex=textSize),
	#    key = list(space="bottom", 
	#		text=c("with Reed-Solomon error correction", 
	#			"without Reed-Solomon error correction"),		
	#		cex=textSize, size=14, pch=16, type="b"),
    scales = list(y=list(log = 10), cex=textSize), 
    yscale.components = yscale.components.log10
   )
  print(theFigure)
  dev.off()
  dev.off()
}

latticePSREfficiency <- function(psrFrame) {
  png("RSEfficiency-lattice-%d.png", width=2048, height=1536)
  theFigure <- xyplot(RSEfficiency ~ distances | packetSizes*channels, 
		data=psrFrame, 
		type="b",
		as.table=T,
	        layout = c(2, 2, 3),
		scales=list(x=list(log=10), y=list(log=10), cex=textSize), 
#		xlab = list(label="Distance (meters)", cex=textSize),
    		ylab = list(label="Bit Error Rate", cex=textSize),
		panel=gridPanel,
		cex=textSize,
		par.strip.text=list(cex=textSize),
		yscale.components = yscale.components.log10
		)
  print(theFigure)
  dev.off()
}

latticeBER <- function(psrFrame) {
#  textSize = textSize + 1
  trellis.device(device=png, theme=col.whitebg)
  png("BER-lattice.png", width=1024, height=768) #width=2048, height=1536)
  data127 <- subset(psrFrame, psrFrame$packetSizes=="127 bytes")
  theFigure <- xyplot( ber ~ distances | channels, 
	data=data127,
	type="o",
	as.table=T,
	xlab= list(label="Distance (meters)", cex=2.5),
  	ylab = list(label="Bit Error Rate", cex=2.5),
        par.settings = simpleTheme(pch=19, cex=2, lwd=2),
	par.strip.text=list(cex=2.5),
	panel=gridPanel,
	layout = c(2, 2),
	scales = list(y=list(log = 10), cex=2), 
	yscale.components = yscale.components.log10
	)
  print(theFigure)
  dev.off()
}

plotAll <-function() {
  psrFrame <- expand.grid(distances=distances, channels=channels, packetSizes=packetSizes) 
  # no akaroa
  psrFrame$ber <- loadDataNoAkaroa(noAkaFields[1]) 
  psrFrame$psrRS <- loadDataNoAkaroa(noAkaFields[2]) / 1000 
  psrFrame$psrNoRS <- loadDataNoAkaroa(noAkaFields[3]) / 1000
  psrFrame$perRS <- 1 - psrFrame$psrRS
  psrFrame$perNoRS <- 1 - psrFrame$psrNoRS
  psrFrame$RSEfficiency <- (psrFrame$psrRS - psrFrame$psrNoRS) / psrFrame$psrNoRS
  latticePSRPlot(psrFrame)
  latticeBER(psrFrame)
}

