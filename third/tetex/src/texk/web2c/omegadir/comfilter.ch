%
% This file is part of the Omega project, which
% is based on the web2c distribution of TeX.
% 
% Copyright (c) 1995--1998 John Plaice and Yannis Haralambous
% 
%---------------------------------------
@x
@!inf_ocp_list_size = 1000;
@y
@!inf_ocp_list_size = 1000;
@!sup_ocp_buf_size = 1000000;
@!inf_ocp_buf_size = 1000;
@!sup_ocp_stack_size = 1000000;
@!inf_ocp_stack_size = 1000;
@z
%---------------------------------------
@x
@!ocp_list_size:integer;
@y
@!ocp_list_size:integer;
@!ocp_buf_size:integer;
@!ocp_stack_size:integer;
@z
%---------------------------------------
@x
for k:=0 to active_max_ptr-1 do dump_wd(active_info[k]);
print_ln; print_int(active_max_ptr); print(" words of active ocps");

@ @<Undump the active ocp information@>=
undump_size(0)(active_mem_size)('active start point')(active_min_ptr);
undump_size(0)(active_mem_size)('active mem size')(active_max_ptr);
for k:=0 to active_max_ptr-1 do undump_wd(active_info[k]);
@y
dump_things(active_info[0], active_max_ptr);
print_ln; print_int(active_max_ptr); print(" words of active ocps");

@ @<Undump the active ocp information@>=
undump_size(0)(active_mem_size)('active start point')(active_min_ptr);
undump_size(0)(active_mem_size)('active mem size')(active_max_ptr);
undump_things(active_info[0], active_max_ptr);
@z
%---------------------------------------
@x
  setup_bound_var(1000)('ocp_list_size')(ocp_list_size);
@y
  setup_bound_var(1000)('ocp_list_size')(ocp_list_size);
  setup_bound_var(1000)('ocp_buf_size')(ocp_buf_size);
  setup_bound_var(1000)('ocp_stack_size')(ocp_stack_size);
@z
%---------------------------------------
@x
  xmalloc_array (ocp_list_list , ocp_list_size);
@y
  xmalloc_array (ocp_list_list , ocp_list_size);
  xmalloc_array (otp_init_input_buf , ocp_buf_size);
  xmalloc_array (otp_input_buf , ocp_buf_size);
  xmalloc_array (otp_output_buf , ocp_buf_size);
  xmalloc_array (otp_stack_buf , ocp_stack_size);
  xmalloc_array (otp_calcs , ocp_stack_size);
  xmalloc_array (otp_states , ocp_stack_size);
@z
%---------------------------------------
@x
@!otp_init_input_buf:array[0..20000] of quarterword;

@!otp_input_start:halfword;
@!otp_input_last:halfword;
@!otp_input_end:halfword;
@!otp_input_buf:array[0..20000] of quarterword;

@!otp_output_end:halfword;
@!otp_output_buf:array[0..20000] of quarterword;

@!otp_stack_used:halfword;
@!otp_stack_last:halfword;
@!otp_stack_new:halfword;
@!otp_stack_buf:array[0..1000] of quarterword;

@!otp_pc:halfword;

@!otp_calc_ptr:halfword;
@!otp_calcs:array[0..1000] of halfword;
@!otp_state_ptr:halfword;
@!otp_states:array[0..1000] of halfword;
@y
@!otp_init_input_buf:^quarterword;

@!otp_input_start:halfword;
@!otp_input_last:halfword;
@!otp_input_end:halfword;
@!otp_input_buf:^quarterword;

@!otp_output_end:halfword;
@!otp_output_buf:^quarterword;

@!otp_stack_used:halfword;
@!otp_stack_last:halfword;
@!otp_stack_new:halfword;
@!otp_stack_buf:^quarterword;

@!otp_pc:halfword;

@!otp_calc_ptr:halfword;
@!otp_calcs:^halfword;
@!otp_state_ptr:halfword;
@!otp_states:^halfword;
@z
