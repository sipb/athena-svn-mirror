%
% This file is part of the Omega project, which
% is based on the web2c distribution of TeX.
% 
% Copyright (c) 1994--1999 John Plaice and Yannis Haralambous
% 
@x limbo l.1 - Omega
% This program is copyright (C) 1982 by D. E. Knuth; all rights are reserved.
@y
% This program is copyright (C) 1994--1999 by John Plaice and
% Yannis Haralambous; all rights are reserved.
% It is designed to be a change file for D. E. Knuth's TeX version 3.14159.
%
% This program is copyright (C) 1982 by D. E. Knuth; all rights are reserved.
@z
%---------------------------------------
@x [1] m.1 l.90 - Omega
This is \TeX, a document compiler intended to produce typesetting of high
quality.
The \PASCAL\ program that follows is the definition of \TeX82, a standard
@:PASCAL}{\PASCAL@>
@!@:TeX82}{\TeX82@>
@y
This is $\Omega$, a document compiler intended to simplify high-quality
typesetting for many of the world's languages.  It is an extension
of D. E. Knuth's \TeX, which was designed essentially for the
typesetting of languages using the Latin alphabet.

The $\Omega$ system loosens many of the restrictions imposed by~\TeX:
register numbers are no longer limited to 8~bits;  fonts may have more 
than 256~characters;  more than 256~fonts may be used;  etc.  In addition,
$\Omega$ allows much more complex ligature mechanisms between characters,
thereby simplifying the typesetting of alphabets that have preserved 
their calligraphic traditions.

The \PASCAL\ program that follows is a modification of the definition of \TeX82,
a standard @:PASCAL}{\PASCAL@> @!@:TeX82}{\TeX82@>
@z
%---------------------------------------
@x [1] m.1 l.97 - Omega
will be obtainable on a great variety of computers.

@y
will be obtainable on a great variety of computers.

As little as possible is changed in this document.  This means that
unless the contrary is explicitly stated, references to \TeX\ in the 
documentation are equally applicable to~$\Omega$.  References to `the
author' in the documentation are to D. E. Knuth.  

@z
%---------------------------------------
@x [1] m.2 l.187 - Omega
@d banner=='This is TeX, Version 3.14159' {printed when \TeX\ starts}
@y
@d banner=='This is Omega, Version 3.14159--1.8' {printed when \TeX\ starts}
@z
%---------------------------------------
@x [1] m.11 l.392 - Omega
@!font_max=75; {maximum internal font number; must not exceed |max_quarterword|
  and must be at most |font_base+256|}
@!font_mem_size=20000; {number of words of |font_info| for all fonts}
@y
@!font_max=65535; {maximum internal font number; must be at most |font_biggest|}
@!font_sort_max=65535;
  {maximum font sort number; must be at most |font_sort_biggest|}
@z
%---------------------------------------
@x [1] m.11 l.412 - Omega
@!pool_name='TeXformats:TEX.POOL                     ';
@y
@!pool_name='TeXformats:OMEGA.POOL                   ';
@z
%---------------------------------------
@x [1] m.12 l.437 - Omega
@d hash_size=2100 {maximum number of control sequences; it should be at most
  about |(mem_max-mem_min)/10|}
@d hash_prime=1777 {a prime number equal to about 85\pct! of |hash_size|}
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@y
@d hash_size=65536 {maximum number of control sequences; it should be at most
  about |(mem_max-mem_min)/10|}
@d hash_prime=55711 {a prime number equal to about 85\pct! of |hash_size|}
@d hyph_size=307 {another prime; the number of \.{\\hyphenation} exceptions}
@d biggest_char=65535 {the largest allowed character number;
   must be |<=max_quarterword|}
@d too_big_char=65536 {|biggest_char+1|}
@d special_char=65537 {|biggest_char+2|}
@d number_chars=65536 {|biggest_char+1|}
@d biggest_reg=65535 {the largest allowed register number;
   must be |<=max_quarterword|}
@d number_regs=65536 {|biggest_reg+1|}
@d font_biggest=65535 {the real biggest font}
@d number_fonts=font_biggest-font_base+2
@d font_sort_base=0
@d font_sort_biggest=65535 {the biggest font sort}
@d number_font_sorts=font_sort_biggest-font_sort_base+2
@d number_math_fonts=768
@d math_font_biggest=767
@d text_size=0 {size code for the largest size in a family}
@d script_size=256 {size code for the medium size in a family}
@d script_script_size=512 {size code for the smallest size in a family}
@d biggest_lang=255
@d too_big_lang=256
@z
%---------------------------------------
@x [2] m.17 l.510 - Omega
In order to make \TeX\ readily portable to a wide variety of
computers, all of its input text is converted to an internal eight-bit
@y
In order to make $\Omega$ readily portable to a wide variety of
computers, all of its input text is converted to an internal 16-bit
@z
%---------------------------------------
@x [2] m.18 l.537 - Omega
@!ASCII_code=0..255; {eight-bit numbers}
@y
@!ASCII_code=0..biggest_char;
@z
%---------------------------------------
@x [2] m.19 l.567 - Omega
@d last_text_char=255 {ordinal number of the largest element of |text_char|}
@y
@d last_text_char=biggest_char 
   {ordinal number of the largest element of |text_char|}
@z
%---------------------------------------
@x [2] m.20 l.577 - Omega
@!xord: array [text_char] of ASCII_code;
  {specifies conversion of input characters}
@!xchr: array [ASCII_code] of text_char;
  {specifies conversion of output characters}
@y
@!xchr: array [0..255] of text_char;
  {specifies conversion of output characters}
@z
%---------------------------------------
@x [2] m.23 l.724 - Omega
for i:=0 to @'37 do xchr[i]:=' ';
for i:=@'177 to @'377 do xchr[i]:=' ';
@y
for i:=0 to @'37 do xchr[i]:=chr(i);
for i:=@'177 to @'377 do xchr[i]:=chr(i);
init_eqtb_table;
@z
%---------------------------------------
@x [2] m.24 l.734 - Omega
for i:=first_text_char to last_text_char do xord[chr(i)]:=invalid_code;
for i:=@'200 to @'377 do xord[xchr[i]]:=i;
for i:=0 to @'176 do xord[xchr[i]]:=i;
@y
@z
%---------------------------------------
@x [3] m.25 l.772 - Omega
@!eight_bits=0..255; {unsigned one-byte quantity}
@!alpha_file=packed file of text_char; {files that contain textual data}
@!byte_file=packed file of eight_bits; {files that contain binary data}
@y
@!eight_bits=0..biggest_char; {unsigned two-byte quantity}
@!real_eight_bits=0..255; {dvi output uses exactly 8 bits}
@!alpha_file=packed file of text_char; {files that contain textual data}
@!byte_file=packed file of real_eight_bits; {files that contain binary data}
@z
%---------------------------------------
@x [3] m.31 l.947 - Omega
    buffer[last]:=xord[f^]; get(f); incr(last);
@y
    buffer[last]:=f^; get(f); incr(last);
@z
%---------------------------------------
@x [4] m.38 l.1099 - Omega
String numbers 0 to 255 are reserved for strings that correspond to single
ASCII characters. This is in accordance with the conventions of \.{WEB},
@y
String numbers 0 to |biggest_char| are reserved for strings that correspond to 
single ASCII characters. This is in accordance with the conventions of \.{WEB},
@z
%---------------------------------------
@x [4] m.38 l.1109 - Omega
ASCII character, so the first 256 strings are used to specify exactly what
should be printed for each of the 256 possibilities.
@y
ASCII character, so the first |biggest_char+1| strings are used to specify 
exactly what should be printed for each of the |biggest_char+1| possibilities.
@z
%---------------------------------------
@x [4] m.38-9 l.1122 - Omega
@d si(#) == # {convert from |ASCII_code| to |packed_ASCII_code|}
@d so(#) == # {convert from |packed_ASCII_code| to |ASCII_code|}

@<Types...@>=
@!pool_pointer = 0..pool_size; {for variables that point into |str_pool|}
@!str_number = 0..max_strings; {for variables that point into |str_start|}
@!packed_ASCII_code = 0..255; {elements of |str_pool| array}

@ @<Glob...@>=
@!str_pool:packed array[pool_pointer] of packed_ASCII_code; {the characters}
@!str_start : array[str_number] of pool_pointer; {the starting pointers}
@y
@d si(#) == # {convert from |ASCII_code| to |packed_ASCII_code|}
@d so(#) == # {convert from |packed_ASCII_code| to |ASCII_code|}
@d str_start(#) == str_start_ar[#-too_big_char]

@<Types...@>=
@!pool_pointer = 0..pool_size; {for variables that point into |str_pool|}
@!str_number = 0..max_strings; {for variables that point into |str_start|}
@!packed_ASCII_code = 0..255; {elements of |str_pool| array}

@ @<Glob...@>=
@!str_pool:packed array[pool_pointer] of packed_ASCII_code; {the characters}
@!str_start_ar : array[str_number] of pool_pointer; {the starting pointers}
@z
%---------------------------------------
@x [4] m.40 l.1145 - Omega
@d length(#)==(str_start[#+1]-str_start[#]) {the number of characters
  in string number \#}
@y
@p function length(s:str_number):integer;
   {the number of characters in string number |s|}
begin if (s>=@"10000) then length:=str_start(s+1)-str_start(s)
else if (s>=@"20) and (s<@"7F) then length:=1
else if (s<=@"7F) then length:=3
else if (s<@"100) then length:=4
else length:=8
end;
@z
%---------------------------------------
@x [4] m.41 l.1150 - Omega
@d cur_length == (pool_ptr - str_start[str_ptr])
@y
@d cur_length == (pool_ptr - str_start(str_ptr))
@z
%---------------------------------------
@x [4] m.43 l.1181 - Omega
incr(str_ptr); str_start[str_ptr]:=pool_ptr;
@y
incr(str_ptr); str_start(str_ptr):=pool_ptr;
@z
%---------------------------------------
@x [4] m.44 l.1187 - Omega
@d flush_string==begin decr(str_ptr); pool_ptr:=str_start[str_ptr];
@y
@d flush_string==begin decr(str_ptr); pool_ptr:=str_start(str_ptr);
@z
%---------------------------------------
@x [4] m.45 l.1201 - Omega
begin j:=str_start[s];
while j<str_start[s+1] do
@y
begin j:=str_start(s);
while j<str_start(s+1) do
@z
%---------------------------------------
@x [4] m.46 l.1222 - Omega
@p function str_eq_str(@!s,@!t:str_number):boolean;
  {test equality of strings}
label not_found; {loop exit}
var j,@!k: pool_pointer; {running indices}
@!result: boolean; {result of comparison}
begin result:=false;
if length(s)<>length(t) then goto not_found;
j:=str_start[s]; k:=str_start[t];
while j<str_start[s+1] do
  begin if str_pool[j]<>str_pool[k] then goto not_found;
  incr(j); incr(k);
  end;
result:=true;
not_found: str_eq_str:=result;
end;
@y
@p function str_eq_str(@!s,@!t:str_number):boolean;
  {test equality of strings}
label found,not_found; {loop exit}
var j,@!k: pool_pointer; {running indices}
@!result: boolean; {result of comparison}
begin result:=false;
if length(s)<>length(t) then goto not_found;
if (length(s)=1) then begin
  if s<65536 then begin
    if t<65536 then begin
      if s<>t then goto not_found;
      end
    else begin
      if s<>str_pool[str_start(t)] then goto not_found;
      end;
    end
  else begin
    if t<65536 then begin
      if str_pool[str_start(s)]<>t then goto not_found;
      end
    else begin
      if str_pool[str_start(s)]<>str_pool[str_start(t)] then
        goto not_found;
      end;
    end;
  end
else begin
  j:=str_start(s); k:=str_start(t);
  while j<str_start(s+1) do
    begin if str_pool[j]<>str_pool[k] then goto not_found;
    incr(j); incr(k);
    end;
  end;
found: result:=true;
not_found: str_eq_str:=result;
end;
@z
%---------------------------------------
@x [4] m.47 l.1240 - Omega
var k,@!l:0..255; {small indices or counters}
@y
var k,@!l:0..biggest_char; {small indices or counters}
@z
%---------------------------------------
@x [4] m.47 l.1245 - Omega
begin pool_ptr:=0; str_ptr:=0; str_start[0]:=0;
@<Make the first 256 strings@>;
@<Read the other strings from the \.{TEX.POOL} file and return |true|,
@y
begin pool_ptr:=0; str_ptr:=0; str_start_ar[0]:=0; str_start_ar[1]:=0;
@<Make the first strings@>;
@<Read the other strings from the \.{OMEGA.POOL} file and return |true|,
@z
%---------------------------------------
% A hack is made for strings 256 through to 65535.
% String 256 becomes "^^^^????".  All the strings
% from 256 to 65535 are made to point at this string,
% which will never be printed:  it's just a placeholder.

@x [4] m.48 l.1255 - Omega
@<Make the first 256...@>=
for k:=0 to 255 do
  begin if (@<Character |k| cannot be printed@>) then
    begin append_char("^"); append_char("^");
    if k<@'100 then append_char(k+@'100)
    else if k<@'200 then append_char(k-@'100)
    else begin app_lc_hex(k div 16); app_lc_hex(k mod 16);
      end;
    end
  else append_char(k);
  g:=make_string;
  end
@y
@<Make the first ...@>=
begin
str_ptr:=too_big_char;
end
@z
%---------------------------------------
@x [4] m.49 l.1279 - Omega
Unprintable characters of codes 128--255 are, similarly, rendered
\.{\^\^80}--\.{\^\^ff}.
@y
Unprintable characters of codes 128--255 are, similarly, rendered
\.{\^\^80}--\.{\^\^ff}.

Unprintable characters of codes 256--|biggest_char| are, similarly, rendered
\.{\^\^\^\^0100}--\.{\^\^\^\^ffff}.

@z
%---------------------------------------
@x [4] m.50 l.1298 - Omega
@ When the \.{WEB} system program called \.{TANGLE} processes the \.{TEX.WEB}
description that you are now reading, it outputs the \PASCAL\ program
\.{TEX.PAS} and also a string pool file called \.{TEX.POOL}. The \.{INITEX}
@y
@ When the \.{WEB} system program called \.{OTANGLE} processes the \.{OMEGA.WEB}
description that you are now reading, it outputs the \PASCAL\ program
\.{OMEGA.PAS} and also a string pool file called \.{OMEGA.POOL}. The \.{INITEX}
@z
%---------------------------------------
@x [4] m.51 l.1322 - Omega
else  bad_pool('! I can''t read TEX.POOL.')
@.I can't read TEX.POOL@>
@y
else  bad_pool('! I can''t read OMEGA.POOL.')
@.I can't read OMEGA.POOL@>
@z
%---------------------------------------
@x [4] m.52 l.1326 - Omega
begin if eof(pool_file) then bad_pool('! TEX.POOL has no check sum.');
@.TEX.POOL has no check sum@>
@y
begin if eof(pool_file) then bad_pool('! OMEGA.POOL has no check sum.');
@.OMEGA.POOL has no check sum@>
@z
%---------------------------------------
@x [4] m.52 l.1332 - Omega
else  begin if (xord[m]<"0")or(xord[m]>"9")or@|
      (xord[n]<"0")or(xord[n]>"9") then
    bad_pool('! TEX.POOL line doesn''t begin with two digits.');
@.TEX.POOL line doesn't...@>
  l:=xord[m]*10+xord[n]-"0"*11; {compute the length}
  if pool_ptr+l+string_vacancies>pool_size then
    bad_pool('! You have to increase POOLSIZE.');
@.You have to increase POOLSIZE@>
  for k:=1 to l do
    begin if eoln(pool_file) then m:=' '@+else read(pool_file,m);
    append_char(xord[m]);
@y
else  begin if (m<"0")or(m>"9")or@|
      (n<"0")or(n>"9") then
    bad_pool('! OMEGA.POOL line doesn''t begin with two digits.');
@.OMEGA.POOL line doesn't...@>
  l:=m*10+n-"0"*11; {compute the length}
  if pool_ptr+l+string_vacancies>pool_size then
    bad_pool('! You have to increase POOLSIZE.');
@.You have to increase POOLSIZE@>
  for k:=1 to l do
    begin if eoln(pool_file) then m:=' '@+else read(pool_file,m);
    append_char(m);
@z
%---------------------------------------
@x [4] m.53 l.1347 - Omega
end of this \.{TEX.POOL} file; any other value means that the wrong pool
@y
end of this \.{OMEGA.POOL} file; any other value means that the wrong pool
@z
%---------------------------------------
@x [4] m.53 l.1354 - Omega
loop@+  begin if (xord[n]<"0")or(xord[n]>"9") then
  bad_pool('! TEX.POOL check sum doesn''t have nine digits.');
@.TEX.POOL check sum...@>
  a:=10*a+xord[n]-"0";
@y
loop@+  begin if (n<"0")or(n>"9") then
  bad_pool('! OMEGA.POOL check sum doesn''t have nine digits.');
@.OMEGA.POOL check sum...@>
  a:=10*a+n-"0";
@z
%---------------------------------------
@x [4] m.53 l.1360 - Omega
done: if a<>@$ then bad_pool('! TEX.POOL doesn''t match; TANGLE me again.');
@.TEX.POOL doesn't match@>
@y
done: if a<>@$ then bad_pool('! OMEGA.POOL doesn''t match; OTANGLE me again.');
@.OMEGA.POOL doesn't match@>
@z
%---------------------------------------
@x [5] m.57 l.1446 - Omega
procedure print_ln; {prints an end-of-line}
begin case selector of
term_and_log: begin wterm_cr; wlog_cr;
  term_offset:=0; file_offset:=0;
  end;
log_only: begin wlog_cr; file_offset:=0;
  end;
term_only: begin wterm_cr; term_offset:=0;
  end;
no_print,pseudo,new_string: do_nothing;
othercases write_ln(write_file[selector])
endcases;@/
end; {|tally| is not affected}
@y
procedure print_ln; {prints an end-of-line}
begin case selector of
term_and_log: begin wterm_cr; wlog_cr;
  term_offset:=0; file_offset:=0;
  end;
log_only: begin wlog_cr; file_offset:=0;
  end;
term_only: begin wterm_cr; term_offset:=0;
  end;
no_print,pseudo,new_string: do_nothing;
othercases if selector>max_selector then
    write_ln(output_files[selector-max_selector])
  else
    write_ln(write_file[selector])
endcases;@/
end; {|tally| is not affected}
@z
%---------------------------------------
@x [5] m.58 l.1465 - Omega
procedure print_char(@!s:ASCII_code); {prints a single character}
label exit;
begin if @<Character |s| is the current new-line character@> then
 if selector<pseudo then
  begin print_ln; return;
  end;
case selector of
term_and_log: begin wterm(xchr[s]); wlog(xchr[s]);
  incr(term_offset); incr(file_offset);
  if term_offset=max_print_line then
    begin wterm_cr; term_offset:=0;
    end;
  if file_offset=max_print_line then
    begin wlog_cr; file_offset:=0;
    end;
  end;
log_only: begin wlog(xchr[s]); incr(file_offset);
  if file_offset=max_print_line then print_ln;
  end;
term_only: begin wterm(xchr[s]); incr(term_offset);
  if term_offset=max_print_line then print_ln;
  end;
no_print: do_nothing;
pseudo: if tally<trick_count then trick_buf[tally mod error_line]:=s;
new_string: begin if pool_ptr<pool_size then append_char(s);
  end; {we drop characters if the string space is full}
othercases write(write_file[selector],xchr[s])
@y
procedure print_char(@!s:ASCII_code); {prints a single character}
label exit;
begin if @<Character |s| is the current new-line character@> then
 if (selector<pseudo) or (selector>max_selector) then
  begin print_ln; return;
  end;
case selector of
term_and_log: begin wterm(xchr[s]); wlog(xchr[s]);
  incr(term_offset); incr(file_offset);
  if term_offset=max_print_line then
    begin wterm_cr; term_offset:=0;
    end;
  if file_offset=max_print_line then
    begin wlog_cr; file_offset:=0;
    end;
  end;
log_only: begin wlog(xchr[s]); incr(file_offset);
  if file_offset=max_print_line then print_ln;
  end;
term_only: begin wterm(xchr[s]); incr(term_offset);
  if term_offset=max_print_line then print_ln;
  end;
no_print: do_nothing;
pseudo: if tally<trick_count then trick_buf[tally mod error_line]:=s;
new_string: begin if pool_ptr<pool_size then append_char(s);
  end; {we drop characters if the string space is full}
othercases if selector>max_selector then
    write(output_files[selector-max_selector],xchr[s])
  else
    write(write_file[selector],xchr[s])
@z
%---------------------------------------
% When we print a string, we must make sure we do the appropriate
% thing for strings 256 through to 65535.  We must generate the
% strings on the fly.

@x [5] m.59 l.1496 - Omega
@ An entire string is output by calling |print|. Note that if we are outputting
the single standard ASCII character \.c, we could call |print("c")|, since
|"c"=99| is the number of a single-character string, as explained above. But
|print_char("c")| is quicker, so \TeX\ goes directly to the |print_char|
routine when it knows that this is safe. (The present implementation
assumes that it is always safe to print a visible ASCII character.)
@^system dependencies@>

@<Basic print...@>=
procedure print(@!s:integer); {prints string |s|}
label exit;
var j:pool_pointer; {current character code position}
@!nl:integer; {new-line character to restore}
begin if s>=str_ptr then s:="???" {this can't happen}
@.???@>
else if s<256 then
  if s<0 then s:="???" {can't happen}
  else begin if selector>pseudo then
      begin print_char(s); return; {internal strings are not expanded}
      end;
    if (@<Character |s| is the current new-line character@>) then
      if selector<pseudo then
        begin print_ln; return;
        end;
    nl:=new_line_char; new_line_char:=-1;
      {temporarily disable new-line character}
    j:=str_start[s];
    while j<str_start[s+1] do
      begin print_char(so(str_pool[j])); incr(j);
      end;
    new_line_char:=nl; return;
    end;
j:=str_start[s];
while j<str_start[s+1] do
@y
@ An entire string is output by calling |print|. Note that if we are outputting
the single standard ASCII character \.c, we could call |print("c")|, since
|"c"=99| is the number of a single-character string, as explained above. But
|print_char("c")| is quicker, so \TeX\ goes directly to the |print_char|
routine when it knows that this is safe. (The present implementation
assumes that it is always safe to print a visible ASCII character.)
@^system dependencies@>

@d print_lc_hex(#)==l:=#;
   if l<10 then print_char(l+"0") else print_char(l-10+"a");

@<Basic print...@>=
procedure print(@!s:integer); {prints string |s|}
label exit;
var j:pool_pointer; {current character code position}
@!nl:integer; {new-line character to restore}
@!l:integer; {for printing 16-bit characters}
begin if s>=str_ptr then s:="???" {this can't happen}
@.???@>
else if s<biggest_char then
  if s<0 then s:="???" {can't happen}
  else begin if selector>pseudo then
      begin print_char(s); return; {internal strings are not expanded}
      end;
    if (@<Character |s| is the current new-line character@>) then
      if selector<pseudo then
        begin print_ln; return;
        end;
    nl:=new_line_char;
    @<Set newline character to -1@>;
    if s<@"20 then begin
      print_char(@"5E); print_char(@"5E); print_char(s+@'100);
      end
    else if s<@"7F then
      print_char(s)
    else if s=@"7F then begin
      print_char(@"5E); print_char(@"5E); print_char(s-@'100);
      end
    else if s<@"100 then begin
      print_char(@"5E); print_char(@"5E);
      print_lc_hex((s mod @"100) div @"10); print_lc_hex(s mod @"10);
      end
    else begin {Here is where we generate the strings on the fly.}
      print_char(@"5E); print_char(@"5E);
      print_char(@"5E); print_char(@"5E);
      print_lc_hex(s div @"1000); print_lc_hex((s mod @"1000) div @"100);
      print_lc_hex((s mod @"100) div @"10); print_lc_hex(s mod @"10);
      end;
    @<Set newline character to nl@>;
    return;
    end;
j:=str_start(s);
while j<str_start(s+1) do
@z
%---------------------------------------
@x [5] m.60 l.1539 - Omega
procedure slow_print(@!s:integer); {prints string |s|}
var j:pool_pointer; {current character code position}
begin if (s>=str_ptr) or (s<256) then print(s)
else begin j:=str_start[s];
  while j<str_start[s+1] do
@y
procedure slow_print(@!s:integer); {prints string |s|}
var j:pool_pointer; {current character code position}
begin if (s>=str_ptr) or (s<=biggest_char) then print(s)
else begin j:=str_start(s);
  while j<str_start(s+1) do
@z
%---------------------------------------
@x [5] m.61 l.1556 - Omega
wterm(banner);
if format_ident=0 then wterm_ln(' (no format preloaded)')
else  begin slow_print(format_ident); print_ln;
  end;
update_terminal;
@y
wterm(banner);
if format_ident=0 then wterm_ln(' (no format preloaded)')
else  begin slow_print(format_ident); print_ln;
  end;
wterm_ln('Copyright (c) 1994--1999 John Plaice and Yannis Haralambous');
update_terminal;
@z
%---------------------------------------
@x [5] m.63 l.1576 - Omega
procedure print_esc(@!s:str_number); {prints escape character, then |s|}
var c:integer; {the escape character code}
begin  @<Set variable |c| to the current escape character@>;
if c>=0 then if c<256 then print(c);
@y
procedure print_esc(@!s:str_number); {prints escape character, then |s|}
var c:integer; {the escape character code}
begin  @<Set variable |c| to the current escape character@>;
if c>=0 then if c<=biggest_char then print(c);
@z
%---------------------------------------
@x [5] m.69 l.1654 - Omega
@p procedure print_roman_int(@!n:integer);
label exit;
var j,@!k: pool_pointer; {mysterious indices into |str_pool|}
@!u,@!v: nonnegative_integer; {mysterious numbers}
begin j:=str_start["m2d5c2l5x2v5i"]; v:=1000;
@y
@p procedure print_roman_int(@!n:integer);
label exit;
var j,@!k: pool_pointer; {mysterious indices into |str_pool|}
@!u,@!v: nonnegative_integer; {mysterious numbers}
begin j:=str_start("m2d5c2l5x2v5i"); v:=1000;
@z
%---------------------------------------
@x [5] m.70 l.1674 - Omega
@p procedure print_current_string; {prints a yet-unmade string}
var j:pool_pointer; {points to current character code}
begin j:=str_start[str_ptr];
@y
@p procedure print_current_string; {prints a yet-unmade string}
var j:pool_pointer; {points to current character code}
begin j:=str_start(str_ptr);
@z
%---------------------------------------
@x [6] m.94 l.2052 - Omega
print_err("TeX capacity exceeded, sorry [");
@y
print_err("Omega capacity exceeded, sorry [");
@z
%---------------------------------------
@x [8] m.110 l.2355 - Omega
In order to make efficient use of storage space, \TeX\ bases its major data
structures on a |memory_word|, which contains either a (signed) integer,
possibly scaled, or a (signed) |glue_ratio|, or a small number of
fields that are one half or one quarter of the size used for storing
integers.

@y
In order to make efficient use of storage space, \TeX\ bases its major data
structures on a |memory_word|, which contains either a (signed) integer,
possibly scaled, or a (signed) |glue_ratio|, or a small number of
fields that are one half or one quarter of the size used for storing
integers.

That is the description for \TeX.  For $\Omega$, we are going to keep
the same terminology, except that |quarterword| is going to refer to
16~bits, and |halfword| is going to refer to 32~bits.  So, in fact,
a |memory_word| will take 64 bits, and, on a 64-bit machine, will run
smaller than \TeX\ will!

@z
%---------------------------------------
@x [8] m.110 l.2378 - Omega
Since we are assuming 32-bit integers, a halfword must contain at least
16 bits, and a quarterword must contain at least 8 bits.
@^system dependencies@>
But it doesn't hurt to have more bits; for example, with enough 36-bit
words you might be able to have |mem_max| as large as 262142, which is
eight times as much memory as anybody had during the first four years of
\TeX's existence.

N.B.: Valuable memory space will be dreadfully wasted unless \TeX\ is compiled
by a \PASCAL\ that packs all of the |memory_word| variants into
the space of a single integer. This means, for example, that |glue_ratio|
words should be |short_real| instead of |real| on some computers. Some
\PASCAL\ compilers will pack an integer whose subrange is `|0..255|' into
an eight-bit field, but others insist on allocating space for an additional
sign bit; on such systems you can get 256 values into a quarterword only
if the subrange is `|-128..127|'.

@y
@z
%---------------------------------------
@x [8] m.110 l.2406 - Omega
@d max_quarterword=255 {largest allowable value in a |quarterword|}
@d min_halfword==0 {smallest allowable value in a |halfword|}
@d max_halfword==65535 {largest allowable value in a |halfword|}
@y
@d max_quarterword=@"FFFF {largest allowable value in a |quarterword|}
@d min_halfword=0 {smallest allowable value in a |halfword|}
@d max_halfword=@"3FFFFFFF {largest allowable value in a |halfword|}
@z
%---------------------------------------
@x [8] m.111 l.2416 - Omega
if (min_quarterword>0)or(max_quarterword<127) then bad:=11;
if (min_halfword>0)or(max_halfword<32767) then bad:=12;
@y
if (min_quarterword>0)or(max_quarterword<@"7FFF) then bad:=11;
if (min_halfword>0)or(max_halfword<@"3FFFFFFF) then bad:=12;
@z
%---------------------------------------
@x [8] m.111 l.2423 - Omega
if font_max>font_base+256 then bad:=16;
@y
if font_max>font_base+@"10000 then bad:=16;
@z
%---------------------------------------
@x [8] m.111 l.2426 - Omega
if max_quarterword-min_quarterword<255 then bad:=19;
@y
if max_quarterword-min_quarterword<@"FFFF then bad:=19;
@z
%---------------------------------------
@x [10] m.134 l.2832 - Omega
Note that the format of a |char_node| allows for up to 256 different
fonts and up to 256 characters per font; but most implementations will
probably limit the total number of fonts to fewer than 75 per job,
and most fonts will stick to characters whose codes are
less than 128 (since higher codes
are more difficult to access on most keyboards).

Extensions of \TeX\ intended for oriental languages will need even more
than $256\times256$ possible characters, when we consider different sizes
@^oriental characters@>@^Chinese characters@>@^Japanese characters@>
and styles of type.  It is suggested that Chinese and Japanese fonts be
handled by representing such characters in two consecutive |char_node|
entries: The first of these has |font=font_base|, and its |link| points
to the second;
the second identifies the font and the character dimensions.
The saving feature about oriental characters is that most of them have
the same box dimensions. The |character| field of the first |char_node|
is a ``\\{charext}'' that distinguishes between graphic symbols whose
dimensions are identical for typesetting purposes. (See the \MF\ manual.)
Such an extension of \TeX\ would not be difficult; further details are
left to the reader.
@y
Note that the format of a |char_node| allows for up to 65536 different
fonts and up to 65536 characters per font. 
@z
%---------------------------------------
@x [10] m.160 l.3270 - Omega
@ In fact, there are still more types coming. When we get to math formula
processing we will see that a |style_node| has |type=14|; and a number
of larger type codes will also be defined, for use in math mode only.
@y
@ In fact, there are still more types coming. When we get to math formula
processing we will see that a |style_node| has |type=14|; and a number
of larger type codes will also be defined, for use in math mode only.

@d sgml_text_node=0
@d sgml_math_node=1

@d sgml_node=unset_node+1 {|type| for an SGML node}
@d sgml_node_size=5
@d sgml_tag(#)==mem[#+1].int
@d sgml_attrs(#)==mem[#+2].int
@d sgml_singleton(#)==info(#+3)
@d sgml_info(#)==link(#+3)
@d sgml_kind(#)==mem[#+4].int

@d sgml_attr_node=unset_node+2 {|type| for an SGML attribute node}
@d sgml_attr_node_size=4

@d sgml_entity_node=unset_node+3 {|type| for an SGML entity node}
@d sgml_entity_node_size=2
@d sgml_entity_string(#)==mem[#+1].int

@d biggest_ordinary_node=sgml_entity_node

@p
function new_sgml_node:pointer;
var p:pointer;
begin p:=get_node(sgml_node_size);
type(p):=sgml_node;
sgml_tag(p):=0;
sgml_attrs(p):=0;
sgml_info(p):=0;
sgml_singleton(p):=0;
sgml_kind(p):=0;
new_sgml_node:=p;
end;

function new_sgml_attr_node:pointer;
var p:pointer;
begin p:=get_node(sgml_attr_node_size);
type(p):=sgml_attr_node;
sgml_tag(p):=0;
sgml_attrs(p):=0;
sgml_info(p):=0;
sgml_singleton(p):=0;
new_sgml_attr_node:=p;
end;

function new_sgml_entity_node:pointer;
var p:pointer;
begin p:=get_node(sgml_entity_node_size);
type(p):=sgml_entity_node;
sgml_entity_string(p):=0;
new_sgml_entity_node:=p;
end;
@z
%---------------------------------------
@x [12] m.174 l.3520 - Omega
@p procedure short_display(@!p:integer); {prints highlights of list |p|}
var n:integer; {for replacement counts}
begin while p>mem_min do
  begin if is_char_node(p) then
    begin if p<=mem_end then
      begin if font(p)<>font_in_short_display then
        begin if (font(p)<font_base)or(font(p)>font_max) then
          print_char("*")
@.*\relax@>
        else @<Print the font identifier for |font(p)|@>;
        print_char(" "); font_in_short_display:=font(p);
        end;
      print_ASCII(qo(character(p)));
@y
@p procedure short_display(@!p:integer); {prints highlights of list |p|}
var n:integer; {for replacement counts}
    fsort:integer;
begin while p>mem_min do
  begin if is_char_node(p) then
    begin if p<=mem_end then
      begin if font(p)<>font_in_short_display then
        begin if (font(p)<font_base)or(font(p)>font_max) then
          print_char("*")
@.*\relax@>
        else @<Print the font identifier for |font(p)|@>;
        print_char(" "); font_in_short_display:=font(p);
        end;
      if not SGML_show_entities then
        print_ASCII(qo(character(p)))
      else begin
        fsort:=font_name_sort(font(p));
        if fsort<>0 then begin
          if (font_sort_char_entity(fsort)(character(p))<>0) then
            slow_print(font_sort_char_entity(fsort)(character(p)))
          else
            print_ASCII(qo(character(p)));
          end
        else
          print_ASCII(qo(character(p)));
        end;
@z
%---------------------------------------
@x [12] m.176 l.3563 - Omega
@p procedure print_font_and_char(@!p:integer); {prints |char_node| data}
begin if p>mem_end then print_esc("CLOBBERED.")
else  begin if (font(p)<font_base)or(font(p)>font_max) then print_char("*")
@.*\relax@>
  else @<Print the font identifier for |font(p)|@>;
  print_char(" "); print_ASCII(qo(character(p)));
  end;
end;
@y
@p procedure print_font_and_char(@!p:integer); {prints |char_node| data}
var fsort:integer;
begin if p>mem_end then print_esc("CLOBBERED.")
else  begin if (font(p)<font_base)or(font(p)>font_max) then print_char("*")
@.*\relax@>
  else @<Print the font identifier for |font(p)|@>;
  print_char(" ");
  if not SGML_show_entities then
    print_ASCII(qo(character(p)))
  else begin
    fsort:=font_name_sort(font(p));
    if fsort<>0 then begin
      if (font_sort_char_entity(fsort)(character(p))<>0) then
        print(font_sort_char_entity(fsort)(character(p)))
      else
        print_ASCII(qo(character(p)));
      end
    else
      print_ASCII(qo(character(p)));
    end;
  end;
end;
@z
%---------------------------------------
@x [13] m.202 l.3928 - Omega
    adjust_node: flush_node_list(adjust_ptr(p));
@y
    adjust_node: flush_node_list(adjust_ptr(p));
    sgml_node: begin
      if sgml_tag(p)<>0 then flush_node_list(sgml_attrs(p));
      free_node(p,sgml_node_size); goto done;
      end;
    sgml_attr_node: begin
      free_node(p,sgml_attr_node_size); goto done;
      end;
    sgml_entity_node: begin
      free_node(p,sgml_entity_node_size); goto done;
      end;
@z
%---------------------------------------
@x [15] m.208 l.4127 - Omega
@d min_internal=68 {the smallest code that can follow \.{\\the}}
@d char_given=68 {character code defined by \.{\\chardef}}
@d math_given=69 {math code defined by \.{\\mathchardef}}
@d last_item=70 {most recent item ( \.{\\lastpenalty},
  \.{\\lastkern}, \.{\\lastskip} )}
@d max_non_prefixed_command=70 {largest command code that can't be \.{\\global}}
@y
@d char_ghost=68 {\.{\\ghostleft}, \.{\\ghostright} character for kerning}
@d min_internal=69 {the smallest code that can follow \.{\\the}}
@d char_given=min_internal {character code defined by \.{\\chardef}}
@d math_given=min_internal+1 {math code defined by \.{\\mathchardef}}
@d omath_given=min_internal+2 {math code defined by \.{\\omathchardef}}
@d last_item=min_internal+3 {most recent item ( \.{\\lastpenalty},
  \.{\\lastkern}, \.{\\lastskip} )}
@d max_non_prefixed_command=min_internal+3
   {largest command code that can't be \.{\\global}}
@z
%---------------------------------------
@x [15] m.209 l.4138 - Omega
@d toks_register=71 {token list register ( \.{\\toks} )}
@d assign_toks=72 {special token list ( \.{\\output}, \.{\\everypar}, etc.~)}
@d assign_int=73 {user-defined integer ( \.{\\tolerance}, \.{\\day}, etc.~)}
@d assign_dimen=74 {user-defined length ( \.{\\hsize}, etc.~)}
@d assign_glue=75 {user-defined glue ( \.{\\baselineskip}, etc.~)}
@d assign_mu_glue=76 {user-defined muglue ( \.{\\thinmuskip}, etc.~)}
@d assign_font_dimen=77 {user-defined font dimension ( \.{\\fontdimen} )}
@d assign_font_int=78 {user-defined font integer ( \.{\\hyphenchar},
  \.{\\skewchar} )}
@d set_aux=79 {specify state info ( \.{\\spacefactor}, \.{\\prevdepth} )}
@d set_prev_graf=80 {specify state info ( \.{\\prevgraf} )}
@d set_page_dimen=81 {specify state info ( \.{\\pagegoal}, etc.~)}
@d set_page_int=82 {specify state info ( \.{\\deadcycles},
  \.{\\insertpenalties} )}
@d set_box_dimen=83 {change dimension of box ( \.{\\wd}, \.{\\ht}, \.{\\dp} )}
@d set_shape=84 {specify fancy paragraph shape ( \.{\\parshape} )}
@d def_code=85 {define a character code ( \.{\\catcode}, etc.~)}
@d def_family=86 {declare math fonts ( \.{\\textfont}, etc.~)}
@d set_font=87 {set current font ( font identifiers )}
@d def_font=88 {define a font file ( \.{\\font} )}
@d register=89 {internal register ( \.{\\count}, \.{\\dimen}, etc.~)}
@d max_internal=89 {the largest code that can follow \.{\\the}}
@d advance=90 {advance a register or parameter ( \.{\\advance} )}
@d multiply=91 {multiply a register or parameter ( \.{\\multiply} )}
@d divide=92 {divide a register or parameter ( \.{\\divide} )}
@d prefix=93 {qualify a definition ( \.{\\global}, \.{\\long}, \.{\\outer} )}
@d let=94 {assign a command code ( \.{\\let}, \.{\\futurelet} )}
@d shorthand_def=95 {code definition ( \.{\\chardef}, \.{\\countdef}, etc.~)}
@d read_to_cs=96 {read into a control sequence ( \.{\\read} )}
@d def=97 {macro definition ( \.{\\def}, \.{\\gdef}, \.{\\xdef}, \.{\\edef} )}
@d set_box=98 {set a box ( \.{\\setbox} )}
@d hyph_data=99 {hyphenation data ( \.{\\hyphenation}, \.{\\patterns} )}
@d set_interaction=100 {define level of interaction ( \.{\\batchmode}, etc.~)}
@d max_command=100 {the largest command code seen at |big_switch|}
@y
@d toks_register=max_non_prefixed_command+1
   {token list register ( \.{\\toks} )}
@d assign_toks=max_non_prefixed_command+2
   {special token list ( \.{\\output}, \.{\\everypar}, etc.~)}
@d assign_int=max_non_prefixed_command+3
   {user-defined integer ( \.{\\tolerance}, \.{\\day}, etc.~)}
@d assign_dimen=max_non_prefixed_command+4
   {user-defined length ( \.{\\hsize}, etc.~)}
@d assign_glue=max_non_prefixed_command+5
   {user-defined glue ( \.{\\baselineskip}, etc.~)}
@d assign_mu_glue=max_non_prefixed_command+6
   {user-defined muglue ( \.{\\thinmuskip}, etc.~)}
@d assign_font_dimen=max_non_prefixed_command+7
   {user-defined font dimension ( \.{\\fontdimen} )}
@d assign_font_int=max_non_prefixed_command+8
   {user-defined font integer ( \.{\\hyphenchar}, \.{\\skewchar} )}
@d set_aux=max_non_prefixed_command+9
   {specify state info ( \.{\\spacefactor}, \.{\\prevdepth} )}
@d set_prev_graf=max_non_prefixed_command+10
   {specify state info ( \.{\\prevgraf} )}
@d set_page_dimen=max_non_prefixed_command+11
   {specify state info ( \.{\\pagegoal}, etc.~)}
@d set_page_int=max_non_prefixed_command+12
   {specify state info ( \.{\\deadcycles},
  \.{\\insertpenalties} )}
@d set_box_dimen=max_non_prefixed_command+13
   {change dimension of box ( \.{\\wd}, \.{\\ht}, \.{\\dp} )}
@d set_shape=max_non_prefixed_command+14
   {specify fancy paragraph shape ( \.{\\parshape} )}
@d def_code=max_non_prefixed_command+15
   {define a character code ( \.{\\catcode}, etc.~)}
@d def_family=max_non_prefixed_command+16
   {declare math fonts ( \.{\\textfont}, etc.~)}
@d set_font=max_non_prefixed_command+17
   {set current font ( font identifiers )}
@d def_font=max_non_prefixed_command+18
   {define a font file ( \.{\\font} )}
@d register=max_non_prefixed_command+19
   {internal register ( \.{\\count}, \.{\\dimen}, etc.~)}
@d max_internal=max_non_prefixed_command+20
   {the largest code that can follow \.{\\the}}
@d advance=max_non_prefixed_command+21
   {advance a register or parameter ( \.{\\advance} )}
@d multiply=max_non_prefixed_command+22
   {multiply a register or parameter ( \.{\\multiply} )}
@d divide=max_non_prefixed_command+23
   {divide a register or parameter ( \.{\\divide} )}
@d prefix=max_non_prefixed_command+24
   {qualify a definition ( \.{\\global}, \.{\\long}, \.{\\outer} )}
@d let=max_non_prefixed_command+25
   {assign a command code ( \.{\\let}, \.{\\futurelet} )}
@d shorthand_def=max_non_prefixed_command+26
   {code definition ( \.{\\chardef}, \.{\\countdef}, etc.~)}
@d read_to_cs=max_non_prefixed_command+27
   {read into a control sequence ( \.{\\read} )}
@d def=max_non_prefixed_command+28
   {macro definition ( \.{\\def}, \.{\\gdef}, \.{\\xdef}, \.{\\edef} )}
@d set_box=max_non_prefixed_command+29
   {set a box ( \.{\\setbox} )}
@d hyph_data=max_non_prefixed_command+30
   {hyphenation data ( \.{\\hyphenation}, \.{\\patterns} )}
@d set_show_sgml_entities=max_non_prefixed_command+31
   {printing hex code or SGML-like entity during \.{\\showbox}}
@d set_mml_mode=max_non_prefixed_command+32
   {for entering or leaving MML mode ( \.{\\mmlmode}~)}
@d mml_tag=max_non_prefixed_command+33
   {for various SGML tags}
@d set_interaction=max_non_prefixed_command+34
   {define level of interaction ( \.{\\batchmode}, etc.~)}
@d max_command=set_interaction
   {the largest command code seen at |big_switch|}
@z
%---------------------------------------
@x [16] m.212--217 l.4303 - Omega
@!list_state_record=record@!mode_field:-mmode..mmode;@+
  @!head_field,@!tail_field: pointer;
  @!pg_field,@!ml_field: integer;@+
  @!aux_field: memory_word;
  end;

@ @d mode==cur_list.mode_field {current mode}
@d head==cur_list.head_field {header node of current list}
@d tail==cur_list.tail_field {final node on current list}
@d prev_graf==cur_list.pg_field {number of paragraph lines accumulated}
@d aux==cur_list.aux_field {auxiliary data about the current list}
@d prev_depth==aux.sc {the name of |aux| in vertical mode}
@d space_factor==aux.hh.lh {part of |aux| in horizontal mode}
@d clang==aux.hh.rh {the other part of |aux| in horizontal mode}
@d incompleat_noad==aux.int {the name of |aux| in math mode}
@d mode_line==cur_list.ml_field {source file line number at beginning of list}

@<Glob...@>=
@!nest:array[0..nest_size] of list_state_record;
@!nest_ptr:0..nest_size; {first unused location of |nest|}
@!max_nest_stack:0..nest_size; {maximum of |nest_ptr| when pushing}
@!cur_list:list_state_record; {the ``top'' semantic state}
@!shown_mode:-mmode..mmode; {most recent mode shown by \.{\\tracingcommands}}

@ Here is a common way to make the current list grow:

@d tail_append(#)==begin link(tail):=#; tail:=link(tail);
  end

@ We will see later that the vertical list at the bottom semantic level is split
into two parts; the ``current page'' runs from |page_head| to |page_tail|,
and the ``contribution list'' runs from |contrib_head| to |tail| of
semantic level zero. The idea is that contributions are first formed in
vertical mode, then ``contributed'' to the current page (during which time
the page-breaking decisions are made). For now, we don't need to know
any more details about the page-building process.

@<Set init...@>=
nest_ptr:=0; max_nest_stack:=0;
mode:=vmode; head:=contrib_head; tail:=contrib_head;
prev_depth:=ignore_depth; mode_line:=0;
prev_graf:=0; shown_mode:=0;
@<Start a new current page@>;

@ When \TeX's work on one level is interrupted, the state is saved by
calling |push_nest|. This routine changes |head| and |tail| so that
a new (empty) list is begun; it does not change |mode| or |aux|.

@p procedure push_nest; {enter a new semantic level, save the old}
begin if nest_ptr>max_nest_stack then
  begin max_nest_stack:=nest_ptr;
  if nest_ptr=nest_size then overflow("semantic nest size",nest_size);
@:TeX capacity exceeded semantic nest size}{\quad semantic nest size@>
  end;
nest[nest_ptr]:=cur_list; {stack the record}
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
end;

@ Conversely, when \TeX\ is finished on the current level, the former
state is restored by calling |pop_nest|. This routine will never be
called at the lowest semantic level, nor will it be called unless |head|
is a node that should be returned to free memory.

@p procedure pop_nest; {leave a semantic level, re-enter the old}
begin free_avail(head); decr(nest_ptr); cur_list:=nest[nest_ptr];
end;

@y
@!list_state_record=record@!mode_field:-mmode..mmode;@+
  @!head_field,@!tail_field: pointer;
  @!pg_field,@!ml_field: integer;@+
  @!sgml_field: integer;
  @!sgml_attr_field: pointer;
  @!aux_field: memory_word;
  end;

@ @d mode==cur_list.mode_field {current mode}
@d head==cur_list.head_field {header node of current list}
@d tail==cur_list.tail_field {final node on current list}
@d prev_graf==cur_list.pg_field {number of paragraph lines accumulated}
@d current_sgml_tag==cur_list.sgml_field {current SGML tag}
@d current_sgml_attrs==cur_list.sgml_attr_field {current SGML tag}
@d aux==cur_list.aux_field {auxiliary data about the current list}
@d prev_depth==aux.sc {the name of |aux| in vertical mode}
@d space_factor==aux.hh.lh {part of |aux| in horizontal mode}
@d clang==aux.hh.rh {the other part of |aux| in horizontal mode}
@d incompleat_noad==aux.int {the name of |aux| in math mode}
@d mode_line==cur_list.ml_field {source file line number at beginning of list}

@<Glob...@>=
@!nest:array[0..nest_size] of list_state_record;
@!nest_ptr:0..nest_size; {first unused location of |nest|}
@!max_nest_stack:0..nest_size; {maximum of |nest_ptr| when pushing}
@!cur_list:list_state_record; {the ``top'' semantic state}
@!shown_mode:-mmode..mmode; {most recent mode shown by \.{\\tracingcommands}}

@ Here is a common way to make the current list grow:

@d tail_append(#)==begin link(tail):=#; tail:=link(tail);
  end

@ We will see later that the vertical list at the bottom semantic level is split
into two parts; the ``current page'' runs from |page_head| to |page_tail|,
and the ``contribution list'' runs from |contrib_head| to |tail| of
semantic level zero. The idea is that contributions are first formed in
vertical mode, then ``contributed'' to the current page (during which time
the page-breaking decisions are made). For now, we don't need to know
any more details about the page-building process.

@<Set init...@>=
nest_ptr:=0; max_nest_stack:=0;
mode:=vmode; head:=contrib_head; tail:=contrib_head;
prev_depth:=ignore_depth; mode_line:=0;
prev_graf:=0; shown_mode:=0;
current_sgml_tag:=0; current_sgml_attrs:=null;
@<Start a new current page@>;

@ When \TeX's work on one level is interrupted, the state is saved by
calling |push_nest|. This routine changes |head| and |tail| so that
a new (empty) list is begun; it does not change |mode| or |aux|.

@p procedure push_nest; {enter a new semantic level, save the old}
begin if nest_ptr>max_nest_stack then
  begin max_nest_stack:=nest_ptr;
  if nest_ptr=nest_size then overflow("semantic nest size",nest_size);
@:TeX capacity exceeded semantic nest size}{\quad semantic nest size@>
  end;
nest[nest_ptr]:=cur_list; {stack the record}
incr(nest_ptr); head:=get_avail; tail:=head; prev_graf:=0; mode_line:=line;
current_sgml_tag:=0; current_sgml_attrs:=null;
end;

@ Conversely, when \TeX\ is finished on the current level, the former
state is restored by calling |pop_nest|. This routine will never be
called at the lowest semantic level, nor will it be called unless |head|
is a node that should be returned to free memory.

@p procedure pop_nest; {leave a semantic level, re-enter the old}
var attrs,p:pointer;
begin attrs:=null;
{
if current_sgml_attrs<>null then
  if current_sgml_tag=null then
    attrs:=current_sgml_attrs
  else
    free_avail(current_sgml_attrs);
}
free_avail(head); decr(nest_ptr); cur_list:=nest[nest_ptr];
{
if attrs<>null then begin
  p:=attrs;
  while link(p)<>null do
    begin
    p:=link(p);
    end;
  link(p):=current_sgml_attrs;
  current_sgml_attrs:=attrs;
  end;
}
end;

@z
%---------------------------------------
@x [17] m.220 l.4486 - Omega
@d eq_level(#)==eq_level_field(eqtb[#]) {level of definition}
@d eq_type(#)==eq_type_field(eqtb[#]) {command code for equivalent}
@d equiv(#)==equiv_field(eqtb[#]) {equivalent value}
@y
@d eq_level(#)==new_eq_level(#) {level of definition}
@d eq_type(#)==new_eq_type(#) {command code for equivalent}
@d equiv(#)==new_equiv(#) {equivalent value}
@d equiv1(#)==new_equiv1(#) {equivalent value}
@z
%---------------------------------------
@x [17] m.222 l.4496 - Omega
In the first region we have 256 equivalents for ``active characters'' that
act as control sequences, followed by 256 equivalents for single-character
control sequences.
@y
In the first region we have |number_chars| equivalents for ``active characters''
that act as control sequences, followed by |number_chars| equivalents for 
single-character control sequences.
@z
%---------------------------------------
@x [17] m.222 l.4507 - Omega
@d single_base=active_base+256 {equivalents of one-character control sequences}
@d null_cs=single_base+256 {equivalent of \.{\\csname\\endcsname}}
@y
@d single_base=active_base+number_chars 
   {equivalents of one-character control sequences}
@d null_cs=single_base+number_chars {equivalent of \.{\\csname\\endcsname}}
@z
%---------------------------------------
@x [17] m.222 l.4525 - Omega
  {begins table of 257 permanent font identifiers}
@d undefined_control_sequence=frozen_null_font+257 {dummy location}
@y
   {begins table of |number_fonts| permanent font identifiers}
@d frozen_null_font_sort=frozen_null_font+number_fonts {permanent null sort}
@d undefined_control_sequence=frozen_null_font_sort+number_font_sorts
   {dummy location}
@z
%---------------------------------------
@x [17] m.222 l.4530 - Omega
eq_type(undefined_control_sequence):=undefined_cs;
equiv(undefined_control_sequence):=null;
eq_level(undefined_control_sequence):=level_zero;
for k:=active_base to undefined_control_sequence-1 do
  eqtb[k]:=eqtb[undefined_control_sequence];
@y
set_eq_type(undefined_control_sequence,undefined_cs);
set_equiv(undefined_control_sequence,null);
set_eq_level(undefined_control_sequence,level_zero);
@z
%---------------------------------------
@x [17] m.224 l.4547 - Omega
@ Region 3 of |eqtb| contains the 256 \.{\\skip} registers, as well as the
glue parameters defined here. It is important that the ``muskip''
parameters have larger numbers than the others.
@y
@ Region 3 of |eqtb| contains the |number_regs| \.{\\skip} registers, 
as well as the glue parameters defined here. It is important that the 
``muskip'' parameters have larger numbers than the others.
@z
%---------------------------------------
@x [17] m.224 l.4572 - Omega
@d skip_base=glue_base+glue_pars {table of 256 ``skip'' registers}
@d mu_skip_base=skip_base+256 {table of 256 ``muskip'' registers}
@d local_base=mu_skip_base+256 {beginning of region 4}
@y
@d skip_base=glue_base+glue_pars {table of |number_regs| ``skip'' registers}
@d mu_skip_base=skip_base+number_regs 
   {table of |number_regs| ``muskip'' registers}
@d local_base=mu_skip_base+number_regs {beginning of region 4}
@z
%---------------------------------------
@x [17] m.228 l.4685 - Omega
equiv(glue_base):=zero_glue; eq_level(glue_base):=level_one;
eq_type(glue_base):=glue_ref;
for k:=glue_base+1 to local_base-1 do eqtb[k]:=eqtb[glue_base];
@y
set_equiv(glue_base,zero_glue); set_eq_level(glue_base,level_one);
set_eq_type(glue_base,glue_ref);
@z
%---------------------------------------
@x [17] m.230 l.4721 - Omega
@d toks_base=local_base+10 {table of 256 token list registers}
@d box_base=toks_base+256 {table of 256 box registers}
@d cur_font_loc=box_base+256 {internal font number outside math mode}
@d math_font_base=cur_font_loc+1 {table of 48 math font numbers}
@d cat_code_base=math_font_base+48
  {table of 256 command codes (the ``catcodes'')}
@d lc_code_base=cat_code_base+256 {table of 256 lowercase mappings}
@d uc_code_base=lc_code_base+256 {table of 256 uppercase mappings}
@d sf_code_base=uc_code_base+256 {table of 256 spacefactor mappings}
@d math_code_base=sf_code_base+256 {table of 256 math mode mappings}
@d int_base=math_code_base+256 {beginning of region 5}
@y
@d toks_base=local_base+10 {table of |number_regs| token list registers}
@d box_base=toks_base+number_regs {table of |number_regs| box registers}
@d cur_font_loc=box_base+number_regs {internal font number outside math mode}
@d math_font_base=cur_font_loc+1
   {table of |number_math_fonts| math font numbers}
@d cat_code_base=math_font_base+number_math_fonts
  {table of |number_chars| command codes (the ``catcodes'')}
@d lc_code_base=cat_code_base+number_chars 
  {table of |number_chars| lowercase mappings}
@d uc_code_base=lc_code_base+number_chars 
  {table of |number_chars| uppercase mappings}
@d sf_code_base=uc_code_base+number_chars 
  {table of |number_chars| spacefactor mappings}
@d math_code_base=sf_code_base+number_chars 
  {table of |number_chars| math mode mappings}
@d int_base=math_code_base+number_chars {beginning of region 5}
@z
%---------------------------------------
@x [17] m.232 l.4802 - Omega
@d var_code==@'70000 {math code meaning ``use the current family''}

@<Initialize table entries...@>=
par_shape_ptr:=null; eq_type(par_shape_loc):=shape_ref;
eq_level(par_shape_loc):=level_one;@/
for k:=output_routine_loc to toks_base+255 do
  eqtb[k]:=eqtb[undefined_control_sequence];
box(0):=null; eq_type(box_base):=box_ref; eq_level(box_base):=level_one;
for k:=box_base+1 to box_base+255 do eqtb[k]:=eqtb[box_base];
cur_font:=null_font; eq_type(cur_font_loc):=data;
eq_level(cur_font_loc):=level_one;@/
for k:=math_font_base to math_font_base+47 do eqtb[k]:=eqtb[cur_font_loc];
equiv(cat_code_base):=0; eq_type(cat_code_base):=data;
eq_level(cat_code_base):=level_one;@/
for k:=cat_code_base+1 to int_base-1 do eqtb[k]:=eqtb[cat_code_base];
for k:=0 to 255 do
  begin cat_code(k):=other_char; math_code(k):=hi(k); sf_code(k):=1000;
  end;
cat_code(carriage_return):=car_ret; cat_code(" "):=spacer;
cat_code("\"):=escape; cat_code("%"):=comment;
cat_code(invalid_code):=invalid_char; cat_code(null_code):=ignore;
for k:="0" to "9" do math_code(k):=hi(k+var_code);
for k:="A" to "Z" do
  begin cat_code(k):=letter; cat_code(k+"a"-"A"):=letter;@/
  math_code(k):=hi(k+var_code+@"100);
  math_code(k+"a"-"A"):=hi(k+"a"-"A"+var_code+@"100);@/
  lc_code(k):=k+"a"-"A"; lc_code(k+"a"-"A"):=k+"a"-"A";@/
  uc_code(k):=k; uc_code(k+"a"-"A"):=k;@/
  sf_code(k):=999;
  end;
@y
@d null_font_sort==font_sort_base
@d var_code==@"7000000 {math code meaning ``use the current family''}

@<Initialize table entries...@>=
set_equiv(par_shape_loc,null); set_eq_type(par_shape_loc,shape_ref);
set_eq_level(par_shape_loc,level_one);@/
set_equiv(cat_code_base+carriage_return,car_ret);
set_equiv(cat_code_base+" ",spacer);
set_equiv(cat_code_base+"\",escape);
set_equiv(cat_code_base+"%",comment);
set_equiv(cat_code_base+invalid_code,invalid_char);
set_equiv(cat_code_base+null_code,ignore);
for k:="0" to "9" do set_equiv(math_code_base+k,hi(k+var_code));
for k:="A" to "Z" do
  begin set_equiv(cat_code_base+k,letter);
  set_equiv(cat_code_base+k+"a"-"A",letter);@/
  set_equiv(math_code_base+k,hi(k+var_code+@"10000));
  set_equiv(math_code_base+k+"a"-"A",hi(k+"a"-"A"+var_code+@"10000));@/
  set_equiv(lc_code_base+k,k+"a"-"A");
  set_equiv(lc_code_base+k+"a"-"A",k+"a"-"A");@/
  set_equiv(uc_code_base+k,k);
  set_equiv(uc_code_base+k+"a"-"A",k);@/
  set_equiv(sf_code_base+k,999);
  end;
@z
%---------------------------------------
@x [17] m.234 l.4857 - Omega
@ @<Show the font identifier in |eqtb[n]|@>=
begin if n=cur_font_loc then print("current font")
else if n<math_font_base+16 then
  begin print_esc("textfont"); print_int(n-math_font_base);
  end
else if n<math_font_base+32 then
  begin print_esc("scriptfont"); print_int(n-math_font_base-16);
  end
else  begin print_esc("scriptscriptfont"); print_int(n-math_font_base-32);
  end;
print_char("=");@/
print_esc(hash[font_id_base+equiv(n)].rh);
  {that's |font_id_text(equiv(n))|}
@y
@ @<Show the font identifier in |eqtb[n]|@>=
begin if n=cur_font_loc then print("current font")
else if n<math_font_base+script_size then
  begin print_esc("textfont"); print_int(n-math_font_base);
  end
else if n<math_font_base+script_script_size then
  begin print_esc("scriptfont"); print_int(n-math_font_base-script_size);
  end
else  begin print_esc("scriptscriptfont");
  print_int(n-math_font_base-script_script_size);
  end;
print_char("=");@/
print_esc(newtext(font_id_base+equiv(n)));
  {that's |font_id_text(equiv(n))|}
@z
%---------------------------------------
@x [17] m.236 l.4955 - Omega
@d count_base=int_base+int_pars {256 user \.{\\count} registers}
@d del_code_base=count_base+256 {256 delimiter code mappings}
@d dimen_base=del_code_base+256 {beginning of region 6}
@#
@d del_code(#)==eqtb[del_code_base+#].int
@d count(#)==eqtb[count_base+#].int
@d int_par(#)==eqtb[int_base+#].int {an integer parameter}
@y
@d count_base=int_base+int_pars {|number_regs| user \.{\\count} registers}
@d del_code_base=count_base+number_regs {|number_chars| delimiter code mappings}
@d dimen_base=del_code_base+number_chars {beginning of region 6}
@#
@d del_code0(#)==new_equiv(del_code_base+#)
@d del_code1(#)==new_equiv1(del_code_base+#)
@d count(#)==new_eqtb_int(count_base+#)
@d int_par(#)==new_eqtb_int(int_base+#) {an integer parameter}
@z
%---------------------------------------
@x [17] m.240 l.5213 - Omega
for k:=int_base to del_code_base-1 do eqtb[k].int:=0;
mag:=1000; tolerance:=10000; hang_after:=1; max_dead_cycles:=25;
escape_char:="\"; end_line_char:=carriage_return;
for k:=0 to 255 do del_code(k):=-1;
del_code("."):=0; {this null delimiter is used in error recovery}
@y
set_new_eqtb_int(int_base+mag_code,1000);
set_new_eqtb_int(int_base+tolerance_code,10000);
set_new_eqtb_int(int_base+hang_after_code,1);
set_new_eqtb_int(int_base+max_dead_cycles_code,25);
set_new_eqtb_int(int_base+escape_char_code,"\");
set_new_eqtb_int(int_base+end_line_char_code,carriage_return);
set_equiv(del_code_base+".",0);
set_equiv1(del_code_base+".",0);
    {this null delimiter is used in error recovery}

@ @<Set newline character to -1@>=
set_new_eqtb_int(int_base+new_line_char_code,-1)

@ @<Set newline character to nl@>=
set_new_eqtb_int(int_base+new_line_char_code,nl)

@z
%---------------------------------------
@x [17] m.242 l.5240 - Omega
print_char("="); print_int(eqtb[n].int);
@y
print_char("="); print_int(new_eqtb_int(n));
@z
%---------------------------------------
@x [17] m.247 l.5273 - Omega
here, and the 256 \.{\\dimen} registers.
@y
here, and the |number_regs| \.{\\dimen} registers.
@z
%---------------------------------------
@x [17] m.247 l.5298 - Omega
  {table of 256 user-defined \.{\\dimen} registers}
@d eqtb_size=scaled_base+255 {largest subscript of |eqtb|}
@#
@d dimen(#)==eqtb[scaled_base+#].sc
@d dimen_par(#)==eqtb[dimen_base+#].sc {a scaled quantity}
@y
  {table of |number_regs| user-defined \.{\\dimen} registers}
@d eqtb_size=scaled_base+biggest_reg {largest subscript of |eqtb|}
@#
@d dimen(#)==new_eqtb_sc(scaled_base+#)
@d dimen_par(#)==new_eqtb_sc(dimen_base+#) {a scaled quantity}
@z
%---------------------------------------
@x [17] m.250 l.5405 - Omega
for k:=dimen_base to eqtb_size do eqtb[k].sc:=0;
@y

@ @p function init_eqtb_entry (p:pointer):memory_word;
var mw:memory_word;
begin
   { Regions 1 and 2 }
   if (p>=active_base) and
      (p<=undefined_control_sequence) then begin
     equiv_field(mw):=null;
     eq_type_field(mw):=undefined_cs;
     eq_level_field(mw):=level_zero;
     end
   { Region 3 }
   else if (p>=glue_base) and
           (p<=local_base+1) then begin
     equiv_field(mw):=zero_glue;
     eq_type_field(mw):=glue_ref;
     eq_level_field(mw):=level_one;
     end
   { Region 4 }
   else if (p>=par_shape_loc) and
           (p<=toks_base+biggest_reg) then begin
     equiv_field(mw):=null;
     eq_type_field(mw):=undefined_cs;
     eq_level_field(mw):=level_zero;
     end
   else if (p>=box_base) and
           (p<=box_base+biggest_reg) then begin
     equiv_field(mw):=null;
     eq_type_field(mw):=box_ref;
     eq_level_field(mw):=level_one;
     end
   else if (p>=cur_font_loc) and
           (p<=math_font_base+math_font_biggest) then begin
     equiv_field(mw):=null_font;
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   else if (p>=cat_code_base) and
           (p<=cat_code_base+biggest_char) then begin
     equiv_field(mw):=other_char;
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   else if (p>=lc_code_base) and
           (p<=uc_code_base+biggest_char) then begin
     equiv_field(mw):=0;
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   else if (p>=sf_code_base) and
           (p<=sf_code_base+biggest_char) then begin
     equiv_field(mw):=1000;
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   else if (p>=math_code_base) and
           (p<=math_code_base+biggest_char) then begin
     equiv_field(mw):=hi(p-math_code_base);
     eq_type_field(mw):=data;
     eq_level_field(mw):=level_one;
     end
   { Region 5 }
   else if (p>=int_base) and (p<=del_code_base-1) then begin
     setintzero(mw,0);
     setintone(mw,0);
     end
   else if (p>=del_code_base) and (p<=del_code_base+biggest_char) then begin
     setintzero(mw,-1);
     setintone(mw,-1);
     end
   { Region 6 }
   else if (p>=dimen_base) and (p<=eqtb_size) then begin
     setintzero(mw,0);
     setintone(mw,0);
     end
   else begin
     equiv_field(mw):=null;
     eq_type_field(mw):=undefined_cs;
     eq_level_field(mw):=level_zero;
     end;
   init_eqtb_entry:=mw;
end;
 
@z
%---------------------------------------
@x [17] m.251 l.5411 - Omega
print_char("="); print_scaled(eqtb[n].sc); print("pt");
@y
print_char("="); print_scaled(new_eqtb_sc(n)); print("pt");
@z
%---------------------------------------
@x [17] m.254 l.5435 - Omega
@ The last two regions of |eqtb| have fullword values instead of the
three fields |eq_level|, |eq_type|, and |equiv|. An |eq_type| is unnecessary,
but \TeX\ needs to store the |eq_level| information in another array
called |xeq_level|.

@<Glob...@>=
@!eqtb:array[active_base..eqtb_size] of memory_word;
@!xeq_level:array[int_base..eqtb_size] of quarterword;
@y
@ The last two regions of |eqtb| have fullword values instead of the
three fields |eq_level|, |eq_type|, and |equiv|. An |eq_type| is unnecessary,
but \TeX\ needs to store the |eq_level| information in another array
called |xeq_level|.

@d xeq_level(#) == new_xeq_level(eqtb_size+eqtb_size+#)
@d set_xeq_level(#) == set_new_eqtb_int(eqtb_size+eqtb_size+#)
@z
%---------------------------------------
@x [17] m.255 l.5439 - Omega
for k:=int_base to eqtb_size do xeq_level[k]:=level_one;
@y
@z
%---------------------------------------
@x [17] m.255 l.5446 - Omega
for q:=active_base to box_base+255 do
@y
for q:=active_base to box_base+biggest_reg do
@z
%---------------------------------------
@x [18] m.256 l.5477 - Omega
@d next(#) == hash[#].lh {link for coalesced lists}
@d text(#) == hash[#].rh {string number for control sequence name}
@d hash_is_full == (hash_used=hash_base) {test if all positions are occupied}
@d font_id_text(#) == text(font_id_base+#) {a frozen font identifier's name}

@<Glob...@>=
@!hash: array[hash_base..undefined_control_sequence-1] of two_halves;
  {the hash table}
@y
@d hash_is_full == (hash_used=hash_base) {test if all positions are occupied}
@d font_id_text(#) == newtext(font_id_base+#) {a frozen font identifier's name}
@d newtext(#) == new_hash_text(eqtb_size+#)
@d newnext(#) == new_hash_next(eqtb_size+#)
@d settext(#) == set_hash_text(eqtb_size+#)
@d setnext(#) == set_hash_next(eqtb_size+#)

@<Glob...@>=
@z
%---------------------------------------
@x [18] m.257 l.5491 - Omega
next(hash_base):=0; text(hash_base):=0;
for k:=hash_base+1 to undefined_control_sequence-1 do hash[k]:=hash[hash_base];
@y
@z
%---------------------------------------
@x [18] m.258 l.5497 - Omega
hash_used:=frozen_control_sequence; {nothing is used}
cs_count:=0;
eq_type(frozen_dont_expand):=dont_expand;
text(frozen_dont_expand):="notexpanded:";
@y
hash_used:=frozen_control_sequence; {nothing is used}
cs_count:=0;
set_eq_type(frozen_dont_expand,dont_expand);
settext(frozen_dont_expand,"notexpanded:");
@z
%---------------------------------------
@x [18] m.259 l.5514 - Omega
@!k:pointer; {index in |buffer| array}
begin @<Compute the hash code |h|@>;
p:=h+hash_base; {we start searching here; note that |0<=h<hash_prime|}
loop@+begin if text(p)>0 then if length(text(p))=l then
    if str_eq_buf(text(p),j) then goto found;
  if next(p)=0 then
    begin if no_new_control_sequence then
      p:=undefined_control_sequence
    else @<Insert a new control sequence after |p|, then make
      |p| point to it@>;
    goto found;
    end;
  p:=next(p);
  end;
found: id_lookup:=p;
@y
@!k:pointer; {index in |buffer| array}
@!newstring:integer;
begin @<Compute the hash code |h|@>;
p:=h+hash_base; {we start searching here; note that |0<=h<hash_prime|}
loop@+begin if newtext(p)>0 then if length(newtext(p))=l then
    if str_eq_buf(newtext(p),j) then goto found;
  if newnext(p)=0 then
    begin if no_new_control_sequence then
      p:=undefined_control_sequence
    else @<Insert a new control sequence after |p|, then make
      |p| point to it@>;
    goto found;
    end;
  p:=newnext(p);
  end;
found: id_lookup:=p;
@z
%---------------------------------------
@x [18] m.260 l.5532 - Omega
begin if text(p)>0 then
  begin repeat if hash_is_full then overflow("hash size",hash_size);
@:TeX capacity exceeded hash size}{\quad hash size@>
  decr(hash_used);
  until text(hash_used)=0; {search for an empty location in |hash|}
  next(p):=hash_used; p:=hash_used;
  end;
str_room(l); d:=cur_length;
while pool_ptr>str_start[str_ptr] do
  begin decr(pool_ptr); str_pool[pool_ptr+l]:=str_pool[pool_ptr];
  end; {move current string up to make room for another}
for k:=j to j+l-1 do append_char(buffer[k]);
text(p):=make_string; pool_ptr:=pool_ptr+d;
@y
begin if newtext(p)>0 then
  begin repeat if hash_is_full then overflow("hash size",hash_size);
@:TeX capacity exceeded hash size}{\quad hash size@>
  decr(hash_used);
  until newtext(hash_used)=0; {search for an empty location in |hash|}
  setnext(p,hash_used); p:=hash_used;
  end;
str_room(l); d:=cur_length;
while pool_ptr>str_start(str_ptr) do
  begin decr(pool_ptr); str_pool[pool_ptr+l]:=str_pool[pool_ptr];
  end; {move current string up to make room for another}
for k:=j to j+l-1 do append_char(buffer[k]);
newstring:=make_string;
settext(p,newstring); pool_ptr:=pool_ptr+d;
@z
%---------------------------------------
@x [18] m.262 l.5584 - Omega
else if (text(p)<0)or(text(p)>=str_ptr) then print_esc("NONEXISTENT.")
@.NONEXISTENT@>
else  begin print_esc(text(p));
@y
else if (newtext(p)<0)or(newtext(p)>=str_ptr) then print_esc("NONEXISTENT.")
@.NONEXISTENT@>
else  begin print_esc(newtext(p));
@z
%---------------------------------------
@x [18] m.263 l.5601 - Omega
else print_esc(text(p));
@y
else print_esc(newtext(p));
@z
%---------------------------------------
@x [18] m.264 l.5610 - Omega
@p @!init procedure primitive(@!s:str_number;@!c:quarterword;@!o:halfword);
var k:pool_pointer; {index into |str_pool|}
@!j:small_number; {index into |buffer|}
@!l:small_number; {length of the string}
begin if s<256 then cur_val:=s+single_base
else  begin k:=str_start[s]; l:=str_start[s+1]-k;
    {we will move |s| into the (empty) |buffer|}
  for j:=0 to l-1 do buffer[j]:=so(str_pool[k+j]);
  cur_val:=id_lookup(0,l); {|no_new_control_sequence| is |false|}
  flush_string; text(cur_val):=s; {we don't want to have the string twice}
  end;
eq_level(cur_val):=level_one; eq_type(cur_val):=c; equiv(cur_val):=o;
@y
@p @!init procedure primitive(@!s:str_number;@!c:quarterword;@!o:halfword);
var k:pool_pointer; {index into |str_pool|}
@!j:small_number; {index into |buffer|}
@!l:small_number; {length of the string}
begin if s<=biggest_char then cur_val:=s+single_base
else  begin k:=str_start(s); l:=str_start(s+1)-k;
    {we will move |s| into the (empty) |buffer|}
  for j:=0 to l-1 do buffer[j]:=so(str_pool[k+j]);
  cur_val:=id_lookup(0,l); {|no_new_control_sequence| is |false|}
  flush_string; settext(cur_val,s); {we don't want to have the string twice}
  end;
set_eq_level(cur_val,level_one); set_eq_type(cur_val,c);
set_equiv(cur_val,o);
@z
%---------------------------------------
@x [18] m.265 l.5648 - Omega
primitive("delimiter",delim_num,0);@/
@!@:delimiter_}{\.{\\delimiter} primitive@>
@y
primitive("delimiter",delim_num,0);@/
@!@:delimiter_}{\.{\\delimiter} primitive@>
primitive("odelimiter",delim_num,1);@/
@!@:delimiter_}{\.{\\odelimiter} primitive@>
primitive("leftghost",char_ghost,0);@/
primitive("rightghost",char_ghost,1);@/
@z
%---------------------------------------
@x [18] m.265 l.5656 - Omega
text(frozen_end_group):="endgroup"; eqtb[frozen_end_group]:=eqtb[cur_val];@/
@y
settext(frozen_end_group,"endgroup");
set_new_eqtb(frozen_end_group,new_eqtb(cur_val));@/
@z
%---------------------------------------
@x [18] m.265 l.5673 - Omega
primitive("mathaccent",math_accent,0);@/
@!@:math_accent_}{\.{\\mathaccent} primitive@>
primitive("mathchar",math_char_num,0);@/
@!@:math_char_}{\.{\\mathchar} primitive@>
@y
primitive("mathaccent",math_accent,0);@/
@!@:math_accent_}{\.{\\mathaccent} primitive@>
primitive("mathchar",math_char_num,0);@/
@!@:math_char_}{\.{\\mathchar} primitive@>
primitive("omathaccent",math_accent,1);@/
@!@:math_accent_}{\.{\\omathaccent} primitive@>
primitive("omathchar",math_char_num,1);@/
@!@:math_char_}{\.{\\omathchar} primitive@>
@z
%---------------------------------------
@x [18] m.265 l.5697 - Omega
primitive("radical",radical,0);@/
@!@:radical_}{\.{\\radical} primitive@>
@y
primitive("radical",radical,0);@/
@!@:radical_}{\.{\\radical} primitive@>
primitive("oradical",radical,1);@/
@!@:radical_}{\.{\\oradical} primitive@>
@z
%---------------------------------------
@x [18] m.265 l.5701 - Omega
primitive("relax",relax,256); {cf.\ |scan_file_name|}
@!@:relax_}{\.{\\relax} primitive@>
text(frozen_relax):="relax"; eqtb[frozen_relax]:=eqtb[cur_val];@/
@y
primitive("relax",relax,too_big_char); {cf.\ |scan_file_name|}
@!@:relax_}{\.{\\relax} primitive@>
settext(frozen_relax,"relax");
set_new_eqtb(frozen_relax,new_eqtb(cur_val));@/
@z
%---------------------------------------
@x [19] m.269 l.5859 - Omega
@d math_left_group=16 {code for `\.{\\left...\\right}'}
@d max_group_code=16
@y
@d math_left_group=16 {code for `\.{\\left...\\right}'}
@d math_mml_group=17 {code for automatic `\.{\\left...\\right}'}
@d text_mml_group=18 {code for `\.{\\text...}'}
@d text_sgml_group=19 {code for SGML tags}
@d font_entity_group=20 
@d empty_tag_group=21 
@d max_group_code=21
@z
%---------------------------------------
@x [19] m.276 l.5951 - Omega
else  begin save_stack[save_ptr]:=eqtb[p]; incr(save_ptr);
@y
else  begin save_stack[save_ptr]:=new_eqtb(p); incr(save_ptr);
@z
%---------------------------------------
@x [19] m.277 l.5967 - Omega
begin if eq_level(p)=cur_level then eq_destroy(eqtb[p])
else if cur_level>level_one then eq_save(p,eq_level(p));
eq_level(p):=cur_level; eq_type(p):=t; equiv(p):=e;
@y
begin if eq_level(p)=cur_level then eq_destroy(new_eqtb(p))
else if cur_level>level_one then eq_save(p,eq_level(p));
set_eq_level(p,cur_level); set_eq_type(p,t); set_equiv(p,e);
@z
%---------------------------------------
@x [19] m.278 l.5980 - Omega
@p procedure eq_word_define(@!p:pointer;@!w:integer);
begin if xeq_level[p]<>cur_level then
  begin eq_save(p,xeq_level[p]); xeq_level[p]:=cur_level;
  end;
eqtb[p].int:=w;
end;
@y
@p procedure eq_word_define(@!p:pointer;@!w:integer);
begin if xeq_level(p)<>cur_level then
  begin eq_save(p,xeq_level(p)); set_xeq_level(p,cur_level);
  end;
set_new_eqtb_int(p,w);
end;

procedure del_eq_word_define(@!p:pointer;@!w,wone:integer);
begin if xeq_level(p)<>cur_level then
  begin eq_save(p,xeq_level(p)); set_xeq_level(p,cur_level);
  end;
set_equiv(p,w); set_equiv1(p,wone);
end;

@z
%---------------------------------------
@x [19] m.279 l.5990 - Omega
begin eq_destroy(eqtb[p]);
eq_level(p):=level_one; eq_type(p):=t; equiv(p):=e;
end;
@#
procedure geq_word_define(@!p:pointer;@!w:integer); {global |eq_word_define|}
begin eqtb[p].int:=w; xeq_level[p]:=level_one;
end;
@y
begin eq_destroy(new_eqtb(p));
set_eq_level(p,level_one); set_eq_type(p,t); set_equiv(p,e);
end;
@#
procedure geq_word_define(@!p:pointer;@!w:integer); {global |eq_word_define|}
begin set_new_eqtb_int(p,w); set_xeq_level(p,level_one);
end;

procedure del_geq_word_define(@!p:pointer;@!w,wone:integer);
  {global |del_eq_word_define|}
begin set_equiv(p,w); set_equiv1(p,wone); set_xeq_level(p,level_one);
end;
@z
%---------------------------------------
@x [19] m.282 l.6036 - Omega
    else save_stack[save_ptr]:=eqtb[undefined_control_sequence];
@y
    else save_stack[save_ptr]:=new_eqtb(undefined_control_sequence);
@z
%---------------------------------------
@x [19] m.283 l.6056 - Omega
  else  begin eq_destroy(eqtb[p]); {destroy the current value}
    eqtb[p]:=save_stack[save_ptr]; {restore the saved value}
    @!stat if tracing_restores>0 then restore_trace(p,"restoring");@+tats@;@/
    end
else if xeq_level[p]<>level_one then
  begin eqtb[p]:=save_stack[save_ptr]; xeq_level[p]:=l;
@y
  else  begin eq_destroy(new_eqtb(p)); {destroy the current value}
    set_new_eqtb(p,save_stack[save_ptr]); {restore the saved value}
    @!stat if tracing_restores>0 then restore_trace(p,"restoring");@+tats@;@/
    end
else if xeq_level(p)<>level_one then
  begin set_new_eqtb(p,save_stack[save_ptr]); set_xeq_level(p,l);
@z
%---------------------------------------
@x [20] m.289 l.6129 - Omega
number $2^8m+c$; the command code is in the range |1<=m<=14|. (2)~A control
sequence whose |eqtb| address is |p| is represented as the number
|cs_token_flag+p|. Here |cs_token_flag=@t$2^{12}-1$@>| is larger than
@y
number $2^16m+c$; the command code is in the range |1<=m<=14|. (2)~A control
sequence whose |eqtb| address is |p| is represented as the number
|cs_token_flag+p|. Here |cs_token_flag=@t$2^{20}-1$@>| is larger than
@z
%---------------------------------------
@x [20] m.289 l.6142 - Omega
@d cs_token_flag==@'7777 {amount added to the |eqtb| location in a
  token that stands for a control sequence; is a multiple of~256, less~1}
@d left_brace_token=@'0400 {$2^8\cdot|left_brace|$}
@d left_brace_limit=@'1000 {$2^8\cdot(|left_brace|+1)$}
@d right_brace_token=@'1000 {$2^8\cdot|right_brace|$}
@d right_brace_limit=@'1400 {$2^8\cdot(|right_brace|+1)$}
@d math_shift_token=@'1400 {$2^8\cdot|math_shift|$}
@d tab_token=@'2000 {$2^8\cdot|tab_mark|$}
@d out_param_token=@'2400 {$2^8\cdot|out_param|$}
@d space_token=@'5040 {$2^8\cdot|spacer|+|" "|$}
@d letter_token=@'5400 {$2^8\cdot|letter|$}
@d other_token=@'6000 {$2^8\cdot|other_char|$}
@d match_token=@'6400 {$2^8\cdot|match|$}
@d end_match_token=@'7000 {$2^8\cdot|end_match|$}
@y
@d cs_token_flag=@"FFFFF {amount added to the |eqtb| location in a
  token that stands for a control sequence; is a multiple of~65536, less~1}
@d max_char_val=@"10000 {to separate char and command code}
@d left_brace_token=@"10000 {$2^16\cdot|left_brace|$}
@d left_brace_limit=@"20000 {$2^16\cdot(|left_brace|+1)$}
@d right_brace_token=@"20000 {$2^16\cdot|right_brace|$}
@d right_brace_limit=@"30000 {$2^16\cdot(|right_brace|+1)$}
@d math_shift_token=@"30000 {$2^16\cdot|math_shift|$}
@d tab_token=@"40000 {$2^16\cdot|tab_mark|$}
@d out_param_token=@"50000 {$2^16\cdot|out_param|$}
@d space_token=@"A0020 {$2^16\cdot|spacer|+|" "|$}
@d letter_token=@"B0000 {$2^16\cdot|letter|$}
@d other_token=@"C0000 {$2^16\cdot|other_char|$}
@d match_token=@"D0000 {$2^16\cdot|match|$}
@d end_match_token=@"E0000 {$2^16\cdot|end_match|$}
@z
%---------------------------------------
@x [20] m.293 l.6256 - Omega
else  begin m:=info(p) div @'400; c:=info(p) mod @'400;
@y
else  begin m:=info(p) div max_char_val; c:=info(p) mod max_char_val;
@z
%---------------------------------------
@x [21] m.298 l.6375 - Omega
procedure print_cmd_chr(@!cmd:quarterword;@!chr_code:halfword);
@y
procedure print_cmd_chr(@!cmd:quarterword;@!chr_code:halfword);
@z
%---------------------------------------
@x [24] m.334 l.7110 - Omega
primitive("par",par_end,256); {cf. |scan_file_name|}
@y
primitive("par",par_end,too_big_char); {cf. |scan_file_name|}
@z
%---------------------------------------
@x [24] m.341 l.7219 - Omega
@!c,@!cc:ASCII_code; {constituents of a possible expanded code}
@!d:2..3; {number of excess characters in an expanded code}
@y
@!c,@!cc,@!ccc,@!cccc:ASCII_code; {constituents of a possible expanded code}
@!d:2..7; {number of excess characters in an expanded code}
@z
%---------------------------------------
@x [24] m.352 l.7349 - Omega
  else cur_chr:=16*cur_chr+cc-"a"+10
@y
  else cur_chr:=16*cur_chr+cc-"a"+10
@d long_hex_to_cur_chr==
  if c<="9" then cur_chr:=c-"0" @+else cur_chr:=c-"a"+10;
  if cc<="9" then cur_chr:=16*cur_chr+cc-"0"
  else cur_chr:=16*cur_chr+cc-"a"+10;
  if ccc<="9" then cur_chr:=16*cur_chr+ccc-"0"
  else cur_chr:=16*cur_chr+ccc-"a"+10;
  if cccc<="9" then cur_chr:=16*cur_chr+cccc-"0"
  else cur_chr:=16*cur_chr+cccc-"a"+10
 
@z
%---------------------------------------
@x [24] m.352 l.7353 - Omega
  begin c:=buffer[loc+1]; @+if c<@'200 then {yes we have an expanded char}
@y
  begin if (cur_chr=buffer[loc+1]) and (cur_chr=buffer[loc+2]) and
           ((loc+6)<=limit) then 
     begin c:=buffer[loc+3]; cc:=buffer[loc+4]; 
       ccc:=buffer[loc+5]; cccc:=buffer[loc+6];
       if is_hex(c) and is_hex(cc) and is_hex(ccc) and is_hex(cccc) then 
       begin loc:=loc+7; long_hex_to_cur_chr; goto reswitch;
       end;
     end;
  c:=buffer[loc+1]; @+if c<@'200 then {yes we have an expanded char}
@z
%---------------------------------------
@x [24] m.355 l.7416 - Omega
begin if buffer[k]=cur_chr then @+if cat=sup_mark then @+if k<limit then
  begin c:=buffer[k+1]; @+if c<@'200 then {yes, one is indeed present}
    begin d:=2;
    if is_hex(c) then @+if k+2<=limit then
      begin cc:=buffer[k+2]; @+if is_hex(cc) then incr(d);
      end;
    if d>2 then
      begin hex_to_cur_chr; buffer[k-1]:=cur_chr;
      end
    else if c<@'100 then buffer[k-1]:=c+@'100
    else buffer[k-1]:=c-@'100;
    limit:=limit-d; first:=first-d;
    while k<=limit do
      begin buffer[k]:=buffer[k+d]; incr(k);
      end;
    goto start_cs;
    end;
  end;
end

@y
begin if buffer[k]=cur_chr then @+if cat=sup_mark then @+if k<limit then
  begin if (cur_chr=buffer[k+1]) and (cur_chr=buffer[k+2]) and 
           ((k+6)<=limit) then 
     begin c:=buffer[k+3]; cc:=buffer[k+4]; 
       ccc:=buffer[k+5]; cccc:=buffer[k+6];
       if is_hex(c) and is_hex(cc) and is_hex(ccc) and is_hex(cccc) then 
       begin d:=7; long_hex_to_cur_chr; buffer[k-1]:=cur_chr;
             while k<=limit do
                begin buffer[k]:=buffer[k+d]; incr(k);
                end;
             goto start_cs;
       end
     end
     else begin 
       c:=buffer[k+1]; @+if c<@'200 then {yes, one is indeed present}
       begin 
          d:=2;
          if is_hex(c) then @+if k+2<=limit then
            begin cc:=buffer[k+2]; @+if is_hex(cc) then incr(d);
            end;
          if d>2 then
            begin hex_to_cur_chr; buffer[k-1]:=cur_chr;
            end
          else if c<@'100 then buffer[k-1]:=c+@'100
          else buffer[k-1]:=c-@'100;
          limit:=limit-d; first:=first-d;
          while k<=limit do
            begin buffer[k]:=buffer[k+d]; incr(k);
            end;
          goto start_cs;
       end
     end
  end
end
@z
%---------------------------------------
@x [24] m.357 l.7462 - Omega
  else  begin cur_cmd:=t div @'400; cur_chr:=t mod @'400;
@y
  else  begin cur_cmd:=t div max_char_val; cur_chr:=t mod max_char_val;
@z
%---------------------------------------
@x [24] m.358 l.7479 - Omega
@d no_expand_flag=257 {this characterizes a special variant of |relax|}
@y
@d no_expand_flag=special_char {this characterizes a special variant of |relax|}
@z
%---------------------------------------
@x [24] m.365 l.7606 - Omega
if cur_cs=0 then cur_tok:=(cur_cmd*@'400)+cur_chr
@y
if cur_cs=0 then cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z
%---------------------------------------
@x [25] m.374 l.7728 - Omega
  begin eq_define(cur_cs,relax,256); {N.B.: The |save_stack| might change}
@y
  begin eq_define(cur_cs,relax,too_big_char); 
        {N.B.: The |save_stack| might change}
@z
%---------------------------------------
@x [25] m.374 l.7750 - Omega
  buffer[j]:=info(p) mod @'400; incr(j); p:=link(p);
@y
  buffer[j]:=info(p) mod max_char_val; incr(j); p:=link(p);
@z
%---------------------------------------
@x [25] m.380 l.7812 - Omega
done: if cur_cs=0 then cur_tok:=(cur_cmd*@'400)+cur_chr
@y
done: if cur_cs=0 then cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z
%---------------------------------------
@x [25] m.381 l.7824 - Omega
if cur_cs=0 then cur_tok:=(cur_cmd*@'400)+cur_chr
@y
if cur_cs=0 then cur_tok:=(cur_cmd*max_char_val)+cur_chr
@z
%---------------------------------------
@x [25] m.391 l.7985 - Omega
if (info(r)>match_token+255)or(info(r)<match_token) then s:=null
@y
if (info(r)>=end_match_token)or(info(r)<match_token) then s:=null
@z
%---------------------------------------
@x [26] m.407 l.8161 - Omega
@ The |scan_left_brace| routine is called when a left brace is supposed to be
the next non-blank token. (The term ``left brace'' means, more precisely,
a character whose catcode is |left_brace|.) \TeX\ allows \.{\\relax} to
appear before the |left_brace|.

@p procedure scan_left_brace; {reads a mandatory |left_brace|}
begin @<Get the next non-blank non-relax non-call token@>;
if cur_cmd<>left_brace then
  begin print_err("Missing { inserted");
@.Missing \{ inserted@>
  help4("A left brace was mandatory here, so I've put one in.")@/
    ("You might want to delete and/or insert some corrections")@/
    ("so that I will find a matching right brace soon.")@/
    ("(If you're confused by all this, try typing `I}' now.)");
  back_error; cur_tok:=left_brace_token+"{"; cur_cmd:=left_brace;
  cur_chr:="{"; incr(align_state);
  end;
end;
@y
@ The |scan_left_brace| routine is called when a left brace is supposed to be
the next non-blank token. (The term ``left brace'' means, more precisely,
a character whose catcode is |left_brace|.) \TeX\ allows \.{\\relax} to
appear before the |left_brace|.

@p procedure scan_left_brace; {reads a mandatory |left_brace|}
begin @<Get the next non-blank non-relax non-call token@>;
if cur_cmd<>left_brace then
  begin print_err("Missing { inserted");
@.Missing \{ inserted@>
  help4("A left brace was mandatory here, so I've put one in.")@/
    ("You might want to delete and/or insert some corrections")@/
    ("so that I will find a matching right brace soon.")@/
    ("(If you're confused by all this, try typing `I}' now.)");
  back_error; cur_tok:=left_brace_token+"{"; cur_cmd:=left_brace;
  cur_chr:="{"; incr(align_state);
  end;
end;

@ The |scan_right_brace| routine is called when a right brace is supposed to be
the next non-blank token. (The term ``right brace'' means, more precisely,
a character whose catcode is |right_brace|.) \TeX\ allows \.{\\relax} to
appear before the |right_brace|.

@p procedure scan_right_brace; {reads a mandatory |right_brace|}
begin @<Get the next non-blank non-relax non-call token@>;
if cur_cmd<>right_brace then
  begin print_err("Missing { inserted");
@.Missing \{ inserted@>
  help4("A right brace was mandatory here, so I've put one in.")@/
    ("You might want to delete and/or insert some corrections")@/
    ("so that I will find a matching right brace soon.")@/
    ("(If you're confused by all this, try typing `I}' now.)");
  back_error; cur_tok:=right_brace_token+"}"; cur_cmd:=right_brace;
  cur_chr:="}"; incr(align_state);
  end;
end;
@z
%---------------------------------------
@x [26] m.407 l.8216 - Omega
begin p:=backup_head; link(p):=null; k:=str_start[s];
while k<str_start[s+1] do
  begin get_x_token; {recursion is possible here}
@^recursion@>
  if (cur_cs=0)and@|
   ((cur_chr=so(str_pool[k]))or(cur_chr=so(str_pool[k])-"a"+"A")) then
    begin store_new_token(cur_tok); incr(k);
    end
  else if (cur_cmd<>spacer)or(p<>backup_head) then
    begin back_input;
    if p<>backup_head then back_list(link(backup_head));
    scan_keyword:=false; return;
    end;
  end;
@y
begin p:=backup_head; link(p):=null;
if s<too_big_char then begin
  while true do
    begin get_x_token; {recursion is possible here}
@^recursion@>
    if (cur_cs=0)and@|
       ((cur_chr=s)or(cur_chr=s-"a"+"A")) then
      begin store_new_token(cur_tok);
      flush_list(link(backup_head)); scan_keyword:=true; return;
      end
    else if (cur_cmd<>spacer)or(p<>backup_head) then
      begin back_input;
      if p<>backup_head then back_list(link(backup_head));
      scan_keyword:=false; return;
      end;
    end;
  end;
k:=str_start(s);
while k<str_start(s+1) do
  begin get_x_token; {recursion is possible here}
@^recursion@>
  if (cur_cs=0)and@|
   ((cur_chr=so(str_pool[k]))or(cur_chr=so(str_pool[k])-"a"+"A")) then
    begin store_new_token(cur_tok); incr(k);
    end
  else if (cur_cmd<>spacer)or(p<>backup_head) then
    begin back_input;
    if p<>backup_head then back_list(link(backup_head));
    scan_keyword:=false; return;
    end;
  end;
@z
%---------------------------------------
@x [26] m.410 l.8293 - Omega
@!cur_val:integer; {value returned by numeric scanners}
@y
@!cur_val:integer; {value returned by numeric scanners}
@!cur_val1:integer; {delcodes are now 51 digits}
@z
%---------------------------------------
@x [26] m.413 l.8335 - Omega
assign_int: scanned_result(eqtb[m].int)(int_val);
assign_dimen: scanned_result(eqtb[m].sc)(dimen_val);
@y
assign_int: scanned_result(new_eqtb_int(m))(int_val);
assign_dimen: scanned_result(new_eqtb_sc(m))(dimen_val);
@z
%---------------------------------------
@x [26] m.413 l.8345 - Omega
char_given,math_given: scanned_result(cur_chr)(int_val);
@y
char_given,math_given,omath_given: scanned_result(cur_chr)(int_val);
@z
%---------------------------------------
@x [26] m.414 l.8356 - Omega
@ @<Fetch a character code from some table@>=
begin scan_char_num;
if m=math_code_base then scanned_result(ho(math_code(cur_val)))(int_val)
else if m<math_code_base then scanned_result(equiv(m+cur_val))(int_val)
else scanned_result(eqtb[m+cur_val].int)(int_val);
@y
@ @<Fetch a character code from some table@>=
begin scan_char_num;
if m=math_code_base then begin
  cur_val1:=ho(math_code(cur_val));
  if ((cur_val1 div @"1000000)>8) or
     (((cur_val1 mod @"1000000) div @"10000)>15) or
     ((cur_val1 mod @"10000)>255) then
    begin print_err("Extended mathchar used as mathchar");
@.Bad mathchar@>
    help2("A mathchar number must be between 0 and ""7FFF.")@/
      ("I changed this one to zero."); int_error(cur_val1); cur_val1:=0;
    end;
  cur_val1:=((cur_val1 div @"1000000)*@"1000) +
            (((cur_val1 mod @"1000000) div @"10000)*@"100) +
            (cur_val1 mod @"10000);
  scanned_result(cur_val1)(int_val)
  end
else if m=(math_code_base+256) then
  scanned_result(ho(math_code(cur_val)))(int_val)
else if m<math_code_base then scanned_result(equiv(m+cur_val))(int_val)
else scanned_result(new_eqtb_int(m+cur_val))(int_val);
@z
%---------------------------------------
@x [26] m.425 l.8524 - Omega
begin find_font_dimen(false); font_info[fmem_ptr].sc:=0;
scanned_result(font_info[cur_val].sc)(dimen_val);
@y
begin find_font_dimen(false);
font_info(dimen_font)(font_file_size(dimen_font)).sc:=0;
scanned_result(font_info(dimen_font)(cur_val).sc)(dimen_val);
@z
%---------------------------------------
@x [26] m.426 l.8530 - Omega
if m=0 then scanned_result(hyphen_char[cur_val])(int_val)
else scanned_result(skew_char[cur_val])(int_val);
@y
if m=0 then scanned_result(hyphen_char(cur_val))(int_val)
else scanned_result(skew_char(cur_val))(int_val);
@z
%---------------------------------------
@x [26] m.433 l.8593 - Omega
procedure scan_eight_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad register code");
@.Bad register code@>
  help2("A register number must be between 0 and 255.")@/
@y
procedure scan_eight_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>biggest_reg) then
  begin print_err("Bad register code");
@.Bad register code@>
  help2("A register number must be between 0 and 65535.")@/
@z
%---------------------------------------
@x [26] m.434 l.8604 - Omega
procedure scan_char_num;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad character code");
@.Bad character code@>
  help2("A character number must be between 0 and 255.")@/
@y
procedure scan_char_num;
begin scan_int;
if (cur_val<0)or(cur_val>biggest_char) then
  begin print_err("Bad character code");
@.Bad character code@>
  help2("A character number must be between 0 and 65535.")@/
@z
%---------------------------------------
@x [26] m.435 l.8618 - Omega
procedure scan_four_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>15) then
  begin print_err("Bad number");
@.Bad number@>
  help2("Since I expected to read a number between 0 and 15,")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@y
procedure scan_four_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>15) then
  begin print_err("Bad number");
@.Bad number@>
  help2("Since I expected to read a number between 0 and 15,")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;

procedure scan_big_four_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>255) then
  begin print_err("Bad number");
@.Bad number@>
  help2("Since I expected to read a number between 0 and 255,")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@z
%---------------------------------------
@x [26] m.436 l.8629 - Omega
procedure scan_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'77777) then
  begin print_err("Bad mathchar");
@.Bad mathchar@>
  help2("A mathchar number must be between 0 and 32767.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@y
procedure scan_real_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'77777) then
  begin print_err("Bad mathchar");
@.Bad mathchar@>
  help2("A mathchar number must be between 0 and 32767.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;

procedure scan_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'77777) then
  begin print_err("Bad mathchar");
@.Bad mathchar@>
  help2("A mathchar number must be between 0 and 32767.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
cur_val := ((cur_val div @"1000) * @"1000000) +
           (((cur_val mod @"1000) div @"100) * @"10000) +
           (cur_val mod @"100);
end;

procedure scan_big_fifteen_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@"7FFFFFF) then
  begin print_err("Bad extended mathchar");
@.Bad mathchar@>
  help2("An extended mathchar number must be between 0 and ""7FFFFFF.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@z
%---------------------------------------
@x [26] m.437 l.8640 - Omega
procedure scan_twenty_seven_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'777777777) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
  help2("A numeric delimiter code must be between 0 and 2^{27}-1.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
end;
@y
procedure scan_twenty_seven_bit_int;
begin scan_int;
if (cur_val<0)or(cur_val>@'777777777) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
  help2("A numeric delimiter code must be between 0 and 2^{27}-1.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
cur_val1 := (((cur_val mod @"1000) div @"100) * @"10000) +
            (cur_val mod @"100);
cur_val := cur_val div @"1000;
cur_val := ((cur_val div @"1000) * @"1000000) +
           (((cur_val mod @"1000) div @"100) * @"10000) +
           (cur_val mod @"100);
end;

procedure scan_fifty_one_bit_int;
var iiii:integer;
begin scan_int;
if (cur_val<0)or(cur_val>@'777777777) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
  help2("A numeric delimiter (first part) must be between 0 and 2^{27}-1.")
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
iiii:=cur_val;
scan_int;
if (cur_val<0)or(cur_val>@"FFFFFF) then
  begin print_err("Bad delimiter code");
@.Bad delimiter code@>
help2("A numeric delimiter (second part) must be between 0 and 2^{24}-1.")@/
    ("I changed this one to zero."); int_error(cur_val); cur_val:=0;
  end;
cur_val1:=cur_val;
cur_val:=iiii;
end;

procedure scan_string_argument;
var s:integer;
begin
scan_left_brace;
get_x_token;
while (cur_cmd<>right_brace) do
  begin
  if (cur_cmd=letter) or (cur_cmd=other_char) then begin
    str_room(1); append_char(cur_chr);
    end
  else if (cur_cmd=spacer) then begin
    str_room(1); append_char(" ");
    end
  else begin
    print("Bad token appearing in string argument");
    end;
  get_x_token;
  end;
s:=make_string;
if str_eq_str("mi",s) then s:="mi";
if str_eq_str("mo",s) then s:="mo";
if str_eq_str("mn",s) then s:="mn";
cur_val:=s;
end;
@z
%---------------------------------------
@x [26] m.442 l.8720 - Omega
if cur_val>255 then
  begin print_err("Improper alphabetic constant");
@y
if cur_val>biggest_char then
  begin print_err("Improper alphabetic constant");
@z
%---------------------------------------
@x [27] m.468 l.9200 - Omega
@d job_name_code=5 {command code for \.{\\jobname}}
@y
@d omega_code=5 {command code for \.{\\OmegaVersion}}
@d job_name_code=6 {command code for \.{\\jobname}}
@z
%---------------------------------------
@x [27] m.468 l.9200 - Omega
primitive("jobname",convert,job_name_code);@/
@!@:job_name_}{\.{\\jobname} primitive@>
@y
primitive("OmegaVersion",convert,omega_code);@/
@!@:omega_version_}{\.{\\OmegaVersion} primitive@>
primitive("jobname",convert,job_name_code);@/
@!@:job_name_}{\.{\\jobname} primitive@>
@z
%---------------------------------------
@x [27] m.469 l.9223 - Omega
  othercases print_esc("jobname")
@y
  omega_code: print_esc("OmegaVersion");
  othercases print_esc("jobname")
@z
%---------------------------------------
@x [27] m.471 l.9248 - Omega
job_name_code: if job_name=0 then open_log_file;
@y
omega_code:;
job_name_code: if job_name=0 then open_log_file;
@z
%---------------------------------------
@x [27] m.472 l.9258 - Omega
font_name_code: begin print(font_name[cur_val]);
  if font_size[cur_val]<>font_dsize[cur_val] then
    begin print(" at "); print_scaled(font_size[cur_val]);
    print("pt");
    end;
  end;
job_name_code: print(job_name);
@y
font_name_code: begin print(font_name(cur_val));
  if font_size(cur_val)<>font_dsize(cur_val) then
    begin print(" at "); print_scaled(font_size(cur_val));
    print("pt");
    end;
  end;
omega_code: print("1.8");
job_name_code: print(job_name);
@z
%---------------------------------------
@x [28] m.491 l.9610 - Omega
text(frozen_fi):="fi"; eqtb[frozen_fi]:=eqtb[cur_val];
@y
settext(frozen_fi,"fi"); set_new_eqtb(frozen_fi,new_eqtb(cur_val));
@z
%---------------------------------------
@x [28] m.506 l.9802 - Omega
if (cur_cmd>active_char)or(cur_chr>255) then {not a character}
  begin m:=relax; n:=256;
@y
if (cur_cmd>active_char)or(cur_chr>biggest_char) then {not a character}
  begin m:=relax; n:=too_big_char;
@z
%---------------------------------------
@x [28] m.506 l.9808 - Omega
if (cur_cmd>active_char)or(cur_chr>255) then
  begin cur_cmd:=relax; cur_chr:=256;
@y
if (cur_cmd>active_char)or(cur_chr>biggest_char) then
  begin cur_cmd:=relax; cur_chr:=too_big_char;
@z
%---------------------------------------
@x [29] m.517 l.10005 - Omega
@p procedure end_name;
begin if str_ptr+3>max_strings then
  overflow("number of strings",max_strings-init_str_ptr);
@:TeX capacity exceeded number of strings}{\quad number of strings@>
if area_delimiter=0 then cur_area:=""
else  begin cur_area:=str_ptr;
  str_start[str_ptr+1]:=str_start[str_ptr]+area_delimiter; incr(str_ptr);
  end;
if ext_delimiter=0 then
  begin cur_ext:=""; cur_name:=make_string;
  end
else  begin cur_name:=str_ptr;
  str_start[str_ptr+1]:=str_start[str_ptr]+ext_delimiter-area_delimiter-1;
@y
@p procedure end_name;
begin if str_ptr+3>max_strings then
  overflow("number of strings",max_strings-init_str_ptr);
@:TeX capacity exceeded number of strings}{\quad number of strings@>
if area_delimiter=0 then cur_area:=""
else  begin cur_area:=str_ptr;
  str_start(str_ptr+1):=str_start(str_ptr)+area_delimiter; incr(str_ptr);
  end;
if ext_delimiter=0 then
  begin cur_ext:=""; cur_name:=make_string;
  end
else  begin cur_name:=str_ptr;
  str_start(str_ptr+1):=str_start(str_ptr)+ext_delimiter-area_delimiter-1;
@z
%---------------------------------------
@x [29] m.519 l.10039 - Omega
  if k<=file_name_size then name_of_file[k]:=xchr[c];
  end

@p procedure pack_file_name(@!n,@!a,@!e:str_number);
var k:integer; {number of positions filled in |name_of_file|}
@!c: ASCII_code; {character being packed}
@!j:pool_pointer; {index into |str_pool|}
begin k:=0;
for j:=str_start[a] to str_start[a+1]-1 do append_to_name(so(str_pool[j]));
for j:=str_start[n] to str_start[n+1]-1 do append_to_name(so(str_pool[j]));
for j:=str_start[e] to str_start[e+1]-1 do append_to_name(so(str_pool[j]));
@y
  if k<=file_name_size then name_of_file[k]:=xchr[c];
  end

@p procedure pack_file_name(@!n,@!a,@!e:str_number);
var k:integer; {number of positions filled in |name_of_file|}
@!c: ASCII_code; {character being packed}
@!j:pool_pointer; {index into |str_pool|}
begin k:=0;
for j:=str_start(a) to str_start(a+1)-1 do append_to_name(so(str_pool[j]));
for j:=str_start(n) to str_start(n+1)-1 do append_to_name(so(str_pool[j]));
for j:=str_start(e) to str_start(e+1)-1 do append_to_name(so(str_pool[j]));
@z
%---------------------------------------
@x [29] m.523 l.10095 - Omega
for j:=1 to n do append_to_name(xord[TEX_format_default[j]]);
for j:=a to b do append_to_name(buffer[j]);
for j:=format_default_length-format_ext_length+1 to format_default_length do
  append_to_name(xord[TEX_format_default[j]]);
@y
for j:=1 to n do append_to_name(TEX_format_default[j]);
for j:=a to b do append_to_name(buffer[j]);
for j:=format_default_length-format_ext_length+1 to format_default_length do
  append_to_name(TEX_format_default[j]);
@z
%---------------------------------------
@x [29] m.524 l.10155 - Omega
else  begin for k:=1 to name_length do append_char(xord[name_of_file[k]]);
@y
else  begin for k:=1 to name_length do append_char(name_of_file[k]);
@z
%---------------------------------------
@x [29] m.526 l.10178 - Omega
loop@+begin if (cur_cmd>other_char)or(cur_chr>255) then {not a character}
@y
loop@+begin if (cur_cmd>other_char)or(cur_chr>biggest_char) then 
    {not a character}
@z
%---------------------------------------
@x [29] m.532 l.10260 - Omega
@ Here's an example of how these conventions are used. Whenever it is time to
ship out a box of stuff, we shall use the macro |ensure_dvi_open|.

@d ensure_dvi_open==if output_file_name=0 then
  begin if job_name=0 then open_log_file;
  pack_job_name(".dvi");
  while not b_open_out(dvi_file) do
    prompt_file_name("file name for output",".dvi");
  output_file_name:=b_make_name_string(dvi_file);
  end

@<Glob...@>=
@!dvi_file: byte_file; {the device-independent output goes here}
@!output_file_name: str_number; {full name of the output file}
@!log_name:str_number; {full name of the log file}
@y
@ Here's an example of how these conventions are used. Whenever it is time to
ship out a box of stuff, we shall use the macro |ensure_dvi_open|.

@d ensure_output_open_end(#)==while not b_open_out(#) do
  prompt_file_name("file name for output",output_m_suffix);
  output_m_name:=b_make_name_string(#);
  end end

@d ensure_output_open_middle(#)==output_m_name:=#; if #=0 then
  begin if job_name=0 then open_log_file;
  pack_job_name(output_m_suffix);
  ensure_output_open_end

@d ensure_output_open(#)==begin output_m_suffix:=#; ensure_output_open_middle

@d ensure_dvi_open==begin
  ensure_output_open(".dvi")(output_file_name)(dvi_file);
  output_file_name:=output_m_name end

@<Glob...@>=
@!dvi_file: byte_file; {the device-independent output goes here}
@!output_file_name: str_number; {full name of the dvi output file}
@!output_m_suffix: str_number; {suffix for the current output file}
@!output_m_name: str_number; {suffix for the current output file}
@!output_file_names:array[1..10] of str_number;
@!output_files:array[1..10] of byte_file;
@!output_file_no:integer; {number of open output files}
@!log_name:str_number; {full name of the log file}
@z
%---------------------------------------
@x [29] m.533 l.10260 - Omega
@ @<Initialize the output...@>=output_file_name:=0;
@y
@ @<Initialize the output...@>=output_file_name:=0;
for output_file_no:=1 to 10 do output_file_names[output_file_no]:=0;
output_file_no:=0;
@z
%---------------------------------------
@x [29] m.536 l.10324 - Omega
begin wlog(banner);
slow_print(format_ident); print("  ");
print_int(day); print_char(" ");
months:='JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC';
for k:=3*month-2 to 3*month do wlog(months[k]);
print_char(" "); print_int(year); print_char(" ");
print_two(time div 60); print_char(":"); print_two(time mod 60);
end
@y
begin wlog(banner);
slow_print(format_ident); print_nl("");
print_int(day); print_char(" ");
months:='JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC';
for k:=3*month-2 to 3*month do wlog(months[k]);
print_char(" "); print_int(year); print_char(" ");
print_two(time div 60); print_char(":"); print_two(time mod 60);
print_nl("Copyright (c) 1994--1999 John Plaice and Yannis Haralambous");
end
@z
%---------------------------------------
@x [30] m.539 l.10392 - Omega
@^Ramshaw, Lyle Harold@>
of information in a compact but useful form.

@y
@^Ramshaw, Lyle Harold@>
of information in a compact but useful form.

$\Omega$ is capable of reading not only \.{TFM} files, but also
\.{XFM} files, which can describe fonts with up to 65536 characters
and with huge lig/kern tables.  These fonts will often be virtual
fonts built up from real fonts with 256 characters, but $\Omega$
is not aware of this.  

The documentation below describes \.{TFM} files, with slight additions
to show where \.{XFM} files differ.

@z
%---------------------------------------
@x [30] m.540 l.10417 - Omega
Note that a font may contain as many as 256 characters (if |bc=0| and |ec=255|),
and as few as 0 characters (if |bc=ec+1|).

Incidentally, when two or more 8-bit bytes are combined to form an integer of
16 or more bits, the most significant bytes appear first in the file.
This is called BigEndian order.
@!@^BigEndian order@>

@y
Note that a \.{TFM} font may contain as many as 256 characters 
(if |bc=0| and |ec=255|), and as few as 0 characters (if |bc=ec+1|).

Incidentally, when two or more 8-bit bytes are combined to form an integer of
16 or more bits, the most significant bytes appear first in the file.
This is called BigEndian order.
@!@^BigEndian order@>

The first 52 bytes (13 words) of an \.{XFM} file contains thirteen
32-bit integers that give the lengths of the various subsequent
portions of the file.  The first word is 0 (future versions of
\.{XFM} files could have different values;  what is important is that
the first two bytes be 0 to differentiate \.{TFM} and \.{XFM} files).
The next twelve integers are as above, all nonegative and less
than~$2^{31}$.  We must have |bc-1<=ec<=65535|, and
$$\hbox{|lf=13+lh+2*(ec-bc+1)+nw+nh+nd+ni+nl+nk+ne+np|.}$$
Note that an \.{XFM} font may contain as many as 65536 characters 
(if |bc=0| and |ec=65535|), and as few as 0 characters (if |bc=ec+1|).

@z
%---------------------------------------
@x [30] m.543 l.10492 - Omega
per character. Each word in this part of the file contains six fields
@y
per character. Each word in this part of a \.{TFM} file contains six fields
@z
%---------------------------------------
@x [30] m.543 l.10506 - Omega
imposes a limit of 16 different heights, 16 different depths, and
64 different italic corrections.

@y
imposes a limit of 16 different heights, 16 different depths, and
64 different italic corrections.

For \.{XFM} files, two words (eight bytes) are used. 
The arrangement is as follows.

\yskip\hang first and second bytes: |@!width_index| (16 bits)\par
\hang third byte: |@!height_index| (8 bits)\par
\hang fourth byte: |@!depth_index| (8~bits)\par
\hang fifth and sixth bytes: 
|@!italic_index| (14 bits) times 4, plus |@!tag| (2~bits)\par
\hang seventh and eighth bytes: |@!remainder| (16 bits)\par
\yskip\noindent
Therefore the \.{XFM} format imposes a limit of 256 different heights,
256 different depths, and 16384 different italic corrections.

@z
%---------------------------------------
@x [30] m.545 l.10547 - Omega
that explains what to do for special letter pairs. Each word in this array is a
|@!lig_kern_command| of four bytes.
@y
that explains what to do for special letter pairs. Each word in this array,
in a \.{TFM} file, is a |@!lig_kern_command| of four bytes.
@z
%---------------------------------------
@x [30] m.545 l.10557 - Omega
\hang fourth byte: |remainder|.\par
\yskip\noindent
@y
\hang fourth byte: |remainder|.\par
\yskip\noindent
In an \.{XFM} file, eight bytes are used, two bytes for each field.

@z
%---------------------------------------
@x [30] m.545 l.10587 - Omega
appear in a location |<=255|.
@y
appear in a location |<=255| in a \.{TFM} file, |<=65535| in an \.{XFM} file.
@z
%---------------------------------------
@x [30] m.545 l.10599 - Omega
@d skip_byte(#)==#.b0
@d next_char(#)==#.b1
@d op_byte(#)==#.b2
@d rem_byte(#)==#.b3
@y
@d top_skip_byte(#)==(#.b0 div 256)
@d skip_byte(#)==(#.b0 mod 256)
@d next_char_end(#)==#.b1
@d next_char(#)==font_offset(#)+next_char_end
@d op_byte(#)==#.b2
@d rem_byte(#)==#.b3
@d rem_char_byte_end(#)==#.b3
@d rem_char_byte(#)==font_offset(#)+rem_char_byte_end
@d rem_top_byte(#)==(#.b3 div 256)
@d rem_bot_byte(#)==(#.b3 mod 256)
@z
%---------------------------------------
@x [30] m.546 l.10604 - Omega
consists of four bytes called |@!top|, |@!mid|, |@!bot|, and |@!rep| (in this
order). These bytes are the character codes of individual pieces used to
@y
consists of four bytes in a \.{TFM} file, 
called |@!top|, |@!mid|, |@!bot|, and |@!rep| (in this order). 
In an \.{XFM} file, each field takes two bytes, for eight in total.
These bytes are the character codes of individual pieces used to
@z
%---------------------------------------
@x [30] m.546 l.10618 - Omega
@d ext_top(#)==#.b0 {|top| piece in a recipe}
@d ext_mid(#)==#.b1 {|mid| piece in a recipe}
@d ext_bot(#)==#.b2 {|bot| piece in a recipe}
@d ext_rep(#)==#.b3 {|rep| piece in a recipe}
@y
@d ext_top_end(#)==#.b0 {|top| piece in a recipe}
@d ext_top(#)==font_offset(#)+ext_top_end
@d ext_mid_end(#)==#.b1 {|mid| piece in a recipe}
@d ext_mid(#)==font_offset(#)+ext_mid_end
@d ext_bot_end(#)==#.b2 {|bot| piece in a recipe}
@d ext_bot(#)==font_offset(#)+ext_bot_end
@d ext_rep_end(#)==#.b3 {|rep| piece in a recipe}
@d ext_rep(#)==font_offset(#)+ext_rep_end
@z
%---------------------------------------
@x [30] m.548 l.10674 - Omega
@!font_index=0..font_mem_size; {index into |font_info|}
@y
@!font_index=integer; {index into |font_info|}
@z
%---------------------------------------
@x [30] m.549 l.10678 - Omega
@d non_char==qi(256) {a |halfword| code that can't match a real character}
@y
@d non_char==qi(too_big_char) {a code that can't match a real character}
@z
%---------------------------------------
@x [30] m.549 l.10686 - Omega
@!font_info:array[font_index] of memory_word;
  {the big collection of font data}
@!fmem_ptr:font_index; {first unused word of |font_info|}
@!font_ptr:internal_font_number; {largest internal font number in use}
@!font_check:array[internal_font_number] of four_quarters; {check sum}
@!font_size:array[internal_font_number] of scaled; {``at'' size}
@!font_dsize:array[internal_font_number] of scaled; {``design'' size}
@!font_params:array[internal_font_number] of font_index; {how many font
  parameters are present}
@!font_name:array[internal_font_number] of str_number; {name of the font}
@!font_area:array[internal_font_number] of str_number; {area of the font}
@!font_bc:array[internal_font_number] of eight_bits;
  {beginning (smallest) character code}
@!font_ec:array[internal_font_number] of eight_bits;
  {ending (largest) character code}
@!font_glue:array[internal_font_number] of pointer;
  {glue specification for interword space, |null| if not allocated}
@!font_used:array[internal_font_number] of boolean;
  {has a character from this font actually appeared in the output?}
@!hyphen_char:array[internal_font_number] of integer;
  {current \.{\\hyphenchar} values}
@!skew_char:array[internal_font_number] of integer;
  {current \.{\\skewchar} values}
@!bchar_label:array[internal_font_number] of font_index;
  {start of |lig_kern| program for left boundary character,
  |non_address| if there is none}
@!font_bchar:array[internal_font_number] of min_quarterword..non_char;
  {right boundary character, |non_char| if there is none}
@!font_false_bchar:array[internal_font_number] of min_quarterword..non_char;
  {|font_bchar| if it doesn't exist in the font, otherwise |non_char|}
@y
@!font_ptr:internal_font_number; {largest internal font number in use}
@!font_sort_ptr:integer; {largest internal font sort number in use}
@z
%---------------------------------------
@x [30] m.550 l.10723 - Omega
|font_info[char_base[f]+c].qqqq|; and if |w| is the |width_index|
part of this word (the |b0| field), the width of the character is
|font_info[width_base[f]+w].sc|. (These formulas assume that
|min_quarterword| has already been added to |c| and to |w|, since \TeX\
stores its quarterwords that way.)

@<Glob...@>=
@!char_base:array[internal_font_number] of integer;
  {base addresses for |char_info|}
@!width_base:array[internal_font_number] of integer;
  {base addresses for widths}
@!height_base:array[internal_font_number] of integer;
  {base addresses for heights}
@!depth_base:array[internal_font_number] of integer;
  {base addresses for depths}
@!italic_base:array[internal_font_number] of integer;
  {base addresses for italic corrections}
@!lig_kern_base:array[internal_font_number] of integer;
  {base addresses for ligature/kerning programs}
@!kern_base:array[internal_font_number] of integer;
  {base addresses for kerns}
@!exten_base:array[internal_font_number] of integer;
  {base addresses for extensible recipes}
@!param_base:array[internal_font_number] of integer;
  {base addresses for font parameters}
@y
|font_info(f)(char_base(f)+c).qqqq|; and if |w| is the |width_index|
part of this word (the |b0| field), the width of the character is
|font_info(f)(width_base(f)+w).sc|. (These formulas assume that
|min_quarterword| has already been added to |c| and to |w|, since \TeX\
stores its quarterwords that way.)

@d font_info_end(#)==#]
@d font_info(#)==font_tables[#,font_info_end
@d offset_file_size=0
@d offset_check=1
@d offset_offset=offset_check+4
@d offset_size=offset_offset+1
@d offset_dsize=offset_size+1
@d offset_params=offset_dsize+1
@d offset_name_sort=offset_params+1
@d offset_name=offset_name_sort+1
@d offset_area=offset_name+1
@d offset_bc=offset_area+1
@d offset_ec=offset_bc+1
@d offset_glue=offset_ec+1
@d offset_used=offset_glue+1
@d offset_hyphen=offset_used+1
@d offset_skew=offset_hyphen+1
@d offset_bchar_label=offset_skew+1
@d offset_bchar=offset_bchar_label+1
@d offset_false_bchar=offset_bchar+1
@d offset_ivalues_start=offset_false_bchar+1
@d offset_fvalues_start=offset_ivalues_start+1
@d offset_mvalues_start=offset_fvalues_start+1
@d offset_rules_start=offset_mvalues_start+1
@d offset_glues_start=offset_rules_start+1
@d offset_penalties_start=offset_glues_start+1
@d offset_ivalues_base=offset_penalties_start+1
@d offset_fvalues_base=offset_ivalues_base+1
@d offset_mvalues_base=offset_fvalues_base+1
@d offset_rules_base=offset_mvalues_base+1
@d offset_glues_base=offset_rules_base+1
@d offset_penalties_base=offset_glues_base+1
@d offset_char_base=offset_penalties_base+1
@d offset_char_attr_base=offset_char_base+1
@d offset_width_base=offset_char_attr_base+1
@d offset_height_base=offset_width_base+1
@d offset_depth_base=offset_height_base+1
@d offset_italic_base=offset_depth_base+1
@d offset_lig_kern_base=offset_italic_base+1
@d offset_kern_base=offset_lig_kern_base+1
@d offset_exten_base=offset_kern_base+1
@d offset_param_base=offset_exten_base+1
@d offset_charinfo_base=offset_param_base+1
@d font_file_size(#)==font_info(#)(offset_file_size).int
@d font_check_0(#)==font_info(#)(offset_check).int
@d font_check_1(#)==font_info(#)(offset_check+1).int
@d font_check_2(#)==font_info(#)(offset_check+2).int
@d font_check_3(#)==font_info(#)(offset_check+3).int
@d font_offset(#)==font_info(#)(offset_offset).int
@d font_size(#)==font_info(#)(offset_size).int
@d font_dsize(#)==font_info(#)(offset_dsize).int
@d font_params(#)==font_info(#)(offset_params).int
@d font_name_sort(#)==font_info(#)(offset_name_sort).int
@d font_name(#)==font_info(#)(offset_name).int
@d font_area(#)==font_info(#)(offset_area).int
@d font_bc(#)==font_info(#)(offset_bc).int
@d font_ec(#)==font_info(#)(offset_ec).int
@d font_glue(#)==font_info(#)(offset_glue).int
@d font_used(#)==font_info(#)(offset_used).int
@d hyphen_char(#)==font_info(#)(offset_hyphen).int
@d skew_char(#)==font_info(#)(offset_skew).int
@d bchar_label(#)==font_info(#)(offset_bchar_label).int
@d font_bchar(#)==font_info(#)(offset_bchar).int
@d font_false_bchar(#)==font_info(#)(offset_false_bchar).int
@d ivalues_start(#)==font_info(#)(offset_ivalues_start).int
@d fvalues_start(#)==font_info(#)(offset_fvalues_start).int
@d mvalues_start(#)==font_info(#)(offset_mvalues_start).int
@d rules_start(#)==font_info(#)(offset_rules_start).int
@d glues_start(#)==font_info(#)(offset_glues_start).int
@d penalties_start(#)==font_info(#)(offset_penalties_start).int
@d ivalues_base(#)==font_info(#)(offset_ivalues_base).int
@d fvalues_base(#)==font_info(#)(offset_fvalues_base).int
@d mvalues_base(#)==font_info(#)(offset_mvalues_base).int
@d rules_base(#)==font_info(#)(offset_rules_base).int
@d glues_base(#)==font_info(#)(offset_glues_base).int
@d penalties_base(#)==font_info(#)(offset_penalties_base).int
@d char_attr_base(#)==font_info(#)(offset_char_attr_base).int
@d char_base(#)==font_info(#)(offset_char_base).int
@d width_base(#)==font_info(#)(offset_width_base).int
@d height_base(#)==font_info(#)(offset_height_base).int
@d depth_base(#)==font_info(#)(offset_depth_base).int
@d italic_base(#)==font_info(#)(offset_italic_base).int
@d lig_kern_base(#)==font_info(#)(offset_lig_kern_base).int
@d kern_base(#)==font_info(#)(offset_kern_base).int
@d exten_base(#)==font_info(#)(offset_exten_base).int
@d param_base(#)==font_info(#)(offset_param_base).int

@d font_sort_info_end(#)==#]
@d font_sort_info(#)==font_sort_tables[#,font_sort_info_end
@d font_sort_offset_file_size=0
@d font_sort_offset_name=font_sort_offset_file_size+1
@d font_sort_offset_bc=font_sort_offset_name+1
@d font_sort_offset_ec=font_sort_offset_bc+1
@d font_sort_offset_char_base=font_sort_offset_bc+1
@d font_sort_file_size(#)==font_sort_info(#)(font_sort_offset_file_size).int
@d font_sort_name(#)==font_sort_info(#)(font_sort_offset_name).int
@d font_sort_bc(#)==font_sort_info(#)(font_sort_offset_bc).int
@d font_sort_ec(#)==font_sort_info(#)(font_sort_offset_ec).int
@d font_sort_char_base(#)==font_sort_info(#)(font_sort_offset_char_base).int
@d font_sort_char_entity_end(#)==#*3].int
@d font_sort_char_entity(#)==
   font_sort_tables[#,font_sort_offset_char_base+font_sort_char_entity_end
@d font_sort_char_tag_end(#)==#*3+1].int
@d font_sort_char_tag(#)==
   font_sort_tables[#,font_sort_offset_char_base+font_sort_char_tag_end
@d font_sort_char_attr_end(#)==#*3+2].int
@d font_sort_char_attr(#)==
   font_sort_tables[#,font_sort_offset_char_base+font_sort_char_attr_end

@<Glob...@>=
@!dimen_font:integer;
@!font_sorts:integer; {to run through font sorts}
@!font_sort_c:integer; {character used to define font entities}
@!font_sort_number:integer; {this particular font sort number}
@z
%---------------------------------------
@x [30] m.551 l.10743 - Omega
for k:=font_base to font_max do font_used[k]:=false;
@y
@z
%---------------------------------------
@x [30] m.552 l.10749 - Omega
font_ptr:=null_font; fmem_ptr:=7;
font_name[null_font]:="nullfont"; font_area[null_font]:="";
hyphen_char[null_font]:="-"; skew_char[null_font]:=-1;
bchar_label[null_font]:=non_address;
font_bchar[null_font]:=non_char; font_false_bchar[null_font]:=non_char;
font_bc[null_font]:=1; font_ec[null_font]:=0;
font_size[null_font]:=0; font_dsize[null_font]:=0;
char_base[null_font]:=0; width_base[null_font]:=0;
height_base[null_font]:=0; depth_base[null_font]:=0;
italic_base[null_font]:=0; lig_kern_base[null_font]:=0;
kern_base[null_font]:=0; exten_base[null_font]:=0;
font_glue[null_font]:=null; font_params[null_font]:=7;
param_base[null_font]:=-1;
for k:=0 to 6 do font_info[k].sc:=0;
@y
font_ptr:=null_font;
allocate_font_table(null_font,offset_charinfo_base+108);
font_file_size(null_font):=offset_charinfo_base+108;
font_used(null_font):=false;
font_name(null_font):="nullfont"; font_area(null_font):="";
hyphen_char(null_font):="-"; skew_char(null_font):=-1;
bchar_label(null_font):=non_address;
font_bchar(null_font):=non_char; font_false_bchar(null_font):=non_char;
font_bc(null_font):=1; font_ec(null_font):=0;
font_size(null_font):=0; font_dsize(null_font):=0;
char_base(null_font):=offset_charinfo_base;
char_attr_base(null_font):=offset_charinfo_base;
ivalues_start(null_font):=offset_charinfo_base;
fvalues_start(null_font):=offset_charinfo_base;
mvalues_start(null_font):=offset_charinfo_base;
rules_start(null_font):=offset_charinfo_base;
glues_start(null_font):=offset_charinfo_base;
penalties_start(null_font):=offset_charinfo_base;
ivalues_base(null_font):=offset_charinfo_base;
fvalues_base(null_font):=offset_charinfo_base;
mvalues_base(null_font):=offset_charinfo_base;
rules_base(null_font):=offset_charinfo_base;
glues_base(null_font):=offset_charinfo_base;
penalties_base(null_font):=offset_charinfo_base;
width_base(null_font):=offset_charinfo_base;
height_base(null_font):=offset_charinfo_base;
depth_base(null_font):=offset_charinfo_base;
italic_base(null_font):=offset_charinfo_base;
lig_kern_base(null_font):=offset_charinfo_base;
kern_base(null_font):=offset_charinfo_base;
exten_base(null_font):=offset_charinfo_base;
font_glue(null_font):=null;
font_params(null_font):=7;
param_base(null_font):=offset_charinfo_base-1;
font_offset(null_font):=0;
for k:=1 to 7 do font_info(null_font)(param_base(null_font)+k).sc:=0;

font_sort_ptr:=null_font_sort;
allocate_font_sort_table(null_font_sort,font_sort_offset_char_base);
font_sort_file_size(null_font_sort):=font_sort_offset_char_base;
font_sort_name(null_font_sort):="nullfontsort";
@z
%---------------------------------------
@x [30] m.553 l.10767 - Omega
text(frozen_null_font):="nullfont"; eqtb[frozen_null_font]:=eqtb[cur_val];
@y
settext(frozen_null_font,"nullfont");
set_new_eqtb(frozen_null_font,new_eqtb(cur_val));
@z
%---------------------------------------
@x [30] m.554 l.10771 - Omega
$$\hbox{|font_info[width_base[f]+font_info[char_base[f]+c].qqqq.b0].sc|}$$
@y
$$\hbox{|font_info(f)(width_base(f)+font_info(f)(char_base(f)+c).qqqq.b0).sc|}$$
@z
%---------------------------------------
@x [30] m.554 l.10785 - Omega
|height_depth(q)| will be the 8-bit quantity
$$b=|height_index|\times16+|depth_index|,$$ and if |b| is such a byte we
@y
|height_depth(q)| will be the 16-bit quantity
$$b=|height_index|\times256+|depth_index|,$$ and if |b| is such a byte we
@z
%---------------------------------------
@x [30] m.554 l.10798 - Omega
@d char_info_end(#)==#].qqqq
@d char_info(#)==font_info[char_base[#]+char_info_end
@d char_width_end(#)==#.b0].sc
@d char_width(#)==font_info[width_base[#]+char_width_end
@d char_exists(#)==(#.b0>min_quarterword)
@d char_italic_end(#)==(qo(#.b2)) div 4].sc
@d char_italic(#)==font_info[italic_base[#]+char_italic_end
@d height_depth(#)==qo(#.b1)
@d char_height_end(#)==(#) div 16].sc
@d char_height(#)==font_info[height_base[#]+char_height_end
@d char_depth_end(#)==(#) mod 16].sc
@d char_depth(#)==font_info[depth_base[#]+char_depth_end
@d char_tag(#)==((qo(#.b2)) mod 4)
@y
@d char_info_end(#)==#].qqqq
@d char_info(#)==font_tables[#,char_base(#)-font_offset(#)+char_info_end
@d char_width_end(#)==#.b0].sc
@d char_width(#)==font_tables[#,width_base(#)+char_width_end
@d char_exists(#)==(#.b0>min_quarterword)
@d char_italic_end(#)==(qo(#.b2)) div 256].sc
@d char_italic(#)==font_tables[#,italic_base(#)+char_italic_end
@d height_depth(#)==qo(#.b1)
@d char_height_end(#)==(#) div 256].sc
@d char_height(#)==font_tables[#,height_base(#)+char_height_end
@d char_depth_end(#)==(#) mod 256].sc
@d char_depth(#)==font_tables[#,depth_base(#)+char_depth_end
@d char_tag(#)==((qo(#.b2)) mod 4)
@z
%---------------------------------------
@x [30] m.557 l.10834 - Omega
|i=font_info[lig_kern_start(f)(j)]| or |font_info[lig_kern_restart(f)(i)]|,
depending on whether or not |skip_byte(i)<=stop_flag|.

The constant |kern_base_offset| should be simplified, for \PASCAL\ compilers
that do not do local optimization.
@^system dependencies@>

@d char_kern_end(#)==256*op_byte(#)+rem_byte(#)].sc
@d char_kern(#)==font_info[kern_base[#]+char_kern_end
@d kern_base_offset==256*(128+min_quarterword)
@d lig_kern_start(#)==lig_kern_base[#]+rem_byte {beginning of lig/kern program}
@d lig_kern_restart_end(#)==256*op_byte(#)+rem_byte(#)+32768-kern_base_offset
@d lig_kern_restart(#)==lig_kern_base[#]+lig_kern_restart_end
@y
|i=font_info(f)(lig_kern_start(f)(j))| or
|font_info(f)(lig_kern_restart(f)(i))|,
depending on whether or not |skip_byte(i)<=stop_flag|.

The constant |kern_base_offset| should be simplified, for \PASCAL\ compilers
that do not do local optimization.
@^system dependencies@>

@d char_kern_end(#)==256*op_byte(#)+rem_byte(#)].sc
@d char_kern(#)==font_tables[#,kern_base(#)+char_kern_end
@d kern_base_offset==256*(128+min_quarterword)
@d lig_kern_start(#)==lig_kern_base(#)+rem_byte {beginning of lig/kern program}
@d lig_kern_restart_end(#)==256*op_byte(#)+rem_byte(#)+32768-kern_base_offset
@d lig_kern_restart(#)==lig_kern_base(#)+lig_kern_restart_end

@d attr_zero_char_ivalue_end(#)==#].int].int
@d attr_zero_char_ivalue(#)==
   font_tables[#,font_tables[#,char_attr_base(#)+attr_zero_char_ivalue_end
@d attr_zero_ivalue_end(#)==#].int
@d attr_zero_ivalue(#)==font_tables[#,ivalues_base(#)+attr_zero_ivalue_end
@d attr_zero_penalty_end(#)==#].int
@d attr_zero_penalty(#)==font_tables[#,penalties_base(#)+attr_zero_penalty_end
@d attr_zero_kern_end(#)==#].int
@d attr_zero_kern(#)==font_tables[#,mvalues_base(#)+attr_zero_kern_end

@z
%---------------------------------------
@x [30] m.558 l.10843 - Omega
@d param_end(#)==param_base[#]].sc
@d param(#)==font_info[#+param_end
@d slant==param(slant_code) {slant to the right, per unit distance upward}
@d space==param(space_code) {normal space between words}
@d space_stretch==param(space_stretch_code) {stretch between words}
@d space_shrink==param(space_shrink_code) {shrink between words}
@d x_height==param(x_height_code) {one ex}
@d quad==param(quad_code) {one em}
@d extra_space==param(extra_space_code) {additional space at end of sentence}
@y
@d slant(#)==font_info(#)(param_base(#)+slant_code).sc
   {slant to the right, per unit distance upward}
@d space(#)==font_info(#)(param_base(#)+space_code).sc
   {normal space between words}
@d space_stretch(#)==font_info(#)(param_base(#)+space_stretch_code).sc
   {stretch between words}
@d space_shrink(#)==font_info(#)(param_base(#)+space_shrink_code).sc
   {shrink between words}
@d x_height(#)==font_info(#)(param_base(#)+x_height_code).sc
   {one ex}
@d quad(#)==font_info(#)(param_base(#)+quad_code).sc
   {one em}
@d extra_space(#)==font_info(#)(param_base(#)+extra_space_code).sc
   {additional space at end of sentence}
@z
%---------------------------------------
@x [30] m.560 l.10876 - Omega
@p function read_font_info(@!u:pointer;@!nom,@!aire:str_number;
  @!s:scaled):internal_font_number; {input a \.{TFM} file}
@y
@p function read_font_info(@!u:pointer;@!nom,@!aire:str_number;
   @!s:scaled;offset:quarterword):internal_font_number; {input a \.{TFM} file}
@z
%---------------------------------------
@x [30] m.560 l.10882 - Omega
  {sizes of subfiles}
@!f:internal_font_number; {the new font's number}
@!g:internal_font_number; {the number to return}
@!a,@!b,@!c,@!d:eight_bits; {byte variables}
@y
@!font_dir:halfword;
  {sizes of subfiles}
@!f:internal_font_number; {the new font's number}
@!g:internal_font_number; {the number to return}
@!a,@!b,@!c,@!d:integer; {byte variables}
@!i,@!k_param,@!param,@!font_counter:integer; {counters}
@!font_level,@!header_length:integer;
@!nco,@!ncw,@!npc,@!nlw,@!neew:integer;
@!nki,@!nwi,@!nkf,@!nwf,@!nkm,@!nwm:integer;
@!nkr,@!nwr,@!nkg,@!nwg,@!nkp,@!nwp:integer;
@!table_size:array [0..31] of integer;
@!bytes_per_entry,@!extra_char_bytes:integer;
@!repeat_no,@!table_counter:integer;
@z
%---------------------------------------
@x [30] m.560 l.10888 - Omega
@!bchar:0..256; {right boundary character, or 256}
@y
@!bchar:0..too_big_char; {right boundary character, or |too_big_char|}
@!first_two:integer;
@z
%---------------------------------------
@x [30] m.563 l.10943 - Omega
if file_opened then print(" not loadable: Bad metric (TFM) file")
else print(" not loadable: Metric (TFM) file not found");
@y
if file_opened then print(" not loadable: Bad metric (TFM/OFM) file")
else print(" not loadable: Metric (TFM/OFM) file not found");
@z
%---------------------------------------
@x [30] m.563 l.10943 - Omega
if aire="" then pack_file_name(nom,TEX_font_area,".tfm")
else pack_file_name(nom,aire,".tfm");
if not b_open_in(tfm_file) then abort;
@y
if aire="" then pack_file_name(nom,TEX_font_area,".ofm")
else pack_file_name(nom,aire,".ofm");
if not b_open_in(tfm_file) then abort;
@z
%---------------------------------------
@x [30] m.564 l.10962 - Omega
@d store_four_quarters(#)==begin fget; a:=fbyte; qw.b0:=qi(a);
  fget; b:=fbyte; qw.b1:=qi(b);
  fget; c:=fbyte; qw.b2:=qi(c);
  fget; d:=fbyte; qw.b3:=qi(d);
  #:=qw;
  end
@y
@d read_sixteen_unsigned(#)==begin #:=fbyte;
  fget; #:=#*@'400+fbyte;
  end
@d read_thirtytwo(#)==begin #:=fbyte;
  if #>127 then abort;
  fget; #:=#*@'400+fbyte;
  fget; #:=#*@'400+fbyte;
  fget; #:=#*@'400+fbyte;
  end
@d store_four_bytes(#)==begin fget; a:=fbyte; qw.b0:=a;
  fget; b:=fbyte; qw.b1:=b;
  fget; c:=fbyte; qw.b2:=c;
  fget; d:=fbyte; qw.b3:=d;
  #:=qw;
  end
@d store_char_info(#)==begin if (font_level<>-1) then
  begin fget; read_sixteen_unsigned(a); qw.b0:=a;
        fget; read_sixteen_unsigned(b); qw.b1:=b;
        fget; read_sixteen_unsigned(c); qw.b2:=c;
        fget; read_sixteen_unsigned(d); qw.b3:=d;
        #:=qw
  end
else 
  begin fget; a:=fbyte; qw.b0:=a;
        fget; b:=fbyte; b:=((b div 16)*256)+(b mod 16); qw.b1:=b;
        fget; c:=fbyte; c:=((c div 4)*256)+(c mod 4); qw.b2:=c;
        fget; d:=fbyte; qw.b3:=d;
        #:=qw
  end
end
@d store_four_quarters(#)==begin if (font_level<>-1) then
  begin fget; read_sixteen_unsigned(a); qw.b0:=a;
        fget; read_sixteen_unsigned(b); qw.b1:=b;
        fget; read_sixteen_unsigned(c); qw.b2:=c;
        fget; read_sixteen_unsigned(d); qw.b3:=d;
        #:=qw
  end
else 
  begin fget; a:=fbyte; qw.b0:=a;
        fget; b:=fbyte; qw.b1:=b;
        fget; c:=fbyte; qw.b2:=c;
        fget; d:=fbyte; qw.b3:=d;
        #:=qw
  end
end
@z
%---------------------------------------
@x [30] m.565 l.10970 - Omega
begin read_sixteen(lf);
fget; read_sixteen(lh);
fget; read_sixteen(bc);
fget; read_sixteen(ec);
if (bc>ec+1)or(ec>255) then abort;
if bc>255 then {|bc=256| and |ec=255|}
  begin bc:=1; ec:=0;
  end;
fget; read_sixteen(nw);
fget; read_sixteen(nh);
fget; read_sixteen(nd);
fget; read_sixteen(ni);
fget; read_sixteen(nl);
fget; read_sixteen(nk);
fget; read_sixteen(ne);
fget; read_sixteen(np);
if lf<>6+lh+(ec-bc+1)+nw+nh+nd+ni+nl+nk+ne+np then abort;
end
@y
nco:=0; ncw:=0; npc:=0;
nki:=0; nwi:=0; nkf:=0; nwf:=0; nkm:=0; nwm:=0;
nkr:=0; nwr:=0; nkg:=0; nwg:=0; nkp:=0; nwp:=0;
read_sixteen(first_two);
if first_two<>0 then begin
  font_level:=-1;
  lf:=first_two;
  fget; read_sixteen(lh);
  fget; read_sixteen(bc);
  fget; read_sixteen(ec);
  if (bc>ec+1)or(ec>255) then abort;
  if bc>255 then {|bc=256| and |ec=255|}
    begin bc:=1; ec:=0;
    end;
  fget; read_sixteen(nw);
  fget; read_sixteen(nh);
  fget; read_sixteen(nd);
  fget; read_sixteen(ni);
  fget; read_sixteen(nl);
  fget; read_sixteen(nk);
  fget; read_sixteen(ne);
  fget; read_sixteen(np);
  header_length:=6;
  ncw:=(ec-bc+1);
  nlw:=nl;
  neew:=ne;
  end
else begin
  fget; read_sixteen(font_level);
  if (font_level<>0) and (font_level<>1) then abort;
  fget; read_thirtytwo(lf);
  fget; read_thirtytwo(lh);
  fget; read_thirtytwo(bc);
  fget; read_thirtytwo(ec);
  if (bc>ec+1)or(ec>biggest_char) then abort;
  if bc>65535 then {|bc=65536| and |ec=65535|}
    begin bc:=1; ec:=0;
    end;
  fget; read_thirtytwo(nw);
  fget; read_thirtytwo(nh);
  fget; read_thirtytwo(nd);
  fget; read_thirtytwo(ni);
  fget; read_thirtytwo(nl);
  fget; read_thirtytwo(nk);
  fget; read_thirtytwo(ne);
  fget; read_thirtytwo(np);
  fget; read_thirtytwo(font_dir);
  nlw:=2*nl;
  neew:=2*ne;
  if font_level=0 then begin
    header_length:=14;
    ncw:=2*(ec-bc+1);
    end
  else begin
    header_length:=29;
    fget; read_thirtytwo(nco);
    fget; read_thirtytwo(ncw);
    fget; read_thirtytwo(npc);
    fget; read_thirtytwo(nki); {Kinds of font ivalues}
    fget; read_thirtytwo(nwi); {Words of font ivalues}
    fget; read_thirtytwo(nkf); {Kinds of font fvalues}
    fget; read_thirtytwo(nwf); {Words of font fvalues}
    fget; read_thirtytwo(nkm); {Kinds of font mvalues}
    fget; read_thirtytwo(nwm); {Words of font mvalues}
    fget; read_thirtytwo(nkr); {Kinds of font rules}
    fget; read_thirtytwo(nwr); {Words of font rules}
    fget; read_thirtytwo(nkg); {Kinds of font glues}
    fget; read_thirtytwo(nwg); {Words of font glues}
    fget; read_thirtytwo(nkp); {Kinds of font penalties}
    fget; read_thirtytwo(nwp); {Words of font penalties}
   end
  end;
if lf<>(header_length+lh+ncw+nw+nh+nd+ni+nlw+nk+neew+np+
        nki+nwi+nkf+nwf+nkm+nwm+nkr+nwr+nkg+nwg+nkp+nwp) then abort;
@z
%---------------------------------------
@x [30] m.566 l.10996 - Omega
lf:=lf-6-lh; {|lf| words should be loaded into |font_info|}
if np<7 then lf:=lf+7-np; {at least seven parameters will appear}
if (font_ptr=font_max)or(fmem_ptr+lf>font_mem_size) then
  @<Apologize for not loading the font, |goto done|@>;
f:=font_ptr+1;
char_base[f]:=fmem_ptr-bc;
width_base[f]:=char_base[f]+ec+1;
height_base[f]:=width_base[f]+nw;
depth_base[f]:=height_base[f]+nh;
italic_base[f]:=depth_base[f]+nd;
lig_kern_base[f]:=italic_base[f]+ni;
kern_base[f]:=lig_kern_base[f]+nl-kern_base_offset;
exten_base[f]:=kern_base[f]+kern_base_offset+nk;
param_base[f]:=exten_base[f]+ne
@y
case font_level of
-1:  lf:=lf-6-lh; {|lf| words should be loaded into |font_info|}
 0:  lf:=lf-14-lh-(ec-bc+1)-nl-ne;
   {some of the sections contain pairs of
    words that become single |memory_word|s}
 1:  lf:=lf-29-lh-ncw+(1+npc)*(ec-bc+1)-nl-ne;
   {We explode the characters}
end;
if np<7 then lf:=lf+7-np; {at least seven parameters will appear}
if (font_ptr=font_max) then
  @<Apologize for not loading the font, |goto done|@>;
f:=font_ptr+1;
allocate_font_table(f,offset_charinfo_base+lf+100);
    {leave room for 100 extra parameters -- potential bug}
font_file_size(f):=offset_charinfo_base+lf+100;
font_used(f):=false;
font_offset(f):=offset;
char_base(f):=offset_charinfo_base-bc;
char_attr_base(f):=char_base(f)+ec+1;
ivalues_start(f):=char_attr_base(f)+npc*(ec-bc+1);
fvalues_start(f):=ivalues_start(f)+nki;
mvalues_start(f):=fvalues_start(f)+nkf;
rules_start(f):=mvalues_start(f)+nkm;
glues_start(f):=rules_start(f)+nkr;
penalties_start(f):=glues_start(f)+nkg;
ivalues_base(f):=penalties_start(f)+nkp;
fvalues_base(f):=ivalues_base(f)+nwi;
mvalues_base(f):=fvalues_base(f)+nwf;
rules_base(f):=mvalues_base(f)+nwm;
glues_base(f):=rules_base(f)+nwr;
penalties_base(f):=glues_base(f)+nwg;
width_base(f):=penalties_base(f)+nwp;
height_base(f):=width_base(f)+nw;
depth_base(f):=height_base(f)+nh;
italic_base(f):=depth_base(f)+nd;
lig_kern_base(f):=italic_base(f)+ni;
kern_base(f):=lig_kern_base(f)+nl-kern_base_offset;
exten_base(f):=kern_base(f)+kern_base_offset+nk;
param_base(f):=exten_base(f)+ne;
@z
%---------------------------------------
@x [30] m.568 l.11026 - Omega
store_four_quarters(font_check[f]);
@y
fget; font_check_0(f):=fbyte;
fget; font_check_1(f):=fbyte;
fget; font_check_2(f):=fbyte;
fget; font_check_3(f):=fbyte;
@z
%---------------------------------------
@x [30] m.568 l.11033 - Omega
font_dsize[f]:=z;
if s<>-1000 then
  if s>=0 then z:=s
  else z:=xn_over_d(z,-s,1000);
font_size[f]:=z;
@y
font_dsize(f):=z;
if s<>-1000 then
  if s>=0 then z:=s
  else z:=xn_over_d(z,-s,1000);
font_size(f):=z;
@z
%---------------------------------------
@x [30] m.569 l.11041 - Omega
for k:=fmem_ptr to width_base[f]-1 do
  begin store_four_quarters(font_info[k].qqqq);
  if (a>=nw)or(b div @'20>=nh)or(b mod @'20>=nd)or
    (c div 4>=ni) then abort;
  case c mod 4 of
  lig_tag: if d>=nl then abort;
  ext_tag: if d>=ne then abort;
  list_tag: @<Check for charlist cycle@>;
  othercases do_nothing {|no_tag|}
  endcases;
  end
@y
if font_level=1 then begin
  i:=0;
  k:=ivalues_start(f);
  font_counter:=ivalues_base(f);
  while k<fvalues_start(f) do       {IVALUE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param;
    table_size[i]:=1;
    incr(i); incr(k);
    end;
  while k<mvalues_start(f) do       {FVALUE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param;
    table_size[i]:=1;
    incr(i); incr(k);
    end;
  while k<rules_start(f) do         {MVALUE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param;
    table_size[i]:=1;
    incr(i); incr(k);
    end;
  while k<glues_start(f) do         {RULE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param*3;
    table_size[i]:=3;
    incr(i); incr(k);
    end;
  while k<penalties_start(f) do     {GLUE starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param*4;
    table_size[i]:=4;
    incr(i); incr(k);
    end;
  while k<ivalues_base(f) do        {PENALTY starts}
    begin
    read_thirtytwo(param);
    font_info(f)(k).int := font_counter;
    font_counter:=font_counter+param*1;
    table_size[i]:=1;
    incr(i); incr(k);
    end;
  while k<fvalues_base(f) do        {IVALUE entries}
    begin
    read_thirtytwo(font_info(f)(k).int);
    incr(k);
    end;
  while k<mvalues_base(f) do        {FVALUE entries}
    begin
    read_thirtytwo(font_info(f)(k).sc);
    incr(k);
    end;
  while k<rules_base(f) do          {MVALUE entries}
    begin
    read_thirtytwo(font_info(f)(k).int);
    incr(k);
    end;
  while k<glues_base(f) do          {RULE entries}
    begin
    store_scaled(font_info(f)(k).sc);
    store_scaled(font_info(f)(k+1).sc);
    store_scaled(font_info(f)(k+2).sc);
    k:=k+3;
    end;
  while k<penalties_base(f) do      {GLUE entries}
    begin
    fget; read_sixteen(font_info(f)(k).hh.lhfield);
    fget; read_sixteen(font_info(f)(k).hh.rh);
    store_scaled(font_info(f)(k+1).sc);
    store_scaled(font_info(f)(k+2).sc);
    store_scaled(font_info(f)(k+3).sc);
    k:=k+4;
    end;
  while k<offset_charinfo_base do      {PENALTY entries}
    begin
    read_thirtytwo(font_info(f)(k).int);
    incr(k);
    end;
  end;
k:=char_base(f)+bc;
k_param:=char_attr_base(f);
bytes_per_entry:=(12 + 2*npc) div 4 * 4;
extra_char_bytes:=bytes_per_entry - (10 + 2*npc);
while k<char_attr_base(f) do begin
  store_char_info(font_info(f)(k).qqqq);
  if (a>=nw)or((b div @"100)>=nh)or((b mod @"100)>=nd)or
     ((c div @"100)>=ni) then abort;
  case c mod 4 of
    lig_tag: if d>=nl then abort;
    ext_tag: if d>=ne then abort;
    list_tag: @<Check for charlist cycle@>;
    othercases do_nothing {|no_tag|}
    endcases;
  incr(k);
  if font_level=1 then begin
    fget; read_sixteen_unsigned(repeat_no);
    for i:=0 to npc-1 do begin
      fget; read_sixteen(param);
      font_info(f)(k_param).int :=
         font_info(f)(ivalues_start(f)+i).int + param*table_size[i];
      incr(k_param);
      end;
    for i:=1 to extra_char_bytes do fget;
    for i:=1 to repeat_no do begin
      font_info(f)(k):=font_info(f)(k-1);
      incr(k);
      for table_counter:=0 to npc-1 do begin
        font_info(f)(k_param).int := font_info(f)(k_param-npc).int;
        incr(k_param);
        end;
      end;
    end;
  end
@z
%---------------------------------------
@x [30] m.570 l.11059 - Omega
@d current_character_being_worked_on==k+bc-fmem_ptr
@y
@d current_character_being_worked_on==k-char_base(f)
@z
%---------------------------------------
@x [30] m.570 l.11065 - Omega
  begin qw:=char_info(f)(d);
  {N.B.: not |qi(d)|, since |char_base[f]| hasn't been adjusted yet}
@y
  begin qw:=char_info(f)(d);
  {N.B.: not |qi(d)|, since |char_base(f)| hasn't been adjusted yet}
@z
%---------------------------------------
@x [30] m.571 l.11098 - Omega
for k:=width_base[f] to lig_kern_base[f]-1 do
  store_scaled(font_info[k].sc);
if font_info[width_base[f]].sc<>0 then abort; {\\{width}[0] must be zero}
if font_info[height_base[f]].sc<>0 then abort; {\\{height}[0] must be zero}
if font_info[depth_base[f]].sc<>0 then abort; {\\{depth}[0] must be zero}
if font_info[italic_base[f]].sc<>0 then abort; {\\{italic}[0] must be zero}
@y
for k:=width_base(f) to lig_kern_base(f)-1 do
  store_scaled(font_info(f)(k).sc);
if font_info(f)(width_base(f)).sc<>0 then abort; {\\{width}[0] must be zero}
if font_info(f)(height_base(f)).sc<>0 then abort; {\\{height}[0] must be zero}
if font_info(f)(depth_base(f)).sc<>0 then abort; {\\{depth}[0] must be zero}
if font_info(f)(italic_base(f)).sc<>0 then abort; {\\{italic}[0] must be zero}
@z
%---------------------------------------
@x [30] m.573 l.11114 - Omega
@ @d check_existence(#)==@t@>@;@/
  begin check_byte_range(#);
  qw:=char_info(f)(#); {N.B.: not |qi(#)|}
  if not char_exists(qw) then abort;
  end

@<Read ligature/kern program@>=
bch_label:=@'77777; bchar:=256;
if nl>0 then
  begin for k:=lig_kern_base[f] to kern_base[f]+kern_base_offset-1 do
    begin store_four_quarters(font_info[k].qqqq);
    if a>128 then
      begin if 256*c+d>=nl then abort;
      if a=255 then if k=lig_kern_base[f] then bchar:=b;
      end
    else begin if b<>bchar then check_existence(b);
      if c<128 then check_existence(d) {check ligature}
      else if 256*(c-128)+d>=nk then abort; {check kern}
      if a<128 then if k-lig_kern_base[f]+a+1>=nl then abort;
      end;
    end;
  if a=255 then bch_label:=256*c+d;
  end;
for k:=kern_base[f]+kern_base_offset to exten_base[f]-1 do
  store_scaled(font_info[k].sc);
@y
@ @d check_existence(#)==@t@>@;@/
  begin check_byte_range(#);
  qw:=char_info(f)(#+font_offset(f)); {N.B.: not |qi(#)|}
  if not char_exists(qw) then abort;
  end

@<Read ligature/kern program@>=
bch_label:=nl {infinity}; bchar:=too_big_char;
if nl>0 then
  begin for k:=lig_kern_base(f) to kern_base(f)+kern_base_offset-1 do
    begin store_four_quarters(font_info(f)(k).qqqq);
    if a>128 then
      begin if 256*c+d>=nl then abort;
      if a=255 then if k=lig_kern_base(f) then bchar:=b;
      end
    else begin if b<>bchar then check_existence(b);
      if c<128 then check_existence(d) {check ligature}
      else if 256*(c-128)+d>=nk then abort; {check kern}
      if a<128 then if k-lig_kern_base(f)+a+1>=nl then abort;
      end;
    end;
  if a=255 then bch_label:=256*c+d;
  end;
for k:=kern_base(f)+kern_base_offset to exten_base(f)-1 do
  store_scaled(font_info(f)(k).sc);
@z
%---------------------------------------
@x [30] m.574 l.11141 - Omega
for k:=exten_base[f] to param_base[f]-1 do
  begin store_four_quarters(font_info[k].qqqq);
@y
for k:=exten_base(f) to param_base(f)-1 do
  begin store_four_quarters(font_info(f)(k).qqqq);
@z
%---------------------------------------
@x [30] m.575 l.11157 - Omega
    fget; font_info[param_base[f]].sc:=
      (sw*@'20)+(fbyte div@'20);
    end
  else store_scaled(font_info[param_base[f]+k-1].sc);
if eof(tfm_file) then abort;
for k:=np+1 to 7 do font_info[param_base[f]+k-1].sc:=0;
@y
    fget; font_info(f)(param_base(f)).sc:=
      (sw*@'20)+(fbyte div@'20);
    end
  else store_scaled(font_info(f)(param_base(f)+k-1).sc);
if eof(tfm_file) then abort;
for k:=np+1 to 7 do font_info(f)(param_base(f)+k-1).sc:=0;
@z
%---------------------------------------
@x [30] m.576 l.11169 - Omega
@d adjust(#)==#[f]:=qo(#[f])
  {correct for the excess |min_quarterword| that was added}

@<Make final adjustments...@>=
if np>=7 then font_params[f]:=np@+else font_params[f]:=7;
hyphen_char[f]:=default_hyphen_char; skew_char[f]:=default_skew_char;
if bch_label<nl then bchar_label[f]:=bch_label+lig_kern_base[f]
else bchar_label[f]:=non_address;
font_bchar[f]:=qi(bchar);
font_false_bchar[f]:=qi(bchar);
if bchar<=ec then if bchar>=bc then
  begin qw:=char_info(f)(bchar); {N.B.: not |qi(bchar)|}
  if char_exists(qw) then font_false_bchar[f]:=non_char;
  end;
font_name[f]:=nom;
font_area[f]:=aire;
font_bc[f]:=bc; font_ec[f]:=ec; font_glue[f]:=null;
adjust(char_base); adjust(width_base); adjust(lig_kern_base);
adjust(kern_base); adjust(exten_base);
decr(param_base[f]);
fmem_ptr:=fmem_ptr+lf; font_ptr:=f; g:=f; goto done
@y
@d adjust(#)==#(f):=qo(#(f))
  {correct for the excess |min_quarterword| that was added}

@<Make final adjustments...@>=
if np>=7 then font_params(f):=np@+else font_params(f):=7;
font_offset(f):=offset;
hyphen_char(f):=default_hyphen_char+offset;
skew_char(f):=default_skew_char+offset;
bchar:=bchar+offset;
if bch_label<nl then bchar_label(f):=bch_label+lig_kern_base(f)
else bchar_label(f):=non_address;
font_bchar(f):=qi(bchar);
font_false_bchar(f):=qi(bchar);
if bchar<=ec then if bchar>=bc then
  begin qw:=char_info(f)(bchar); {N.B.: not |qi(bchar)|}
  if char_exists(qw) then font_false_bchar(f):=non_char;
  end;
font_name(f):=nom;
font_area(f):=aire;
font_bc(f):=bc; font_ec(f):=ec; font_glue(f):=null;
font_bc(f):=bc+offset; font_ec(f):=ec+offset; font_glue(f):=null;
adjust(char_base); adjust(width_base); adjust(lig_kern_base);
adjust(kern_base); adjust(exten_base);
decr(param_base(f));
font_ptr:=f; g:=f; goto done
@z
%---------------------------------------
@x [30] m.577 l.11202 - Omega
  begin m:=cur_chr; scan_four_bit_int; f:=equiv(m+cur_val);
@y
  begin m:=cur_chr; scan_big_four_bit_int; f:=equiv(m+cur_val);
@z
%---------------------------------------
@x [30] m.578 l.11223 - Omega
if n<=0 then cur_val:=fmem_ptr
else  begin if writing and(n<=space_shrink_code)and@|
    (n>=space_code)and(font_glue[f]<>null) then
    begin delete_glue_ref(font_glue[f]);
    font_glue[f]:=null;
    end;
  if n>font_params[f] then
    if f<font_ptr then cur_val:=fmem_ptr
    else @<Increase the number of parameters in the last font@>
  else cur_val:=n+param_base[f];
  end;
@<Issue an error message if |cur_val=fmem_ptr|@>;
end;
@y
dimen_font:=f;
if n<=0 then cur_val:=font_file_size(f)
else  begin if writing and(n<=space_shrink_code)and@|
    (n>=space_code)and(font_glue(f)<>null) then
    begin delete_glue_ref(font_glue(f));
    font_glue(f):=null;
    end;
  if n>font_params(f) then
    if f<font_ptr then cur_val:=font_file_size(f)
    else @<Increase the number of parameters in the last font@>
  else cur_val:=n+param_base(f);
  end;
@<Issue an error message if |cur_val=font_file_size(f)|@>;
end;
@z
%---------------------------------------
@x [30] m.579 l.11240 - Omega
@ @<Issue an error message if |cur_val=fmem_ptr|@>=
if cur_val=fmem_ptr then
  begin print_err("Font "); print_esc(font_id_text(f));
  print(" has only "); print_int(font_params[f]);
  print(" fontdimen parameters");
@.Font x has only...@>
  help2("To increase the number of font parameters, you must")@/
    ("use \fontdimen immediately after the \font is loaded.");
  error;
  end
@y
@ @<Issue an error message if |cur_val=font_file_size(f)|@>=
if cur_val=font_file_size(f) then
  begin print_err("Font "); print_esc(font_id_text(f));
  print(" has only "); print_int(font_params(f));
  print(" fontdimen parameters");
@.Font x has only...@>
  help2("To increase the number of font parameters, you must")@/
    ("use \fontdimen immediately after the \font is loaded.");
  error;
  end
@z
%---------------------------------------
@x [30] m.580 l.11252 - Omega
@ @<Increase the number of parameters...@>=
begin repeat if fmem_ptr=font_mem_size then
  overflow("font memory",font_mem_size);
@:TeX capacity exceeded font memory}{\quad font memory@>
font_info[fmem_ptr].sc:=0; incr(fmem_ptr); incr(font_params[f]);
until n=font_params[f];
cur_val:=fmem_ptr-1; {this equals |param_base[f]+font_params[f]|}
@y
@ @<Increase the number of parameters...@>=
begin if n+font_params(f)>font_file_size(f) then
  begin print_nl("Out of font parameter space"); succumb; end;
repeat
font_info(f)(param_base(f)+font_params(f)).sc:=0; incr(font_params(f));
until n=font_params(f);
cur_val:=param_base(f)+font_params(f);
@z
%---------------------------------------
@x [30] m.581 l.11268 - Omega
  slow_print(font_name[f]); print_char("!"); end_diagnostic(false);
@y
  slow_print(font_name(f)); print_char("!"); end_diagnostic(false);
@z
%---------------------------------------
@x [30] m.582 l.11276 - Omega
begin if font_bc[f]<=c then if font_ec[f]>=c then
@y
begin if font_bc(f)<=c then if font_ec(f)>=c then
@z
%---------------------------------------
@x [32] m.592 l.11820 - Omega
@!c,@!f:quarterword; {character and font in current |char_node|}
@y
@!c,@!f:quarterword; {character and font in current |char_node|}
@!oval,@!ocmd:integer; {used by |out_cmd| for generating
  |set|, |fnt| and |fnt_def| commands}
@z
%---------------------------------------
@x [32] m.595 l.11860 - Omega
@!dvi_buf:array[dvi_index] of eight_bits; {buffer for \.{DVI} output}
@y
@!dvi_buf:array[dvi_index] of real_eight_bits; {buffer for \.{DVI} output}
@z
%---------------------------------------
@x [32] m.602 l.11939 - Omega
@ Here's a procedure that outputs a font definition. Since \TeX82 uses at
most 256 different fonts per job, |fnt_def1| is always used as the command code.
@y
@ Here's a procedure that outputs a font definition. $\Omega$ allows
more than 256 different fonts per job, so the right font definition 
command must be selected.

@d Incr_Decr(#) == #
@d Incr(#) == #:=#+Incr_Decr {increase a variable}
@d Decr(#) == #:=#-Incr_Decr {decrease a variable}

@d dvi_debug_out(#)==begin
dvi_out(#);
end

@d out_cmd ==
begin
if (oval<@"100)and(oval>=0) then begin
  if (ocmd<>set1)or(oval>127) then
  if (ocmd=fnt1)and(oval<64) then Incr(oval)(fnt_num_0) @+ else dvi_debug_out(ocmd);
  end
else
  begin if (oval<@"10000)and(oval>=0) then dvi_debug_out(ocmd+1) @+ else @;
    begin if (oval<@"1000000)and(oval>=0) then dvi_debug_out(ocmd+2) @+ else @;
      begin dvi_debug_out(ocmd+3);
      if oval>=0 then dvi_debug_out(oval div @"1000000)
      else  begin Incr(oval)(@"40000000); Incr(oval)(@"40000000);
        dvi_debug_out((oval div @"1000000) + 128); oval:=oval mod @"1000000;
        end;
      dvi_debug_out(oval div @"10000); oval:=oval mod @"10000;
      end;
    dvi_debug_out(oval div @"10000); oval:=oval mod @"10000;
    end;
  dvi_debug_out(oval div @"100); oval:=oval mod @"100;
  end;
dvi_debug_out(oval)
end

@z
%---------------------------------------
@x [32] m.602 l.11944 - Omega
begin dvi_out(fnt_def1);
dvi_out(f-font_base-1);@/
dvi_out(qo(font_check[f].b0));
dvi_out(qo(font_check[f].b1));
dvi_out(qo(font_check[f].b2));
dvi_out(qo(font_check[f].b3));@/
dvi_four(font_size[f]);
dvi_four(font_dsize[f]);@/
dvi_out(length(font_area[f]));
dvi_out(length(font_name[f]));
@y
begin
oval:=f-font_base-1; ocmd:= fnt_def1; out_cmd;@/
dvi_out(qo(font_check_0(f)));
dvi_out(qo(font_check_1(f)));
dvi_out(qo(font_check_2(f)));
dvi_out(qo(font_check_3(f)));@/
dvi_four(font_size(f));
dvi_four(font_dsize(f));@/
dvi_out(length(font_area(f)));
dvi_out(length(font_name(f)));
@z
%---------------------------------------
@x [32] m.603 l.11958 - Omega
for k:=str_start[font_area[f]] to str_start[font_area[f]+1]-1 do
  dvi_out(so(str_pool[k]));
for k:=str_start[font_name[f]] to str_start[font_name[f]+1]-1 do
  dvi_out(so(str_pool[k]))
@y
for k:=str_start(font_area(f)) to str_start(font_area(f)+1)-1 do
  dvi_out(so(str_pool[k]));
for k:=str_start(font_name(f)) to str_start(font_name(f)+1)-1 do
  dvi_out(so(str_pool[k]))
@z
%---------------------------------------
@x [32] m.617 l.12262 - Omega
  print(" TeX output "); print_int(year); print_char(".");
@y
  print("Omega output, Version 3.14159--1.8, ");
  print_int(year); print_char(".");
@z
%---------------------------------------
@x [32] m.617 l.12267 - Omega
  for s:=str_start[str_ptr] to pool_ptr-1 do dvi_out(so(str_pool[s]));
  pool_ptr:=str_start[str_ptr]; {flush the current string}
@y
  for s:=str_start(str_ptr) to pool_ptr-1 do dvi_out(so(str_pool[s]));
  pool_ptr:=str_start(str_ptr); {flush the current string}
@z
%---------------------------------------
@x [32] m.620 l.12334 - Omega
  if c>=qi(128) then dvi_out(set1);
  dvi_out(qo(c));@/
@y
  oval:=c-font_offset(f); ocmd:=set1; out_cmd;@/
@z
%---------------------------------------
@x [32] m.621 l.12345 - Omega
begin if not font_used[f] then
  begin dvi_font_def(f); font_used[f]:=true;
  end;
if f<=64+font_base then dvi_out(f-font_base-1+fnt_num_0)
else  begin dvi_out(fnt1); dvi_out(f-font_base-1);
  end;
@y
begin if not font_used(f) then
  begin dvi_font_def(f); font_used(f):=true;
  end;
oval:=f-font_base-1; ocmd:=fnt1; out_cmd;@/
@z
%---------------------------------------
@x [32] m.638 l.12656 - Omega
@<Ship box |p| out@>;
@y
if not MML_mode then begin @<Ship box |p| out@>; end;
@z
%---------------------------------------
@x [32] m.643 l.12757 - Omega
  begin if font_used[font_ptr] then dvi_font_def(font_ptr);
@y
  begin if font_used(font_ptr) then dvi_font_def(font_ptr);
@z
%---------------------------------------
@x [34] m.681 l.13371 - Omega
@d math_char=1 {|math_type| when the attribute is simple}
@d sub_box=2 {|math_type| when the attribute is a box}
@d sub_mlist=3 {|math_type| when the attribute is a formula}
@d math_text_char=4 {|math_type| when italic correction is dubious}
@y
@d math_char=1 {|math_type| when the attribute is simple}
@d sub_box=2 {|math_type| when the attribute is a box}
@d sub_mlist=3 {|math_type| when the attribute is a formula}
@d math_text_char=4 {|math_type| when italic correction is dubious}
@z
%---------------------------------------
@x [34] m.682 l.13395 - Omega
@d ord_noad=unset_node+3 {|type| of a noad classified Ord}
@y
@d ord_noad=biggest_ordinary_node+3 {|type| of a noad classified Ord}
@z
%---------------------------------------
@x [34] m.688 l.13515 - Omega
@d style_node=unset_node+1 {|type| of a style node}
@y
@d style_node=biggest_ordinary_node+1 {|type| of a style node}
@z
%---------------------------------------
@x [34] m.688 l.13515 - Omega
@d choice_node=unset_node+2 {|type| of a choice node}
@y
@d choice_node=biggest_ordinary_node+2 {|type| of a choice node}
@z
%---------------------------------------
@x [35] m.699 l.13742 - Omega
@d text_size=0 {size code for the largest size in a family}
@d script_size=16 {size code for the medium size in a family}
@d script_script_size=32 {size code for the smallest size in a family}
@y
@z
%---------------------------------------
@x [35] m.700 l.13762 - Omega
@d mathsy_end(#)==fam_fnt(2+#)]].sc
@d mathsy(#)==font_info[#+param_base[mathsy_end
@y
@d mathsy_end(#)==sc
@d mathsy(#)==font_info(fam_fnt(2+cur_size))(#+param_base(fam_fnt(2+cur_size))).mathsy_end
@z
%---------------------------------------
@x [35] m.701 l.13789 - Omega
@d mathex(#)==font_info[#+param_base[fam_fnt(3+cur_size)]].sc
@y
@d mathex(#)==font_info(fam_fnt(3+cur_size))(#+param_base(fam_fnt(3+cur_size))).sc
@z
%---------------------------------------
@x [35] m.703 l.13813 - Omega
@<Set up the values of |cur_size| and |cur_mu|, based on |cur_style|@>=
begin if cur_style<script_style then cur_size:=text_size
else cur_size:=16*((cur_style-text_style) div 2);
cur_mu:=x_over_n(math_quad(cur_size),18);
end
@y
@<Set up the values of |cur_size| and |cur_mu|, based on |cur_style|@>=
begin if cur_style<script_style then cur_size:=text_size
else cur_size:=script_size*((cur_style-text_style) div 2);
cur_mu:=x_over_n(math_quad(cur_size),18);
end
@z
%---------------------------------------
@x [35] m.706 l.13855 - Omega
function var_delimiter(@!d:pointer;@!s:small_number;@!v:scaled):pointer;
label found,continue;
var b:pointer; {the box that will be constructed}
@!f,@!g: internal_font_number; {best-so-far and tentative font codes}
@!c,@!x,@!y: quarterword; {best-so-far and tentative character codes}
@!m,@!n: integer; {the number of extensible pieces}
@!u: scaled; {height-plus-depth of a tentative character}
@!w: scaled; {largest height-plus-depth so far}
@!q: four_quarters; {character info}
@!hd: eight_bits; {height-depth byte}
@!r: four_quarters; {extensible pieces}
@!z: small_number; {runs through font family members}
@y
function var_delimiter(@!d:pointer;@!s:integer;@!v:scaled):pointer;
label found,continue;
var b:pointer; {the box that will be constructed}
@!f,@!g: internal_font_number; {best-so-far and tentative font codes}
@!c,@!x,@!y: quarterword; {best-so-far and tentative character codes}
@!m,@!n: integer; {the number of extensible pieces}
@!u: scaled; {height-plus-depth of a tentative character}
@!w: scaled; {largest height-plus-depth so far}
@!q: four_quarters; {character info}
@!hd: eight_bits; {height-depth byte}
@!r: four_quarters; {extensible pieces}
@!z: integer; {runs through font family members}
@z
%---------------------------------------
@x [35] m.706 l.13881 - Omega
shift_amount(b):=half(height(b)-depth(b)) - axis_height(s);
@y
z:=cur_size; cur_size:=s;
shift_amount(b):=half(height(b)-depth(b)) - axis_height(cur_size);
cur_size:=z;
@z
%---------------------------------------
@x [35] m.707 l.13889 - Omega
@<Look at the variants of |(z,x)|; set |f| and |c|...@>=
if (z<>0)or(x<>min_quarterword) then
  begin z:=z+s+16;
  repeat z:=z-16; g:=fam_fnt(z);
  if g<>null_font then
    @<Look at the list of characters starting with |x| in
      font |g|; set |f| and |c| whenever
      a better character is found; |goto found| as soon as a
      large enough variant is encountered@>;
  until z<16;
  end
@y
@<Look at the variants of |(z,x)|; set |f| and |c|...@>=
if (z<>0)or(x<>min_quarterword) then
  begin z:=z+s+script_size;
  repeat z:=z-script_size; g:=fam_fnt(z);
  if g<>null_font then
    @<Look at the list of characters starting with |x| in
      font |g|; set |f| and |c| whenever
      a better character is found; |goto found| as soon as a
      large enough variant is encountered@>;
  until z<script_size;
  end
@z
%---------------------------------------
@x [35] m.708 l.13903 - Omega
if (qo(y)>=font_bc[g])and(qo(y)<=font_ec[g]) then
@y
if (qo(y)>=font_bc(g))and(qo(y)<=font_ec(g)) then
@z
%---------------------------------------
@x [35] m.713 l.13974 - Omega
r:=font_info[exten_base[f]+rem_byte(q)].qqqq;@/
@<Compute the minimum suitable height, |w|, and the corresponding
  number of extension steps, |n|; also set |width(b)|@>;
c:=ext_bot(r);
if c<>min_quarterword then stack_into_box(b,f,c);
c:=ext_rep(r);
for m:=1 to n do stack_into_box(b,f,c);
c:=ext_mid(r);
if c<>min_quarterword then
  begin stack_into_box(b,f,c); c:=ext_rep(r);
  for m:=1 to n do stack_into_box(b,f,c);
  end;
c:=ext_top(r);
@y
r:=font_info(f)(exten_base(f)+rem_byte(q)).qqqq;@/
@<Compute the minimum suitable height, |w|, and the corresponding
  number of extension steps, |n|; also set |width(b)|@>;
c:=ext_bot(f)(r);
if c<>min_quarterword then stack_into_box(b,f,c);
c:=ext_rep(f)(r);
for m:=1 to n do stack_into_box(b,f,c);
c:=ext_mid(f)(r);
if c<>min_quarterword then
  begin stack_into_box(b,f,c); c:=ext_rep(f)(r);
  for m:=1 to n do stack_into_box(b,f,c);
  end;
c:=ext_top(f)(r);
@z
%---------------------------------------
@x [35] m.714 l.13997 - Omega
c:=ext_rep(r); u:=height_plus_depth(f,c);
w:=0; q:=char_info(f)(c); width(b):=char_width(f)(q)+char_italic(f)(q);@/
c:=ext_bot(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
c:=ext_mid(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
c:=ext_top(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
n:=0;
if u>0 then while w<v do
  begin w:=w+u; incr(n);
  if ext_mid(r)<>min_quarterword then w:=w+u;
  end
@y
c:=ext_rep(f)(r); u:=height_plus_depth(f,c);
w:=0; q:=char_info(f)(c); width(b):=char_width(f)(q)+char_italic(f)(q);@/
c:=ext_bot(f)(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
c:=ext_mid(f)(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
c:=ext_top(f)(r);@+if c<>min_quarterword then w:=w+height_plus_depth(f,c);
n:=0;
if u>0 then while w<v do
  begin w:=w+u; incr(n);
  if (ext_mid(f)(r))<>min_quarterword then w:=w+u;
  end
@z
%---------------------------------------
@x [36] m.719 l.14107 - Omega
@!cur_style:small_number; {style code at current place in the list}
@!cur_size:small_number; {size code corresponding to |cur_style|}
@y
@!cur_style:integer; {style code at current place in the list}
@!cur_size:integer; {size code corresponding to |cur_style|}
@z
%---------------------------------------
@x [36] m.720 l.14122 - Omega
function clean_box(@!p:pointer;@!s:small_number):pointer;
label found;
var q:pointer; {beginning of a list to be boxed}
@!save_style:small_number; {|cur_style| to be restored}
@y
function clean_box(@!p:pointer;@!s:integer):pointer;
label found;
var q:pointer; {beginning of a list to be boxed}
@!save_style:integer; {|cur_style| to be restored}
@z
%---------------------------------------
@x [36] m.722 l.14172 - Omega
else  begin if (qo(cur_c)>=font_bc[cur_f])and(qo(cur_c)<=font_ec[cur_f]) then
@y
else  begin if (qo(cur_c)>=font_bc(cur_f))and(qo(cur_c)<=font_ec(cur_f)) then
@z
%---------------------------------------
@x [36] m.726 l.14228 - Omega
procedure mlist_to_hlist;
label reswitch, check_dimensions, done_with_noad, done_with_node, delete_q,
  done;
var mlist:pointer; {beginning of the given list}
@!penalties:boolean; {should penalty nodes be inserted?}
@!style:small_number; {the given style}
@!save_style:small_number; {holds |cur_style| during recursion}
@!q:pointer; {runs through the mlist}
@!r:pointer; {the most recent noad preceding |q|}
@!r_type:small_number; {the |type| of noad |r|, or |op_noad| if |r=null|}
@!t:small_number; {the effective |type| of noad |q| during the second pass}
@!p,@!x,@!y,@!z: pointer; {temporary registers for list construction}
@!pen:integer; {a penalty to be inserted}
@!s:small_number; {the size of a noad to be deleted}
@y
procedure mlist_to_hlist;
label reswitch, check_dimensions, done_with_noad, done_with_node, delete_q,
  done;
var mlist:pointer; {beginning of the given list}
@!penalties:boolean; {should penalty nodes be inserted?}
@!style:integer; {the given style}
@!save_style:integer; {holds |cur_style| during recursion}
@!q:pointer; {runs through the mlist}
@!r:pointer; {the most recent noad preceding |q|}
@!r_type:integer; {the |type| of noad |r|, or |op_noad| if |r=null|}
@!t:integer; {the effective |type| of noad |q| during the second pass}
@!p,@!x,@!y,@!z: pointer; {temporary registers for list construction}
@!pen:integer; {a penalty to be inserted}
@!s:integer; {the size of a noad to be deleted}
@z
%---------------------------------------
@x [36] m.741 l.14495 - Omega
if math_type(nucleus(q))=math_char then
  begin fetch(nucleus(q));
  if char_tag(cur_i)=lig_tag then
    begin a:=lig_kern_start(cur_f)(cur_i);
    cur_i:=font_info[a].qqqq;
    if skip_byte(cur_i)>stop_flag then
      begin a:=lig_kern_restart(cur_f)(cur_i);
      cur_i:=font_info[a].qqqq;
      end;
    loop@+ begin if qo(next_char(cur_i))=skew_char[cur_f] then
        begin if op_byte(cur_i)>=kern_flag then
          if skip_byte(cur_i)<=stop_flag then s:=char_kern(cur_f)(cur_i);
        goto done1;
        end;
      if skip_byte(cur_i)>=stop_flag then goto done1;
      a:=a+qo(skip_byte(cur_i))+1;
      cur_i:=font_info[a].qqqq;
      end;
    end;
  end;
done1:
@y
if math_type(nucleus(q))=math_char then
  begin fetch(nucleus(q));
  if char_tag(cur_i)=lig_tag then
    begin a:=lig_kern_start(cur_f)(cur_i);
    cur_i:=font_info(cur_f)(a).qqqq;
    if skip_byte(cur_i)>stop_flag then
      begin a:=lig_kern_restart(cur_f)(cur_i);
      cur_i:=font_info(cur_f)(a).qqqq;
      end;
    loop@+ begin if qo((next_char(cur_f)(cur_i)))=skew_char(cur_f) then
        begin if op_byte(cur_i)>=kern_flag then
          if skip_byte(cur_i)<=stop_flag then s:=char_kern(cur_f)(cur_i);
        goto done1;
        end;
      if skip_byte(cur_i)>=stop_flag then goto done1;
      a:=a+qo(skip_byte(cur_i))+1;
      cur_i:=font_info(cur_f)(a).qqqq;
      end;
    end;
  end;
done1:
@z
%---------------------------------------
@x [36] m.752 l.14722 - Omega
        cur_i:=font_info[a].qqqq;
        if skip_byte(cur_i)>stop_flag then
          begin a:=lig_kern_restart(cur_f)(cur_i);
          cur_i:=font_info[a].qqqq;
          end;
        loop@+ begin @<If instruction |cur_i| is a kern with |cur_c|, attach
            the kern after~|q|; or if it is a ligature with |cur_c|, combine
            noads |q| and~|p| appropriately; then |return| if the cursor has
            moved past a noad, or |goto restart|@>;
          if skip_byte(cur_i)>=stop_flag then return;
          a:=a+qo(skip_byte(cur_i))+1;
          cur_i:=font_info[a].qqqq;
@y
        cur_i:=font_info(cur_f)(a).qqqq;
        if skip_byte(cur_i)>stop_flag then
          begin a:=lig_kern_restart(cur_f)(cur_i);
          cur_i:=font_info(cur_f)(a).qqqq;
          end;
        loop@+ begin @<If instruction |cur_i| is a kern with |cur_c|, attach
            the kern after~|q|; or if it is a ligature with |cur_c|, combine
            noads |q| and~|p| appropriately; then |return| if the cursor has
            moved past a noad, or |goto restart|@>;
          if skip_byte(cur_i)>=stop_flag then return;
          a:=a+qo(skip_byte(cur_i))+1;
          cur_i:=font_info(cur_f)(a).qqqq;
@z
%---------------------------------------
@x [36] m.753 l.14751 - Omega
if next_char(cur_i)=cur_c then if skip_byte(cur_i)<=stop_flag then
@y
if (next_char(cur_f)(cur_i))=cur_c then if skip_byte(cur_i)<=stop_flag then
@z
%---------------------------------------
@x [36] m.756 l.14833 - Omega
procedure make_scripts(@!q:pointer;@!delta:scaled);
var p,@!x,@!y,@!z:pointer; {temporary registers for box construction}
@!shift_up,@!shift_down,@!clr:scaled; {dimensions in the calculation}
@!t:small_number; {subsidiary size code}
begin p:=new_hlist(q);
if is_char_node(p) then
  begin shift_up:=0; shift_down:=0;
  end
else  begin z:=hpack(p,natural);
  if cur_style<script_style then t:=script_size@+else t:=script_script_size;
  shift_up:=height(z)-sup_drop(t);
  shift_down:=depth(z)+sub_drop(t);
@y
procedure make_scripts(@!q:pointer;@!delta:scaled);
var p,@!x,@!y,@!z:pointer; {temporary registers for box construction}
@!shift_up,@!shift_down,@!clr:scaled; {dimensions in the calculation}
@!t:integer; {subsidiary size code}
begin p:=new_hlist(q);
if is_char_node(p) then
  begin shift_up:=0; shift_down:=0;
  end
else  begin z:=hpack(p,natural);
  t:=cur_size;
  if cur_style<script_style
  then cur_size:=script_size@+else cur_size:=script_script_size;
  shift_up:=height(z)-sup_drop(cur_size);
  shift_down:=depth(z)+sub_drop(cur_size);
  cur_size:=t;
@z
%---------------------------------------
@x [36] m.762 l.14967 - Omega
function make_left_right(@!q:pointer;@!style:small_number;
  @!max_d,@!max_h:scaled):small_number;
var delta,@!delta1,@!delta2:scaled; {dimensions used in the calculation}
begin if style<script_style then cur_size:=text_size
else cur_size:=16*((style-text_style) div 2);
@y
function make_left_right(@!q:pointer;@!style:integer;
  @!max_d,@!max_h:scaled):integer;
var delta,@!delta1,@!delta2:scaled; {dimensions used in the calculation}
begin if style<script_style then cur_size:=text_size
else cur_size:=script_size*((style-text_style) div 2);
@z
%---------------------------------------
@x [36] m.765 l.15021 - Omega
magic_offset:=str_start[math_spacing]-9*ord_noad
@y
magic_offset:=str_start(math_spacing)-9*ord_noad
@z
%---------------------------------------
@x [37] m.780 l.15354 - Omega
@d span_code=256 {distinct from any character}
@d cr_code=257 {distinct from |span_code| and from any character}
@y
@d span_code=special_char {distinct from any character}
@d cr_code=span_code+1 {distinct from |span_code| and from any character}
@z
%---------------------------------------
@x [37] m.780 l.15364 - Omega
text(frozen_cr):="cr"; eqtb[frozen_cr]:=eqtb[cur_val];@/
primitive("crcr",car_ret,cr_cr_code);
@!@:cr_cr_}{\.{\\crcr} primitive@>
text(frozen_end_template):="endtemplate"; text(frozen_endv):="endtemplate";
eq_type(frozen_endv):=endv; equiv(frozen_endv):=null_list;
eq_level(frozen_endv):=level_one;@/
eqtb[frozen_end_template]:=eqtb[frozen_endv];
eq_type(frozen_end_template):=end_template;
@y
settext(frozen_cr,"cr");
set_new_eqtb(frozen_cr,new_eqtb(cur_val));@/
primitive("crcr",car_ret,cr_cr_code);
@!@:cr_cr_}{\.{\\crcr} primitive@>
settext(frozen_end_template,"endtemplate"); settext(frozen_endv,"endtemplate");
set_eq_type(frozen_endv,endv); set_equiv(frozen_endv,null_list);
set_eq_level(frozen_endv,level_one);@/
set_new_eqtb(frozen_end_template,new_eqtb(frozen_endv));
set_eq_type(frozen_end_template,end_template);
@z
%---------------------------------------
@x [37] m.798 l.15651 - Omega
if n>max_quarterword then confusion("256 spans"); {this can happen, but won't}
@^system dependencies@>
@:this can't happen 256 spans}{\quad 256 spans@>
@y
if n>max_quarterword then confusion("too many spans"); 
   {this can happen, but won't}
@^system dependencies@>
@:this can't happen too many spans}{\quad too many spans@>
@z
%---------------------------------------
@x [37] m.804 l.15794 - Omega
  overfull_rule:=0; {prevent rule from being packaged}
  p:=hpack(preamble,saved(1),saved(0)); overfull_rule:=rule_save;
@y
  set_new_eqtb_sc(dimen_base+overfull_rule_code,0);
      {prevent rule from being packaged}
  p:=hpack(preamble,saved(1),saved(0));
  set_new_eqtb_sc(dimen_base+overfull_rule_code,rule_save);
@z
%---------------------------------------
@x [37] m.827 l.16255 - Omega
check_shrinkage(left_skip); check_shrinkage(right_skip);@/
@y
if (shrink_order(left_skip)<>normal)and(shrink(left_skip)<>0) then
  begin set_equiv(glue_base+left_skip_code,finite_shrink(left_skip));
  end;
if (shrink_order(right_skip)<>normal)and(shrink(right_skip)<>0) then
  begin set_equiv(glue_base+right_skip_code,finite_shrink(right_skip));
  end;
@z
%---------------------------------------
@x [40] m.891 l.17434 - Omega
$c_1$ will be treated as nonletters. The |hyphen_char| for that font
must be between 0 and 255, otherwise hyphenation will not be attempted.
@y
$c_1$ will be treated as nonletters. The |hyphen_char| for that font must 
be between 0 and |biggest_char|, otherwise hyphenation will not be attempted.
@z
%---------------------------------------
@x [40] m.892 l.17469 - Omega
@!hc:array[0..65] of 0..256; {word to be hyphenated}
@!hn:small_number; {the number of positions occupied in |hc|}
@!ha,@!hb:pointer; {nodes |ha..hb| should be replaced by the hyphenated result}
@!hf:internal_font_number; {font number of the letters in |hc|}
@!hu:array[0..63] of 0..256; {like |hc|, before conversion to lowercase}
@!hyf_char:integer; {hyphen character of the relevant font}
@!cur_lang,@!init_cur_lang:ASCII_code; {current hyphenation table of interest}
@!l_hyf,@!r_hyf,@!init_l_hyf,@!init_r_hyf:integer; {limits on fragment sizes}
@!hyf_bchar:halfword; {boundary character after $c_n$}
@y
@!hc:array[0..65] of 0..too_big_char; {word to be hyphenated}
@!hn:small_number; {the number of positions occupied in |hc|}
@!ha,@!hb:pointer; {nodes |ha..hb| should be replaced by the hyphenated result}
@!hf:internal_font_number; {font number of the letters in |hc|}
@!hu:array[0..63] of 0..too_big_char; 
     {like |hc|, before conversion to lowercase}
@!hyf_char:integer; {hyphen character of the relevant font}
@!cur_lang,@!init_cur_lang:0..biggest_lang; 
     {current hyphenation table of interest}
@!l_hyf,@!r_hyf,@!init_l_hyf,@!init_r_hyf:integer; {limits on fragment sizes}
@!hyf_bchar:halfword; {boundary character after $c_n$}
@!max_hyph_char:integer;

@ @<Set initial values of key variables@>=
max_hyph_char:=too_big_lang;

@z
%---------------------------------------
@x [40] m.893 l.17483 - Omega
@!c:0..255; {character being considered for hyphenation}
@y
@!c:ASCII_code; {character being considered for hyphenation}
@z
%---------------------------------------
@x [40] m.896 l.17534 - Omega
done2: hyf_char:=hyphen_char[hf];
if hyf_char<0 then goto done1;
if hyf_char>255 then goto done1;
@y
done2: hyf_char:=hyphen_char(hf);
if hyf_char<0 then goto done1;
if hyf_char>biggest_char then goto done1;
@z
%---------------------------------------
@x [40] m.897 l.17546 - Omega
    if lc_code(c)=0 then goto done3;
@y
    if lc_code(c)=0 then goto done3;
    if lc_code(c)>max_hyph_char then goto done3;
@z
%---------------------------------------
@x [40] m.897 l.17555 - Omega
    hyf_bchar:=font_bchar[hf];
@y
    hyf_bchar:=font_bchar(hf);
@z
%---------------------------------------
@x [40] m.898 l.17572 - Omega
  if lc_code(c)=0 then goto done3;
@y
  if lc_code(c)=0 then goto done3;
  if lc_code(c)>max_hyph_char then goto done3;
@z
%---------------------------------------
@x [40] m.898 l.17578 - Omega
if odd(subtype(s)) then hyf_bchar:=font_bchar[hf]@+else hyf_bchar:=non_char;
@y
if odd(subtype(s)) then hyf_bchar:=font_bchar(hf)@+else hyf_bchar:=non_char;
@z
%---------------------------------------
@x [41] m.903 l.17648 - Omega
      begin hu[0]:=256; init_lig:=false;
@y
      begin hu[0]:=max_hyph_char; init_lig:=false;
@z
%---------------------------------------
@x [41] m.903 l.17660 - Omega
found2: s:=ha; j:=0; hu[0]:=256; init_lig:=false; init_list:=null;
@y
found2: s:=ha; j:=0; hu[0]:=max_hyph_char; init_lig:=false; init_list:=null;
@z
%---------------------------------------
@x [41] m.905 l.17707 - Omega
getting the input $x_j\ldots x_n$ from the |hu| array. If $x_j=256$,
we consider $x_j$ to be an implicit left boundary character; in this
case |j| must be strictly less than~|n|. There is a
parameter |bchar|, which is either 256 or an implicit right boundary character
@y
getting the input $x_j\ldots x_n$ from the |hu| array. If $x_j=|max_hyph_char|$,
we consider $x_j$ to be an implicit left boundary character; in this
case |j| must be strictly less than~|n|. There is a
parameter |bchar|, which is either |max_hyph_char| 
or an implicit right boundary character
@z
%---------------------------------------
@x [41] m.909 l.17800 - Omega
  begin k:=bchar_label[hf];
  if k=non_address then goto done@+else q:=font_info[k].qqqq;
  end
else begin q:=char_info(hf)(cur_l);
  if char_tag(q)<>lig_tag then goto done;
  k:=lig_kern_start(hf)(q); q:=font_info[k].qqqq;
  if skip_byte(q)>stop_flag then
    begin k:=lig_kern_restart(hf)(q); q:=font_info[k].qqqq;
@y
  begin k:=bchar_label(hf);
  if k=non_address then goto done@+else q:=font_info(hf)(k).qqqq;
  end
else begin q:=char_info(hf)(cur_l);
  if char_tag(q)<>lig_tag then goto done;
  k:=lig_kern_start(hf)(q); q:=font_info(hf)(k).qqqq;
  if skip_byte(q)>stop_flag then
    begin k:=lig_kern_restart(hf)(q); q:=font_info(hf)(k).qqqq;
@z
%---------------------------------------
@x [41] m.909 l.17811 - Omega
loop@+begin if next_char(q)=test_char then if skip_byte(q)<=stop_flag then
@y
loop@+begin if (next_char(hf)(q))=test_char then if skip_byte(q)<=stop_flag then
@z
%---------------------------------------
@x [41] m.909 l.17829 - Omega
  k:=k+qo(skip_byte(q))+1; q:=font_info[k].qqqq;
@y
  k:=k+qo(skip_byte(q))+1; q:=font_info(hf)(k).qqqq;
@z
%---------------------------------------
@x [41] m.915 l.17963 - Omega
  begin l:=reconstitute(l,i,font_bchar[hf],non_char)+1;
@y
  begin l:=reconstitute(l,i,font_bchar(hf),non_char)+1;
@z
%---------------------------------------
@x [41] m.916 l.17980 - Omega
if bchar_label[hf]<>non_address then {put left boundary at beginning of new line}
  begin decr(l); c:=hu[l]; c_loc:=l; hu[l]:=256;
@y
if bchar_label(hf)<>non_address then
  {put left boundary at beginning of new line}
  begin decr(l); c:=hu[l]; c_loc:=l; hu[l]:=max_hyph_char;
@z
%---------------------------------------
@x [42] m.921 l.18079 - Omega
@!op_start:array[ASCII_code] of 0..trie_op_size; {offset for current language}
@y
@!op_start:array[0..biggest_lang] of 0..trie_op_size; 
           {offset for current language}
@z
%---------------------------------------
@x [42] m.923 l.18086 - Omega
hyphenation algorithm is quite short. In the following code we set |hc[hn+2]|
to the impossible value 256, in order to guarantee that |hc[hn+3]| will
@y
hyphenation algorithm is quite short. In the following code we set |hc[hn+2]| to
the impossible value |max_hyph_char|, in order to guarantee that |hc[hn+3]| will
@z
%---------------------------------------
@x [42] m.923 l.18095 - Omega
hc[0]:=0; hc[hn+1]:=0; hc[hn+2]:=256; {insert delimiters}
@y
hc[0]:=0; hc[hn+1]:=0; hc[hn+2]:=max_hyph_char; {insert delimiters}
@z
%---------------------------------------
@x [42] m.931 l.18172 - Omega
@ @<If the string |hyph_word[h]| is less than \(hc)...@>=
k:=hyph_word[h]; if k=0 then goto not_found;
if length(k)<hn then goto not_found;
if length(k)=hn then
  begin j:=1; u:=str_start[k];
@y
@ @<If the string |hyph_word[h]| is less than \(hc)...@>=
k:=hyph_word[h]; if k=0 then goto not_found;
if length(k)<hn then goto not_found;
if length(k)=hn then
  begin j:=1; u:=str_start(k);
@z
%---------------------------------------
@x [42] m.934 l.18206 - Omega
@d set_cur_lang==if language<=0 then cur_lang:=0
  else if language>255 then cur_lang:=0
@y
@d set_cur_lang==if language<=0 then cur_lang:=0
  else if language>biggest_lang then cur_lang:=0
@z
%---------------------------------------
@x [42] m.940 l.18296 - Omega
u:=str_start[k]; v:=str_start[s];
repeat if str_pool[u]<str_pool[v] then goto found;
if str_pool[u]>str_pool[v] then goto not_found;
incr(u); incr(v);
until u=str_start[k+1];
@y
u:=str_start(k); v:=str_start(s);
repeat if str_pool[u]<str_pool[v] then goto found;
if str_pool[u]>str_pool[v] then goto not_found;
incr(u); incr(v);
until u=str_start(k+1);
@z
%---------------------------------------
@x [43] m.943 l.18348 - Omega
@!trie_used:array[ASCII_code] of quarterword;
  {largest opcode used so far for this language}
@!trie_op_lang:array[1..trie_op_size] of ASCII_code;
@y
@!trie_used:array[0..biggest_lang] of quarterword;
  {largest opcode used so far for this language}
@!trie_op_lang:array[1..trie_op_size] of 0..biggest_lang;
@z
%---------------------------------------
@x [43] m.945 l.18400 - Omega
for j:=1 to 255 do op_start[j]:=op_start[j-1]+qo(trie_used[j-1]);
@y
for j:=1 to biggest_lang do op_start[j]:=op_start[j-1]+qo(trie_used[j-1]);
@z
%---------------------------------------
@x [43] m.946 l.18416 - Omega
for k:=0 to 255 do trie_used[k]:=min_quarterword;
@y
for k:=0 to biggest_lang do trie_used[k]:=min_quarterword;
@z
%---------------------------------------
@x [43] m.947 l.18438 - Omega
@!init @!trie_c:packed array[trie_pointer] of packed_ASCII_code;
@y
@!init @!trie_c:packed array[trie_pointer] of ASCII_code;
@z
%---------------------------------------
@x [43] m.952 l.18551 - Omega
for p:=0 to 255 do trie_min[p]:=p+1;
@y
for p:=0 to biggest_char do trie_min[p]:=p+1;
@z
%---------------------------------------
@x [43] m.953 l.18569 - Omega
@!ll:1..256; {upper limit of |trie_min| updating}
@y
@!ll:1..too_big_char; {upper limit of |trie_min| updating}
@z
%---------------------------------------
@x [43] m.953 l.18573 - Omega
  @<Ensure that |trie_max>=h+256|@>;
@y
  @<Ensure that |trie_max>=h+max_hyph_char|@>;
@z
%---------------------------------------
@x [43] m.954 l.18582 - Omega
@ By making sure that |trie_max| is at least |h+256|, we can be sure that
|trie_max>z|, since |h=z-c|. It follows that location |trie_max| will
never be occupied in |trie|, and we will have |trie_max>=trie_link(z)|.

@<Ensure that |trie_max>=h+256|@>=
if trie_max<h+256 then
  begin if trie_size<=h+256 then overflow("pattern memory",trie_size);
@y
@ By making sure that |trie_max| is at least |h+max_hyph_char|, 
we can be sure that
|trie_max>z|, since |h=z-c|. It follows that location |trie_max| will
never be occupied in |trie|, and we will have |trie_max>=trie_link(z)|.

@<Ensure that |trie_max>=h+max_hyph_char|@>=
if trie_max<h+max_hyph_char then
  begin if trie_size<=h+max_hyph_char then overflow("pattern memory",trie_size);
@z
%---------------------------------------
@x [43] m.954 l.18592 - Omega
  until trie_max=h+256;
@y
  until trie_max=h+max_hyph_char;
@z
%---------------------------------------
@x [43] m.956 l.18607 - Omega
if l<256 then
  begin if z<256 then ll:=z @+else ll:=256;
@y
if l<max_hyph_char then
  begin if z<max_hyph_char then ll:=z @+else ll:=max_hyph_char;
@z
%---------------------------------------
@x [43] m.958 l.18637 - Omega
if trie_root=0 then {no patterns were given}
  begin for r:=0 to 256 do trie[r]:=h;
  trie_max:=256;
@y
if trie_root=0 then {no patterns were given}
  begin for r:=0 to max_hyph_char do trie[r]:=h;
  trie_max:=max_hyph_char;
@z
%---------------------------------------
@x [43] m.962 l.18714 - Omega
@ @<Append a new letter or a hyphen level@>=
if digit_sensed or(cur_chr<"0")or(cur_chr>"9") then
  begin if cur_chr="." then cur_chr:=0 {edge-of-word delimiter}
  else  begin cur_chr:=lc_code(cur_chr);
    if cur_chr=0 then
      begin print_err("Nonletter");
@.Nonletter@>
      help1("(See Appendix H.)"); error;
      end;
    end;
@y
@ @<Append a new letter or a hyphen level@>=
if digit_sensed or(cur_chr<"0")or(cur_chr>"9") then
  begin if cur_chr="." then cur_chr:=0 {edge-of-word delimiter}
  else  begin cur_chr:=lc_code(cur_chr);
    if cur_chr=0 then
      begin print_err("Nonletter");
@.Nonletter@>
      help1("(See Appendix H.)"); error;
      end;
    end;
    if cur_chr>max_hyph_char then max_hyph_char:=cur_chr;
@z
%---------------------------------------
@x [43] m.966 l. - Omega
procedure init_trie;
var @!p:trie_pointer; {pointer for initialization}
@!j,@!k,@!t:integer; {all-purpose registers for initialization}
@!r,@!s:trie_pointer; {used to clean up the packed |trie|}
@!h:two_halves; {template used to zero out |trie|'s holes}
begin @<Get ready to compress the trie@>;
@y
procedure init_trie;
var @!p:trie_pointer; {pointer for initialization}
@!j,@!k,@!t:integer; {all-purpose registers for initialization}
@!r,@!s:trie_pointer; {used to clean up the packed |trie|}
@!h:two_halves; {template used to zero out |trie|'s holes}
begin
incr(max_hyph_char);
@<Get ready to compress the trie@>;
@z
%---------------------------------------
@x [44] m.977 l.19008 - Omega
if q=null then box(n):=null {the |eq_level| of the box stays the same}
else box(n):=vpack(q,natural);
@y
if q=null then set_equiv(box_base+n,null)
    {the |eq_level| of the box stays the same}
else set_equiv(box_base+n,vpack(q,natural));
@z
%---------------------------------------
@x [45] m.992 l.24928 - Omega
flush_node_list(box(n)); box(n):=null;
@y
flush_node_list(box(n)); set_equiv(box_base+n,null);
@z
%---------------------------------------
@x [45] m.994 l.19364 - Omega
@!n:min_quarterword..255; {insertion box number}
@y
@!n:min_quarterword..biggest_reg; {insertion box number}
@z
%---------------------------------------
@x [45] m.1012 l.19655 - Omega
@!n:min_quarterword..255; {insertion box number}
@y
@!n:min_quarterword..biggest_reg; {insertion box number}
@z
%---------------------------------------
@x [45] m.1014 l.19710 - Omega
split_top_skip:=save_split_top_skip;
@y
set_equiv(glue_base+split_top_skip_code,save_split_top_skip);
@z
%---------------------------------------
@x [45] m.1017 l.19753 - Omega
save_vbadness:=vbadness; vbadness:=inf_bad;
save_vfuzz:=vfuzz; vfuzz:=max_dimen; {inhibit error messages}
box(255):=vpackage(link(page_head),best_size,exactly,page_max_depth);
vbadness:=save_vbadness; vfuzz:=save_vfuzz;
@y
save_vbadness:=vbadness;
set_new_eqtb_int(int_base+vbadness_code,inf_bad);
save_vfuzz:=vfuzz;
set_new_eqtb_sc(dimen_base+vfuzz_code,max_dimen);
    {inhibit error messages}
set_equiv(box_base+255,
          vpackage(link(page_head),best_size,exactly,page_max_depth));
set_new_eqtb_int(int_base+vbadness_code,save_vbadness);
set_new_eqtb_sc(dimen_base+vfuzz_code,save_vfuzz);
@z
%---------------------------------------
@x [45] m.1018 l.19774 - Omega
    if box(n)=null then box(n):=new_null_box;
@y
    if box(n)=null then set_equiv(box_base+n,new_null_box);
@z
%---------------------------------------
@x [45] m.1021 l.19817 - Omega
    split_top_skip:=split_top_ptr(p);
    ins_ptr(p):=prune_page_top(broken_ptr(r));
    if ins_ptr(p)<>null then
      begin temp_ptr:=vpack(ins_ptr(p),natural);
      height(p):=height(temp_ptr)+depth(temp_ptr);
      free_node(temp_ptr,box_node_size); wait:=true;
      end;
    end; 
best_ins_ptr(r):=null;
n:=qo(subtype(r));
temp_ptr:=list_ptr(box(n));
free_node(box(n),box_node_size);
box(n):=vpack(temp_ptr,natural);
@y
    set_equiv(glue_base+split_top_skip_code,split_top_ptr(p));
    ins_ptr(p):=prune_page_top(broken_ptr(r));
    if ins_ptr(p)<>null then
      begin temp_ptr:=vpack(ins_ptr(p),natural);
      height(p):=height(temp_ptr)+depth(temp_ptr);
      free_node(temp_ptr,box_node_size); wait:=true;
      end;
    end; 
best_ins_ptr(r):=null;
n:=qo(subtype(r));
temp_ptr:=list_ptr(box(n));
free_node(box(n),box_node_size);
set_equiv(box_base+n,vpack(temp_ptr,natural));
@z
%---------------------------------------
@x [45] m.1023 l.19854 - Omega
ship_out(box(255)); box(255):=null;
@y
ship_out(box(255)); set_equiv(box_base+255,null);
@z
%---------------------------------------
@x [46] m. l.19987 - Omega
hmode+no_boundary: begin get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or(cur_cmd=char_given)or
   (cur_cmd=char_num) then cancel_boundary:=true;
  goto reswitch;
  end;
@y
hmode+no_boundary: begin get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or(cur_cmd=char_given)or
   (cur_cmd=char_num) then cancel_boundary:=true;
  goto reswitch;
  end;
hmode+char_ghost: begin t:=cur_chr; get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or(cur_cmd=char_given)or
   (cur_cmd=char_num) then begin
    if t=0 then new_left_ghost:=true
    else new_right_ghost:=true;
    end;
  goto reswitch;
  end;
@z
%---------------------------------------
@x [46] m.1030 l. - Omega
hmode+spacer: if space_factor=1000 then goto append_normal_space
  else app_space;
hmode+ex_space,mmode+ex_space: goto append_normal_space;
@y
hmode+spacer:
  if MML_mode then begin
    make_sgml_entity(" ");
    goto big_switch;
    end
  else if space_factor=1000 then goto append_normal_space
  else app_space;
hmode+ex_space,mmode+ex_space: goto append_normal_space;
@z
%---------------------------------------
@x [46] m.1032 l.20030 - Omega
$$|cur_r|=\cases{|character(lig_stack)|,&if |lig_stack>null|;\cr
  |font_bchar[cur_font]|,&otherwise;\cr}$$
except when |character(lig_stack)=font_false_bchar[cur_font]|.
@y
$$|cur_r|=\cases{|character(lig_stack)|,&if |lig_stack>null|;\cr
  |font_bchar(cur_font)|,&otherwise;\cr}$$
except when |character(lig_stack)=font_false_bchar(cur_font)|.
@z
%---------------------------------------
@x [46] m. l.20044 - Omega
@!cancel_boundary:boolean; {should the left boundary be ignored?}
@!ins_disc:boolean; {should we insert a discretionary node?}
@y
@!cancel_boundary:boolean; {should the left boundary be ignored?}
@!left_ghost:boolean; {character left of cursor is a left ghost}
@!right_ghost:boolean; {character left of cursor is a right ghost}
@!new_left_ghost:boolean; {character right of cursor is a left ghost}
@!new_right_ghost:boolean; {character right of cursor is a right ghost}
@!ins_disc:boolean; {should we insert a discretionary node?}
@!k_glue:integer;
@!font_glue_spec:pointer;
@z
%---------------------------------------
@x [46] m. l.20052 - Omega
ligature_present:=false; cancel_boundary:=false; lft_hit:=false; rt_hit:=false;
@y
ligature_present:=false; cancel_boundary:=false; lft_hit:=false; rt_hit:=false;
left_ghost:=false; right_ghost:=false;
new_left_ghost:=false; new_right_ghost:=false;
@z
%---------------------------------------
@x [46] m.1034 l.20075 - Omega
adjust_space_factor;@/
main_f:=cur_font;
bchar:=font_bchar[main_f]; false_bchar:=font_false_bchar[main_f];
if mode>0 then if language<>clang then fix_language;
fast_get_avail(lig_stack); font(lig_stack):=main_f; cur_l:=qi(cur_chr);
character(lig_stack):=cur_l;@/
cur_q:=tail;
if cancel_boundary then
  begin cancel_boundary:=false; main_k:=non_address;
  end
else main_k:=bchar_label[main_f];
if main_k=non_address then goto main_loop_move+2; {no left boundary processing}
cur_r:=cur_l; cur_l:=non_char;
goto main_lig_loop+1; {begin with cursor after left boundary}
@y
adjust_space_factor;@/
main_f:=cur_font;
bchar:=font_bchar(main_f); false_bchar:=font_false_bchar(main_f);
if mode>0 then if language<>clang then fix_language;
fast_get_avail(lig_stack); font(lig_stack):=main_f; cur_l:=qi(cur_chr);
character(lig_stack):=cur_l;@/
cur_q:=tail;
if cancel_boundary then
  begin cancel_boundary:=false; main_k:=non_address;
  left_ghost:=new_left_ghost; right_ghost:=new_right_ghost;
  new_left_ghost:=false; new_right_ghost:=false;
  end
else main_k:=bchar_label(main_f);
if main_k=non_address then goto main_loop_move+2; {no left boundary processing}
cur_r:=cur_l; cur_l:=non_char;
if not new_right_ghost then
  goto main_lig_loop+1; {begin with cursor after left boundary}
@z
%---------------------------------------
@x [46] m.1035 l.20122 - Omega
  begin if character(tail)=qi(hyphen_char[main_f]) then if link(cur_q)>null then
@y
  begin if character(tail)=qi(hyphen_char(main_f)) then if link(cur_q)>null then
@z
%---------------------------------------
@x [46] m.1036 l.20135 - Omega
if lig_stack=null then goto reswitch;
cur_q:=tail; cur_l:=character(lig_stack);
main_loop_move+1:if not is_char_node(lig_stack) then goto main_loop_move_lig;
main_loop_move+2:if(cur_chr<font_bc[main_f])or(cur_chr>font_ec[main_f]) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
main_i:=char_info(main_f)(cur_l);
if not char_exists(main_i) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
tail_append(lig_stack) {|main_loop_lookahead| is next}
@y
if lig_stack=null then goto reswitch;
cur_q:=tail; cur_l:=character(lig_stack);
main_loop_move+1:if not is_char_node(lig_stack) then goto main_loop_move_lig;
main_loop_move+2:
left_ghost:=new_left_ghost; right_ghost:=new_right_ghost;
new_left_ghost:=false; new_right_ghost:=false;
if(cur_chr<font_bc(main_f))or(cur_chr>font_ec(main_f)) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
main_i:=char_info(main_f)(cur_l);
if not char_exists(main_i) then
  begin char_warning(main_f,cur_chr); free_avail(lig_stack); goto big_switch;
  end;
if not (left_ghost or right_ghost) then
  tail_append(lig_stack) {|main_loop_lookahead| is next}
@z
%---------------------------------------
@x [46] m. l.20177 - Omega
if cur_cmd=no_boundary then bchar:=non_char;
@y
if cur_cmd=char_ghost then begin t:=cur_chr; get_x_token;
  if (cur_cmd=letter)or(cur_cmd=other_char)or(cur_cmd=char_given)or
   (cur_cmd=char_num) then begin
    if t=0 then new_left_ghost:=true
    else new_right_ghost:=true;
    back_input;
    goto main_loop_lookahead;
    end;
  end;
if cur_cmd=no_boundary then bchar:=non_char;
@z
%---------------------------------------
@x [46] m.1039 l.20193 - Omega
@<If there's a ligature/kern command...@>=
if char_tag(main_i)<>lig_tag then goto main_loop_wrapup;
main_k:=lig_kern_start(main_f)(main_i); main_j:=font_info[main_k].qqqq;
if skip_byte(main_j)<=stop_flag then goto main_lig_loop+2;
main_k:=lig_kern_restart(main_f)(main_j);
main_lig_loop+1:main_j:=font_info[main_k].qqqq;
main_lig_loop+2:if next_char(main_j)=cur_r then
 if skip_byte(main_j)<=stop_flag then
  @<Do ligature or kern command, returning to |main_lig_loop|
  or |main_loop_wrapup| or |main_loop_move|@>;
if skip_byte(main_j)=qi(0) then incr(main_k)
else begin if skip_byte(main_j)>=stop_flag then goto main_loop_wrapup;
  main_k:=main_k+qo(skip_byte(main_j))+1;
  end;
goto main_lig_loop+1
@y
@<If there's a ligature/kern command...@>=
if new_right_ghost or left_ghost then goto main_loop_wrapup;
if char_tag(main_i)<>lig_tag then goto main_loop_wrapup;
main_k:=lig_kern_start(main_f)(main_i);
main_j:=font_info(main_f)(main_k).qqqq;
if skip_byte(main_j)<=stop_flag then goto main_lig_loop+2;
main_k:=lig_kern_restart(main_f)(main_j);
main_lig_loop+1: main_j:=font_info(main_f)(main_k).qqqq;
main_lig_loop+2:if top_skip_byte(main_j)=0 then begin
    if next_char(main_f)(main_j)=cur_r then
      if skip_byte(main_j)<=stop_flag then
        @<Do ligature or kern command, returning to |main_lig_loop|
          or |main_loop_wrapup| or |main_loop_move|@>;
    end
  else begin
    if (font_bc(main_f)<=cur_r) then
      if (font_ec(main_f)>=cur_r) then
        if (char_exists(char_info(main_f)(cur_r))) then
          if cur_r<>bchar then
            if next_char(main_f)(main_j)=
               attr_zero_char_ivalue(main_f)(cur_r) then
              @<Do glue or penalty command@>;
    end;
if skip_byte(main_j)=qi(0) then incr(main_k)
else begin if skip_byte(main_j)>=stop_flag then goto main_loop_wrapup;
  main_k:=main_k+qo(skip_byte(main_j))+1;
  end;
goto main_lig_loop+1

@ There are currently three commands.  Command 17 will add a penalty
node between the two characters.  Command 18 will add a glue
node between the two characters.  Command 19 will add a penalty node,
then a glue node between the two characters.  Command 20
will add a kern nore between the two characters.

@<Do glue or penalty command@>=
begin
if new_left_ghost or right_ghost then goto main_loop_wrapup;
case op_byte(main_j) of
  qi(20): begin wrapup(rt_hit);
    tail_append(new_kern(attr_zero_kern(main_f)(rem_byte(main_j))));
    end;
  qi(18): begin wrapup(rt_hit);
    k_glue:=glues_base(main_f) + (rem_byte(main_j)*4);
    font_glue_spec:=new_spec(zero_glue);
    width(font_glue_spec):= font_info(main_f)(k_glue+1).sc;
    stretch(font_glue_spec):=font_info(main_f)(k_glue+2).sc;
    shrink(font_glue_spec):=font_info(main_f)(k_glue+3).sc;
    tail_append(new_glue(font_glue_spec));
    end;
  qi(19): begin wrapup(rt_hit);
    tail_append(new_penalty(attr_zero_penalty(main_f)(rem_top_byte(main_j))));
    k_glue:=glues_base(main_f) + (rem_bot_byte(main_j)*4);
    font_glue_spec:=new_spec(zero_glue);
    width(font_glue_spec):= font_info(main_f)(k_glue+1).sc;
    stretch(font_glue_spec):=font_info(main_f)(k_glue+2).sc;
    shrink(font_glue_spec):=font_info(main_f)(k_glue+3).sc;
    tail_append(new_glue(font_glue_spec));
    end;
  qi(17): begin wrapup(rt_hit);
    tail_append(new_penalty(attr_zero_penalty(main_f)(rem_byte(main_j))));
    end;
  end;
goto main_loop_move;
end
@z
%---------------------------------------
@x [46] m.1040 l.20219 - Omega
begin if op_byte(main_j)>=kern_flag then
  begin wrapup(rt_hit);
  tail_append(new_kern(char_kern(main_f)(main_j))); goto main_loop_move;
  end;
if cur_l=non_char then lft_hit:=true
else if lig_stack=null then rt_hit:=true;
check_interrupt; {allow a way out in case there's an infinite ligature loop}
case op_byte(main_j) of
qi(1),qi(5):begin cur_l:=rem_byte(main_j); {\.{=:\?}, \.{=:\?>}}
  main_i:=char_info(main_f)(cur_l); ligature_present:=true;
  end;
qi(2),qi(6):begin cur_r:=rem_byte(main_j); {\.{\?=:}, \.{\?=:>}}
  if lig_stack=null then {right boundary character is being consumed}
    begin lig_stack:=new_lig_item(cur_r); bchar:=non_char;
    end
  else if is_char_node(lig_stack) then {|link(lig_stack)=null|}
    begin main_p:=lig_stack; lig_stack:=new_lig_item(cur_r);
    lig_ptr(lig_stack):=main_p;
    end
  else character(lig_stack):=cur_r;
  end;
qi(3):begin cur_r:=rem_byte(main_j); {\.{\?=:\?}}
  main_p:=lig_stack; lig_stack:=new_lig_item(cur_r);
  link(lig_stack):=main_p;
  end;
qi(7),qi(11):begin wrapup(false); {\.{\?=:\?>}, \.{\?=:\?>>}}
  cur_q:=tail; cur_l:=rem_byte(main_j);
  main_i:=char_info(main_f)(cur_l); ligature_present:=true;
  end;
othercases begin cur_l:=rem_byte(main_j); ligature_present:=true; {\.{=:}}
  if lig_stack=null then goto main_loop_wrapup
  else goto main_loop_move+1;
  end
endcases;
if op_byte(main_j)>qi(4) then
  if op_byte(main_j)<>qi(7) then goto main_loop_wrapup;
if cur_l<non_char then goto main_lig_loop;
main_k:=bchar_label[main_f]; goto main_lig_loop+1;
end
@y
begin if op_byte(main_j)>=kern_flag then
  begin wrapup(rt_hit);
  tail_append(new_kern(char_kern(main_f)(main_j))); goto main_loop_move;
  end;
if new_left_ghost or right_ghost then goto main_loop_wrapup;
if cur_l=non_char then lft_hit:=true
else if lig_stack=null then rt_hit:=true;
check_interrupt; {allow a way out in case there's an infinite ligature loop}
case op_byte(main_j) of
qi(1),qi(5):begin cur_l:=rem_char_byte(main_f)(main_j); {\.{=:\?}, \.{=:\?>}}
  main_i:=char_info(main_f)(cur_l); ligature_present:=true;
  end;
qi(2),qi(6):begin cur_r:=rem_char_byte(main_f)(main_j); {\.{\?=:}, \.{\?=:>}}
  if lig_stack=null then {right boundary character is being consumed}
    begin lig_stack:=new_lig_item(cur_r); bchar:=non_char;
    end
  else if is_char_node(lig_stack) then {|link(lig_stack)=null|}
    begin main_p:=lig_stack; lig_stack:=new_lig_item(cur_r);
    lig_ptr(lig_stack):=main_p;
    end
  else character(lig_stack):=cur_r;
  end;
qi(3):begin cur_r:=rem_char_byte(main_f)(main_j); {\.{\?=:\?}}
  main_p:=lig_stack; lig_stack:=new_lig_item(cur_r);
  link(lig_stack):=main_p;
  end;
qi(7),qi(11):begin wrapup(false); {\.{\?=:\?>}, \.{\?=:\?>>}}
  cur_q:=tail; cur_l:=rem_char_byte(main_f)(main_j);
  main_i:=char_info(main_f)(cur_l); ligature_present:=true;
  end;
othercases begin cur_l:=rem_char_byte(main_f)(main_j); 
                 ligature_present:=true; {\.{=:}}
  if lig_stack=null then goto main_loop_wrapup
  else goto main_loop_move+1;
  end
endcases;
if op_byte(main_j)>qi(4) then
  if op_byte(main_j)<>qi(7) then goto main_loop_wrapup;
if cur_l<non_char then goto main_lig_loop;
main_k:=bchar_label(main_f); goto main_lig_loop+1;
end
@z
%---------------------------------------
@x [46] m.1042 l.20285 - Omega
begin main_p:=font_glue[cur_font];
if main_p=null then
  begin main_p:=new_spec(zero_glue); main_k:=param_base[cur_font]+space_code;
  width(main_p):=font_info[main_k].sc; {that's |space(cur_font)|}
  stretch(main_p):=font_info[main_k+1].sc; {and |space_stretch(cur_font)|}
  shrink(main_p):=font_info[main_k+2].sc; {and |space_shrink(cur_font)|}
  font_glue[cur_font]:=main_p;
@y
begin main_p:=font_glue(cur_font);
if main_p=null then
  begin main_p:=new_spec(zero_glue); main_k:=param_base(cur_font)+space_code;
  width(main_p):=font_info(cur_font)(main_k).sc; {that's |space(cur_font)|}
  stretch(main_p):=font_info(cur_font)(main_k+1).sc;
     {and |space_stretch(cur_font)|}
  shrink(main_p):=font_info(cur_font)(main_k+2).sc;
     {and |space_shrink(cur_font)|}
  font_glue(cur_font):=main_p;
@z
%---------------------------------------
@x [46] m.1046 l.20339 - Omega
non_math(math_given), non_math(math_comp), non_math(delim_num),
@y
non_math(math_given), non_math(omath_given),
non_math(math_comp), non_math(delim_num),
@z
%---------------------------------------
@x [47] m.1060 l.20533 - Omega
procedure append_glue;
var s:small_number; {modifier of skip command}
begin s:=cur_chr;
case s of
fil_code: cur_val:=fil_glue;
fill_code: cur_val:=fill_glue;
ss_code: cur_val:=ss_glue;
fil_neg_code: cur_val:=fil_neg_glue;
skip_code: scan_glue(glue_val);
mskip_code: scan_glue(mu_val);
end; {now |cur_val| points to the glue specification}
tail_append(new_glue(cur_val));
if s>=skip_code then
  begin decr(glue_ref_count(cur_val));
  if s>skip_code then subtype(tail):=mu_glue;
  end;
end;
@y
procedure append_glue;
var s:small_number; {modifier of skip command}
begin s:=cur_chr;
case s of
fil_code: cur_val:=fil_glue;
fill_code: cur_val:=fill_glue;
ss_code: cur_val:=ss_glue;
fil_neg_code: cur_val:=fil_neg_glue;
skip_code: scan_glue(glue_val);
mskip_code: scan_glue(mu_val);
end; {now |cur_val| points to the glue specification}
if (abs(mode)=mmode) and MML_mode then begin
  end
else begin
  tail_append(new_glue(cur_val));
  if s>=skip_code then
    begin decr(glue_ref_count(cur_val));
    if s>skip_code then subtype(tail):=mu_glue;
    end;
  end;
end;
@z
%---------------------------------------
@x [47] m.1061 l.20339 - Omega
procedure append_kern;
var s:quarterword; {|subtype| of the kern node}
begin s:=cur_chr; scan_dimen(s=mu_glue,false,false);
tail_append(new_kern(cur_val)); subtype(tail):=s;
end;
@y
procedure append_kern;
var s:quarterword; {|subtype| of the kern node}
begin s:=cur_chr; scan_dimen(s=mu_glue,false,false);
if (abs(mode)=mmode) and MML_mode then begin
  end
else begin
  tail_append(new_kern(cur_val)); subtype(tail):=s;
  end;
end;
@z
%---------------------------------------
@x [47] m.1071 l.20718 - Omega
|box_flag+255| represent `\.{\\setbox0}' through `\.{\\setbox255}';
codes |box_flag+256| through |box_flag+511| represent `\.{\\global\\setbox0}'
through `\.{\\global\\setbox255}';
code |box_flag+512| represents `\.{\\shipout}'; and codes |box_flag+513|
through |box_flag+515| represent `\.{\\leaders}', `\.{\\cleaders}',
@y
|box_flag+biggest_reg| represent `\.{\\setbox0}' through 
`\.{\\setbox}|biggest_reg|'; codes |box_flag+biggest_reg+1| through 
|box_flag+2*biggest_reg| represent `\.{\\global\\setbox0}'
through `\.{\\global\\setbox}|biggest_reg|'; code |box_flag+2*number_regs| 
represents `\.{\\shipout}'; and codes |box_flag+2*number_regs+1|
through |box_flag+2*number_regs+3| represent `\.{\\leaders}', `\.{\\cleaders}',
@z
%---------------------------------------
@x [47] m.1071 l.20732 - Omega
@d ship_out_flag==box_flag+512 {context code for `\.{\\shipout}'}
@d leader_flag==box_flag+513 {context code for `\.{\\leaders}'}
@y
@d ship_out_flag==box_flag+(number_regs+number_regs) 
   {context code for `\.{\\shipout}'}
@d leader_flag==ship_out_flag+1 {context code for `\.{\\leaders}'}
@z
%---------------------------------------
@x [47] m.1073 l.20795 - Omega
vmode+hmove,hmode+vmove,mmode+vmove: begin t:=cur_chr;
  scan_normal_dimen;
  if t=0 then scan_box(cur_val)@+else scan_box(-cur_val);
  end;
any_mode(leader_ship): scan_box(leader_flag-a_leaders+cur_chr);
any_mode(make_box): begin_box(0);
@y
vmode+hmove,hmode+vmove,mmode+vmove: begin
  if abs(mode)=mmode and MML_mode then begin
    print_err("Unauthorized entry in math expression: ");
    print_cmd_chr(cur_cmd,cur_chr); print_ln;
    print_nl("The MathML translator cannot continue");
    succumb;
    end
  else begin
    t:=cur_chr; scan_normal_dimen;
    if t=0 then scan_box(cur_val)@+else scan_box(-cur_val);
    end;
  end;
any_mode(leader_ship): begin
  if abs(mode)=mmode and MML_mode then begin
    print_err("Unauthorized entry in math expression: ");
    print_cmd_chr(cur_cmd,cur_chr); print_ln;
    print_nl("The MathML translator cannot continue");
    succumb;
    end
  else
    scan_box(leader_flag-a_leaders+cur_chr);
  end;
any_mode(make_box): begin
  if abs(mode)=mmode and MML_mode then begin
    print_err("Unauthorized entry in math expression: ");
    print_cmd_chr(cur_cmd,cur_chr); print_ln;
    print_nl("The MathML translator cannot continue");
    succumb;
    end
  else
    begin_box(0);
  end;
@z
%---------------------------------------
@x [47] m.1077 l.20853 - Omega
if box_context<box_flag+256 then
  eq_define(box_base-box_flag+box_context,box_ref,cur_box)
else geq_define(box_base-box_flag-256+box_context,box_ref,cur_box)
@y
if box_context<box_flag+number_regs then
  eq_define(box_base-box_flag+box_context,box_ref,cur_box)
else geq_define(box_base-box_flag-number_regs+box_context,box_ref,cur_box)
@z
%---------------------------------------
@x [47] m.1079 l.20888 - Omega
  box(cur_val):=null; {the box becomes void, at the same level}
@y
  set_equiv(box_base+cur_val,null);
      {the box becomes void, at the same level}
@z
%---------------------------------------
@x [47] m.l.21042 - Omega
vmode+letter,vmode+other_char,vmode+char_num,vmode+char_given,
   vmode+math_shift,vmode+un_hbox,vmode+vrule,
   vmode+accent,vmode+discretionary,vmode+hskip,vmode+valign,
   vmode+ex_space,vmode+no_boundary:@t@>@;@/
@y
vmode+letter,vmode+other_char,vmode+char_num,vmode+char_given,
   vmode+char_ghost,
   vmode+math_shift,vmode+un_hbox,vmode+vrule,
   vmode+accent,vmode+discretionary,vmode+hskip,vmode+valign,
   vmode+ex_space,vmode+no_boundary:@t@>@;@/
@z
%---------------------------------------
@x [47] m. l.21055 - Omega
procedure new_graf(@!indented:boolean);
begin prev_graf:=0;
if (mode=vmode)or(head<>tail) then
  tail_append(new_param_glue(par_skip_code));
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
if indented then
  begin tail:=new_null_box; link(head):=tail; width(tail):=par_indent;@+
  end;
if every_par<>null then begin_token_list(every_par,every_par_text);
if nest_ptr=1 then build_page; {put |par_skip| glue on current page}
end;
@y
procedure sgml_start_graf; forward;

procedure new_graf(@!indented:boolean);
begin prev_graf:=0;
if (mode=vmode)or(head<>tail) then
  tail_append(new_param_glue(par_skip_code));
if MML_mode then begin
  sgml_start_graf
  end
else begin
push_nest; mode:=hmode; space_factor:=1000; set_cur_lang; clang:=cur_lang;
prev_graf:=(norm_min(left_hyphen_min)*@'100+norm_min(right_hyphen_min))
             *@'200000+cur_lang;
if indented then
  begin tail:=new_null_box; link(head):=tail; width(tail):=par_indent;@+
  end;
if every_par<>null then begin_token_list(every_par,every_par_text);
if nest_ptr=1 then build_page; {put |par_skip| glue on current page}
end;
end;
@z
%---------------------------------------
@x [47] m.1100 l.21156 - Omega
procedure end_graf;
begin if mode=hmode then
  begin if head=tail then pop_nest {null paragraphs are ignored}
  else line_break(widow_penalty);
  normal_paragraph;
  error_count:=0;
  end;
end;
@y
procedure sgml_end_graf; forward;

procedure end_graf;
begin if mode=hmode then
  begin if head=tail then pop_nest {null paragraphs are ignored}
  else if MML_mode then sgml_end_graf
  else line_break(widow_penalty);
  normal_paragraph;
  error_count:=0;
  end;
end;
@z
%---------------------------------------
@x [47] m.1100 l.21156 - Omega
  if saved(0)<255 then
@y
  if saved(0)<>255 then
@z
%---------------------------------------
@x [47] m.1109 l.21265 - Omega
@ The |un_hbox| and |un_vbox| commands unwrap one of the 256 current boxes.
@y
@ The |un_hbox| and |un_vbox| commands unwrap one of the current boxes.
@z
%---------------------------------------
@x [47] m.1110 l.21287 - Omega
else  begin link(tail):=list_ptr(p); box(cur_val):=null;
@y
else  begin link(tail):=list_ptr(p);
set_equiv(box_base+cur_val,null);
@z
%---------------------------------------
@x [47] m.1117 l.21342 - Omega
  begin c:=hyphen_char[cur_font];
  if c>=0 then if c<256 then pre_break(tail):=new_character(cur_font,c);
@y
  begin c:=hyphen_char(cur_font);
  if c>=0 then if c<=biggest_char then 
     pre_break(tail):=new_character(cur_font,c);
@z
%---------------------------------------
@x [47] m.1130 l.21562 - Omega
vmode+halign,hmode+valign:init_align;
mmode+halign: if privileged then
  if cur_group=math_shift_group then init_align
  else off_save;
vmode+endv,hmode+endv: do_endv;
@y
vmode+halign,hmode+valign:init_align;
mmode+halign: if MML_mode then begin
    print_err("Unauthorized entry in math expression: ");
    print_esc("halign"); print_ln;
    print_nl("The MathML translator cannot continue");
    succumb;
    end
  else begin
    if privileged then
    if cur_group=math_shift_group then init_align
    else off_save;
    end;
vmode+endv,hmode+endv: do_endv;
@z
%---------------------------------------
@x [48] m.1136 l.21605 - Omega
@* \[48] Building math lists.
The routines that \TeX\ uses to create mlists are similar to those we have
just seen for the generation of hlists and vlists. But it is necessary to
make ``noads'' as well as nodes, so the reader should review the
discussion of math mode data structures before trying to make sense out of
the following program.

Here is a little routine that needs to be done whenever a subformula
is about to be processed. The parameter is a code like |math_group|.

@<Declare act...@>=
procedure push_math(@!c:group_code);
begin push_nest; mode:=-mmode; incompleat_noad:=null; new_save_level(c);
end;
@y
@* \[48] Building math lists.
The routines that \TeX\ uses to create mlists are similar to those we have
just seen for the generation of hlists and vlists. But it is necessary to
make ``noads'' as well as nodes, so the reader should review the
discussion of math mode data structures before trying to make sense out of
the following program.

Here is a little routine that needs to be done whenever a subformula
is about to be processed. The parameter is a code like |math_group|.

@<Glob...@>=
@!MML_mode:boolean;
@!SGML_show_entities:boolean;
@!MML_level:integer;
@!mml_file_no:integer; {the \.{MML} output file}
@!mml_indent:integer;
@!mml_depth_level:integer;
@!mml_last_print_char:boolean;

@ @<Set init...@>=
MML_mode:=false;
SGML_show_entities:=true;
MML_level:=0;
mml_indent:=0;
mml_file_no:=0;
mml_depth_level:=0;
mml_last_print_char:=false;

@ 

@d sgml_out_pointer(#)==
  case math_type(#(p)) of
    math_char: begin
      fetch(#(p));
      fsort:=font_name_sort(cur_f);
      if char_exists(cur_i) then begin
        if (font_sort_char_tag(fsort)(cur_c)<>0) then begin
          if not inside_mop then begin
            for i:=1 to mml_indent do print("  ");
            print("<"); print(font_sort_char_tag(fsort)(cur_c));
            if (font_sort_char_attr(fsort)(cur_c)<>null) then begin
              sgml_attrs_out(font_sort_char_attr(fsort)(cur_c));
              end;
            print("> "); 
            end;
          if (font_sort_char_entity(fsort)(cur_c)<>0) then begin
            print(font_sort_char_entity(fsort)(cur_c));
            end;
          if not inside_mop then begin
            print(" </"); print(font_sort_char_tag(fsort)(cur_c));
            print(">"); print(new_line_char);
            back_at_bol:=true;
            end;
          end;
        end;
      end;
    sub_mlist: begin
      case type(p) of
        op_noad,bin_noad,rel_noad,
        open_noad,close_noad,punct_noad,inner_noad: begin
          for i:=1 to mml_indent do print("  ");
          print("<mo");
          if (type(p)=op_noad) and (subtype(p)=limits) then begin
            print(" limits=""true""");
            end
          else if (type(p)=op_noad) and (subtype(p)=no_limits) then begin
            print(" limits=""false""");
            end;
          print("> ");
          q:=p; cur_mlist:=info(#(p));
          if link(cur_mlist)=null then
            if type(cur_mlist)=sgml_node then
              if str_eq_str(sgml_tag(cur_mlist),"mrow") then
                cur_mlist:=sgml_info(cur_mlist);
          sgml_out_mlist(false,false,true);  p:=q;
          print(" </mo>");
          print(new_line_char);
          back_at_bol:=true;
          end;
        othercases begin
          q:=p; cur_mlist:=info(#(p));
          sgml_out_mlist(false,false,false);  p:=q;
          end
        endcases;
      end;
    othercases begin
      for i:=1 to mml_indent do print("  ");
      print("<merror> Unrecognized math stuff </merror>");
      print(new_line_char);
      end
    endcases;

@<Declare act...@>=
procedure push_math(@!c:group_code);
begin push_nest; mode:=-mmode; incompleat_noad:=null; new_save_level(c);
end;

function sgml_out_on_one_line(p:pointer):boolean;
begin
if (sgml_tag(p)="mi") or (sgml_tag(p)="mo") or (sgml_tag(p)="mn") then
  sgml_out_on_one_line:=true
else sgml_out_on_one_line:=false;
end;

procedure sgml_attrs_out(p:pointer);
var q:pointer;
begin
q:=p;
while q<>null do begin
  print(" ");
  print(sgml_tag(q)); print("=""");
  print(sgml_attrs(q)); print("""");
  q:=link(q);
  end;
end;

procedure sgml_out_mlist(break_line,inside_text,inside_mop:boolean);
var p,q:pointer;
    old_selector:integer;
    i,fsort:integer;
    back_at_bol:boolean;
    first_inside_text:boolean;
begin
old_selector:=selector;
selector:=max_selector+mml_file_no;
p:=cur_mlist;
back_at_bol:=false;
first_inside_text:=inside_text;
while p<>null do begin
  if is_char_node(p) then begin
    fsort:=font_name_sort(font(p));
    if (font_sort_char_entity(fsort)(character(p))<>0) then begin
      if back_at_bol then
        for i:=1 to mml_indent do print("  ");
      print(font_sort_char_entity(fsort)(character(p)));
      back_at_bol:=false;
      mml_last_print_char:=true;
      first_inside_text:=false;
      end;
    end
  else begin
    case type(p) of
      hlist_node,vlist_node,rule_node,
      ins_node,mark_node,adjust_node: do_nothing;
      ligature_node: begin
        fsort:=font_name_sort(font(lig_char(p)));
        if (font_sort_char_entity(fsort)(character(lig_char(p)))<>0) then begin
          if back_at_bol then
            for i:=1 to mml_indent do print("  ");
          print(font_sort_char_entity(fsort)(character(lig_char(p))));
          back_at_bol:=false;
          end;
        mml_last_print_char:=false;
        first_inside_text:=false;
        end;
      disc_node,whatsit_node,math_node,glue_node,
      kern_node,penalty_node,unset_node: do_nothing;
      sgml_node: begin
        if sgml_tag(p)=0 then begin
          print(sgml_attrs(p));
          end
        else if sgml_singleton(p)=1 then begin
          if not (first_inside_text) then begin
            if inside_text then print(new_line_char);
            for i:=1 to mml_indent do print("  ");
            end;
          print("<"); print(sgml_tag(p));
          sgml_attrs_out(sgml_attrs(p)); print("/>");
          print(new_line_char);
          back_at_bol:=true;
          end
        else begin
          if not (first_inside_text) then begin
            if inside_text and not(back_at_bol) then print(new_line_char);
            for i:=1 to mml_indent do print("  ");
            end;
          print("<"); print(sgml_tag(p));
          sgml_attrs_out(sgml_attrs(p)); print(">");
          if sgml_out_on_one_line(p) then
            print(" ")
          else begin mml_indent:=mml_indent+1;
            print(new_line_char);
            end;
          cur_mlist:=sgml_info(p);
          if cur_mlist<>null then begin
            if (sgml_kind(p)=sgml_text_node) or
               (sgml_tag(p)="mtext") then begin
              for i:=1 to mml_indent do print("  ");
              sgml_out_mlist(false,true,false);
              if mml_last_print_char then print(new_line_char);
              end
            else sgml_out_mlist(false,false,false);
            end;
          if sgml_out_on_one_line(p) then
            print(" ")
          else begin
            mml_indent:=mml_indent-1;
            for i:=1 to mml_indent do print("  ");
            end;
          print("</"); print(sgml_tag(p)); print(">");
          print(new_line_char);
          back_at_bol:=true;
          end;
        mml_last_print_char:=false;
        first_inside_text:=false;
        end;
      sgml_entity_node:
      if not(first_inside_text and 
             (str_eq_str(sgml_entity_string(p)," "))) then begin
          if back_at_bol then
            for i:=1 to mml_indent do print("  ");
          print(sgml_entity_string(p));
          first_inside_text:=false;
          mml_last_print_char:=true;
          back_at_bol:=false;
          if break_line then begin
            print(new_line_char);
            back_at_bol:=true;
          end;
        end;
      fraction_noad: begin
        for i:=1 to mml_indent do print("  ");
        print("<mfrac> Arguments </mfrac>"); print(new_line_char);
        back_at_bol:=true;
        mml_last_print_char:=false;
        first_inside_text:=false;
        end;
      othercases begin
        if (info(subscr(p))<>null) or
           (info(supscr(p))<>null) then begin
          if inside_text then print(new_line_char);
          for i:=1 to mml_indent do print("  ");
          print("<m");
          if info(subscr(p))<>empty then print("sub");
          if info(supscr(p))<>empty then print("sup");
          print(">");
          mml_indent:=mml_indent+1;
          print(new_line_char);
          end;
        sgml_out_pointer(nucleus);
        if (info(subscr(p))<>null) or
           (info(supscr(p))<>null) then begin
          if info(subscr(p))<>null then begin
            sgml_out_pointer(subscr);
            end;
          if info(supscr(p))<>null then begin
            sgml_out_pointer(supscr);
            end;
          mml_indent:=mml_indent-1;
          for i:=1 to mml_indent do print("  ");
          print("</m");
          if info(subscr(p))<>null then print("sub");
          if info(supscr(p))<>null then print("sup");
          print(">");
          print(new_line_char);
          back_at_bol:=true;
          end;
        mml_last_print_char:=false;
        first_inside_text:=false;
        end;
      endcases;
    end;
  p:=link(p);
  end;
selector:=old_selector;
end;

procedure make_sgml_entity(s:str_number);
var p:pointer;
begin
link(tail):=new_sgml_entity_node;
tail:=link(tail);
sgml_entity_string(tail):=s;
end;

procedure scan_font_entity;
var cv1,cv2,cv3,cv4:integer;
begin
scan_string_argument; cv1:=cur_val;
scan_left_brace; scan_int; cv2:=cur_val; scan_right_brace;
scan_string_argument; cv3:=cur_val;
scan_string_argument; cv4:=cur_val;
save_ptr:=save_ptr+4;
saved(-1):=cv1; saved(-2):=cv2; saved(-3):=cv3; saved(-4):=cv4;
new_save_level(font_entity_group); scan_left_brace; push_nest;
end;

procedure scan_empty_tag;
begin
scan_string_argument;
link(tail):=new_sgml_node;
tail:=link(tail);
sgml_tag(tail):=cur_val;
sgml_kind(tail):=sgml_math_node;
sgml_singleton(tail):=1;
incr(save_ptr); saved(-1):=tail;
new_save_level(empty_tag_group); scan_left_brace; push_nest;
end;

procedure sgml_startmathtag(s:str_number);
begin
push_math(math_group); current_sgml_tag:=s;
end;


procedure sgml_starttexttag(s:str_number);
begin
push_nest; new_save_level(text_sgml_group);
current_sgml_tag:=s;
mode:=-hmode;
end;


procedure sgml_attribute(s,s1:str_number);
var p:pointer;
begin
p:=new_sgml_attr_node;
sgml_tag(p):=s;
sgml_attrs(p):=s1;
sgml_singleton(p):=0;
link(p):=current_sgml_attrs;
current_sgml_attrs:=p;
end;

procedure sgml_endtexttag(s:str_number);
var q:pointer;
begin
if not str_eq_str(s,current_sgml_tag) then begin
  print_err("Tags do not match: ");
  print(current_sgml_tag); print(","); print(s);
  print_ln;
  print_nl("The MathML translator cannot continue");
  succumb;
  end;
q:=new_sgml_node;
sgml_tag(q):=s;
sgml_kind(q):=sgml_text_node;
sgml_info(q):=link(head);
sgml_attrs(q):=current_sgml_attrs;
pop_nest;
unsave;
if mode=vmode then begin
  ensure_mml_open;
  cur_mlist:=q;
  sgml_out_mlist(false,false,false);
  end
else begin
  link(tail):=q;
  tail:=q;
  end;
end;

procedure sgml_write(s:str_number);
var old_selector:integer;
begin
ensure_mml_open;
old_selector:=selector;
selector:=max_selector+mml_file_no;
print(s);
selector:=old_selector;
end;

procedure sgml_endmathtag(s:str_number);
var q:pointer;
begin
if not str_eq_str(s,current_sgml_tag) then begin
  print_err("Tags do not match: ");
  print(current_sgml_tag); print(","); print(s);
  print_ln;
  print_nl("The MathML translator cannot continue");
  succumb;
  end;
q:=new_sgml_node;
sgml_tag(q):=s;
sgml_kind(q):=sgml_math_node;
if s<>"mtext" then sgml_attrs(q):=current_sgml_attrs;
sgml_info(q):=link(head);
if current_sgml_attrs=null then
  if (link(head)<>null) then
    if (str_eq_str(s,"mtr") or str_eq_str(s,"mtd")) and
       (link(link(head))=null) then begin
      q:=sgml_info(q);
      end;
pop_nest;
unsave;
link(tail):=q;
tail:=q;
end;

procedure sgml_start_graf;
begin
sgml_starttexttag("par")
end;

procedure sgml_end_graf;
begin
sgml_endtexttag("par")
end;

@ Entering or leaving MML mode 
by using the routine called |primitive|, defined below. Let us enter them
now, so that we don't have to list all those parameter names anywhere else.

@<Put each of \TeX's primitives into the hash table@>=
primitive("showSGMLentities",set_show_sgml_entities,1);
primitive("noshowSGMLentities",set_show_sgml_entities,0);
primitive("MMLmode",set_mml_mode,1);
@!@:MML_mode_}{\.{\\MMLmode} primitive@>
primitive("noMMLmode",set_mml_mode,0);
@!@:noMML_mode_}{\.{\\noMMLmode} primitive@>
primitive("SGMLstartmathtag",mml_tag,0);
primitive("SGMLendmathtag",mml_tag,1);
primitive("SGMLstarttexttag",mml_tag,2);
primitive("SGMLendtexttag",mml_tag,3);
primitive("SGMLattribute",mml_tag,4);
primitive("MMLstarttext",mml_tag,5);
primitive("MMLendtext",mml_tag,6);

primitive("SGMLampersand",other_char,"&");
primitive("SGMLbackslash",other_char,"\");
primitive("SGMLcarret",other_char,"^");
primitive("SGMLdollar",other_char,"$");
primitive("SGMLhash",other_char,"#");
primitive("SGMLleftbrace",other_char,"{");
primitive("SGMLpercent",other_char,"%");
primitive("SGMLrightbrace",other_char,"}");
primitive("SGMLunderscore",other_char,"_");

primitive("SGMLentity",mml_tag,7);
primitive("SGMLemptytag",mml_tag,8);
primitive("SGMLFontEntity",mml_tag,9);
primitive("SGMLwrite",mml_tag,10);
primitive("SGMLwriteln",mml_tag,11);

@ @<Cases of |main_control| that build boxes and lists@>=
mmode+set_mml_mode: report_illegal_case;
non_math(set_mml_mode):
  if cur_chr=0 then MML_mode:=false else MML_mode:=true;
any_mode(set_show_sgml_entities):
  if cur_chr=0 then SGML_show_entities:=false
               else SGML_show_entities:=true;
any_mode(mml_tag): begin
  case cur_chr of
    0: begin scan_string_argument; sgml_startmathtag(cur_val); end;
    1: begin scan_string_argument; sgml_endmathtag(cur_val); end;
    2: begin scan_string_argument; sgml_starttexttag(cur_val); end;
    3: begin scan_string_argument; sgml_endtexttag(cur_val); end;
    4: begin scan_string_argument; cur_val1:=cur_val;
             scan_string_argument; sgml_attribute(cur_val1,cur_val); end;
    5: begin
       push_nest; new_save_level(text_mml_group);
       current_sgml_tag:="mtext"; mode:=-hmode;
       end;
    6: begin sgml_endtexttag("mtext"); end;
    7: begin scan_string_argument; make_sgml_entity(cur_val); end;
    8: begin scan_empty_tag; end;
    9: begin scan_font_entity; end;
   10: begin scan_string_argument; sgml_write(cur_val); end;
   11: begin sgml_write(new_line_char); end;
  end;
  end;

@ The following code opens \.{MML} output file if neccesary.
@p procedure ensure_mml_open;
begin
  if mml_file_no=0 then begin
    incr(output_file_no);
    mml_file_no:=output_file_no;
    ensure_output_open(".mml")
                      (output_file_names[mml_file_no])
                      (output_files[mml_file_no]);
    output_file_names[mml_file_no]:=output_m_name;
    end;
end;

@ @<Declare act...@>=

@z
%---------------------------------------
@x [48] m.1137 l.21620 - Omega
@ We get into math mode from horizontal mode when a `\.\$' (i.e., a
|math_shift| character) is scanned. We must check to see whether this
`\.\$' is immediately followed by another, in case display math mode is
called for.

@<Cases of |main_control| that build...@>=
hmode+math_shift:init_math;
@y
@ We get into math mode from horizontal mode when a `\.\$' (i.e., a
|math_shift| character) is scanned. We must check to see whether this
`\.\$' is immediately followed by another, in case display math mode is
called for.

@<Cases of |main_control| that build...@>=
hmode+math_shift: begin
  if MML_mode then begin
    ensure_mml_open;
    incr(MML_level);
    end;
  init_math;
  end;

@z
%---------------------------------------
@x [48] m.1139 l.21648 - Omega
if every_math<>null then begin_token_list(every_math,every_math_text);
@y
current_sgml_tag:="mrow";
sgml_attribute("displaystyle","false");
if every_math<>null then begin_token_list(every_math,every_math_text);
@z
%---------------------------------------
@x [48] m.1145 l.21703 - Omega
if every_display<>null then begin_token_list(every_display,every_display_text);
@y
current_sgml_tag:="mrow";
sgml_attribute("displaystyle","true");
if every_display<>null then begin_token_list(every_display,every_display_text);
@z
%---------------------------------------
@x [48] m.1151 l.21810 - Omega
@ Recall that the |nucleus|, |subscr|, and |supscr| fields in a noad are
broken down into subfields called |math_type| and either |info| or
|(fam,character)|. The job of |scan_math| is to figure out what to place
in one of these principal fields; it looks at the subformula that
comes next in the input, and places an encoding of that subformula
into a given word of |mem|.

@d fam_in_range==((cur_fam>=0)and(cur_fam<16))

@<Declare act...@>=
procedure scan_math(@!p:pointer);
label restart,reswitch,exit;
var c:integer; {math character code}
begin restart:@<Get the next non-blank non-relax...@>;
reswitch:case cur_cmd of
letter,other_char,char_given: begin c:=ho(math_code(cur_chr));
    if c=@'100000 then
      begin @<Treat |cur_chr| as an active character@>;
      goto restart;
      end;
    end;
char_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=char_given;
  goto reswitch;
  end;
math_char_num: begin scan_fifteen_bit_int; c:=cur_val;
  end;
math_given: c:=cur_chr;
delim_num: begin scan_twenty_seven_bit_int; c:=cur_val div @'10000;
  end;
othercases @<Scan a subformula enclosed in braces and |return|@>
endcases;@/
math_type(p):=math_char; character(p):=qi(c mod 256);
if (c>=var_code)and fam_in_range then fam(p):=cur_fam
else fam(p):=(c div 256) mod 16;
exit:end;
@y
@ Recall that the |nucleus|, |subscr|, and |supscr| fields in a noad are
broken down into subfields called |math_type| and either |info| or
|(fam,character)|. The job of |scan_math| is to figure out what to place
in one of these principal fields; it looks at the subformula that
comes next in the input, and places an encoding of that subformula
into a given word of |mem|.

@d fam_in_range==((cur_fam>=0)and(cur_fam<16))

@<Declare act...@>=
@t\4@>@<Declare the function called |fin_mlist|@>@t@>@;@/

procedure scan_math(@!p:pointer);
label restart,reswitch,exit;
var c:integer; {math character code}
begin restart:@<Get the next non-blank non-relax...@>;
reswitch:
case cur_cmd of
letter,other_char,char_given: begin
  c:=ho(math_code(cur_chr));
  if c=@"8000000 then
    begin @<Treat |cur_chr| as an active character@>;
    goto restart;
    end;
  end;
char_num: begin scan_char_num; cur_chr:=cur_val; cur_cmd:=char_given;
  goto reswitch;
  end;
math_char_num: begin
  if cur_chr=0 then scan_fifteen_bit_int
  else scan_big_fifteen_bit_int;
  c:=cur_val;
  end;
math_given: begin
  c := ((cur_chr div @"1000) * @"1000000) +
         (((cur_chr mod @"1000) div @"100) * @"10000) +
         (cur_chr mod @"100);
  end;
omath_given: begin c:=cur_chr;
  end;
delim_num: begin
  if cur_chr=0 then scan_twenty_seven_bit_int
  else scan_fifty_one_bit_int;
  c:=cur_val;
  end;
othercases @<Scan a subformula enclosed in braces and |return|@>
endcases;@/
math_type(p):=math_char; character(p):=qi(c mod @"10000);
if (c>=var_code)and fam_in_range then fam(p):=cur_fam
else fam(p):=(c div @"10000) mod @"100;
exit:end;
@z
%---------------------------------------
@x [48] m.1154 l.21860 - Omega
mmode+letter,mmode+other_char,mmode+char_given:
  set_math_char(ho(math_code(cur_chr)));
mmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  set_math_char(ho(math_code(cur_chr)));
  end;
mmode+math_char_num: begin scan_fifteen_bit_int; set_math_char(cur_val);
  end;
mmode+math_given: set_math_char(cur_chr);
mmode+delim_num: begin scan_twenty_seven_bit_int;
  set_math_char(cur_val div @'10000);
@y
mmode+letter,mmode+other_char,mmode+char_given:
  set_math_char(ho(math_code(cur_chr)));
mmode+char_num: begin scan_char_num; cur_chr:=cur_val;
  set_math_char(ho(math_code(cur_chr)));
  end;
mmode+math_char_num: begin
  if cur_chr=0 then scan_fifteen_bit_int
  else scan_big_fifteen_bit_int;
  set_math_char(cur_val);
  end;
mmode+math_given: begin
  set_math_char(((cur_chr div @"1000) * @"1000000) +
                (((cur_chr mod @"1000) div @"100) * @"10000) +
                (cur_chr mod @"100));
  end;
mmode+omath_given: begin
  set_math_char(cur_chr);
  end;
mmode+delim_num: begin
  if cur_chr=0 then scan_twenty_seven_bit_int
  else scan_fifty_one_bit_int;
  set_math_char(cur_val);
@z
%---------------------------------------
@x [48] m.1155 l.21878 - Omega
procedure set_math_char(@!c:integer);
var p:pointer; {the new noad}
begin if c>=@'100000 then
  @<Treat |cur_chr|...@>
else  begin p:=new_noad; math_type(nucleus(p)):=math_char;
  character(nucleus(p)):=qi(c mod 256);
  fam(nucleus(p)):=(c div 256) mod 16;
  if c>=var_code then
    begin if fam_in_range then fam(nucleus(p)):=cur_fam;
    type(p):=ord_noad;
    end
  else  type(p):=ord_noad+(c div @'10000);
  link(tail):=p; tail:=p;
  end;
end;
@y
procedure set_math_char(@!c:integer);
var p,q,r:pointer; {the new noad}
begin if c>=@"8000000 then
  @<Treat |cur_chr|...@>
else  begin p:=new_noad; math_type(nucleus(p)):=math_char;
  character(nucleus(p)):=qi(c mod @"10000);
  fam(nucleus(p)):=(c div @"10000) mod @"100;
  if c>=var_code then
    begin if fam_in_range then fam(nucleus(p)):=cur_fam;
    type(p):=ord_noad;
    end
  else  type(p):=ord_noad+(c div @"1000000);
  if not MML_mode then
    tail_append(p)
  else if (c div @"1000000)=4 then begin
    saved(0):=p;
    incr(save_ptr);
    push_math(math_mml_group);
    end
  else if (cur_group=math_mml_group) and
          ((c div @"1000000)=5) then begin
    unsave; decr(save_ptr);
    q:=saved(0);
    r:=fin_mlist(null);
    push_math(math_group);
    tail_append(q); tail_append(r); tail_append(p);
    unsave;
    p:=fin_mlist(null);
    tail_append(p);
    end
  else tail_append(p);
  end;
end;
@z
%---------------------------------------
@x [48] m.1158 l.21944 - Omega
mmode+math_comp: begin tail_append(new_noad);
  type(tail):=cur_chr; scan_math(nucleus(tail));
  end;
mmode+limit_switch: math_limit_switch;
@y
mmode+math_comp: begin tail_append(new_noad);
  type(tail):=cur_chr; scan_math(nucleus(tail));
  end;
mmode+limit_switch: math_limit_switch;
@z
%---------------------------------------
@x [48] m.1158 l.21950 - Omega
procedure math_limit_switch;
label exit;
begin if head<>tail then if type(tail)=op_noad then
  begin subtype(tail):=cur_chr; return;
  end;
print_err("Limit controls must follow a math operator");
@.Limit controls must follow...@>
help1("I'm ignoring this misplaced \limits or \nolimits command."); error;
exit:end;
@y
procedure math_limit_switch;
label exit;
begin if head<>tail then if type(tail)=op_noad then
  begin subtype(tail):=cur_chr; return;
  end;
print_err("Limit controls must follow a math operator");
@.Limit controls must follow...@>
help1("I'm ignoring this misplaced \limits or \nolimits command."); error;
exit:end;
@z
%---------------------------------------
@x [48] m.1160 l.21966 - Omega
procedure scan_delimiter(@!p:pointer;@!r:boolean);
begin if r then scan_twenty_seven_bit_int
else  begin @<Get the next non-blank non-relax...@>;
  case cur_cmd of
  letter,other_char: cur_val:=del_code(cur_chr);
  delim_num: scan_twenty_seven_bit_int;
  othercases cur_val:=-1
  endcases;
  end;
if cur_val<0 then @<Report that an invalid delimiter code is being changed
   to null; set~|cur_val:=0|@>;
small_fam(p):=(cur_val div @'4000000) mod 16;
small_char(p):=qi((cur_val div @'10000) mod 256);
large_fam(p):=(cur_val div 256) mod 16;
large_char(p):=qi(cur_val mod 256);
@y
procedure scan_delimiter(@!p:pointer;@!r:integer);
begin if r=1 then scan_twenty_seven_bit_int
else if r=2 then scan_fifty_one_bit_int
else  begin @<Get the next non-blank non-relax...@>;
  case cur_cmd of
  letter,other_char: begin
    cur_val:=del_code0(cur_chr); cur_val1:=del_code1(cur_chr);
    end;
  delim_num: if cur_chr=0 then scan_twenty_seven_bit_int
             else scan_fifty_one_bit_int;
  othercases begin cur_val:=-1; cur_val1:=-1; end;
  endcases;
  end;
if cur_val<0 then begin @<Report that an invalid delimiter code is being changed
   to null; set~|cur_val:=0|@>;
 cur_val1:=0;
 end;
small_fam(p):=(cur_val div @"10000) mod @"100;
small_char(p):=qi(cur_val mod @"10000);
large_fam(p):=(cur_val1 div @"10000) mod @"100;
large_char(p):=qi(cur_val1 mod @"10000);
@z
%---------------------------------------
@x [48] m.1163 l.22005 - Omega
scan_delimiter(left_delimiter(tail),true); scan_math(nucleus(tail));
@y
scan_delimiter(left_delimiter(tail),cur_chr+1); scan_math(nucleus(tail));
@z
%---------------------------------------
@x [48] m.1165 l.22012 - Omega
procedure math_ac;
begin if cur_cmd=accent then
  @<Complain that the user should have said \.{\\mathaccent}@>;
tail_append(get_node(accent_noad_size));
type(tail):=accent_noad; subtype(tail):=normal;
mem[nucleus(tail)].hh:=empty_field;
mem[subscr(tail)].hh:=empty_field;
mem[supscr(tail)].hh:=empty_field;
math_type(accent_chr(tail)):=math_char;
scan_fifteen_bit_int;
character(accent_chr(tail)):=qi(cur_val mod 256);
if (cur_val>=var_code)and fam_in_range then fam(accent_chr(tail)):=cur_fam
else fam(accent_chr(tail)):=(cur_val div 256) mod 16;
scan_math(nucleus(tail));
end;
@y
procedure math_ac;
begin if cur_cmd=accent then
  @<Complain that the user should have said \.{\\mathaccent}@>;
tail_append(get_node(accent_noad_size));
type(tail):=accent_noad; subtype(tail):=normal;
mem[nucleus(tail)].hh:=empty_field;
mem[subscr(tail)].hh:=empty_field;
mem[supscr(tail)].hh:=empty_field;
math_type(accent_chr(tail)):=math_char;
if cur_chr=0 then scan_fifteen_bit_int
else scan_big_fifteen_bit_int;
character(accent_chr(tail)):=qi(cur_val mod @"10000);
if (cur_val>=var_code)and fam_in_range then fam(accent_chr(tail)):=cur_fam
else fam(accent_chr(tail)):=(cur_val div @"10000) mod @"100;
scan_math(nucleus(tail));
end;
@z
%---------------------------------------
@x [48] m.1174 l.22085 - Omega
@t\4@>@<Declare the function called |fin_mlist|@>@t@>@;@/
@y
@z
%---------------------------------------
@x [48] m.1176--1177 l.22109 - Omega
procedure sub_sup;
var t:small_number; {type of previous sub/superscript}
@!p:pointer; {field to be filled by |scan_math|}
begin t:=empty; p:=null;
if tail<>head then if scripts_allowed(tail) then
  begin p:=supscr(tail)+cur_cmd-sup_mark; {|supscr| or |subscr|}
  t:=math_type(p);
  end;
if (p=null)or(t<>empty) then @<Insert a dummy noad to be sub/superscripted@>;
scan_math(p);
end;
@y
procedure sub_sup;
var t:small_number; {type of previous sub/superscript}
@!p,q,r:pointer; {field to be filled by |scan_math|}
mark_kind:integer;
begin t:=empty; p:=null;
if tail<>head then begin
  if MML_mode then
    if type(tail)=sgml_node then begin
      q:=head;
      while link(q)<>tail do q:=link(q);
      r:=new_noad;
      info(nucleus(r)):=tail;
      math_type(nucleus(r)):=sub_mlist;
      link(q):=r;
      tail:=r;
      end;
  if scripts_allowed(tail) then
    begin p:=supscr(tail)+cur_cmd-sup_mark; {|supscr| or |subscr|}
    t:=math_type(p);
    end;
  end;
if (p=null)or(t<>empty) then @<Insert a dummy noad to be sub/superscripted@>;
scan_math(p);
end;
@z
%---------------------------------------
@x [48] m.1181 l.22199 - Omega
procedure math_fraction;
var c:small_number; {the type of generalized fraction we are scanning}
begin c:=cur_chr;
if incompleat_noad<>null then
  @<Ignore the fraction operation and complain about this ambiguous case@>
else  begin incompleat_noad:=get_node(fraction_noad_size);
  type(incompleat_noad):=fraction_noad;
  subtype(incompleat_noad):=normal;
  math_type(numerator(incompleat_noad)):=sub_mlist;
  info(numerator(incompleat_noad)):=link(head);
  mem[denominator(incompleat_noad)].hh:=empty_field;
  mem[left_delimiter(incompleat_noad)].qqqq:=null_delimiter;
  mem[right_delimiter(incompleat_noad)].qqqq:=null_delimiter;@/
  link(head):=null; tail:=head;
  @<Use code |c| to distinguish between generalized fractions@>;
  end;
end;
@y
procedure math_fraction;
var c:small_number; {the type of generalized fraction we are scanning}
begin c:=cur_chr;
if incompleat_noad<>null then
  @<Ignore the fraction operation and complain about this ambiguous case@>
else  begin incompleat_noad:=get_node(fraction_noad_size);
  type(incompleat_noad):=fraction_noad;
  subtype(incompleat_noad):=normal;
  math_type(numerator(incompleat_noad)):=sub_mlist;
  info(numerator(incompleat_noad)):=link(head);
  mem[denominator(incompleat_noad)].hh:=empty_field;
  if MML_mode then begin
    mem[left_delimiter(incompleat_noad)].int:=0;
    mem[right_delimiter(incompleat_noad)].int:=0;@/
    end
  else begin
    mem[left_delimiter(incompleat_noad)].qqqq:=null_delimiter;
    mem[right_delimiter(incompleat_noad)].qqqq:=null_delimiter;@/
    end;
  link(head):=null; tail:=head;
  @<Use code |c| to distinguish between generalized fractions@>;
  end;
end;
@z
%---------------------------------------
@x [48] m.1182 l.22199 - Omega
  begin scan_delimiter(left_delimiter(incompleat_noad),false);
  scan_delimiter(right_delimiter(incompleat_noad),false);
@y
  begin if MML_mode then begin
    mem[left_delimiter(incompleat_noad)].int:=new_noad;
    scan_math(nucleus(mem[left_delimiter(incompleat_noad)].int));
    mem[right_delimiter(incompleat_noad)].int:=new_noad;
    scan_math(nucleus(mem[right_delimiter(incompleat_noad)].int));
    end
  else begin
    scan_delimiter(left_delimiter(incompleat_noad),0);
    scan_delimiter(right_delimiter(incompleat_noad),0);
    end;
@z
%---------------------------------------
@x [48] m.1183 l.22212 - Omega
  begin scan_delimiter(garbage,false); scan_delimiter(garbage,false);
@y
  begin if MML_mode then begin
    scan_math(garbage); scan_math(garbage);
    end
  else begin
    scan_delimiter(garbage,0); scan_delimiter(garbage,0);
    end;
@z
%---------------------------------------
@x [48] m.1184 l.22230 - Omega
function fin_mlist(@!p:pointer):pointer;
var q:pointer; {the mlist to return}
begin if incompleat_noad<>null then @<Compleat the incompleat noad@>
else  begin link(tail):=p; q:=link(head);
  end;
pop_nest; fin_mlist:=q;
end;
@y
function fin_mlist(@!p:pointer):pointer;
var q,q1,q2:pointer; {the mlist to return}
    tag:integer;
begin if incompleat_noad<>null then @<Compleat the incompleat noad@>
else  begin link(tail):=p; q:=link(head);
  end;
if current_sgml_tag=0 then
  tag:="mrow"
else tag:=current_sgml_tag;
pop_nest;
if MML_mode then
  if q<>null then
   if link(q)<>null then begin
     q1:=new_sgml_node;
     sgml_tag(q1):=tag;
     sgml_info(q1):=q;
     sgml_kind(q1):=sgml_math_node;
     fin_mlist:=q1
     end
   else fin_mlist:=q
  else fin_mlist:=q
else
  fin_mlist:=q;
end;
@z
%---------------------------------------
@x [48] m.1185 l.22256 - Omega
@ @<Compleat...@>=
begin math_type(denominator(incompleat_noad)):=sub_mlist;
info(denominator(incompleat_noad)):=link(head);
if p=null then q:=incompleat_noad
else  begin q:=info(numerator(incompleat_noad));
  if type(q)<>left_noad then confusion("right");
@:this can't happen right}{\quad right@>
  info(numerator(incompleat_noad)):=link(q);
  link(q):=incompleat_noad; link(incompleat_noad):=p;
  end;
end
@y
@ @<Compleat...@>=
begin math_type(denominator(incompleat_noad)):=sub_mlist;
info(denominator(incompleat_noad)):=link(head);
if p=null then
  if not MML_mode then
    q:=incompleat_noad
  else begin
    if link(info(numerator(incompleat_noad)))<>null then begin
      q1:=new_sgml_node;
      sgml_tag(q1):="mrow";
      sgml_kind(q1):=sgml_math_node;
      sgml_info(q1):=info(numerator(incompleat_noad));
      end
    else q1:=info(numerator(incompleat_noad));
    if link(info(denominator(incompleat_noad)))<>null then begin
      q2:=new_sgml_node;
      sgml_tag(q2):="mrow";
      sgml_kind(q2):=sgml_math_node;
      sgml_info(q2):=info(denominator(incompleat_noad));
      end
    else q2:=info(denominator(incompleat_noad));
    q:=new_sgml_node;
    sgml_tag(q):="mfrac";
    sgml_kind(q):=sgml_math_node;
    sgml_info(q):=q1;
    link(q1):=q2;
    current_sgml_attrs:=null;
    if thickness(incompleat_noad)<>default_code then
      if thickness(incompleat_noad)=0 then
        sgml_attribute("linethickness","0ex")
      else if thickness(incompleat_noad)<default_rule_thickness then
        sgml_attribute("linethickness","thin")
      else if thickness(incompleat_noad)>default_rule_thickness then
        sgml_attribute("linethickness","thick");
    sgml_attrs(q):=current_sgml_attrs;
    if (mem[left_delimiter(incompleat_noad)].int<>0) or
       (mem[right_delimiter(incompleat_noad)].int<>0) then begin
      q1:=q;
      q:=new_sgml_node;
      sgml_tag(q):="mrow";
      sgml_kind(q):=sgml_math_node;
      sgml_info(q):=mem[left_delimiter(incompleat_noad)].int;
      link(mem[left_delimiter(incompleat_noad)].int):=q1;
      link(q1):=mem[right_delimiter(incompleat_noad)].int;
      end
    end
else  begin q:=info(numerator(incompleat_noad));
  if type(q)<>left_noad then confusion("right");
@:this can't happen right}{\quad right@>
  info(numerator(incompleat_noad)):=link(q);
  link(q):=incompleat_noad; link(incompleat_noad):=p;
  end;
end
@z
%---------------------------------------
@x [48] m.1186 l.22256 - Omega
math_group: begin unsave; decr(save_ptr);@/
  math_type(saved(0)):=sub_mlist; p:=fin_mlist(null); info(saved(0)):=p;
  if p<>null then if link(p)=null then
   if type(p)=ord_noad then
    begin if math_type(subscr(p))=empty then
     if math_type(supscr(p))=empty then
      begin mem[saved(0)].hh:=mem[nucleus(p)].hh;
      free_node(p,noad_size);
      end;
    end
  else if type(p)=accent_noad then if saved(0)=nucleus(tail) then
   if type(tail)=ord_noad then @<Replace the tail of the list by |p|@>;
  end;
@y
text_mml_group: begin
  unsave; decr(save_ptr);
  p:=saved(0);
  sgml_info(p):=link(head);
  pop_nest;
  tail_append(p);
  end;
text_sgml_group: begin
  print_err("Expecting closing tag </");
  print(current_sgml_tag); print(">.");
  print_ln;
  print_nl("The MathML translator cannot continue");
  succumb;
  end;
math_mml_group: begin
  unsave; decr(save_ptr);
  link(saved(0)):=link(head);
  p:=saved(0);
  pop_nest;
    if type(p)=sgml_node then
      if str_eq_str(sgml_tag(p),"mrow") then
        if sgml_attrs(p)=null then begin
          if link(sgml_info(p))=null then
          p:=sgml_info(p);
          end;
  tail_append(p);
  back_input;
  end;
math_group: begin unsave; decr(save_ptr);@/
  if MML_mode then 
    math_type(saved(0)):=sub_mlist
  else
    math_type(saved(0)):=sub_mlist;
  p:=fin_mlist(null); info(saved(0)):=p;
  if p<>null then if link(p)=null then
   if type(p)=ord_noad then
    begin if math_type(subscr(p))=empty then
     if math_type(supscr(p))=empty then
      begin mem[saved(0)].hh:=mem[nucleus(p)].hh;
      free_node(p,noad_size);
      end;
    end
  else if type(p)=accent_noad then if saved(0)=nucleus(tail) then
   if type(tail)=ord_noad then @<Replace the tail of the list by |p|@>;
  end;
font_entity_group: begin
  unsave;
  font_sort_number:=font_sort_ptr+1;
  for font_sorts:=font_sort_base+1 to font_sort_ptr do
    if str_eq_str(font_sort_name(font_sorts),saved(-1)) then begin
      font_sort_number:=font_sorts;
      break;
      end;
  font_sort_c:=saved(-2);
  font_sort_char_entity(font_sort_number)(font_sort_c):=saved(-3);
  font_sort_char_tag(font_sort_number)(font_sort_c):=saved(-4);
  font_sort_char_attr(font_sort_number)(font_sort_c):=current_sgml_attrs;
  current_sgml_attrs:=null;
  save_ptr:=save_ptr-4; pop_nest;
  end;
empty_tag_group: begin
  unsave; sgml_attrs(saved(-1)):=current_sgml_attrs;
  decr(save_ptr); pop_nest;
  end;
@z
%---------------------------------------
@x [48] m.1188 l.22284 - Omega
text(frozen_right):="right"; eqtb[frozen_right]:=eqtb[cur_val];
@y
settext(frozen_right,"right");
set_new_eqtb(frozen_right,new_eqtb(cur_val));
@z
%---------------------------------------
@x [48] m.1191 l.22301 - Omega
procedure math_left_right;
var t:small_number; {|left_noad| or |right_noad|}
@!p:pointer; {new noad}
begin t:=cur_chr;
if (t=right_noad)and(cur_group<>math_left_group) then
  @<Try to recover from mismatched \.{\\right}@>
else  begin p:=new_noad; type(p):=t;
  scan_delimiter(delimiter(p),false);
  if t=left_noad then
    begin push_math(math_left_group); link(head):=p; tail:=p;
    end
  else  begin p:=fin_mlist(p); unsave; {end of |math_left_group|}
    tail_append(new_noad); type(tail):=inner_noad;
    math_type(nucleus(tail)):=sub_mlist;
    info(nucleus(tail)):=p;
    end;
  end;
end;
@y
procedure math_left_right;
var t:small_number; {|left_noad| or |right_noad|}
@!p,q,r:pointer; {new noad}
begin t:=cur_chr;
if (t=right_noad)and(cur_group<>math_left_group) then
  @<Try to recover from mismatched \.{\\right}@>
else  begin p:=new_noad;
  if MML_mode then begin
    scan_math(nucleus(p));
    if t=left_noad then begin
      saved(0):=p; incr(save_ptr);
      push_math(math_left_group);
      end
    else begin
      unsave; decr(save_ptr);
      q:=saved(0); r:=fin_mlist(null);
      push_math(math_group);
      tail_append(q); tail_append(r);
      tail_append(p);
      unsave;
      p:=fin_mlist(null);
      tail_append(p);
      end
    end
  else begin
    type(p):=t;
    scan_delimiter(delimiter(p),false);
    if t=left_noad then
      begin push_math(math_left_group); link(head):=p; tail:=p;
      end
    else  begin p:=fin_mlist(p); unsave; {end of |math_left_group|}
      tail_append(new_noad); type(tail):=inner_noad;
      math_type(nucleus(tail)):=sub_mlist;
      info(nucleus(tail)):=p;
      end;
    end;
  end;
end;
@z
%---------------------------------------
@x [48] m.1192 l.22284 - Omega
begin if cur_group=math_shift_group then
  begin scan_delimiter(garbage,false);
  print_err("Extra "); print_esc("right");
@.Extra \\right.@>
  help1("I'm ignoring a \right that had no matching \left.");
  error;
  end
else off_save;
end
@y
begin if cur_group=math_shift_group then
  begin if MML_mode then scan_math(garbage)
  else scan_delimiter(garbage,0);
  print_err("Extra "); print_esc("right");
@.Extra \\right.@>
  help1("I'm ignoring a \right that had no matching \left.");
  error;
  end
else off_save;
end
@z
%---------------------------------------
@x [48] m.1193 l.22327 - Omega
mmode+math_shift: if cur_group=math_shift_group then after_math
  else off_save;
@y
mmode+math_shift: if cur_group=math_mml_group then after_mml
  else if cur_group=math_shift_group then after_math
  else off_save;
@z
%---------------------------------------
@x [48] m.1194 l.22284 - Omega
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
if mode=-m then {end of equation number}
  begin @<Check that another \.\$ follows@>;
  cur_mlist:=p; cur_style:=text_style; mlist_penalties:=false;
  mlist_to_hlist; a:=hpack(link(temp_head),natural);
  unsave; decr(save_ptr); {now |cur_group=math_shift_group|}
  if saved(0)=1 then l:=true;
  danger:=false;
  @<Check that the necessary fonts for math symbols are present;
    if not, flush the current math lists and set |danger:=true|@>;
  m:=mode; p:=fin_mlist(null);
  end
else a:=null;
if m<0 then @<Finish math in text@>
else  begin if a=null then @<Check that another \.\$ follows@>;
  @<Finish displayed math@>;
  end;
end;
@y
procedure after_mml;
var p:pointer;
begin
unsave; decr(save_ptr);
link(saved(0)):=link(head);
p:=saved(0);
pop_nest;
tail_append(p);
back_input;
end;

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
if MML_mode then decr(MML_level);
if mode=-m then {end of equation number}
  begin @<Check that another \.\$ follows@>;
  cur_mlist:=p; cur_style:=text_style; mlist_penalties:=false;
  if MML_mode then begin
    sgml_out_mlist(true,false,false)
    end
  else begin
    mlist_to_hlist; a:=hpack(link(temp_head),natural);
    end;
  unsave; decr(save_ptr); {now |cur_group=math_shift_group|}
  if saved(0)=1 then l:=true;
  danger:=false;
  @<Check that the necessary fonts for math symbols are present;
    if not, flush the current math lists and set |danger:=true|@>;
  m:=mode; p:=fin_mlist(null);
  end
else a:=null;
if m<0 then @<Finish math in text@>
else  begin if a=null then @<Check that another \.\$ follows@>;
  @<Finish displayed math@>;
  end;
end;
@z
%---------------------------------------
@x [49] m.1195 l.22361 - Omega
if (font_params[fam_fnt(2+text_size)]<total_mathsy_params)or@|
   (font_params[fam_fnt(2+script_size)]<total_mathsy_params)or@|
   (font_params[fam_fnt(2+script_script_size)]<total_mathsy_params) then
  begin print_err("Math formula deleted: Insufficient symbol fonts");@/
@.Math formula deleted...@>
  help3("Sorry, but I can't typeset math unless \textfont 2")@/
    ("and \scriptfont 2 and \scriptscriptfont 2 have all")@/
    ("the \fontdimen values needed in math symbol fonts.");
  error; flush_math; danger:=true;
  end
else if (font_params[fam_fnt(3+text_size)]<total_mathex_params)or@|
   (font_params[fam_fnt(3+script_size)]<total_mathex_params)or@|
   (font_params[fam_fnt(3+script_script_size)]<total_mathex_params) then
@y
if (font_params(fam_fnt(2+text_size))<total_mathsy_params)or@|
   (font_params(fam_fnt(2+script_size))<total_mathsy_params)or@|
   (font_params(fam_fnt(2+script_script_size))<total_mathsy_params) then
  begin print_err("Math formula deleted: Insufficient symbol fonts");@/
@.Math formula deleted...@>
  help3("Sorry, but I can't typeset math unless \textfont 2")@/
    ("and \scriptfont 2 and \scriptscriptfont 2 have all")@/
    ("the \fontdimen values needed in math symbol fonts.");
  error; flush_math; danger:=true;
  end
else if (font_params(fam_fnt(3+text_size))<total_mathex_params)or@|
   (font_params(fam_fnt(3+script_size))<total_mathex_params)or@|
   (font_params(fam_fnt(3+script_script_size))<total_mathex_params) then
@z
%---------------------------------------
@x [49] m.1196 l.22388 - Omega
@<Finish math in text@>=
begin tail_append(new_math(math_surround,before));
cur_mlist:=p; cur_style:=text_style; mlist_penalties:=(mode>0); mlist_to_hlist;
link(tail):=link(temp_head);
while link(tail)<>null do tail:=link(tail);
tail_append(new_math(math_surround,after));
space_factor:=1000; unsave;
end
@y
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
@z
%---------------------------------------
@x [49] m.1197 l.22430 - Omega
@<Finish displayed math@>=
cur_mlist:=p; cur_style:=display_style; mlist_penalties:=false;
mlist_to_hlist; p:=link(temp_head);@/
adjust_tail:=adjust_head; b:=hpack(p,natural); p:=list_ptr(b);
t:=adjust_tail; adjust_tail:=null;@/
w:=width(b); z:=display_width; s:=display_indent;
if (a=null)or danger then
  begin e:=0; q:=0;
  end
else  begin e:=width(a); q:=e+math_quad(text_size);
  end;
if w+q>z then
  @<Squeeze the equation as much as possible; if there is an equation
    number that should go on a separate line by itself,
    set~|e:=0|@>;
@<Determine the displacement, |d|, of the left edge of the equation, with
  respect to the line size |z|, assuming that |l=false|@>;
@<Append the glue or equation number preceding the display@>;
@<Append the display and perhaps also the equation number@>;
@<Append the glue or equation number following the display@>;
resume_after_display
@y
@<Finish displayed math@>=
cur_mlist:=p; cur_style:=display_style; mlist_penalties:=false;
if MML_mode then begin
{
  if MML_level=0 then sgml_out_mlist(true,false,false)
  else tail_append(cur_mlist);
}
  sgml_starttexttag("displaymath");
  sgml_starttexttag("math");
  tail_append(cur_mlist);
  sgml_endtexttag("math");
  sgml_endtexttag("displaymath");
  end
else begin
  mlist_to_hlist; p:=link(temp_head);@/
  adjust_tail:=adjust_head; b:=hpack(p,natural); p:=list_ptr(b);
  t:=adjust_tail; adjust_tail:=null;@/
  w:=width(b); z:=display_width; s:=display_indent;
  if (a=null)or danger then
    begin e:=0; q:=0;
    end
  else  begin e:=width(a); q:=e+math_quad(text_size);
    end;
  if w+q>z then
    @<Squeeze the equation as much as possible; if there is an equation
      number that should go on a separate line by itself,
      set~|e:=0|@>;
  @<Determine the displacement, |d|, of the left edge of the equation, with
    respect to the line size |z|, assuming that |l=false|@>;
  @<Append the glue or equation number preceding the display@>;
  @<Append the display and perhaps also the equation number@>;
  @<Append the glue or equation number following the display@>;
  end;
resume_after_display
@z
%---------------------------------------
@x [49] m.1214 l.22700 - Omega
@d word_define(#)==if global then geq_word_define(#)@+else eq_word_define(#)
@y
@d word_define(#)==if global then geq_word_define(#)@+else eq_word_define(#)
@d del_word_define(#)==if global
                       then del_geq_word_define(#)@+else del_eq_word_define(#)
@z
%---------------------------------------
@x [49] m.1216 l.22734 - Omega
text(frozen_protection):="inaccessible";
@y
settext(frozen_protection,"inaccessible");
@z
%---------------------------------------
@x [49] m.1222 l.22789 - Omega
@d char_def_code=0 {|shorthand_def| for \.{\\chardef}}
@d math_char_def_code=1 {|shorthand_def| for \.{\\mathchardef}}
@d count_def_code=2 {|shorthand_def| for \.{\\countdef}}
@d dimen_def_code=3 {|shorthand_def| for \.{\\dimendef}}
@d skip_def_code=4 {|shorthand_def| for \.{\\skipdef}}
@d mu_skip_def_code=5 {|shorthand_def| for \.{\\muskipdef}}
@d toks_def_code=6 {|shorthand_def| for \.{\\toksdef}}

@<Put each...@>=
primitive("chardef",shorthand_def,char_def_code);@/
@!@:char_def_}{\.{\\chardef} primitive@>
primitive("mathchardef",shorthand_def,math_char_def_code);@/
@!@:math_char_def_}{\.{\\mathchardef} primitive@>
@y
@d char_def_code=0 {|shorthand_def| for \.{\\chardef}}
@d math_char_def_code=1 {|shorthand_def| for \.{\\mathchardef}}
@d omath_char_def_code=2 {|shorthand_def| for \.{\\omathchardef}}
@d count_def_code=3 {|shorthand_def| for \.{\\countdef}}
@d dimen_def_code=4 {|shorthand_def| for \.{\\dimendef}}
@d skip_def_code=5 {|shorthand_def| for \.{\\skipdef}}
@d mu_skip_def_code=6 {|shorthand_def| for \.{\\muskipdef}}
@d toks_def_code=7 {|shorthand_def| for \.{\\toksdef}}

@<Put each...@>=
primitive("chardef",shorthand_def,char_def_code);@/
@!@:char_def_}{\.{\\chardef} primitive@>
primitive("mathchardef",shorthand_def,math_char_def_code);@/
@!@:math_char_def_}{\.{\\mathchardef} primitive@>
primitive("omathchardef",shorthand_def,omath_char_def_code);@/
@!@:math_char_def_}{\.{\\omathchardef} primitive@>
@z
%---------------------------------------
@x [49] m.1224 l.22833 - Omega
shorthand_def: begin n:=cur_chr; get_r_token; p:=cur_cs; define(p,relax,256);
  scan_optional_equals;
  case n of
  char_def_code: begin scan_char_num; define(p,char_given,cur_val);
    end;
  math_char_def_code: begin scan_fifteen_bit_int; define(p,math_given,cur_val);
    end;
@y
shorthand_def: begin n:=cur_chr; get_r_token; p:=cur_cs; 
  define(p,relax,too_big_char); scan_optional_equals;
  case n of
  char_def_code: begin scan_char_num; define(p,char_given,cur_val);
    end;
  math_char_def_code: begin scan_real_fifteen_bit_int;
    define(p,math_given,cur_val);
    end;
  omath_char_def_code: begin scan_big_fifteen_bit_int;
    define(p,omath_given,cur_val);
    end;
@z
%---------------------------------------
@x [49] m.1230 l. - Omega
primitive("mathcode",def_code,math_code_base);
@!@:math_code_}{\.{\\mathcode} primitive@>
primitive("lccode",def_code,lc_code_base);
@!@:lc_code_}{\.{\\lccode} primitive@>
primitive("uccode",def_code,uc_code_base);
@!@:uc_code_}{\.{\\uccode} primitive@>
primitive("sfcode",def_code,sf_code_base);
@!@:sf_code_}{\.{\\sfcode} primitive@>
primitive("delcode",def_code,del_code_base);
@!@:del_code_}{\.{\\delcode} primitive@>
@y
primitive("mathcode",def_code,math_code_base);
@!@:math_code_}{\.{\\mathcode} primitive@>
primitive("omathcode",def_code,math_code_base+256);
@!@:math_code_}{\.{\\omathcode} primitive@>
primitive("lccode",def_code,lc_code_base);
@!@:lc_code_}{\.{\\lccode} primitive@>
primitive("uccode",def_code,uc_code_base);
@!@:uc_code_}{\.{\\uccode} primitive@>
primitive("sfcode",def_code,sf_code_base);
@!@:sf_code_}{\.{\\sfcode} primitive@>
primitive("delcode",def_code,del_code_base);
@!@:del_code_}{\.{\\delcode} primitive@>
primitive("odelcode",def_code,del_code_base+256);
@!@:del_code_}{\.{\\odelcode} primitive@>
@z
%---------------------------------------
@x [49] m.1232 l.22990 - Omega
def_code: begin @<Let |n| be the largest legal code value, based on |cur_chr|@>;
  p:=cur_chr; scan_char_num; p:=p+cur_val; scan_optional_equals;
  scan_int;
  if ((cur_val<0)and(p<del_code_base))or(cur_val>n) then
    begin print_err("Invalid code ("); print_int(cur_val);
@.Invalid code@>
    if p<del_code_base then print("), should be in the range 0..")
    else print("), should be at most ");
    print_int(n);
    help1("I'm going to use 0 instead of that illegal code value.");@/
    error; cur_val:=0;
    end;
  if p<math_code_base then define(p,data,cur_val)
  else if p<del_code_base then define(p,data,hi(cur_val))
  else word_define(p,cur_val);
@y
def_code: begin
 if cur_chr=(del_code_base+256) then begin
   p:=cur_chr-256; scan_char_num; p:=p+cur_val; scan_optional_equals;
   scan_int; cur_val1:=cur_val; scan_int; {backwards}
   if (cur_val1>@"FFFFFF) or (cur_val>@"FFFFFF) then
     begin print_err("Invalid code ("); print_int(cur_val1); print(" ");
     print_int(cur_val);
     print("), should be at most ""FFFFFF ""FFFFFF");
     help1("I'm going to use 0 instead of that illegal code value.");@/
     error; cur_val1:=0; cur_val:=0;
     end;
   del_word_define(p,cur_val1,cur_val);
   end
 else begin
  @<Let |n| be the largest legal code value, based on |cur_chr|@>;
  p:=cur_chr; scan_char_num; p:=p+cur_val; scan_optional_equals;
  scan_int;
  if ((cur_val<0)and(p<del_code_base))or(cur_val>n) then
    begin print_err("Invalid code ("); print_int(cur_val);
@.Invalid code@>
    if p<del_code_base then print("), should be in the range 0..")
    else print("), should be at most ");
    print_int(n);
    help1("I'm going to use 0 instead of that illegal code value.");@/
    error; cur_val:=0;
    end;
  if p<math_code_base then define(p,data,cur_val)
  else if p<(math_code_base+256) then begin
    if cur_val=@"8000 then cur_val:=@"8000000
    else cur_val:=((cur_val div @"1000) * @"1000000) +
                  (((cur_val mod @"1000) div @"100) * @"10000) +
                  (cur_val mod @"100);
    define(p,data,hi(cur_val));
    end
  else if p<del_code_base then define(p-256,data,hi(cur_val))
  else begin
   cur_val1:=cur_val div @"1000;
   cur_val1:=(cur_val1 div @"100)*@"10000 + (cur_val1 mod @"100);
   cur_val:=cur_val mod @"1000;
   cur_val:=(cur_val div @"100)*@"10000 + (cur_val mod @"100);
   del_word_define(p,cur_val1,cur_val);
   end;
  end;
@z
%---------------------------------------
@x [49] m.1233 l.22990 - Omega
if cur_chr=cat_code_base then n:=max_char_code
else if cur_chr=math_code_base then n:=@'100000
else if cur_chr=sf_code_base then n:=@'77777
else if cur_chr=del_code_base then n:=@'77777777
else n:=255
@y
if cur_chr=cat_code_base then n:=max_char_code
else if cur_chr=math_code_base then n:=@'100000
else if cur_chr=(math_code_base+256) then n:=@"8000000
else if cur_chr=sf_code_base then n:=@'77777
else if cur_chr=del_code_base then n:=@'77777777
else n:=biggest_char
@z
%---------------------------------------
@x [49] m.1234 l.22990 - Omega
def_family: begin p:=cur_chr; scan_four_bit_int; p:=p+cur_val;
@y
def_family: begin p:=cur_chr; scan_big_four_bit_int; p:=p+cur_val;
@z
%---------------------------------------
@x [49] m.1238 l.23059 - Omega
  if q=advance then cur_val:=cur_val+eqtb[l].int;
@y
  if q=advance then cur_val:=cur_val+new_eqtb_int(l);
@z
%---------------------------------------
@x [49] m.1240 l.23086 - Omega
    if p=int_val then cur_val:=mult_integers(eqtb[l].int,cur_val)
    else cur_val:=nx_plus_y(eqtb[l].int,cur_val,0)
  else cur_val:=x_over_n(eqtb[l].int,cur_val)
@y
    if p=int_val then cur_val:=mult_integers(new_eqtb_int(l),cur_val)
    else cur_val:=nx_plus_y(new_eqtb_int(l),cur_val,0)
  else cur_val:=x_over_n(new_eqtb_int(l),cur_val)
@z
%---------------------------------------
@x [49] m.1241 l.23109 - Omega
  if global then n:=256+cur_val@+else n:=cur_val;
@y
  if global then n:=number_regs+cur_val@+else n:=cur_val;
@z
%---------------------------------------
@x [49] m.1252 l.23232 - Omega
    print_err("Patterns can be loaded only by INITEX");
@y
    print_err("Patterns can be loaded only by INIOMEGA");
@z
%---------------------------------------
@x [49] m.1253 l.23250 - Omega
assign_font_dimen: begin find_font_dimen(true); k:=cur_val;
  scan_optional_equals; scan_normal_dimen; font_info[k].sc:=cur_val;
  end;
assign_font_int: begin n:=cur_chr; scan_font_ident; f:=cur_val;
  scan_optional_equals; scan_int;
  if n=0 then hyphen_char[f]:=cur_val@+else skew_char[f]:=cur_val;
@y
assign_font_dimen: begin find_font_dimen(true); k:=cur_val;
  scan_optional_equals; scan_normal_dimen;
  font_info(dimen_font)(k).sc:=cur_val;
  end;
assign_font_int: begin n:=cur_chr; scan_font_ident; f:=cur_val;
  scan_optional_equals; scan_int;
  if n=0 then hyphen_char(f):=cur_val@+else skew_char(f):=cur_val;
@z
%---------------------------------------
@x [49] m.1257 l.23269 - Omega
@!flushable_string:str_number; {string not yet referenced}
begin if job_name=0 then open_log_file;
  {avoid confusing \.{texput} with the font name}
@.texput@>
get_r_token; u:=cur_cs;
if u>=hash_base then t:=text(u)
@y
@!flushable_string:str_number; {string not yet referenced}
@!offset:quarterword; {how much we will offset the characters}
@!cur_font_sort_name:str_number; {the name without the digits at the end}
@!new_length:integer; {length of font name, to become font sort name}
@!last_character:integer; {last character in font name}
@!i:integer; {to run through characters of name}
@!found_font_sort:boolean; {we have already seen this font sort}
@!this_is_a_new_font:boolean;
begin if job_name=0 then open_log_file;
  {avoid confusing \.{texput} with the font name}
@.texput@>
get_r_token; u:=cur_cs;
this_is_a_new_font:=false;
if u>=hash_base then t:=newtext(u)
@z
%---------------------------------------
@x [49] m.1257 l.23290 - Omega
@<Scan the font size specification@>;
@<If this font has already been loaded, set |f| to the internal
  font number and |goto common_ending|@>;
f:=read_font_info(u,cur_name,cur_area,s);
common_ending: equiv(u):=f; eqtb[font_id_base+f]:=eqtb[u]; font_id_text(f):=t;
@y
@<Scan the font size specification@>;
name_in_progress:=true;
if scan_keyword("offset") then begin
  scan_int;
  offset:=cur_val;
  if (cur_val<0) then begin
    print_err("Illegal offset has been changed to 0");
    help1("The offset must be bigger than 0."); int_error(cur_val);
    offset:=0;
    end
  end
else offset:=0;
name_in_progress:=false;
@<If this font has already been loaded, set |f| to the internal
  font number and |goto common_ending|@>;
f:=read_font_info(u,cur_name,cur_area,s,offset);
this_is_a_new_font:=true;
common_ending: set_equiv(u,f);
set_new_eqtb(font_id_base+f,new_eqtb(u)); settext(font_id_base+f,t);
if this_is_a_new_font then 
  begin
  if cur_name>=@"10000 then begin
    new_length:=length(cur_name);
    last_character:=str_pool[str_start(cur_name)+new_length-1];
    while (last_character>="0") and (last_character<="9") do begin
      decr(new_length);
      last_character:=str_pool[str_start(cur_name)+new_length-1];
      end;
    for i:=1 to new_length do begin
      append_char(str_pool[str_start(cur_name)+i-1]);
      end;
    cur_font_sort_name:=make_string;
    print("Loaded font "); print(cur_name);
    font_sort_number:=font_sort_ptr+1;
    for font_sorts:=font_sort_base+1 to font_sort_ptr do
      if str_eq_str(font_sort_name(font_sorts),cur_font_sort_name) then begin
        font_sort_number:=font_sorts;
        break;
        end;
    font_name_sort(f):=font_sort_number;
    if font_sort_number=(font_sort_ptr+1) then begin
      incr(font_sort_ptr);
      allocate_font_sort_table
        (font_sort_ptr,
         font_sort_offset_char_base+3*(font_ec(f)-font_bc(f)+1));
      font_sort_file_size(font_sort_ptr):=
         font_sort_offset_char_base+3*(font_ec(f)-font_bc(f)+1);
      font_sort_name(font_sort_ptr):=cur_font_sort_name;
      font_sort_ec(font_sort_ptr):=font_ec(f);
      font_sort_bc(font_sort_ptr):=font_bc(f);
      cur_name:=cur_font_sort_name;
      cur_ext:=".onm";
      pack_cur_name;
      begin_file_reading;
      if a_open_in(cur_file,kpse_tex_format) then begin
        print(" ; Loading font sort ");
        print(cur_font_sort_name); print(".onm");
        name:=a_make_name_string(cur_file);
        @<Read the first line of the new file@>;
        end
      else end_file_reading;
      end;
    print_ln;
    end;
  end;
@z
%---------------------------------------
@x [49] m.1260 l.23333 - Omega
for f:=font_base+1 to font_ptr do
  if str_eq_str(font_name[f],cur_name)and str_eq_str(font_area[f],cur_area) then
    begin if cur_name=flushable_string then
      begin flush_string; cur_name:=font_name[f];
      end;
    if s>0 then
      begin if s=font_size[f] then goto common_ending;
      end
    else if font_size[f]=xn_over_d(font_dsize[f],-s,1000) then
      goto common_ending;
    end
@y
for f:=font_base+1 to font_ptr do begin
  if str_eq_str(font_name(f),cur_name) and
     str_eq_str(font_area(f),cur_area) then
    begin if cur_name=flushable_string then
      begin flush_string; cur_name:=font_name(f);
      end;
    if s>0 then
      begin if s=font_size(f) then goto common_ending;
      end
    else if font_size(f)=xn_over_d(font_dsize(f),-s,1000) then
      goto common_ending;
    end
  end
@z
%---------------------------------------
@x [49] m.1261 l.23345 - Omega
set_font:begin print("select font "); slow_print(font_name[chr_code]);
  if font_size[chr_code]<>font_dsize[chr_code] then
    begin print(" at "); print_scaled(font_size[chr_code]);
@y
set_font:begin print("select font "); slow_print(font_name(chr_code));
  if font_size(chr_code)<>font_dsize(chr_code) then
    begin print(" at "); print_scaled(font_size(chr_code));
@z
%---------------------------------------
@x [49] m.1289 l.23546 - Omega
|cs_token_flag+active_base| is a multiple of~256.
@y
|cs_token_flag+active_base| is a multiple of~|max_char_val|.
@z
%---------------------------------------
@x [49] m.1289 l.23552 - Omega
  begin c:=t mod 256;
@y
  begin c:=t mod max_char_val;
@z
%---------------------------------------
@x [50] m.1301 l.23682 - Omega
format_ident:=" (INITEX)";
@y
format_ident:=" (INIOMEGA)";
@z
%---------------------------------------
% We do not store any information for strings between 257 and 65535.

@x [50] m.1309 l.23814 - Omega
for k:=0 to str_ptr do dump_int(str_start[k]);
@y
for k:=too_big_char to str_ptr do dump_int(str_start(k));
@z
%---------------------------------------
% We recreate the information for strings between 257 and 65535.

@x [50] m.1310 l.23831 - Omega
for k:=0 to str_ptr do undump(0)(pool_ptr)(str_start[k]);
@y
for k:=too_big_char to str_ptr do undump(0)(pool_ptr)(str_start(k));
@z
%---------------------------------------
@x [50] m.1313 l.23892 - Omega
@<Dump regions 1 to 4 of |eqtb|@>;
@<Dump regions 5 and 6 of |eqtb|@>;
@y
dump_eqtb_table;
@z
%---------------------------------------
@x [50] m.1314 l.23904 - Omega
@<Undump regions 1 to 6 of |eqtb|@>;
@y
undump_eqtb_table;
@z
%---------------------------------------
@x [50] m.1315 l.23904 - Omega
@ The table of equivalents usually contains repeated information, so we dump it
in compressed form: The sequence of $n+2$ values $(n,x_1,\ldots,x_n,m)$ in the
format file represents $n+m$ consecutive entries of |eqtb|, with |m| extra
copies of $x_n$, namely $(x_1,\ldots,x_n,x_n,\ldots,x_n)$.
 
@<Dump regions 1 to 4 of |eqtb|@>=
k:=active_base;
repeat j:=k;
while j<int_base-1 do
  begin if (equiv(j)=equiv(j+1))and(eq_type(j)=eq_type(j+1))and@|
    (eq_level(j)=eq_level(j+1)) then goto found1;
  incr(j);
  end;
l:=int_base; goto done1; {|j=int_base-1|}
found1: incr(j); l:=j;
while j<int_base-1 do
  begin if (equiv(j)<>equiv(j+1))or(eq_type(j)<>eq_type(j+1))or@|
    (eq_level(j)<>eq_level(j+1)) then goto done1;
  incr(j);
  end;
done1:dump_int(l-k);
while k<l do
  begin dump_wd(eqtb[k]); incr(k);
  end;
k:=j+1; dump_int(k-l);
until k=int_base

@ @<Dump regions 5 and 6 of |eqtb|@>=
repeat j:=k;
while j<eqtb_size do
  begin if eqtb[j].int=eqtb[j+1].int then goto found2;
  incr(j);
  end;
l:=eqtb_size+1; goto done2; {|j=eqtb_size|}
found2: incr(j); l:=j;
while j<eqtb_size do
  begin if eqtb[j].int<>eqtb[j+1].int then goto done2;
  incr(j);
  end;
done2:dump_int(l-k);
while k<l do
  begin dump_wd(eqtb[k]); incr(k);
  end;
k:=j+1; dump_int(k-l);
until k>eqtb_size
 
@ @<Undump regions 1 to 6 of |eqtb|@>=
k:=active_base;
repeat undump_int(x);
if (x<1)or(k+x>eqtb_size+1) then goto bad_fmt;
for j:=k to k+x-1 do undump_wd(eqtb[j]);
k:=k+x;
undump_int(x);
if (x<0)or(k+x>eqtb_size+1) then goto bad_fmt;
for j:=k to k+x-1 do eqtb[j]:=eqtb[k-1];
k:=k+x;
until k>eqtb_size
@y

@z
%---------------------------------------
@x [50] m.1318 l.23967 - Omega
@<Dump the hash table@>=
dump_int(hash_used); cs_count:=frozen_control_sequence-1-hash_used;
for p:=hash_base to hash_used do if text(p)<>0 then
  begin dump_int(p); dump_hh(hash[p]); incr(cs_count);
  end;
for p:=hash_used+1 to undefined_control_sequence-1 do dump_hh(hash[p]);
dump_int(cs_count);@/
print_ln; print_int(cs_count); print(" multiletter control sequences")
@y
@<Dump the hash table@>=
dump_int(hash_used);
@z
%---------------------------------------
@x [50] m.1319 l.23976 - Omega
undump(hash_base)(frozen_control_sequence)(hash_used); p:=hash_base-1;
repeat undump(p+1)(hash_used)(p); undump_hh(hash[p]);
until p=hash_used;
for p:=hash_used+1 to undefined_control_sequence-1 do undump_hh(hash[p]);
undump_int(cs_count)
@y
undump(hash_base)(frozen_control_sequence)(hash_used);
@z
%---------------------------------------
@x [50] m.1320 l.23983 - Omega
@ @<Dump the font information@>=
dump_int(fmem_ptr);
for k:=0 to fmem_ptr-1 do dump_wd(font_info[k]);
dump_int(font_ptr);
for k:=null_font to font_ptr do
  @<Dump the array info for internal font number |k|@>;
print_ln; print_int(fmem_ptr-7); print(" words of font info for ");
print_int(font_ptr-font_base); print(" preloaded font");
if font_ptr<>font_base+1 then print_char("s")
@y
@ @<Dump the font information@>=
dump_int(font_sort_ptr);
for k:=null_font_sort to font_sort_ptr do
  dump_font_sort_table(k,font_sort_file_size(k));
dump_int(font_ptr);
for k:=null_font to font_ptr do
  @<Dump the array info for internal font number |k|@>;
print_ln; print_int(font_ptr-font_base); print(" preloaded font");
if font_ptr<>font_base+1 then print_char("s")
@z
%---------------------------------------
@x [50] m.1321 l.23993 - Omega
@ @<Undump the font information@>=
undump_size(7)(font_mem_size)('font mem size')(fmem_ptr);
for k:=0 to fmem_ptr-1 do undump_wd(font_info[k]);
undump_size(font_base)(font_max)('font max')(font_ptr);
for k:=null_font to font_ptr do
  @<Undump the array info for internal font number |k|@>
@y
@ @<Undump the font information@>=
undump_size(font_base)(font_max)('font sort max')(font_sort_ptr);
for k:=null_font_sort to font_sort_ptr do
  undump_font_sort_table(k);
undump_size(font_base)(font_max)('font max')(font_ptr);
for k:=null_font to font_ptr do
  @<Undump the array info for internal font number |k|@>
@z
%---------------------------------------
@x [50] m.1322 l.24000 - Omega
@ @<Dump the array info for internal font number |k|@>=
begin dump_qqqq(font_check[k]);
dump_int(font_size[k]);   
dump_int(font_dsize[k]);
dump_int(font_params[k]);@/
dump_int(hyphen_char[k]); 
dump_int(skew_char[k]);@/
dump_int(font_name[k]); 
dump_int(font_area[k]);@/
dump_int(font_bc[k]);   
dump_int(font_ec[k]);@/
dump_int(char_base[k]);
dump_int(width_base[k]);
dump_int(height_base[k]);@/
dump_int(depth_base[k]);  
dump_int(italic_base[k]);
dump_int(lig_kern_base[k]);@/
dump_int(kern_base[k]);
dump_int(exten_base[k]);
dump_int(param_base[k]);@/
dump_int(font_glue[k]);@/
dump_int(bchar_label[k]);
dump_int(font_bchar[k]);
dump_int(font_false_bchar[k]);@/
print_nl("\font"); print_esc(font_id_text(k)); print_char("=");
print_file_name(font_name[k],font_area[k],"");
if font_size[k]<>font_dsize[k] then
  begin print(" at "); print_scaled(font_size[k]); print("pt");
  end;
end
@y
@ @<Dump the array info for internal font number |k|@>=
begin dump_font_table(k,param_base(k)+font_params(k)+1);
print_nl("\font"); print_esc(font_id_text(k)); print_char("=");
print_file_name(font_name(k),font_area(k),"");
if font_size(k)<>font_dsize(k) then
  begin print(" at "); print_scaled(font_size(k)); print("pt");
  end;
end
@z
%---------------------------------------
@x [50] m.1323 l.24031 - Omega
@ @<Undump the array info for internal font number |k|@>=
begin undump_qqqq(font_check[k]);@/
undump_int(font_size[k]);
undump_int(font_dsize[k]);
undump(min_halfword)(max_halfword)(font_params[k]);@/
undump_int(hyphen_char[k]);
undump_int(skew_char[k]);@/
undump(0)(str_ptr)(font_name[k]);
undump(0)(str_ptr)(font_area[k]);@/
undump(0)(255)(font_bc[k]);
undump(0)(255)(font_ec[k]);@/
undump_int(char_base[k]);
undump_int(width_base[k]);
undump_int(height_base[k]);@/
undump_int(depth_base[k]);
undump_int(italic_base[k]);
undump_int(lig_kern_base[k]);@/
undump_int(kern_base[k]);
undump_int(exten_base[k]);
undump_int(param_base[k]);@/
undump(min_halfword)(lo_mem_max)(font_glue[k]);@/
undump(0)(fmem_ptr-1)(bchar_label[k]);
undump(min_quarterword)(non_char)(font_bchar[k]);
undump(min_quarterword)(non_char)(font_false_bchar[k]);
end
@y
@ @<Undump the array info for internal font number |k|@>=
begin undump_font_table(k);@/
end
@z
%---------------------------------------
@x [50] m.1324 l.24066 - Omega
for k:=0 to trie_max do dump_hh(trie[k]);
@y
for k:=0 to trie_max do dump_hh(trie[k]);
dump_int(max_hyph_char);
@z
%---------------------------------------
@x [50] m.1324 l.24078 - Omega
for k:=255 downto 0 do if trie_used[k]>min_quarterword then
@y
for k:=biggest_lang downto 0 do if trie_used[k]>min_quarterword then
@z
%---------------------------------------
@x [50] m.1325 l.24094 - Omega
for k:=0 to j do undump_hh(trie[k]);
@y
for k:=0 to j do undump_hh(trie[k]);
undump_int(max_hyph_char);
@z
%---------------------------------------
@x [50] m.1325 l.24101 - Omega
init for k:=0 to 255 do trie_used[k]:=min_quarterword;@+tini@;@/
k:=256;
@y
init for k:=0 to biggest_lang do trie_used[k]:=min_quarterword;@+tini@;@/
k:=biggest_lang+1;
@z
%---------------------------------------
@x [50] m.1326 l.24100 - Omega
dump_int(interaction); dump_int(format_ident); dump_int(69069);
tracing_stats:=0
@y
dump_int(interaction); dump_int(format_ident); dump_int(69069);
set_new_eqtb_int(int_base+tracing_stats_code,0)
@z
%---------------------------------------
@x [50] m.1328 l.24125 - Omega - Year 2000
print_int(year mod 100); print_char(".");
@y
print_int(year); print_char(".");
@z
%---------------------------------------
@x [51] m.1333 l.24244 - Omega
procedure close_files_and_terminate;
var k:integer; {all-purpose index}
begin @<Finish the extensions@>;
@!stat if tracing_stats>0 then @<Output statistics about this job@>;@;@+tats@/
wake_up_terminal; @<Finish the \.{DVI} file@>;
@y
procedure close_files_and_terminate;
var k:integer; {all-purpose index}
begin @<Finish the extensions@>;
@!stat if tracing_stats>0 then @<Output statistics about this job@>;@;@+tats@/
wake_up_terminal;
if not MML_mode then begin @<Finish the \.{DVI} file@>; end;
@z
%---------------------------------------
@x [51] m.1334 l.24266 - Omega
  wlog_ln('Here is how much of TeX''s memory',' you used:');
@y
  wlog_ln('Here is how much of Omega''s memory',' you used:');
@z
%---------------------------------------
@x [51] m.1334 l.24277 - Omega
  wlog(' ',fmem_ptr:1,' words of font info for ',
    font_ptr-font_base:1,' font');
  if font_ptr<>font_base+1 then wlog('s');
@y
  wlog(font_ptr-font_base:1,' font');
  if font_ptr<>font_base+1 then wlog('s');
@z
%---------------------------------------
@x [51] m.1335 l.24338 - Omega
  print_nl("(\dump is performed only by INITEX)"); return;
@:dump_}{\.{\\dump...only by INITEX}@>
@y
  print_nl("(\dump is performed only by INIOMEGA)"); return;
@:dump_}{\.{\\dump...only by INIOMEGA}@>
@z
%---------------------------------------
@x [52] m.1339 l.24429 - Omega
4: print_word(eqtb[n]);
5: print_word(font_info[n]);
@y
4: print_word(new_eqtb(n));
5:  ;
@z
%---------------------------------------
@x [53] m.1341 l.24506 - Omega
@d what_lang(#)==link(#+1) {language number, in the range |0..255|}
@y
@d what_lang(#)==link(#+1) {language number, in the range |0..biggest_lang|}
@z
%---------------------------------------
@x [53] m.1368 l.24747 - Omega
for k:=str_start[str_ptr] to pool_ptr-1 do dvi_out(so(str_pool[k]));
pool_ptr:=str_start[str_ptr]; {erase the string}
@y
for k:=str_start(str_ptr) to pool_ptr-1 do dvi_out(so(str_pool[k]));
pool_ptr:=str_start(str_ptr); {erase the string}
@z
%---------------------------------------
@x [53] m.1369 l.24761 - Omega
text(end_write):="endwrite"; eq_level(end_write):=level_one;
eq_type(end_write):=outer_call; equiv(end_write):=null;
@y
settext(end_write,"endwrite"); set_eq_level(end_write,level_one);
set_eq_type(end_write,outer_call); set_equiv(end_write,null);
@z
%---------------------------------------
@x [53] m.1376 l.24878 - Omega
else if language>255 then l:=0
@y
else if language>biggest_lang then l:=0
@z
%---------------------------------------
@x [53] m.1377 l.24893 - Omega
  else if cur_val>255 then clang:=0
@y
  else if cur_val>biggest_lang then clang:=0
@z
%---------------------------------------
@x [54] 
This section should be replaced, if necessary, by any special
modifications of the program
that are necessary to make \TeX\ work at a particular installation.
It is usually best to design your change file so that all changes to
previous sections preserve the section numbering; then everybody's version
will be consistent with the published program. More extensive changes,
which introduce new sections, can be inserted here; then only the index
itself will get a new section number.
@^system dependencies@>

@y
@* \[54] $\Omega$ changes.

@z
%---------------------------------------
