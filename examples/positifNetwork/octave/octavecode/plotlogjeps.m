function plotlogjeps(data,plottitle,areasize)
				# There is a row in data containing the
				# final position for each node:
				# n rows containing
				# <node nr> <x> <y> <truex> <truey> <status>
				# where status means:
				# 0 POSITIONED
				# 1 ANCHOR
				# 2 UNKNOWN
				# 3 BAD
				# The first row contains

  if(nargin!=1 && nargin!=2 && nargin!=3)
    usage("plotlogjeps(data,[plottitle=logdata],[areasize=100])");
  endif
  if(nargin==1)
    areasize=100;
    plottitle="logdata";
  endif
  if(nargin==2)
    areasize=100;
  endif

  plotdata=zeros(2,size(data,1));

  plotcmd="__gnuplot_plot__";
				# the result of this is a matrix
				# containing two 2x2 matrices for each
				# node. The first draws a line from the
				# real node coordinates to the real node
				# coordinates and is used to draw a
				# shape.
				# The second draws a line from the real
				# coordinates to the final coordinates.
				# x_r y_r x_r y_r
				# x_r y_r x_f y_f

  for i=1:size(data,1)
				# line from real to final coordinates
      if data(i,6)!=2 #!UNKNOWN
	plotdata(1,4*i-1)=data(i,2);
	plotdata(1,4*i)=data(i,3);
	plotdata(2,4*i-1)=data(i,4);
	plotdata(2,4*i)=data(i,5);
	if data(i,6)==3 #BAD
	  plotcmd=sprintf("%s plotdata using %i:%i w lines 1,", plotcmd, 4*i-1, 4*i);
	else
	  plotcmd=sprintf("%s plotdata using %i:%i w lines 3,", plotcmd, 4*i-1, 4*i);
	endif
      endif

				# shape @ real coordinates
      plotdata(1,4*i-3)=data(i,4);
      plotdata(1,4*i-2)=data(i,5);
      plotdata(2,4*i-3)=data(i,4);
      plotdata(2,4*i-2)=data(i,5);
      if data(i,6)==0 #POSITIONED
	plotcmd=sprintf("%s plotdata using %i:%i w points 3,", plotcmd, 4*i-3, 4*i-2);
      endif
      if data(i,6)==1 #ANCHOR
	plotcmd=sprintf("%s plotdata using %i:%i w points 7,", plotcmd, 4*i-3, 4*i-2);
      endif
      if data(i,6)==2 #UNKNOWN
	plotcmd=sprintf("%s plotdata using %i:%i w points 8,", plotcmd, 4*i-3, 4*i-2);
      endif
      if data(i,6)==3 #BAD
	plotcmd=sprintf("%s plotdata using %i:%i w points 1,", plotcmd, 4*i-3, 4*i-2);
      endif
  endfor

				# note: __gnuplot_set__ size square kind of screws
				# a ps up when viewed in gv (no resize
				# anymore), but it looks fine in an x11
				# window or when the ps is printed.

  eval(sprintf("__gnuplot_set__ title \"%s\"", plottitle));
  __gnuplot_set__ size square
  __gnuplot_set__ nokey
  __gnuplot_set__ term postscript enhanced colour
  eval(sprintf("__gnuplot_set__ output \"%s.ps\"", "plottitle"));
  eval(sprintf("__gnuplot_set__ xrange [0:%f]", areasize));
  eval(sprintf("__gnuplot_set__ yrange [0:%f]", areasize));

  eval(plotcmd(1,1:size(plotcmd,2)-1));
  __gnuplot_set__ term x11
endfunction



