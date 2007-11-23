function result = groupby(data, grp_cols, avg_cols)

if( nargin!=3 )
  usage("groupby(data, grp_cols, avg_cols)")
endif

result=groupby_cond(data,grp_cols,avg_cols,"");

endfunction