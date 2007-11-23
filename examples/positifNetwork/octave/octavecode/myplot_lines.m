# called from myplot.m when the individual line conditions have been determined.

function myplot_lines(data, select_condition, lines, titles, x_column, y_column)
  if( nargin!=6 )
    usage("myplot_lines(data, select_condition, [line_conditions] (each condition on a seperate row), [titles], x_column, y_column)")
  endif

  if( !isempty(titles) )
    if( size(titles,1) != size(lines,1) )
      usage("Number of rows in lines and titles parameters must be equal")
    endif
  endif

  if !isempty(select_condition)
    data = select_records(data, select_condition);
  endif

  plotcmd = "__gnuplot_plot__";
  plotdata=[];
  skipped=0;
  for i=1:size(lines, 1)

    tmpdata=groupby_cond(data, [x_column], [y_column], lines(i,:));

    if( isnan(tmpdata(1,2)) ) # Only plot if there is data
      skipped = skipped+1; # Relevant for column indexing later on.
    else
      if isempty(plotdata)
				# Create the structure that will hold
				# the different plot lines
	plotdata=tmpdata(:,[1,2]);
      else
				# Make sure both contain the same number
				# of columns
	diflength=size(plotdata,1)-size(tmpdata,1);
	if(diflength < 0 )
	  plotdata=lengthen(plotdata, size(tmpdata,1));
	endif
	if(diflength > 0 )
	  tmpdata=lengthen(tmpdata, size(plotdata,1));
	endif
				# copy the data from tmpdata to plotdata
				# ignore the third column which contains
				# the standard deviation.
	plotdata=[plotdata,tmpdata(:,[1,2])];
      endif

				# Add the plotline to the command

      j=i-skipped; # Dont count the lines we skipped because of lack
				# of data
      if( isempty(titles) )
	plotcmd = [plotcmd, sprintf( " plotdata using %i:%i with linespoints %d t ""%s"",", 2*j-1, 2*j, 2+2*j, deblank(lines(i,:)) ) ];
      else
	plotcmd = [plotcmd, sprintf( " plotdata using %i:%i with linespoints %d t ""%s"",", 2*j-1, 2*j, 2+2*j, deblank(titles(i,:)) ) ];
      endif
    endif
  endfor
  if( size(plotcmd,2)==5 ) # plotcmd="__gnuplot_plot__"
    sprintf( "Nothing to plot!!" )
    return
  endif
  plotcmd=plotcmd(:,1:size(plotcmd,2)-1);
  eval(plotcmd);

endfunction








