function data=lengthen(data, length)
  cur_length=size(data,1);
  dif=length-cur_length;

  for i=1:dif
    data(cur_length+i,:)=data(cur_length,:);
  endfor
endfunction
