				# Function myplot:
				# Selects rows from data that satisfy
				# select condidion. Selects all distinct
				# combinations from values found in the
				# columns in [lines], if not empty
				# [linenames] specifies the name of
				# those columns and must therefore be of
				# equal length.
				# A graph is drawn containing a line for
				# each combination from [lines] with the
				# data in the x_column as x coordinates
				# and in the y_column as y coordinates.
				# If multiple rows for a single line/x_column
				# value are found the average is used.

function myplot(data, select_condition, lines, linenames, x_column, y_column)
  if( nargin!=6 )
    usage("myplot(data, select_condition, [lines], [linenames], x_column, y_column)")
  endif

  if( !isempty(linenames) && (size(lines,2)!=size(linenames,1)) )
      usage("The number of columns in lines must equal the number of rows in linenames.")
  endif

  if !isempty(select_condition)
    data = select_records(data, select_condition);
  endif

  line_conditions=[];
  line_titles=[];

  for i=1:size(lines,2)
    linevalues = create_set(data(:,lines(i)));
    tmp_conditions = [];
    tmp_titles = [];
    for j=1:size(linevalues,2)
      if( isempty(tmp_conditions) )
	tmp_conditions = sprintf("datarow(%i)==%f", lines(i), linevalues(j) );
	if( !isempty(linenames) )
	  tmp_titles = sprintf("%s=%f ", linenames(i,:), linevalues(j));
	endif
      else
	tmp_conditions = [tmp_conditions; sprintf("datarow(%i)==%f", lines(i), linevalues(j) )];
	if( !isempty(linenames) )
	  tmp_titles = [tmp_titles; sprintf("%s=%f ", linenames(i,:), linevalues(j))];
	endif
      endif
    endfor

    if isempty(line_conditions)
      line_conditions = tmp_conditions;

      if( !isempty(linenames) )
	line_titles = tmp_titles;
      endif
    else
      line_conditions = [ string_column_by_row(line_conditions,size(tmp_conditions,1)), string_column(" && ",size(tmp_conditions,1)*size(line_conditions,1)), string_column(tmp_conditions,size(line_conditions,1)) ];

      if( !isempty(linenames) )
	line_titles = [ string_column_by_row(line_titles,size(tmp_titles,1)), string_column(tmp_titles,size(line_titles,1)) ];
      endif
    endif
  endfor

  line_conditions=[line_conditions,string_column(";",size(line_conditions,1)) ];

			  
  myplot_lines(data, [], line_conditions, line_titles, x_column, y_column);
endfunction

