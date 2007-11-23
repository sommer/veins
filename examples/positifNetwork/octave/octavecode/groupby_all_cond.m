function result = groupby_all_cond(data, grp_col, condition)

if( nargin!=3 )
  usage("groupby_all_cond(data, grp_col, condition)")
endif

result=groupby_cond(data,grp_col,[1:size(data,2)],condition);
result=result(:,2:size(result,2));

endfunction