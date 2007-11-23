function plotscenariops(data)
				# the data from the scenario file has
				# this layout:
				# 1 row containgin <num_nodes> 0 0 0
				# 1 row containing <dimension> <areasize> <radiorange> 0
				# n rows containing <node nr> <x> <y> <anchor>
				# where <node nr> increases with step 1
				# and <anchor> is 0 or 1
				# m rows containing <node nr> <node nr> <link distance> <0>

  if(nargin!=1)
    usage("plotscenariops(data)");
  endif
  if data(2,1) != 2
    usage("Can only plot 2D scenarios, sorry.");
  endif

  numnodes=data(1,1);
  areasize=data(2,2);
  data=data(3:size(data,1),:); # Strip the first two rows.


  i=numnodes+1; # Start at the first link record. 

				#allocate space for all the plot data
				#2x2 for each link between nodes
				#There are links from i+1 to size(data)
				# We allocate a bit more to be able to
				# plot the anchors as red dots as well.

  plotdata=[0;0];
  plotdata(1,2*(size(data,1)))=0;



				# the result of this is a matrix
				# containing 2x2 matrices for each link
				# between nodes a and b:
				# x_a y_a
				# x_b y_b
  plotcmd="__gnuplot_plot__";
  j=1;
  for k=i:size(data,1)
				# node row numbers
    node1=data(k,1)+1; #add 1 to get the row (node nrs start at 0)
    node2=data(k,2)+1;

    if(node1<node2)
				#coordinates of first node
      plotdata(1,2*j-1)=data(node1,2); # x in data column 2
      plotdata(1,2*j)=data(node1,3); # y in data column 3

				#coordinates of second node
      plotdata(2,2*j-1)=data(node2,2);
      plotdata(2,2*j)=data(node2,3);
      plotcmd=sprintf("%s plotdata using %i:%i w linespoints 1 ,", plotcmd, 2*j-1, 2*j);
      j=j+1;
    endif
  endfor

				# plot the anchor positions as red dots
				# (type 1) by plotting from two
				# identical coordinates.
				# Easy to implement. I'm lazy.
  for k=1:i-1
      plotdata(1,2*j-1)=data(k,2);
      plotdata(1,2*j)=data(k,3);
      plotdata(2,2*j-1)=data(k,2);
      plotdata(2,2*j)=data(k,3);
    if data(k,4)==1
      plotcmd=sprintf("%s plotdata using %i:%i w points 7,", plotcmd, 2*j-1, 2*j);
    endif
    if data(k,4)==0
      plotcmd=sprintf("%s plotdata using %i:%i w points 1,", plotcmd, 2*j-1, 2*j);
    endif
      j=j+1;
  endfor

				# note: __gnuplot_set__ size square kind of screws
				# a ps up when viewed in gv (no resize
				# anymore), but it looks fine in an x11
				# window or when the ps is printed.

  __gnuplot_set__ size square
  __gnuplot_set__ nokey
  __gnuplot_set__ term postscript
  eval(sprintf("__gnuplot_set__ output \"scenario.ps\""));
  eval(sprintf("__gnuplot_set__ xrange [0:%f]", areasize));
  eval(sprintf("__gnuplot_set__ yrange [0:%f]", areasize));

  eval(plotcmd(1,1:size(plotcmd,2)-1));
  __gnuplot_set__ term x11
endfunction


