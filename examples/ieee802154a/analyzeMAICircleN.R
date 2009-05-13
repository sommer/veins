#!/usr/bin/R

# Parameters
channels <- c("ghassemzadeh-los",  "ghassemzadeh-nlos", "cm1", "cm2", "cm5")
interferers <- 1:4

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

runs <- length(channels)*length(interferers)



loadCSV <- function(thefilename) {
  theData <- read.table(thefilename)
  colnames(theData) <- list("type", "module", "name", "value")
  nbNodes <- theData$value[theData$name=="NbNodes"][1]
  nbRxPacketsRS <- sum(theData$value[theData$name=="nbReceivedPacketsRS"])
  nbRxPacketsnoRS <- sum(theData$value[theData$name=="nbReceivedPacketsnoRS"])
  nbSyncs <- sum(theData$value[theData$name == "nbSuccessfulSyncs" ])
  nbRxBits <- sum(theData$value[theData$name=="Total received bits"])
  nbErrBits <- sum(theData$value[theData$name=="Erroneous bits"])
  ber <- nbErrBits/nbRxBits
  c(ber, nbNodes, nbRxBits, nbErrBits, nbRxPacketsRS, nbRxPacketsnoRS)
}

loadInfo <- function(filename) {
  scafile <- sub("csv$", "sca", filename)
  info <- scan(scafile, what=list(type="", name="", value=""), skip=2, 
	flush=TRUE, quiet=TRUE)
  channel <- info$value[info$name=="Channel"][1]
  nbNodes <- info$value[info$name=="NbNodes"][1]
  receiver <- info$value[info$name=="Receiver"][1]
  c(nbNodes, channel, receiver)
}

loadScaData <-function() {
  fileList <- sort(list.files(path="./results", pattern="csv$"))
  res <- array(0, c(length(fileList), 6))
  info <- array(0, c(length(fileList), 3))
  i <- 1
  for (filename in fileList) {
    res[i, ] <- loadCSV(paste("./results/", filename, sep=""))
    info[i, ] <- loadInfo(paste("./results/", filename, sep=""))
    i <- i+1
  }
  list(v1=res, v2=info)
}

loadAkaData <- function() {
  p1 <- read.table("./akresults/maiunif-1.csv", header=TRUE)
  p2 <- read.table("./akresults/maiunif-2.csv", header=TRUE)
  list(p1=p1, p2=p2)
}

plotPSR <- function() {
  
  par("cex.main"=titleSize)
  par("cex.sub"=axisSize) # unused
  par("cex.axis"=axisSize)
  par("cex.lab"=axisSize)
  par(mar=c(5.1, 7.1, 4.1, 2.1)) # wide left margin

  chan = 1
  for ( channel in channels ) {
    valuesToPlot <- psrs[ seq(chan, length(psrs), length(channels)) ]
    if (chan == 1) {
      plot(interferers[1:length(valuesToPlot)], valuesToPlot,  type="b", # log="y",
        main=title, xlab="", ylab="",
        las=1, 			# all axis values are horizontal
  	#lab=c(length(valuesToPlot)-1, 5, 2),
        	#ylim=c(0.01, 1),
        xaxp = c(1, length(valuesToPlot), length(valuesToPlot)-1), # n-1 intervals between 1 and n
  #      yaxp= c(0.6, 1, -4),  # 4 tick intervals between 0.6 and 1 (log mode)
  	yaxp = c( 0.6, 1, 4),
        pch=symbs[[channel]], lty=styles[[channel]], 
        col= cols[[channel]], cex=symbolSize, lwd=lineWidth)
    } else {
        lines(interferers[1:length(valuesToPlot)], valuesToPlot, type="b", 
  	pch=symbs[[channel]], lty=styles[[channel]], col= cols[[channel]], 
          cex=symbolSize, lwd=lineWidth )
    }

    chan = chan + 1
  }
  grid(col="grey")

  # confidence intervals
  #plotCI(x = rep( interferers, each=length(channels) ), y = bers[1:runs], uiw=deltas, add=TRUE, pch=NA_integer_)
  #confidence intervals with arrows
  xerrs = rep( interferers, each=length(channels) )
  arrows(xerrs, psrs[1:runs]-deltas[1:runs], xerrs, psrs[1:runs]+deltas[1:runs], angle=90, code=3)

  # replot
  chan = 1
  for ( channel in channels ) {
    valuesToPlot <- psrs[ seq(chan, length(psrs), length(channels)) ]
        lines(interferers[1:length(valuesToPlot)], valuesToPlot, type="b", 
	  pch=symbs[[channel]], lty=styles[[channel]], col= cols[[channel]], 
          cex=symbolSize, lwd=lineWidth )
    chan = chan + 1
  }
  # axis labels
#  mtext(xtitle, side=1, cex=axisSize, line=4) # bottom
  mtext(ytitle, side=2, cex=axisSize, line=5) # left
  legend(x="bottomleft", channels, pch=as.numeric(symbs), col=paste(cols), 
  lty=as.numeric(styles), inset=0.05, pt.cex=symbolSize, cex=axisSize)

}

plotBER <- function() {
  par("cex.main"=titleSize)
  par("cex.sub"=axisSize) # unused
  par("cex.axis"=axisSize)
  par("cex.lab"=axisSize)
  par(mar=c(5.1, 7.1, 4.1, 2.1)) # wide left margin
  chan <- 1
#  plot.new()
#  grid(col="grey")
  for (channel in channels) {
    # put everything in increasing order of number of interferers
    tmp <- sort(res[,2][info[,2]==channel], index.return=TRUE)
    xdataToPlot <- tmp$x - 1 				# ignore receiver node
    xdataToPlot
    ydataToPlot <- res[,1][info[,2]==channel][tmp$ix]
    if(chan==1) {
      plot(xdataToPlot[1:length(interferers)], ydataToPlot[1:length(interferers)],
  	log="y",
        xlab="", ylab="Bit Error Rate", type="b",
        xaxp = c(1, length(valuesToPlot), length(valuesToPlot)-1), # n-1 intervals between 1 and n
        yaxp= c(1e-6, 1, 1),  # see axis below
 	pch=symbs[[channel]], lty=styles[[channel]], col= cols[[channel]], 
        cex=symbolSize, lwd=lineWidth )
    } else {
      lines(xdataToPlot[1:length(interferers)], ydataToPlot[1:length(interferers)], type="b",
	pch=symbs[[channel]], lty=styles[[channel]], col= cols[[channel]], 
        cex=symbolSize, lwd=lineWidth )
    }
    chan <- chan + 1
  }
  axis(side=2, at=c(1e-5, 1e-4, 1e-3, 1e-2, 1e-1) )
  grid(col="grey")
  mtext(xtitle, side=1, cex=axisSize, line=4) # bottom
}

data <- loadScaData()
res <- data$v1
info <- data$v2
akadata <- loadAkaData()
#p1 <- akadata$p1
#p2 <- akadata$p2

psrs <- akadata$p1[2]$Estimate
deltas <- akadata$p1[3]$Delta

par(mfrow=c(2, 1))
#mfg=c(1, 1, 2, 1)
plotPSR()
plotBER()
#mfg=c(1, 1, 2, 1)


