function reduceddata=plotall(data,select_condition,lines,linenames,x_column)
  if( nargin!=5 )
    usage("plotall(data, select_condition, [lines], [linenames], x_column)")
  endif

  sprintf( "%s: Start", ctime(time) )
  oldsize=size(data);

				# First reduce the data set as much as
				# possible.
				# Step 1: apply the select condition to
				# filter records

  if !isempty(select_condition)
    data = select_records(data, select_condition);

    sprintf("%s: Select condition applied", ctime(time) )
    sprintf("Size reduced from %ix%i", oldsize(1), oldsize(2) )
    oldsize=size(data);
    sprintf(" to %ix%i", oldsize(1), oldsize(2) )
  endif

				# Step 2: remove unused columns. We
				# only use those in x_column, lines and
				# in y_columns (one value for
				# each graph to be used as y_column)
  y_columns=[10,13,16,21,41,42,43,25,23,29,31,33];
  columns=[x_column,lines,y_columns];
  num_line_cols=size(lines,2);

  data=data(:,columns);# Throw away unused columns


  sprintf("%s: Unused columns removed", ctime(time) )
  sprintf("Size reduced from %ix%i", oldsize(1), oldsize(2) )
  oldsize=size(data);
  sprintf(" to %ix%i", oldsize(1), oldsize(2) )


				#After this, the column numbers have
				#changed, so update these variables
  x_column=1;
  lines=[2:num_line_cols+1];
  y_columns=[2+num_line_cols:size(columns,2)];

				# Step 3: do a groupby on x_column and
				# lines
  data=groupby( data, [1:num_line_cols+1], [num_line_cols+2:size(columns,2)] );
				# And just select the averages, discard
				# the standard deviation for now
  data=data(:, [1:size(columns,2)] );

				# Since [grp_cols,avg_cols] in the
				# groupby call are all the columns of
				# the matrix, the column numbers won't
				# change during this step.

  sprintf("%s: Groupby applied to average measurements", ctime(time) )
  sprintf("Size reduced from %ix%i", oldsize(1), oldsize(2) )
  oldsize=size(data);
  sprintf(" to %ix%i", oldsize(1), oldsize(2) )
  

  reduceddata=data;

				# Possible further optimization: the
				# myplot code now does an unnecessary
				# groupby. (but on much less data)
  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "01finalerror.ps";
  __gnuplot_set__ title "Final error";
  myplot(data,[],lines,linenames,x_column,y_columns(1));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "021stphaseerror.ps";
  __gnuplot_set__ title "1st phase error";
  myplot(data,[],lines,linenames,x_column,y_columns(2));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "032nderror.ps";
  __gnuplot_set__ title "2nd error measure (phase2 or alternate measure)";
  myplot(data,[],lines,linenames,x_column,y_columns(3));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "04confidence.ps";
  __gnuplot_set__ title "Confidence";
  myplot(data,[],lines,linenames,x_column,y_columns(4));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "05countunknown.ps";
  __gnuplot_set__ title "Number of unknown nodes";
  myplot(data,[],lines,linenames,x_column,y_columns(5));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "06countpositioned.ps";
  __gnuplot_set__ title "Number of positioned nodes";
  myplot(data,[],lines,linenames,x_column,y_columns(6));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "07countbad.ps";
  __gnuplot_set__ title "Number of bad nodes";
  myplot(data,[],lines,linenames,x_column,y_columns(7));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "08bcast.ps";
  __gnuplot_set__ title "Broadcasts per node";
  myplot(data,[],lines,linenames,x_column,y_columns(8));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "09flops.ps";
  __gnuplot_set__ title "Number of FLOPS";
  myplot(data,[],lines,linenames,x_column,y_columns(9));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "10bcastt5.ps";
  __gnuplot_set__ title "bcasts per node of the 1st defined type";
  myplot(data,[],lines,linenames,x_column,y_columns(10));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "11bcastt6.ps";
  __gnuplot_set__ title "bcasts per node of the 2nd defined type";
  myplot(data,[],lines,linenames,x_column,y_columns(11));

  __gnuplot_set__ term postscript;
  __gnuplot_set__ output "12bcastt7.ps";
  __gnuplot_set__ title "bcasts per node of the 3rd defined type";
  myplot(data,[],lines,linenames,x_column,y_columns(12));

  sprintf("%s: All done", ctime(time) )
endfunction

