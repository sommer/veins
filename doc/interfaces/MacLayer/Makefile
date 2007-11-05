PAPER=mac

all: mac.pdf

$(PAPER).pdf: $(PAPER).ps
	ps2pdf $(PAPER).ps

$(PAPER).ps: $(PAPER).dvi
	dvips -Ppdf -G0 $(PAPER).dvi
mac.tex: mac.lyx
	lyx --export latex mac.lyx

mac.dvi: mac.tex
	latex mac.tex

