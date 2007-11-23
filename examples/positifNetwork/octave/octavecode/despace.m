function result=despace(str)
  result="";
  j=1;
  for i=1:size(str,2)
    if str(1,i)!=" "
      result=[result,str(1,i)];
      j=j+1;
    endif
  endfor
endfunction
