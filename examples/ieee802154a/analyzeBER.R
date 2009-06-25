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
channels <- c("ghassemzadeh-los", "ghassemzadeh-nlos", "cm1", "cm2")
distances <- c(1:10, seq(20, 100, 10))
measures <- c("Packet Success Rate (R-S)", "Packet Success Rate (No R-S)", "Bit error rate")
outputFiles <- c("PSR_Distance_RS.png", "PSR_Distance_NoRS.png", "BER_Distance.png")
latticeFile <- "Distance_Lattice.png"
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

latticePSRPlot <- function(data) {
#  lattice.options(default.theme = col.whitebg)#canonical.theme(color = TRUE), new=TRUE)
  trellis.device(device=png, theme=col.whitebg)
  png(filename="PSR-Distance_Lattice.png", width=1024, height=768) #, bg="transparent")
  print(xyplot(psr ~ distances | channels, data=data, groups=ECC,
    type="b",
    xlab = "Distance (meters)",
    ylab = "Packet Delivery Success Rate"
#   scales=list(x = list(log = 10)),
   ))
}

plotAll <-function() {
  png(outputFiles[1], width=1024, height=768, bg="transparent")
  psrRS <- loadData(1)
  psrNoRS <- loadData(2)
  plotPSR(psrRS, 1)
  psrFrame <- expand.grid(distances=distances, channels=channels, ECC=c(TRUE,FALSE)) #, packetSize=c(10,20,100)
  psrFrame$psr <- c(psrRS$Estimate, psrNoRS$Estimate)
  latticePSRPlot(psrFrame)
#  png(outputFiles[2], width=800, height=600)
#  plotPSR(psrNoRS)
#  ber <- loadData(3)
#  png(outputFiles[3], width=800, height=600)
#  plotBER(ber)
}

