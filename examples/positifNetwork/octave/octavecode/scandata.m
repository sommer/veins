function result = scandata(datamatrix)
  if( nargin!=1 )
    datamatrix=data;
  endif

  columns = [2,3,4,5,6,44,45,46,47,48,49,50,51,52,53,60];
  result = create_rowset(datamatrix(:,columns));

  params=create_set(datamatrix(:,columns(1)))';
  for i=2:size(columns,2)
    tmp=create_set(datamatrix(:,columns(i)))';

    diflength=size(params,1)-size(tmp,1);
    if(diflength < 0 )
      params=[params;zeros(-diflength,size(params,2))./0];
    endif
    if(diflength > 0 )
      tmp=[tmp;zeros(diflength,1)./0];
    endif
    params=[params,tmp];
  endfor

  [" "; "   algorithm   range  rangevariance anch_frac num_nodes alg_version  do_2nd_phase p1ancmin p1ancmax floodlim topology"]
  [" "; "       2         3          4           5         6           44           45           46       47     48      49   "]
  params
  
endfunction
