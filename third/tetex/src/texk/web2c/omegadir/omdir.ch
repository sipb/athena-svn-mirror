%
% This file is part of the Omega project, which
% is based on the web2c distribution of TeX.
% 
% Copyright (c) 1995--1998 John Plaice and Yannis Haralambous
% 
%-------------------------
@x [10] m.135
@d hlist_node=0 {|type| of hlist nodes}
@d box_node_size=7 {number of words to allocate for a box node}
@d width_offset=1 {position of |width| field in a box node}
@d depth_offset=2 {position of |depth| field in a box node}
@d height_offset=3 {position of |height| field in a box node}
@d width(#) == mem[#+width_offset].sc {width of the box, in sp}
@d depth(#) == mem[#+depth_offset].sc {depth of the box, in sp}
@d height(#) == mem[#+height_offset].sc {height of the box, in sp}
@d shift_amount(#) == mem[#+4].sc {repositioning distance, in sp}
@d list_offset=5 {position of |list_ptr| field in a box node}
@d list_ptr(#) == link(#+list_offset) {beginning of the list inside the box}
@d glue_order(#) == subtype(#+list_offset) {applicable order of infinity}
@d glue_sign(#) == type(#+list_offset) {stretching or shrinking}
@d normal=0 {the most common case when several cases are named}
@d stretching = 1 {glue setting applies to the stretch components}
@d shrinking = 2 {glue setting applies to the shrink components}
@d glue_offset = 6 {position of |glue_set| in a box node}
@d glue_set(#) == mem[#+glue_offset].gr
  {a word of type |glue_ratio| for glue setting}
@y
@d hlist_node=0 {|type| of hlist nodes}
@d box_node_size=9 {number of words to allocate for a box node}
@d width_offset=1 {position of |width| field in a box node}
@d depth_offset=2 {position of |depth| field in a box node}
@d height_offset=3 {position of |height| field in a box node}
@d width(#) == mem[#+width_offset].sc {width of the box, in sp}
@d depth(#) == mem[#+depth_offset].sc {depth of the box, in sp}
@d height(#) == mem[#+height_offset].sc {height of the box, in sp}
@d shift_amount(#) == mem[#+4].sc {repositioning distance, in sp}
@d list_offset=5 {position of |list_ptr| field in a box node}
@d list_ptr(#) == link(#+list_offset) {beginning of the list inside the box}
@d glue_order(#) == subtype(#+list_offset) {applicable order of infinity}
@d glue_sign(#) == type(#+list_offset) {stretching or shrinking}
@d normal=0 {the most common case when several cases are named}
@d stretching = 1 {glue setting applies to the stretch components}
@d shrinking = 2 {glue setting applies to the shrink components}
@d glue_offset = 6 {position of |glue_set| in a box node}
@d glue_set(#) == mem[#+glue_offset].gr
  {a word of type |glue_ratio| for glue setting}
@d dir_offset = 7 {position of |box_dir| in a box node}
@d box_dir(#) == mem[#+dir_offset].int
@d hsize_offset = 8 {position of |box_hsize| in a box node}
@d box_hsize(#) == mem[#+hsize_offset].sc
@z
%-------------------------
@x [10] m.136
@p function new_null_box:pointer; {creates a new box node}
var p:pointer; {the new node}
begin p:=get_node(box_node_size); type(p):=hlist_node;
subtype(p):=min_quarterword;
width(p):=0; depth(p):=0; height(p):=0; shift_amount(p):=0; list_ptr(p):=null;
glue_sign(p):=normal; glue_order(p):=normal; set_glue_ratio_zero(glue_set(p));
new_null_box:=p;
@y
@p function new_null_box:pointer; {creates a new box node}
var p:pointer; {the new node}
begin p:=get_node(box_node_size); type(p):=hlist_node;
subtype(p):=min_quarterword;
width(p):=0; depth(p):=0; height(p):=0; shift_amount(p):=0; list_ptr(p):=null;
glue_sign(p):=normal; glue_order(p):=normal; set_glue_ratio_zero(glue_set(p));
box_dir(p):=-1; box_hsize(p):=hsize;
new_null_box:=p;
@z
%-------------------------
@x [10] m.161
However, other references to the nodes are made symbolically in terms of
the \.{WEB} macro definitions above, so that format changes will leave
\TeX's other algorithms intact.
@^system dependencies@>
@y
However, other references to the nodes are made symbolically in terms of
the \.{WEB} macro definitions above, so that format changes will leave
\TeX's other algorithms intact.
@^system dependencies@>

@p @<Declare functions needed for special kinds of nodes@>
@z
%-------------------------
@x [10] m.206
hlist_node,vlist_node,unset_node: begin r:=get_node(box_node_size);
  mem[r+6]:=mem[p+6]; mem[r+5]:=mem[p+5]; {copy the last two words}
@y
hlist_node,vlist_node,unset_node: begin r:=get_node(box_node_size);
  mem[r+8]:=mem[p+8]; mem[r+7]:=mem[p+7]; {copy the last four words}
  mem[r+6]:=mem[p+6]; mem[r+5]:=mem[p+5];
@z
%-------------------------
@x [10] m.208-209
@d max_non_prefixed_command=min_internal+3
   {largest command code that can't be \.{\\global}}
@y
@d LR=min_internal+4
   {text direction (\.{\\beginL},\.{\\beginR},\.{\\endL},\.{\\endR})}
@d max_non_prefixed_command=min_internal+4
   {largest command code that can't be \.{\\global}}
@z
%-------------------------
@x [16] m.212
  @!aux_field: memory_word;
@y
  @!aux_field: memory_word;
  @!LRs_field: halfword;
  @!dir_field: integer;
  @!hwidth_field: scaled;
  @!math_field: integer;
@z
%-------------------------
@x [16] m.212
@d mode_line==cur_list.ml_field {source file line number at beginning of list}
@y
@d mode_line==cur_list.ml_field {source file line number at beginning of list}
@d LR_save==cur_list.LRs_field {LR stack when a paragraph is interrupted}
@d LR_par_dir==cur_list.dir_field {direction of paragraph}
@d LR_par_width==cur_list.hwidth_field {hsize when paragraph is being built}
@d LR_real_math==cur_list.math_field {direction of paragraph}
@z
%-------------------------
@x [16] m.215
prev_graf:=0; shown_mode:=0;
@y
prev_graf:=0; shown_mode:=0;
LR_save:=null; LR_real_math:=1;
LR_par_dir:=par_direction; LR_par_width:=hsize;
@z
%-------------------------
@x [16] m.215
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
@y
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
LR_save:=null; LR_real_math:=1;
LR_par_dir:=par_direction; LR_par_width:=hsize;
@z
%-------------------------
@x [31] m.585
\yskip\noindent Commands 250--255 are undefined at the present time.
@y
\yskip\noindent|begin_reflect| 250. Begin a (possibly recursive)
reflected segment.

\yskip\noindent|begin_reflect| 251. End a (possibly recursive)
reflected segment.

\yskip\noindent Commands 252--255 are undefined at the present time.

When a DVI-IVD driver encounters a |begin_reflect| command, it should
skim ahead (as previously described) until finding the matching
|end_reflect|; these will be properly nested with respect to each
other and with respect to |push| and |pop|.  After skimming has
located a segment of material to be reflected, that segment should be
re-scanned and obeyed in mirror-image mode as described earlier.  The
reflected segment might recursively involve
|begin_reflect/end_reflect| pairs that need to be reflected again.
@z
%-------------------------
@x [31] m.586
@d set_rule=132 {typeset a rule and move right}
@y
@d set_rule=132 {typeset a rule and move right}
@d put1=133 {typeset a character without moving}
@z
%-------------------------
@x [31] m.586
@d right1=143 {move right}
@y
@d right1=143 {move right}
@d right4=146 {move right, 4 bytes}
@z
%-------------------------
@x [31] m.586
@d down1=157 {move down}
@y
@d down1=157 {move down}
@d down4=160 {move down, 4 bytes}
@z
%-------------------------
@x [31] m.586
@d post_post=249 {postamble ending}
@y
@d post_post=249 {postamble ending}
@d begin_reflect=250
   {begin a reflected segment (allowed in DVI-IVD files only)}
@d end_reflect=251
   {end a reflected segment (allowed in DVI-IVD files only)}
@z
%-------------------------
@x [32] m.607
@p procedure movement(@!w:scaled;@!o:eight_bits);
label exit,found,not_found,2,1;
var mstate:small_number; {have we seen a |y| or |z|?}
@!p,@!q:pointer; {current and top nodes on the stack}
@!k:integer; {index into |dvi_buf|, modulo |dvi_buf_size|}
begin q:=get_node(movement_node_size); {new node for the top of the stack}
@y
@p procedure movement(@!w:scaled;@!o:eight_bits);
label exit,found,not_found,2,1;
var mstate:small_number; {have we seen a |y| or |z|?}
@!p,@!q:pointer; {current and top nodes on the stack}
@!k:integer; {index into |dvi_buf|, modulo |dvi_buf_size|}
begin
if dvi_direction=1 then
  if o=right1 then negate(w);
q:=get_node(movement_node_size); {new node for the top of the stack}
@z
%-------------------------
@x [32] m.619
@!outer_doing_leaders:boolean; {were we doing leaders?}
@!edge:scaled; {left edge of sub-box, or right edge of leader space}
@!glue_temp:real; {glue value before rounding}
begin this_box:=temp_ptr; g_order:=glue_order(this_box);
g_sign:=glue_sign(this_box); p:=list_ptr(this_box);
incr(cur_s);
if cur_s>0 then dvi_out(push);
if cur_s>max_push then max_push:=cur_s;
save_loc:=dvi_offset+dvi_ptr; base_line:=cur_v; left_edge:=cur_h;
while p<>null do @<Output node |p| for |hlist_out| and move to the next node,
  maintaining the condition |cur_v=base_line|@>;
prune_movements(save_loc);
if cur_s>0 then dvi_pop(save_loc);
decr(cur_s);
end;
@y
@!outer_doing_leaders:boolean; {were we doing leaders?}
@!edge:scaled; {left edge of sub-box, or right edge of leader space}
@!glue_temp:real; {glue value before rounding}
@!dvi_temp_ptr:integer;
@!dvi_LR_ptr:integer;
@!dvi_LR_h:scaled;
@!old_direction:integer;
@!old_LR_ptr:integer;
begin this_box:=temp_ptr; g_order:=glue_order(this_box);
g_sign:=glue_sign(this_box); p:=list_ptr(this_box);
incr(cur_s);
if cur_s>0 then dvi_out(push);
if cur_s>max_push then max_push:=cur_s;
save_loc:=dvi_offset+dvi_ptr; base_line:=cur_v; left_edge:=cur_h;
old_direction:=dvi_direction;
old_LR_ptr:=info(LR_ptr);
if (box_dir(this_box)<>-1) then
  if (box_dir(this_box)<>page_direction) then
    begin
    info(LR_ptr):=box_dir(this_box);
    if page_direction=0 then begin
      synch_h; synch_v;
      dvi_out(right4); dvi_four(box_hsize(this_box)); dvi_direction:=1;
      end
    else begin
      synch_h; synch_v;
      neg_wd:=box_hsize(this_box); negate(neg_wd);
      dvi_out(right4); dvi_four(neg_wd); dvi_direction:=0;
      end;
    end;
while p<>null do @<Output node |p| for |hlist_out| and move to the next node,
  maintaining the condition |cur_v=base_line|@>;
prune_movements(save_loc);
if cur_s>0 then dvi_pop(save_loc);
dvi_direction:=old_direction;
info(LR_ptr):=old_LR_ptr;
decr(cur_s);
end;
@z
%-------------------------
@x [32] m.620
  oval:=c-font_offset(f); ocmd:=set1; out_cmd;@/
@y
  case dvi_direction of
  0:begin
    oval:=c-font_offset(f); ocmd:=set1; out_cmd;@/
    end;
  1:begin
    neg_wd:=char_width(f)(char_info(f)(c)); negate(neg_wd);
    dvi_out(right4); dvi_four(neg_wd);
    oval:=c-font_offset(f); ocmd:=put1; out_cmd;
    end;
  end;
@z
%-------------------------
@x [32] m.624
  dvi_out(set_rule); dvi_four(rule_ht); dvi_four(rule_wd);
@y
  case dvi_direction of
  0:begin
    dvi_out(set_rule); dvi_four(rule_ht); dvi_four(rule_wd);
    end;
  1:begin
    neg_wd:=rule_wd; negate(neg_wd);
    dvi_out(right4); dvi_four(neg_wd);
    dvi_out(put_rule); dvi_four(rule_ht); dvi_four(rule_wd);
    end;
  end;
@z
%-------------------------
@x [32] m.633
  dvi_out(put_rule); dvi_four(rule_ht); dvi_four(rule_wd);
@y
  case dvi_direction of
  0:begin
    dvi_out(put_rule); dvi_four(rule_ht); dvi_four(rule_wd);
    end;
  1:begin
    neg_wd:=rule_wd; negate(neg_wd);
    dvi_out(right4); dvi_four(neg_wd);
    dvi_out(set_rule); dvi_four(rule_ht); dvi_four(rule_wd);
    end;
  end;
@z
%-------------------------
@x [32] m.638
@ The |hlist_out| and |vlist_out| procedures are now complete, so we are
ready for the |ship_out| routine that gets them started in the first place.

@p procedure ship_out(@!p:pointer); {output the box |p|}
label done;
var page_loc:integer; {location of the current |bop|}
@!j,@!k:0..9; {indices to first ten count registers}
@!s:pool_pointer; {index into |str_pool|}
@!old_setting:0..max_selector; {saved |selector| setting}
begin if tracing_output>0 then
@y
@ The |hlist_out| and |vlist_out| procedures are now complete, so we are
ready for the |ship_out| routine that gets them started in the first place.

At the beginning of |ship_out|, we will initialize a stack of
\.{\\beginL} and \.{\\beginR} instructions that are currently in force;
this is called the LR stack, and it is maintained with the help of
two global variables called |LR_ptr| and |LR_tmp| that will be
defined later.  The instructions inserted here (just before testing
if |tracing_output > 0|) say that on the outermost level we are
typesetting in left-to-right mode.

@d LR_node=7 {|subtype in whatsits that represent \.{\\beginL}, etc.}
@d LR_node_size=4

@p procedure ship_out(@!p:pointer); {output the box |p|}
label done;
var page_loc:integer; {location of the current |bop|}
@!j,@!k:0..9; {indices to first ten count registers}
@!s:pool_pointer; {index into |str_pool|}
@!old_setting:0..max_selector; {saved |selector| setting}
begin
LR_ptr:=get_node(LR_node_size);
info(LR_ptr):=page_direction; {outer level direction}
if tracing_output>0 then
@z
%-------------------------
@x [32] m.639
@ @<Flush the box from memory, showing statistics if requested@>=
@!stat if tracing_stats>1 then
  begin print_nl("Memory usage before: ");
@.Memory usage...@>
  print_int(var_used); print_char("&");
  print_int(dyn_used); print_char(";");
  end;
tats@/
flush_node_list(p);
@y
@ At the end of |ship_out|, we want to clear out the LR stack.

@<Flush the box from memory, showing statistics if requested@>=
@!stat if tracing_stats>1 then
  begin print_nl("Memory usage before: ");
@.Memory usage...@>
  print_int(var_used); print_char("&");
  print_int(dyn_used); print_char(";");
  end;
tats@/
flush_node_list(p); @<Flush the LR stack@>;
@z
%-------------------------
@x [32] m.640
if type(p)=vlist_node then vlist_out@+else hlist_out;
@y
dvi_direction:=page_direction;
if page_direction=1 then begin
  dvi_out(right4); dvi_four(page_right_corner);
  end;
if type(p)=vlist_node then vlist_out@+else hlist_out;
@z
%-------------------------
@x [33] m.649
@ Here now is |hpack|, which contains few if any surprises.

@p function hpack(@!p:pointer;@!w:scaled;@!m:small_number):pointer;
label reswitch, common_ending, exit;
var r:pointer; {the box node that will be returned}
@!q:pointer; {trails behind |p|}
@!h,@!d,@!x:scaled; {height, depth, and natural width}
@!s:scaled; {shift amount}
@!g:pointer; {points to a glue specification}
@!o:glue_ord; {order of infinity}
@!f:internal_font_number; {the font in a |char_node|}
@!i:four_quarters; {font information about a |char_node|}
@!hd:eight_bits; {height and depth indices for a character}
begin last_badness:=0; r:=get_node(box_node_size); type(r):=hlist_node;
subtype(r):=min_quarterword; shift_amount(r):=0;
q:=r+list_offset; link(q):=p;@/
h:=0; @<Clear dimensions to zero@>;
while p<>null do @<Examine node |p| in the hlist, taking account of its effect
  on the dimensions of the new box, or moving it to the adjustment list;
  then advance |p| to the next node@>;
if adjust_tail<>null then link(adjust_tail):=null;
height(r):=h; depth(r):=d;@/
@<Determine the value of |width(r)| and the appropriate glue setting;
  then |return| or |goto common_ending|@>;
common_ending: @<Finish issuing a diagnostic message
      for an overfull or underfull hbox@>;
exit: hpack:=r;
end;
@y
@ Here now is |hpack|, which contains few if any surprises.
The |hpack| routine is modified to keep an LR stack as it
packages a horizontal list, so that errors of mismatched
\.{\\beginL}\ldots\.{\\endL} and \.{\\beginR}\ldots\.{\\endR} pairs
can be detected and corrected.

@p function hpack(@!p:pointer;@!w:scaled;@!m:small_number):pointer;
label reswitch, common_ending, exit;
var r:pointer; {the box node that will be returned}
@!q:pointer; {trails behind |p|}
@!h,@!d,@!x:scaled; {height, depth, and natural width}
@!s:scaled; {shift amount}
@!g:pointer; {points to a glue specification}
@!o:glue_ord; {order of infinity}
@!f:internal_font_number; {the font in a |char_node|}
@!i:four_quarters; {font information about a |char_node|}
@!hd:eight_bits; {height and depth indices for a character}
@!LR_ptr,LR_tmp:pointer; {for LR stack maintenance}
@!LR_problems:integer; {counts missing begins and ends}
begin LR_ptr:=null; LR_problems:=0;
last_badness:=0; r:=get_node(box_node_size); type(r):=hlist_node;
subtype(r):=min_quarterword; shift_amount(r):=0;
box_dir(r):=-1; box_hsize(r):=hsize;
q:=r+list_offset; link(q):=p;@/
h:=0; @<Clear dimensions to zero@>;
while p<>null do @<Examine node |p| in the hlist, taking account of its effect
  on the dimensions of the new box, or moving it to the adjustment list;
  then advance |p| to the next node@>;
if adjust_tail<>null then link(adjust_tail):=null;
height(r):=h; depth(r):=d;@/
@<Determine the value of |width(r)| and the appropriate glue setting;
  then |return| or |goto common_ending|@>;
common_ending: @<Finish issuing a diagnostic message
      for an overfull or underfull hbox@>;
exit: @<Check for LR anomalies at the end of |hpack|@>;
hpack:=r;
end;
@z
%-------------------------
@x [33] m.668
@p function vpackage(@!p:pointer;@!h:scaled;@!m:small_number;@!l:scaled):
  pointer;
label common_ending, exit;
var r:pointer; {the box node that will be returned}
@!w,@!d,@!x:scaled; {width, depth, and natural height}
@!s:scaled; {shift amount}
@!g:pointer; {points to a glue specification}
@!o:glue_ord; {order of infinity}
begin last_badness:=0; r:=get_node(box_node_size); type(r):=vlist_node;
subtype(r):=min_quarterword; shift_amount(r):=0;
@y
@p function vpackage(@!p:pointer;@!h:scaled;@!m:small_number;@!l:scaled):
  pointer;
label common_ending, exit;
var r:pointer; {the box node that will be returned}
@!w,@!d,@!x:scaled; {width, depth, and natural height}
@!s:scaled; {shift amount}
@!g:pointer; {points to a glue specification}
@!o:glue_ord; {order of infinity}
begin last_badness:=0; r:=get_node(box_node_size); type(r):=vlist_node;
subtype(r):=min_quarterword; shift_amount(r):=0;
box_dir(r):=-1; box_hsize(r):=hsize;
@z
%-------------------------
@x [39] m.816
link(tail):=new_param_glue(par_fill_skip_code);
@y
final_par_glue:=new_param_glue(par_fill_skip_code);
link(tail):=final_par_glue;
@z
%-------------------------
@x [39] m.821
@!pass_number:halfword; {the number of passive nodes allocated on this pass}
@y
@!pass_number:halfword; {the number of passive nodes allocated on this pass}
@!final_par_glue:pointer;
@z
%-------------------------
@x [39] m.877
@ The total number of lines that will be set by |post_line_break|
is |best_line-prev_graf-1|. The last breakpoint is specified by
|break_node(best_bet)|, and this passive node points to the other breakpoints
via the |prev_break| links. The finishing-up phase starts by linking the
relevant passive nodes in forward order, changing |prev_break| to
|next_break|. (The |next_break| fields actually reside in the same memory
space as the |prev_break| fields did, but we give them a new name because
of their new significance.) Then the lines are justified, one by one.

@d next_break==prev_break {new name for |prev_break| after links are reversed}

@<Declare subprocedures for |line_break|@>=
procedure post_line_break(@!final_widow_penalty:integer);
label done,done1,continue;
var q,@!r,@!s:pointer; {temporary registers for list manipulation}
@!disc_break:boolean; {was the current break at a discretionary node?}
@!post_disc_break:boolean; {and did it have a nonempty post-break part?}
@!cur_width:scaled; {width of line number |cur_line|}
@!cur_indent:scaled; {left margin of line number |cur_line|}
@!t:quarterword; {used for replacement counts in discretionary nodes}
@!pen:integer; {use when calculating penalties between lines}
@!cur_line: halfword; {the current line number being justified}
begin @<Reverse the links of the relevant passive nodes, setting |cur_p| to the
  first breakpoint@>;
cur_line:=prev_graf+1;
repeat @<Justify the line ending at breakpoint |cur_p|, and append it to the
  current vertical list, together with associated penalties and other
  insertions@>;
incr(cur_line); cur_p:=next_break(cur_p);
if cur_p<>null then if not post_disc_break then
  @<Prune unwanted nodes at the beginning of the next line@>;
until cur_p=null;
if (cur_line<>best_line)or(link(temp_head)<>null) then
  confusion("line breaking");
@:this can't happen line breaking}{\quad line breaking@>
prev_graf:=best_line-1;
end;
@y
@ The total number of lines that will be set by |post_line_break|
is |best_line-prev_graf-1|. The last breakpoint is specified by
|break_node(best_bet)|, and this passive node points to the other breakpoints
via the |prev_break| links. The finishing-up phase starts by linking the
relevant passive nodes in forward order, changing |prev_break| to
|next_break|. (The |next_break| fields actually reside in the same memory
space as the |prev_break| fields did, but we give them a new name because
of their new significance.) Then the lines are justified, one by one.

The |post_line_break| must also keep an LR stack, so that it can
output \.{\\endL} or \.{\\endR} instructions at the ends of lines
and \.{\\beginL} or \.{\\beginR} instructions at the beginnings of lines.

@d next_break==prev_break {new name for |prev_break| after links are reversed}

@<Declare subprocedures for |line_break|@>=
procedure post_line_break(@!final_widow_penalty:integer);
label done,done1,continue;
var q,@!r,@!s:pointer; {temporary registers for list manipulation}
@!disc_break:boolean; {was the current break at a discretionary node?}
@!post_disc_break:boolean; {and did it have a nonempty post-break part?}
@!cur_width:scaled; {width of line number |cur_line|}
@!cur_indent:scaled; {left margin of line number |cur_line|}
@!t:quarterword; {used for replacement counts in discretionary nodes}
@!pen:integer; {use when calculating penalties between lines}
@!cur_line: halfword; {the current line number being justified}
LR_ptr, LR_tmp: pointer; {for LR stack maintenance}
begin LR_ptr:=LR_save;
@<Reverse the links of the relevant passive nodes, setting |cur_p| to the
  first breakpoint@>;
cur_line:=prev_graf+1;
repeat @<Justify the line ending at breakpoint |cur_p|, and append it to the
  current vertical list, together with associated penalties and other
  insertions@>;
incr(cur_line); cur_p:=next_break(cur_p);
if cur_p<>null then if not post_disc_break then
  @<Prune unwanted nodes at the beginning of the next line@>;
until cur_p=null;
if (cur_line<>best_line)or(link(temp_head)<>null) then
  confusion("line breaking");
@:this can't happen line breaking}{\quad line breaking@>
prev_graf:=best_line-1; LR_save:=LR_ptr;
end;
@z
%-------------------------
@x [39] m.880
@<Justify the line ending at breakpoint |cur_p|, and append it...@>=
@<Modify the end of the line to reflect the nature of the break and to include
  \.{\\rightskip}; also set the proper value of |disc_break|@>;
@<Put the \(l)\.{\\leftskip} glue at the left and detach this line@>;
@y
@<Justify the line ending at breakpoint |cur_p|, and append it...@>=
@<Insert LR nodes at the beginning of the current line@>;
@<Adjust the LR stack based on LR nodes in this line@>;
@<Modify the end of the line to reflect the nature of the break and to include
  \.{\\rightskip}; also set the proper value of |disc_break|@>;
@<Insert LR nodes at the end of the current line@>;
@<Put the \(l)\.{\\leftskip} glue at the left and detach this line@>;
@z
%-------------------------
@x [39] m.888
append_to_vlist(just_box);
if adjust_head<>adjust_tail then
  begin link(tail):=link(adjust_head); tail:=adjust_tail;
   end;
adjust_tail:=null
@y
if par_direction<>page_direction then begin
  box_dir(just_box):=par_direction;
  box_hsize(just_box):=hsize;
  end;
append_to_vlist(just_box);
if adjust_head<>adjust_tail then
  begin link(tail):=link(adjust_head); tail:=adjust_tail;
   end;
adjust_tail:=null
@z
%-------------------------
@x [47] m.1090
   vmode+ex_space,vmode+no_boundary:@t@>@;@/
@y
   vmode+ex_space,vmode+LR,vmode+no_boundary:@t@>@;@/
@z
%-------------------------
@x [47] m.1091
procedure new_graf(@!indented:boolean);
begin prev_graf:=0; 
if (mode=vmode)or(head<>tail) then
  tail_append(new_param_glue(par_skip_code));
if MML_mode then begin
  sgml_start_graf
  end
else begin
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
@y
procedure new_graf(@!indented:boolean);
begin prev_graf:=0; 
if (mode=vmode)or(head<>tail) then
  tail_append(new_param_glue(par_skip_code));
if MML_mode then begin
  sgml_start_graf
  end
else begin
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
LR_par_dir:=par_direction; LR_par_width:=hsize;
@z
%-------------------------
@x [47] m.1096
procedure end_graf;
begin if mode=hmode then
  begin if (head=tail) or (link(head)=tail) then pop_nest
        {null paragraphs are ignored, all contain a |local_paragraph| node}
  else if MML_mode then sgml_end_graf
  else line_break(widow_penalty);
  normal_paragraph;
  error_count:=0;
  end;
end;
@y
procedure end_graf;
begin if mode=hmode then
  begin if (head=tail) or (link(head)=tail) then pop_nest
        {null paragraphs are ignored, all contain a |local_paragraph| node}
  else if MML_mode then sgml_end_graf
  else line_break(widow_penalty);
  if LR_save<>null then
    begin flush_list(LR_save); LR_save:=null;
    end;
  normal_paragraph;
  error_count:=0;
  end;
end;
@z
%-------------------------
@x [48] m.1136
procedure push_math(@!c:group_code);
begin push_nest; mode:=-mmode; incompleat_noad:=null; new_save_level(c);
end;
@y
procedure push_math(@!c:group_code);
begin push_nest; mode:=-mmode; incompleat_noad:=null; new_save_level(c);
if c=math_shift_group then begin
  LR_real_math:=real_math; real_math:=1;
  end;
end;
@z
%-------------------------
@x [48] m.1194
procedure after_math;
var l:boolean; {`\.{\\leqno}' instead of `\.{\\eqno}'}
@!danger:boolean; {not enough symbol fonts are present}
@!m:integer; {|mmode| or |-mmode|}
@!p:pointer; {the formula}
@!a:pointer; {box containing equation number}
@<Local variables for finishing a displayed formula@>@;
begin danger:=false;
@<Check that the necessary fonts for math symbols are present;
  if not, flush the current math lists and set |danger:=true|@>;
m:=mode; l:=false; p:=fin_mlist(null); {this pops the nest}
@y
procedure after_math;
var l:boolean; {`\.{\\leqno}' instead of `\.{\\eqno}'}
@!danger:boolean; {not enough symbol fonts are present}
@!m:integer; {|mmode| or |-mmode|}
@!p:pointer; {the formula}
@!a:pointer; {box containing equation number}
@!doing_real_math:integer;
@<Local variables for finishing a displayed formula@>@;
begin danger:=false;
@<Check that the necessary fonts for math symbols are present;
  if not, flush the current math lists and set |danger:=true|@>;
doing_real_math:=LR_real_math;
m:=mode; l:=false; p:=fin_mlist(null); {this pops the nest}
@z
%-------------------------
@x [48] m.1196
@<Finish math in text@>=
begin
if not MML_mode then tail_append(new_math(math_surround,before));
cur_mlist:=p; cur_style:=text_style; mlist_penalties:=(mode>0);
if MML_mode then begin
{
  if MML_level=0 then sgml_out_mlist(true,false,false)
  else tail_append(cur_mlist);
}
  sgml_starttexttag("inlinemath");
  sgml_starttexttag("math");
  tail_append(cur_mlist);
  sgml_endtexttag("math");
  sgml_endtexttag("inlinemath");
  end
else begin
  mlist_to_hlist;
  link(tail):=link(temp_head);
  while link(tail)<>null do tail:=link(tail);
  tail_append(new_math(math_surround,after));
  end;
space_factor:=1000; unsave;
end
@y
@<Finish math in text@> =
begin
if not MML_mode then begin
  tail_append(new_math(math_surround,before));
  if doing_real_math then
    @<Append a |begin_L| to the tail of the current list@>;
  end;
cur_mlist:=p; cur_style:=text_style; mlist_penalties:=(mode>0);
if MML_mode then begin
{ 
  if MML_level=0 then sgml_out_mlist(true,false,false)
  else tail_append(cur_mlist);
}
  sgml_starttexttag("inlinemath");
  sgml_starttexttag("math");
  tail_append(cur_mlist);
  sgml_endtexttag("math");
  sgml_endtexttag("inlinemath");
  end
else begin
  mlist_to_hlist;
  link(tail):=link(temp_head);
  while link(tail)<>null do tail:=link(tail);
  if doing_real_math then
    @<Append an |end_L| to the tail of the current list@>;
  tail_append(new_math(math_surround,after));
  end;
space_factor:=1000; unsave;
end
@z
%-------------------------
@x [48] m.1203
  begin shift_amount(a):=s; append_to_vlist(a);
@y
  begin if doing_real_math and (page_direction<>0) then
    begin shift_amount(a):=-s; box_dir(a):=0;
    end
  else shift_amount(a):=s;
  append_to_vlist(a);
@z
%-------------------------
@x [48] m.1204
shift_amount(b):=s+d; append_to_vlist(b)
@y
if doing_real_math and (page_direction<>0) then
begin shift_amount(b):=-(s+d); box_dir(b):=0;
end
else shift_amount(b):=s+d;
append_to_vlist(b)
@z
%-------------------------
@x [48] m.1205
  shift_amount(a):=s+z-width(a);
  append_to_vlist(a);
@y
  if doing_real_math and (page_direction<>0) then
    begin shift_amount(a):=-(s+z-width(a)); box_dir(a):=0;
    end
  else shift_amount(a):=s+z-width(a);
  append_to_vlist(a);
@z
%-------------------------
@x [49] m.1228
assign_dimen: begin p:=cur_chr; scan_optional_equals;
  scan_normal_dimen; word_define(p,cur_val);
  end;
@y
assign_dimen: begin p:=cur_chr; scan_optional_equals;
  scan_normal_dimen; word_define(p,cur_val);
  if p=(dimen_base+hsize_code) then
    LR_par_width:=hsize;
  end;
@z
%-------------------------
@x [53] m.1341
@d language_node=4 {|subtype| in whatsits that change the current language}
@y
@d language_node=4 {|subtype| in whatsits that change the current language}
@d LR_type(#)==mem[#+1].int {the sub-subtype}
@d LR_dvi_ptr(#)==mem[#+2].int {used for storing where to put offset}
@d LR_dvi_h(#)==mem[#+3].sc {used for computing the offset}
@z
%-------------------------
@x [53] m.1342
@!write_open:array[0..17] of boolean;
@y
@!write_open:array[0..17] of boolean;
@!dvi_direction:integer;
@!page_direction:integer;
@!par_direction:integer;
@!page_width:scaled;
@!page_height:scaled;
@!page_right_corner:scaled;
@!real_math:integer;
@!neg_wd:scaled;
@!pos_wd:scaled;
@!neg_ht:scaled;
@z
%-------------------------
@x [53] m.1343
for k:=0 to 17 do write_open[k]:=false;
@y
for k:=0 to 17 do write_open[k]:=false;
page_direction:=0; par_direction:=0;
page_width:=(210*7227 div 2540) * @"10000;
page_right_corner:=page_width-((2*7227*@"10000) div 100);
real_math:=1;
@z
%-------------------------
@x [53] m.1344
@d set_language_code=5 {command modifier for \.{\\setlanguage}}
@d local_par_node=6 {|subtype| in whatsits for local paragraph node}
@y
@d set_language_code=5 {command modifier for \.{\\setlanguage}}
@d local_par_node=6 {|subtype| in whatsits for local paragraph node}
@d set_page_width=8
@d set_page_height=9
@d set_page_HL=10
@d set_page_HR=11
@d set_par_HL=12
@d set_par_HR=13
@d next_math_fake=14
@d begin_L_code=0 {command modifier for \\.{\\beginL}}
@d begin_R_code=1 {command modifier for \\.{\\beginR}}
@d end_L_code=2 {command modifier for \\.{\\endL}}
@d end_R_code=3 {command modifier for \\.{\\endR}}
@d begin_LR(#)==(LR_type(#) < end_L_code)
@d begin_LR_type(#)==(LR_type(#) - end_L_code)
@z
%-------------------------
@x [53] m.1344
primitive("setlanguage",extension,set_language_code);@/
@!@:set_language_}{\.{\\setlanguage} primitive@>
@y
primitive("setlanguage",extension,set_language_code);@/
@!@:set_language_}{\.{\\setlanguage} primitive@>
primitive("pagewidth",extension,set_page_width);@/
primitive("beginL", LR, begin_L_code);
primitive("beginR", LR, begin_R_code);
primitive("endL", LR, end_L_code);
primitive("endR", LR, end_R_code);
primitive("pagedirHL",extension,set_page_HL);@/
primitive("pagedirHR",extension,set_page_HR);@/
primitive("pardirHL",extension,set_par_HL);@/
primitive("pardirHR",extension,set_par_HR);@/
primitive("nextmathfake",extension,next_math_fake);@/
@z
%-------------------------
@x [53] m.1346
  set_language_code:print_esc("setlanguage");
  othercases print("[unknown extension!]")
  endcases;
@y
  set_language_code:print_esc("setlanguage");
  set_page_width:print_esc("pagewidth");
  set_page_height:print_esc("pageheight");
  set_page_HL:print_esc("pagedirHL");
  set_page_HR:print_esc("pagedirHR");
  set_par_HL:print_esc("pardirHL");
  set_par_HR:print_esc("pardirHR");
  next_math_fake:print_esc("nextmathfake");
  othercases print("[unknown extension!]")
  endcases;
LR: case chr_code of
  begin_L_code: print_esc("beginL");
  begin_R_code: print_esc("beginR");
  end_L_code: print_esc("endL");
  end_R_code: print_esc("endR");
  end;
@z
%-------------------------
@x [53] m.1348
set_language_code:@<Implement \.{\\setlanguage}@>;
@y
set_language_code:@<Implement \.{\\setlanguage}@>;
set_page_HL: begin page_direction:=0; par_direction:=0; LR_par_dir:=0;
  end;
set_page_HR: begin page_direction:=1; par_direction:=1; LR_par_dir:=1;
  end;
set_par_HL: begin par_direction:=0; LR_par_dir:=0; end;
set_par_HR: begin par_direction:=1; LR_par_dir:=1; end;
next_math_fake: real_math:=0;
set_page_width: begin
  scan_normal_dimen; page_width:=cur_val;
  page_right_corner:= page_width - ((2*7227*@"10000) div 100);
  end;
@z
%-------------------------
@x [53] m.1356
language_node:begin print_esc("setlanguage");
  print_int(what_lang(p)); print(" (hyphenmin ");
  print_int(what_lhm(p)); print_char(",");
  print_int(what_rhm(p)); print_char(")");
  end;
@y
language_node:begin print_esc("setlanguage");
  print_int(what_lang(p)); print(" (hyphenmin ");
  print_int(what_lhm(p)); print_char(",");
  print_int(what_rhm(p)); print_char(")");
  end;
LR_node: case LR_type(p) of
  begin_L_code: print_esc("beginL");
  begin_R_code: print_esc("beginR");
  end_L_code: print_esc("endL");
  end_R_code: print_esc("endR");
  end;
@z
%-------------------------
@x [53] m.1357
close_node,language_node: begin r:=get_node(small_node_size);
  words:=small_node_size;
  end;
@y
close_node,language_node: begin r:=get_node(small_node_size);
  words:=small_node_size;
  end;
LR_node: begin r:=get_node(LR_node_size);
  words:=LR_node_size;
  end;
@z
%-------------------------
@x [53] m.1358
close_node,language_node: free_node(p,small_node_size);
@y
close_node,language_node: free_node(p,small_node_size);
LR_node: free_node(p,LR_node_size);
@z
%-------------------------
@x [53] m.1360
@ @<Incorporate a whatsit node into an hbox@>=do_nothing
@y
@ @<Incorporate a whatsit node into an hbox@>=
if subtype(p)=LR_node then 
@<Adjust the LR stack for the |hpack| routine@>
@z
%-------------------------
@x [53] m.1366
@ @<Output the whatsit node |p| in an hlist@>=
out_what(p)
@y
@ @<Output the whatsit node |p| in an hlist@>= 
if subtype(p)<>LR_node then out_what(p)
else @<Output a reflection instruction if the direction has changed@>
@z
%-------------------------
@x [54] The rest.
@* \[54] $\Omega$ changes.

@y
@* \[54] $\Omega$ changes.

@ Most of the changes have been saved up for the end, so that the
section numbers of \TeX in {\sl \TeX: The Program} can be left
unchanged.  Now we come to the real guts of this extension to
mixed-direction texts.

First, we allow the new primitives in horizontal mode, but not in
math mode:

@<Cases of |main_control| that build boxes and lists@>=
hmode+LR: begin new_whatsit(LR_node,LR_node_size);
  LR_type(tail):=cur_chr;
  end;
mmode+LR: report_illegal_case;

@ A number of routines are based on a stack of one-word nodes
whose |info| fields contain either |begin_L_code| or |begin_R_code|.
The top of the stack is pointed to by |LR_ptr|, and an auxiliary
variable |LR_tmp| is available for stack manipulation.

@<Global variables@>=
LR_ptr,LR_tmp:pointer; {stack of LR codes and temp for manipulation}

@ @<Declare functions needed for special kinds of nodes@>=
function new_LR(s:small_number): pointer;
var p:pointer; {the new node}
begin p:=get_node(LR_node_size); type(p):=whatsit_node;
subtype(p):=LR_node; LR_type(p):=s; new_LR:=p;
end;

@ @<Declare functions needed for special kinds of nodes@>=
function safe_info(p:pointer): integer;
begin if p=null then safe_info:=-1 else safe_info:=info(p);
end;

@ @<Append a |begin_L| to the tail of the current list@>=
tail_append(new_LR(begin_L_code))

@ @<Append an |end_L| to the tail of the current list@>=
tail_append(new_LR(end_L_code))

@ When the stack-manipulation macros of this section are used
below, variables |LR_ptr| and |LR_tmp| might be the global variables
declared above, or they might be local to |hpack| or
|post_line_break|.

@d push_LR(#)==
begin LR_tmp:=get_node(LR_node_size); info(LR_tmp):=LR_type(#);
link(LR_tmp):=LR_ptr;
LR_dvi_h(LR_tmp):=LR_dvi_h(#); LR_dvi_ptr(LR_tmp):=LR_dvi_ptr(#);
LR_ptr:=LR_tmp;
end
@d pop_LR==
begin LR_tmp:=LR_ptr; LR_ptr:=link(LR_tmp); free_node(LR_tmp,LR_node_size);
end

@<Flush the LR stack@>=
while LR_ptr<>null do pop_LR

@ @<Insert LR nodes at the beginning of the current line@>=
while LR_ptr<>null do
  begin LR_tmp:=new_LR(info(LR_ptr)); link(LR_tmp):=link(temp_head);
  link(temp_head):=LR_tmp; pop_LR;
  end

@ @<Adjust the LR stack based on LR nodes in this line@>=
q:=link(temp_head);
while q<>cur_break(cur_p) do
  begin if not is_char_node(q) then
    if type(q)=whatsit_node then
      if subtype(q)=LR_node then
        if begin_LR(q) then push_LR(q)
        else if LR_ptr<>null then
          if info(LR_ptr)=begin_LR_type(q) then pop_LR;
  q:=link(q);
  end

@ We use the fact that |q| points to the node with
\.{\\rightskip} glue.

@<Insert LR nodes at the end of the current line@>=
if LR_ptr<>null then
  begin s:=temp_head; r:=link(s);
  while (r<>q) and (r<>final_par_glue) do
    begin s:=r; r:=link(s);
    end;
  if r=final_par_glue then begin
    r:=LR_ptr;
    while r<>null do
      begin LR_tmp:=new_LR(info(r)+end_L_code); link(s):=LR_tmp;
      s:=LR_tmp; r:=link(r);
      end;
    link(s):=final_par_glue;
    end
  else begin
    r:=LR_ptr;
    while r<>null do
      begin LR_tmp:=new_LR(info(r)+end_L_code); link(s):=LR_tmp;
      s:=LR_tmp; r:=link(r);
      end;
    link(s):=q;
    end
  end

@ @<Adjust the LR stack for the |hpack| routine@>=
if begin_LR(p) then push_LR(p)
else if safe_info(LR_ptr)=begin_LR_type(p) then pop_LR
  else begin incr(LR_problems);
    while link(q)<>p do q:=link(q);
    link(q):=link(p); free_node(p,LR_node_size); p:=q;
    end

@ @<Check for LR anomalies at the end of |hpack|@>=
if LR_ptr<>null then
  begin while link(q)<>null do q:=link(q);
  repeat link(q) :=new_LR(info(LR_ptr)+end_L_code); q:=link(q);
    LR_problems:=LR_problems+10000; pop_LR;
  until LR_ptr=null;
  end;
if LR_problems>0 then
  begin print_ln; print_nl("\endL or \endR problem (");
  print_int(LR_problems div 10000); print(" missing, ");
  print_int(LR_problems mod 10000); print(" extra");
  LR_problems:=0; goto common_ending;
  end

@ @<Output a reflection instruction if the direction has changed@>=
if begin_LR(p) then
  begin if safe_info(LR_ptr)<>LR_type(p) then
    begin synch_h; synch_v;
    dvi_out(right4); LR_dvi_ptr(p):=dvi_ptr;
    LR_dvi_h(p):=dvi_h; dvi_four(0);
    dvi_direction:=1-dvi_direction;
    end;
  push_LR(p);
  end
else if safe_info(LR_ptr)=begin_LR_type(p) then
  begin dvi_LR_h:=LR_dvi_h(LR_ptr);
  dvi_LR_ptr:=LR_dvi_ptr(LR_ptr);
  pop_LR;
  if safe_info(LR_ptr)+end_L_code <> LR_type(p) then
    begin synch_h; synch_v;
    dvi_temp_ptr:=dvi_ptr;
    dvi_ptr:=dvi_LR_ptr;
    if begin_LR_type(p)=0 then dvi_four(dvi_LR_h-dvi_h)
    else dvi_four(dvi_h-dvi_LR_h);
    dvi_ptr:=dvi_temp_ptr;
    dvi_out(right4);
    if begin_LR_type(p)=0 then dvi_four(dvi_LR_h-dvi_h)
    else dvi_four(dvi_h-dvi_LR_h);
    dvi_direction:=1-dvi_direction;
    end;
  end
else confusion("LR")

@z
