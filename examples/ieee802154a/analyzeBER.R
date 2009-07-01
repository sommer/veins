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
channels <- c("ghassemzadeh-los", "ghassemzadeh-nlos", "cm1", "cm2")
distances <- seq(5, 30, 5)
packetSizes <- c("8 bytes", "32 bytes", "128 bytes") 

measures <- c("Packet Success Rate (R-S)", "Packet Success Rate (No R-S)", "Bit error rate")

outputFiles <- c("PSR_Distance_RS.png", "PSR_Distance_NoRS.png", "BER_Distance.png")
latticeFile <- "Distance_Lattice.png"

textSize <- 2

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
    tick.at.major <- seq(0.1, 1, 0.1) # c(0.25, 0.5, 0.75, 1) # seq(0.2, 1, 0.2)
    ans$left$labels$labels <- tick.at.major
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
#  lattice.options(default.theme = col.whitebg)#canonical.theme(color = TRUE), new=TRUE)
  trellis.device(device=png, theme=col.whitebg)
  png(filename="PSR-Distance_Lattice-%d.png", width=2048, height=1536) #, bg="transparent")
  theFigure <- xyplot(
    psrRS + psrNoRS ~ distances | packetSizes * channels, data=data, 
    type="b",
    xlab = list(label="Distance (meters)", cex=textSize),
    ylab = list(label="Packet Delivery Success Rate", cex=textSize),
	#    ylim= c(0, 1),
    cex = textSize,
    par.strip.text=list(cex=textSize),
    prepanel=prepanel.loess,
	#    panel=panel.loess,
	#    panel=panel.grid(h=-1, v=-1),
    panel = gridPanel,
	#    aspect ="xy", # change aspect ratio to make lines appear as 45 degrees oriented
    as.table = TRUE, # start drawing from top left to bottom right
	#    layout = c(0, 10), # put 10 figures per page as it pleases you
    layout = c(3, 2, 2), # 3 columns, 2 lines, 2 pages
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
}

latticePSREfficiency <- function(data) {
  png("RSEfficiency-lattice-%d.png", width=2048, height=1536)
  theFigure <- xyplot(RSEfficiency ~ distances | packetSizes*channels, 
		data=psrFrame, 
		type="b",
		as.table=T,
	        layout = c(3, 2, 2),
		scales=list(x=list(log=10), y=list(log=10), cex=textSize), 
		panel=gridPanel,
		cex=textSize,
		par.strip.text=list(cex=textSize),
		xlab=list(cex=textSize),
		ylab=list(cex=textSize),
		)
  print(theFigure)
  dev.off()
}

plotAll <-function() {
  psrRS <- loadData(1)
  psrNoRS <- loadData(2)
  psrFrame <- expand.grid(distances=distances, packetSizes=packetSizes, channels=channels) 
  psrFrame$psrRS <- psrRS$Estimate
  psrFrame$psrNoRS <- psrNoRS$Estimate
  latticePSRPlot(psrFrame)
  psrFrame$RSEfficiency <- (psrFrame$psrRS - psrFrame$psrNoRS) / psrFrame$psrNoRS
  latticePSREfficiency(psrFrame)
  png(outputFiles[1], width=1024, height=768, bg="transparent")
  plotPSR(psrRS, 1)
  #  png(outputFiles[2], width=800, height=600)
  #  plotPSR(psrNoRS)
  #  ber <- loadData(3)
  #  png(outputFiles[3], width=800, height=600)
  #  plotBER(ber)
}

