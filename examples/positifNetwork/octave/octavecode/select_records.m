function result = select_records(data, condition)

if( nargin!=2 )
  usage("select_records(data, condition)")
endif

j=1;
for i=1:size(data,1) # for each data row, see if the condition is true
  datarow=data(i,:);
  if( eval(condition) ) # if so, copy it to the top of the list of rows we keep
    data(j,:) = data(i,:);
    j=j+1;
  endif
endfor
result=data(1:j-1,:);

endfunction





