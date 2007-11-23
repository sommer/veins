function result = create_rowset(data)
  [s,i]=sort(data(:,1));
  data=data(i,:);
  
  if( size(data,2) == 1 ) # This was the last column, terminate the
			  # recursion
    result=create_set(data)';
  else
				# Find the next block with the same
				# value in column 1
    result = [];
    start_row=1;
    stop_row=start_row;

    while( stop_row <= size(data,1) )
      while( stop_row <= size(data,1) &&
	    data(start_row,1) == data(stop_row,1) )
	stop_row=stop_row+1;
      endwhile
      stop_row=stop_row-1; # This is the last row with the same value in
				# column 1.
      tmpresult = create_rowset(data(start_row:stop_row,2:size(data,2)));
      tmpresult = [ ones(size(tmpresult,1),1) * data(start_row,1), tmpresult ];
      if( isempty(result) )
	result = tmpresult;
      else
	result = [result; tmpresult];
      endif
      start_row = stop_row+1;
      stop_row=start_row;
    endwhile
  endif
endfunction
