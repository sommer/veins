function result = groupby_all(data, grp_col)

if( nargin!=2 )
  usage("groupby_all(data, grp_col)")
endif

result=groupby_cond(data,grp_col,[1:size(data,2)],"");
result=result(:,2:size(result,2));

endfunction