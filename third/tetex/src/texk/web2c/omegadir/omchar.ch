%
% This file is part of the Omega project, which
% is based on the web2c distribution of TeX.
% 
% Copyright (c) 1995--1998 John Plaice and Yannis Haralambous
% 
%---------------------------------------
@x [26] m.416 l.8407 - Omega char dimensions
primitive("dp",set_box_dimen,depth_offset);
@!@:dp_}{\.{\\dp} primitive@>
@y
primitive("dp",set_box_dimen,depth_offset);
@!@:dp_}{\.{\\dp} primitive@>
primitive("charwd",set_box_dimen,(width_offset+3));
@!@:charwd_}{\.{\\charwd} primitive@>
primitive("charht",set_box_dimen,(height_offset+3));
@!@:charht_}{\.{\\charht} primitive@>
primitive("chardp",set_box_dimen,(depth_offset+3));
@!@:chardp_}{\.{\\chardp} primitive@>
primitive("charit",set_box_dimen,(depth_offset+4));
@!@:charit_}{\.{\\charit} primitive@>
@z
%---------------------------------------
@x [26] m.416 l.8425 - Omega char dimensions
set_box_dimen: if chr_code=width_offset then print_esc("wd")
else if chr_code=height_offset then print_esc("ht")
else print_esc("dp");
@y
set_box_dimen: if chr_code=width_offset then print_esc("wd")
else if chr_code=height_offset then print_esc("ht")
else if chr_code=depth_offset then print_esc("dp")
else if chr_code=(width_offset+3) then print_esc("charwd")
else if chr_code=(height_offset+3) then print_esc("charht")
else if chr_code=(depth_offset+3) then print_esc("chardp")
else print_esc("charit");
@z
%---------------------------------------
@x [26] m.419 l.8461 - Omega char dimensions
begin scan_eight_bit_int;
if box(cur_val)=null then cur_val:=0 @+else cur_val:=mem[box(cur_val)+m].sc;
cur_val_level:=dimen_val;
end
@y
if m<=3 then 
begin
   scan_eight_bit_int;
   if box(cur_val)=null then cur_val:=0 @+else
   cur_val:=mem[box(cur_val)+m].sc;
   cur_val_level:=dimen_val;
end
else
begin
   scan_char_num;
   if m=(width_offset+3) then begin
      cur_val:= char_width(main_f)(char_info(main_f)(cur_val))
      end
   else if m=(height_offset+3) then begin
      cur_val:= char_height(main_f)(height_depth(char_info(main_f)(cur_val)))
      end
   else if m=(depth_offset+3) then begin
      cur_val:= char_depth(main_f)(height_depth(char_info(main_f)(cur_val)))
      end
   else begin
      cur_val:= char_italic(main_f)(char_info(main_f)(cur_val));
      end;
   cur_val_level:=dimen_val;
end
@z
%---------------------------------------
