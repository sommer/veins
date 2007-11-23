				# example: string_column( ["aaa";"bb"],2 )
				# = ["aaa";"bb";"aaa";"bb"]
function result=string_column(string, count)
  result=string;
  if(count>1)
    for i=2:count
      result=[result;string];
    endfor
  endif
endfunction

