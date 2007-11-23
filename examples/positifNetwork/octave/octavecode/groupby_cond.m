function result = groupby_cond(data, grp_cols, avg_cols, condition)

if( nargin!=4 )
  usage("groupby_cond(data, grp_col, avg_cols, condition)")
endif

if( condition!="" )
  data=select_records(data,condition);
endif

num_grp_cols=size(grp_cols,2);
num_avg_cols=size(avg_cols,2);

tmp = create_rowset(data(:,grp_cols));
tmp(1,num_grp_cols+(num_avg_cols*2)+1)=0;
# format: [grpcol vals] [sum of avg col vals] [squared sum of avg col vals] [number of samples]

for i=1:size(data,1)
  for j=1:size(tmp,1)
    if( data(i,grp_cols) == tmp(j,[1:num_grp_cols] ))
      for k=1:num_avg_cols
	tmp(j,num_grp_cols+k) = tmp(j,num_grp_cols+k)+data(i,avg_cols(k));
	tmp(j,num_grp_cols+num_avg_cols+k) = tmp(j,num_grp_cols+num_avg_cols+k)+data(i,avg_cols(k))^2;
      endfor
      tmp(j,num_grp_cols+2*num_avg_cols+1) = tmp(j,num_grp_cols+2*num_avg_cols+1)+1;
      break
    endif
  endfor
endfor

result=tmp(:,[1:num_grp_cols]);
for k=1:num_avg_cols
  num_samples = tmp(:,num_grp_cols+(2*num_avg_cols)+1);
  normal_sum = tmp(:,num_grp_cols+k);
  squared_sum = tmp(:,num_grp_cols+num_avg_cols+k);
  mean = normal_sum./num_samples;

				# the abs function is here to prevent
				# the expression from becoming negative
				# due to rounding errors. This could
				# happen when stddev=0
  stddev = sqrt(abs(  (squared_sum.-((normal_sum.^2)./num_samples)) ./ (num_samples.-1)   ));

  result(:,num_grp_cols+k)=mean;
  result(:,num_grp_cols+num_avg_cols+k)=stddev;
endfor

endfunction










