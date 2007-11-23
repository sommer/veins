				# example: string_column_by_row( ["aaa";"bb"],2 )
				# = ["aaa";"aaa";"bb";"bb"]
function result=string_column_by_row(string, count)
  result=[];
  for i=1:size(string,1)
    rowresult=string(i,:);
    if(count>1)
      for j=2:count
	rowresult=[rowresult;string(i,:)];
      endfor
    endif
    if isempty(result)
      result=rowresult;
    else
      result=[result;rowresult];
    endif
  endfor
endfunction
