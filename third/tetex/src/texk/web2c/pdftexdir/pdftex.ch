% WEB change file containing code for pdfTeX feature extending TeX;
% to be applied to tex.web (Version 3.14159) in order to define the
% pdfTeX program.
%
% (WEB2C!) indicates parts that may need adjusting in tex.pch
% (ETEX!) indicates parts that may need adjusting in pdfetex.ch[12]
%
% Copyright (C) 1996-98 H\`an Th\^e\llap{\raise 0.5ex\hbox{\'{}}} Th\`anh,
% Petr Sojka, and Ji\v{r}\'i Zlatu\v{s}ka
%
% The TeX program is copyright (C) 1982 by D. E. Knuth.
% TeX is a trademark of the American Mathematical Society.

@x limbo
\def\PASCAL{Pascal}
@y
\def\PASCAL{Pascal}
\def\pdfTeX{pdf\TeX}
\def\PDF{PDF}
@z

@x [1.2] - This change is made for TeX 3.14159
@d banner=='This is TeX, Version 3.14159' {printed when \TeX\ starts}
@y
@d banner=='This is pdfTeX, Version 3.14159','-',pdftex_version_string 
{printed when \pdfTeX\ starts}
@d pdftex_version==13 { \.{\\pdftexversion} }
@d pdftex_revision=="d" { \.{\\pdftexrevision} }
@d pdftex_version_string=='13d' {current pdf\TeX\ version}
@z

% Some procedures that need to be declared forward
@x [12.173]
@* \[12] Displaying boxes.
@y
@<Declare procedures that need to be declared forward for pdftex@>
@* \[12] Displaying boxes.
@z

% Define pdftex tokens parameters (ETEX!)
@x [17.230] 
@d err_help_loc=local_base+9 {points to token list for \.{\\errhelp}}
@d toks_base=local_base+10 {table of 256 token list registers}
@y
@d pdf_font_prefix_loc=local_base+9 {points to token list for \.{\\pdffontprefix}}
@d pdf_form_prefix_loc=local_base+10 {points to token list for \.{\\pdfformprefix}}
@d pdf_image_prefix_loc=local_base+11 {points to token list for \.{\\pdfimageprefix}}
@d pdf_page_attr_loc=local_base+12 {points to token list for \.{\\pdfpageattr}}
@d pdf_pages_attr_loc=local_base+13 {points to token list for \.{\\pdfpagesattr}}
@d pdf_form_attr_loc=local_base+14 {points to token list for \.{\\pdfformattr}}
@d pdf_image_attr_loc=local_base+15 {points to token list for \.{\\pdfimageattr}}
@d err_help_loc=local_base+16 {points to token list for \.{\\errhelp}}
@d toks_base=local_base+17 {table of 256 token list registers}
@z

@x [17.230] - HZ
@d sf_code_base=uc_code_base+256 {table of 256 spacefactor mappings}
@d math_code_base=sf_code_base+256 {table of 256 math mode mappings}
@y
@d sf_code_base=uc_code_base+256 {table of 256 spacefactor mappings}
@d ef_code_base=sf_code_base+256 {table of 256 font-expand factor mappings}
@d math_code_base=ef_code_base+256 {table of 256 math mode mappings}
@z

@x [17.230]
@d err_help==equiv(err_help_loc)
@y
@d pdf_font_prefix==equiv(pdf_font_prefix_loc)
@d pdf_form_prefix==equiv(pdf_form_prefix_loc)
@d pdf_image_prefix==equiv(pdf_image_prefix_loc)
@d pdf_page_attr==equiv(pdf_page_attr_loc)
@d pdf_pages_attr==equiv(pdf_pages_attr_loc)
@d pdf_form_attr==equiv(pdf_form_attr_loc)
@d pdf_image_attr==equiv(pdf_image_attr_loc)
@d err_help==equiv(err_help_loc)
@z

@x [17.230] - HZ
@d sf_code(#)==equiv(sf_code_base+#)
@y
@d sf_code(#)==equiv(sf_code_base+#)
@d ef_code(#)==equiv(ef_code_base+#)
@z

@x [17.230]
primitive("errhelp",assign_toks,err_help_loc);
@!@:err_help_}{\.{\\errhelp} primitive@>
@y
primitive("pdffontprefix",assign_toks,pdf_font_prefix_loc);
@!@:pdf_font_prefix_}{\.{\\pdffontprefix} primitive@>
primitive("pdfformprefix",assign_toks,pdf_form_prefix_loc);
@!@:pdf_form_prefix_}{\.{\\pdfformprefix} primitive@>
primitive("pdfimageprefix",assign_toks,pdf_image_prefix_loc);
@!@:pdf_image_prefix_}{\.{\\pdfimageprefix} primitive@>
primitive("pdfpageattr",assign_toks,pdf_page_attr_loc);
@!@:pdf_page_attr_}{\.{\\pdfpageattr} primitive@>
primitive("pdfpagesattr",assign_toks,pdf_pages_attr_loc);
@!@:pdf_pages_attr_}{\.{\\pdfpagesattr} primitive@>
primitive("pdfformattr",assign_toks,pdf_form_attr_loc);
@!@:pdf_form_attr_}{\.{\\pdfformattr} primitive@>
primitive("pdfimageattr",assign_toks,pdf_image_attr_loc);
@!@:pdf_image_attr_}{\.{\\pdfimageattr} primitive@>
primitive("errhelp",assign_toks,err_help_loc);
@!@:err_help_}{\.{\\errhelp} primitive@>
@z

@x [17.231]
  othercases print_esc("errhelp")
@y
  pdf_font_prefix_loc: print_esc("pdffontprefix");
  pdf_form_prefix_loc: print_esc("pdfformprefix");
  pdf_image_prefix_loc: print_esc("pdfimageprefix");
  pdf_page_attr_loc: print_esc("pdfpageattr");
  pdf_pages_attr_loc: print_esc("pdfpagesattr");
  pdf_form_attr_loc: print_esc("pdfformattr");
  pdf_image_attr_loc: print_esc("pdfimageattr");
  othercases print_esc("errhelp")
@z

@x [17.233] - HZ
  begin cat_code(k):=other_char; math_code(k):=hi(k); sf_code(k):=1000;
  end;
@y
  begin cat_code(k):=other_char; math_code(k):=hi(k); sf_code(k):=1000;
        ef_code(k):=1000;
  end;
@z

@x [17.2??] - HZ
  else  begin print_esc("sfcode"); print_int(n-sf_code_base);
    end;
@y
  else if n<ef_code_base then
    begin print_esc("sfcode"); print_int(n-sf_code_base)
    end
  else  begin print_esc("efcode"); print_int(n-ef_code_base);
    end;
@z

% Define pdftex integer parameters -- (WEB2C!)
@x [17.236]
@d error_context_lines_code=54 {maximum intermediate line pairs shown}
@y
@d error_context_lines_code=54 {maximum intermediate line pairs shown}
@d pdf_output_code=55 {switch on \PDF{} output if positive}
@d pdf_image_resolution_code=56 {image resolution of text contents}
@d pdf_compress_level_code=57 {compress level of text contents}
@d pdf_last_form_code=58 {object number of the last form}
@d pdf_last_annot_code=59 {object number of the last annotation}
@d pdf_last_obj_code=60 {object number of the last object}
@d pdf_adjust_spacing_code=61 {switch on spacing adjusting}
@#
{|int_pars| is redefined in \.{tex.pch}}
@z

@x [17.236] 
@d error_context_lines==int_par(error_context_lines_code)
@y
@d error_context_lines==int_par(error_context_lines_code)
@d pdf_output==int_par(pdf_output_code)
@d pdf_image_resolution==int_par(pdf_image_resolution_code)
@d pdf_compress_level==int_par(pdf_compress_level_code)
@d pdf_last_form==int_par(pdf_last_form_code)
@d pdf_last_annot==int_par(pdf_last_annot_code)
@d pdf_last_obj==int_par(pdf_last_obj_code)
@d pdf_adjust_spacing==int_par(pdf_adjust_spacing_code)
@z

@x [17.237]
error_context_lines_code:print_esc("errorcontextlines");
@y
error_context_lines_code:print_esc("errorcontextlines");
pdf_output_code:print_esc("pdfoutput");
pdf_image_resolution_code:print_esc("pdfimageresolution");
pdf_compress_level_code:print_esc("pdfcompresslevel");
pdf_last_form_code:print_esc("pdflastform");
pdf_last_annot_code:print_esc("pdflastannot");
pdf_last_obj_code:print_esc("pdflastobj");
pdf_adjust_spacing_code:print_esc("pdfadjustspacing");
@z

@x [17.238]
primitive("errorcontextlines",assign_int,int_base+error_context_lines_code);@/
@!@:error_context_lines_}{\.{\\errorcontextlines} primitive@>
@y
primitive("errorcontextlines",assign_int,int_base+error_context_lines_code);@/
@!@:error_context_lines_}{\.{\\errorcontextlines} primitive@>
primitive("pdfoutput",assign_int,int_base+pdf_output_code);@/
@!@:pdf_output_}{\.{\\pdfoutput} primitive@>
primitive("pdfimageresolution",assign_int,int_base+pdf_image_resolution_code);@/
@!@:pdf_image_resolution_}{\.{\\pdfimageresolution} primitive@>
primitive("pdfcompresslevel",assign_int,int_base+pdf_compress_level_code);@/
@!@:pdf_compress_level_}{\.{\\pdfcompresslevel} primitive@>
primitive("pdflastform",assign_int,int_base+pdf_last_form_code);@/
@!@:pdf_last_form_}{\.{\\pdflastform} primitive@>
primitive("pdflastannot",assign_int,int_base+pdf_last_annot_code);@/
@!@:pdf_last_annot_}{\.{\\pdflastannot} primitive@>
primitive("pdflastobj",assign_int,int_base+pdf_last_obj_code);@/
@!@:pdf_last_obj_}{\.{\\pdflastobj} primitive@>
primitive("pdfadjustspacing",assign_int,int_base+pdf_adjust_spacing_code);@/
@!@:pdf_adjust_spacing_}{\.{\\pdfadjustspacing} primitive@>
@z

% Define pdftex dimension parameters
@x [17.247] 
@d emergency_stretch_code=20 {reduces badnesses on final pass of line-breaking}
@d dimen_pars=21 {total number of dimension parameters}
@y
@d emergency_stretch_code=20 {reduces badnesses on final pass of line-breaking}
@d pdf_h_origin_code=21 {horigin of \PDF{} output}
@d pdf_v_origin_code=22 {horigin of \PDF{} output}
@d pdf_page_width_code=23 {page width of \PDF{} output}
@d pdf_page_height_code=24 {page height of \PDF{} output}
@d pdf_thread_hoffset_code=25 {thread hoffset of \PDF{} output}
@d pdf_thread_voffset_code=26 {thread voffset of \PDF{} output}
@d dimen_pars=27 {total number of dimension parameters}
@z

@x [17.247]
@d emergency_stretch==dimen_par(emergency_stretch_code)
@y
@d emergency_stretch==dimen_par(emergency_stretch_code)
@d pdf_h_origin==dimen_par(pdf_h_origin_code)
@d pdf_v_origin==dimen_par(pdf_v_origin_code)
@d pdf_page_width==dimen_par(pdf_page_width_code)
@d pdf_page_height==dimen_par(pdf_page_height_code)
@d pdf_thread_hoffset==dimen_par(pdf_thread_hoffset_code)
@d pdf_thread_voffset==dimen_par(pdf_thread_voffset_code)
@z

@x [17.247]
emergency_stretch_code:print_esc("emergencystretch");
@y
emergency_stretch_code:print_esc("emergencystretch");
pdf_h_origin_code: print_esc("pdfhorigin");
pdf_v_origin_code: print_esc("pdfvorigin");
pdf_page_width_code: print_esc("pdfpagewidth");
pdf_page_height_code: print_esc("pdfpageheight");
pdf_thread_hoffset_code: print_esc("pdfthreadhoffset");
pdf_thread_voffset_code: print_esc("pdfthreadvoffset");
@z

@x [17.248]
primitive("emergencystretch",assign_dimen,dimen_base+emergency_stretch_code);@/
@!@:emergency_stretch_}{\.{\\emergencystretch} primitive@>
@y
primitive("emergencystretch",assign_dimen,dimen_base+emergency_stretch_code);@/
@!@:emergency_stretch_}{\.{\\emergencystretch} primitive@>
primitive("pdfhorigin",assign_dimen,dimen_base+pdf_h_origin_code);@/
@!@:pdf_h_origin_}{\.{\\pdfhorigin} primitive@>
primitive("pdfvorigin",assign_dimen,dimen_base+pdf_v_origin_code);@/
@!@:pdf_v_origin_}{\.{\\pdfvorigin} primitive@>
primitive("pdfpagewidth",assign_dimen,dimen_base+pdf_page_width_code);@/
@!@:pdf_page_width_}{\.{\\pdfpagewidth} primitive@>
primitive("pdfpageheight",assign_dimen,dimen_base+pdf_page_height_code);@/
@!@:pdf_page_height_}{\.{\\pdfpageheight} primitive@>
primitive("pdfthreadhoffset",assign_dimen,dimen_base+pdf_thread_hoffset_code);@/
@!@:pdf_thread_hoffset_}{\.{\\pdfthreadhoffset} primitive@>
primitive("pdfthreadvoffset",assign_dimen,dimen_base+pdf_thread_voffset_code);@/
@!@:pdf_thread_voffset_}{\.{\\pdfthreadvoffset} primitive@>
@z

% Define pdftex tokens parameters
@x [22.307] 
@d write_text=15 {|token_type| code for \.{\\write}}
@y
@d write_text=15 {|token_type| code for \.{\\write}}
@d pdf_font_prefix_text=16 {|token_type| code for \.{\\pdffontprefix}}
@d pdf_form_prefix_text=17 {|token_type| code for \.{\\pdfformprefix}}
@d pdf_image_prefix_text=18 {|token_type| code for \.{\\pdfimageprefix}}
@d pdf_page_attr_text=19 {|token_type| code for \.{\\pdfpageattr}}
@d pdf_pages_attr_text=20 {|token_type| code for \.{\\pdfpagesattr}}
@d pdf_form_attr_text=21 {|token_type| code for \.{\\pdfformattr}}
@d pdf_image_attr_text=22 {|token_type| code for \.{\\pdfimageattr}}
@z

@x [22.314]
write_text: print_nl("<write> ");
othercases print_nl("?") {this should never happen}
@y
write_text: print_nl("<write> ");
pdf_font_prefix_text: print_nl("<pdffontprefix> ");
pdf_form_prefix_text: print_nl("<pdfformprefix> ");
pdf_image_prefix_text: print_nl("<pdfimageprefix> ");
pdf_page_attr_text: print_nl("<pdfpageattr> ");
pdf_pages_attr_text: print_nl("<pdfpagesattr> ");
pdf_form_attr_text: print_nl("<pdfformattr> ");
pdf_image_attr_text: print_nl("<pdfimageattr> ");
othercases print_nl("?") {this should never happen}
@z

% Define pdftex version/revision (borrowed from eTeX)
@x [26.410]
@d tok_val=5 {token lists}
@y
@d tok_val=5 {token lists}
@d pdftex_revision_code=6 {command code for \.{\\pdftexrevision}}
@d pdf_fontname_code=7 {command code for \.{\\pdffontname}}
@z

@x [26.416] 
|glue_val|, |input_line_no_code|, or |badness_code|.

@d input_line_no_code=glue_val+1 {code for \.{\\inputlineno}}
@d badness_code=glue_val+2 {code for \.{\\badness}}
@y
|glue_val|, |input_line_no_code|, |badness_code|, or |pdftex_version_code|.

@d input_line_no_code=glue_val+1 {code for \.{\\inputlineno}}
@d badness_code=glue_val+2 {code for \.{\\badness}}
@d pdftex_version_code=glue_val+3 {code for \.{\\pdftexversion}}
@z

@x [26.416] 
primitive("badness",last_item,badness_code);
@!@:badness_}{\.{\\badness} primitive@>
@y
primitive("badness",last_item,badness_code);@/
@!@:badness_}{\.{\\badness} primitive@>
primitive("pdftexversion",last_item,pdftex_version_code);@/
@!@:pdftex_version_}{\.{\\pdftexversion} primitive@>
primitive("pdftexrevision",convert,pdftex_revision_code);@/
@!@:pdftex_revision_}{\.{\\pdftexrevision} primitive@>
primitive("pdffontname",convert,pdf_fontname_code);@/
@!@:pdf_fontname_}{\.{\\pdffontname} primitive@>
@z

@x [26.417] 
  othercases print_esc("badness")
@y
  pdftex_version_code: print_esc("pdftexversion");
  othercases print_esc("badness")
@z

@x [26.424] 
if cur_chr>glue_val then
  begin if cur_chr=input_line_no_code then cur_val:=line
  else cur_val:=last_badness; {|cur_chr=badness_code|}
@y
if cur_chr>glue_val then
  begin case cur_chr of
  input_line_no_code: cur_val:=line;
  badness_code: cur_val:=last_badness;
  pdftex_version_code: cur_val:=pdftex_version;
  endcases;
@z

@x [27.469] 
  othercases print_esc("jobname")
@y
  pdftex_revision_code: print_esc("pdftexrevision");
  pdf_fontname_code: print_esc("pdffontname");
  othercases print_esc("jobname")
@z

@x [27.471]
end {there are no other cases}
@y
pdftex_revision_code: do_nothing;
pdf_fontname_code: scan_font_ident;
end {there are no other cases}
@z

@x [27.472]
end {there are no other cases}
@y
pdftex_revision_code: print(pdftex_revision);
pdf_fontname_code: begin
    if (cur_val <= 0) or (cur_val > font_ptr) then begin
        print_err("invalid parametr of \pdffontname");
        error;
    end
    else begin
        if not font_used[cur_val] then
            pdf_create_font_obj(cur_val);
        print_int(obj_info(pdf_font_num[cur_val]));
    end;
end;
end {there are no other cases}
@z

% Shipping out to PDF
@x [32.638]
@ The |hlist_out| and |vlist_out| procedures are now complete, so we are
ready for the |ship_out| routine that gets them started in the first place.

@p procedure ship_out(@!p:pointer); {output the box |p|}
@y
@ The |hlist_out| and |vlist_out| procedures are now complete, so we are
ready for the |dvi_ship_out| routine that gets them started in the first place.

@p procedure dvi_ship_out(@!p:pointer); {output the box |p|}
@z

@x [33.644]
@* \[33] Packaging.
@y

@* \[32a] \pdfTeX\ basic.

The following code reads values from config file when \pdfTeX\ starts.

@d pdf_1bp == 65782 {scaled value corresponds to 1bp |= xn_over_d(unity, 7227,
7200)|}

@<Read values from config file if necessary@>=
@!Init
end else begin
read_config;
pdf_output := cfg_output;
pdf_image_resolution := cfg_image_resolution;
pdf_compress_level := cfg_compress_level;
pdf_include_form_resources := cfg_include_form_resources;
decimal_digits := cfg_decimal_digits;
if decimal_digits < 0 then
    decimal_digits := 0
else if decimal_digits > 5 then
    decimal_digits := 5;
min_bp_val := pdf_1bp div ten_pow[decimal_digits] +
    2*(pdf_1bp mod ten_pow[decimal_digits]) div ten_pow[decimal_digits];
Tini

@ The next functions just define the corresponding macros so we can use them
in C. 

@p function getxheight(f: internal_font_number): scaled;
begin
    getxheight := x_height(f);
end;

function getcharwidth(f: internal_font_number; k: eight_bits): scaled;
begin
    getcharwidth := char_width(f)(orig_char_info(f)(k));
end;

function getcharheight(f: internal_font_number; k: eight_bits): scaled;
begin
    getcharheight := char_height(f)(height_depth(orig_char_info(f)(k)));
end;

function getchardepth(f: internal_font_number; k: eight_bits): scaled;
begin
    getchardepth := char_depth(f)(height_depth(orig_char_info(f)(k)));
end;

function getquad(f: internal_font_number): scaled;
begin
    getquad := quad(f);
end;

function getslant(f: internal_font_number): scaled;
begin
    getslant := slant(f);
end;

function getnullstr: str_number;
begin
    getnullstr :=  "";
end;

function getnullfont: internal_font_number;
begin
    getnullfont := null_font;
end;

@ Sometimes it is neccesary to allocate memory for \PDF{} output that cannot
be deallocated then, so we use |pdf_mem| for this purpose.

@ @<Glob...@>=
@!pdf_mem_size: integer;
@!pdf_mem: ^integer;
@!pdf_mem_ptr: integer;

@ @<Set init...@>=
pdf_mem_ptr := 0;

@ We use |pdf_get_mem| to allocate memory in |pdf_mem|

@p function pdf_get_mem(s: small_number): integer; {allocate |s| words in
|pdf_mem|}
begin
    if pdf_mem_ptr + s > pdf_mem_size then
        overflow("PDF memory size", pdf_mem_size);
    pdf_get_mem := pdf_mem_ptr;
    pdf_mem_ptr := pdf_mem_ptr + s;
end;


@* \[32b] \pdfTeX\ output low-level subroutines.
We use the similiar subroutines to handle the output buffer for
\PDF{} output. When compress is used, the state of writting to buffer
is held in |zip_write_state|. We must write the header of \PDF{}
output file in initialization to ensure that it will be the first
written bytes.

@<Constants...@>=
@!pdf_buf_size = 16384; {size of the \PDF{} buffer}

@ The following macros are similiar as \.{DVI} buffer handling

@d pdf_offset == (pdf_gone + pdf_ptr) {the file offset of last byte in \PDF{}
buffer that |pdf_ptr| points to}
@#
@d no_zip == 0 {no \.{ZIP} compression}
@d zip_writting == 1 {writting by \.{ZIP} compression}
@d zip_finish == 2 {finish \.{ZIP} compression}
@#
@d pdf_quick_out(#) == {output a byte to \PDF{} buffer without checking of
overflow}
begin
    pdf_buf[pdf_ptr] := #;
    incr(pdf_ptr);
end

@d pdf_out(#) == {do the same as |pdf_quick_out| and flush the \PDF{}
buffer if necessary}
begin
    pdf_quick_out(#);
    if pdf_ptr = pdf_buf_size then
        pdf_flush;
end

@d flush_last_string(#) == {flush the most recently created string}
    if # = str_ptr - 1 then
        flush_string

@d pdf_remove_last_space == {remove the last space in \PDF{} buffer}
    if (pdf_ptr > 0) and (pdf_buf[pdf_ptr - 1] = 32) then 
        decr(pdf_ptr)

@d pdf_room(#) == {make sure that there are at least |n| bytes free in \PDF{}
buffer}
begin
    if # + pdf_ptr >= pdf_buf_size then
        pdf_flush;
end

@ @<Glob...@>=
@!pdf_file: byte_file; {the \PDF{} output file}
@!pdf_buf: array[0..pdf_buf_size] of eight_bits; {the \PDF{} buffer}
@!pdf_ptr: integer; {pointer to the first unused byte in the \PDF{} buffer}
@!pdf_gone: integer; {number of bytes that were flushed to output}
@!pdf_save_offset: integer; {to save |pdf_offset|}
@!zip_write_state: integer; {which state of compression we are in}

@ @<Set init...@>=
pdf_buf[0] := "%";
pdf_buf[1] := "P";
pdf_buf[2] := "D";
pdf_buf[3] := "F";
pdf_buf[4] := "-";
pdf_buf[5] := "1";
pdf_buf[6] := ".";
pdf_buf[7] := "2";
pdf_buf[8] := pdf_new_line_char;
pdf_ptr := 9;
pdf_gone := 0;
zip_write_state := no_zip;

@ The following code opens \PDF{} output file if neccesary.
@p procedure ensure_pdf_open;
begin
    if output_file_name <> 0 then
        return;
    if job_name = 0 then 
        open_log_file;
    pack_job_name(".pdf");
    while not b_open_out(pdf_file) do
        prompt_file_name("file name for output",".pdf");
    output_file_name := b_make_name_string(pdf_file);
end;

@ The \PDF{} buffer is flushed by calling |pdf_flush|, which checks the
variable |zip_write_state| and will compress the buffer before flushing if
neccesary. We call |pdf_begin_stream| to begin a stream  and |pdf_end_stream|
to finish it. The stream contents will be compressed if compression is turn on.
These procedures are very simple, but we don't define them as
macros, as we need to call them from other modules too.

@p procedure pdf_flush; {flush out the |pdf_buf|}
var compress_level: integer;
begin
    ensure_pdf_open;
    if pdf_compress_level < 0 then
        compress_level := 0
    else if pdf_compress_level > 9 then
        compress_level := 9
    else
        compress_level := pdf_compress_level;
    case zip_write_state of
        no_zip: if pdf_ptr > 0 then begin
            write_pdf(0, pdf_ptr - 1);
            pdf_gone := pdf_offset;
        end;
        zip_writting:
            write_zip(false, compress_level);
        zip_finish: begin
            write_zip(true, compress_level);
            pdf_gone := pdf_gone + pdf_stream_length;
            zip_write_state := no_zip;
        end;
    end;
    pdf_ptr := 0;
end;

procedure pdf_begin_stream; {begin a stream}
begin
    pdf_create_obj(obj_type_others, 0);
    pdf_last_length := obj_ptr;
    pdf_print("/Length ");
    pdf_print_int(pdf_last_length);
    pdf_print_ln(" 0 R");
    if pdf_compress_level > 0 then begin
        pdf_print_ln("/Filter /FlateDecode");
        pdf_print_ln(">>");
        pdf_print_ln("stream");
        pdf_flush;
        pdf_stream_length := 0;
        zip_write_state := zip_writting;
    end
    else begin
        pdf_print_ln(">>");
        pdf_print_ln("stream");
        pdf_save_offset := pdf_offset;
    end;
end;

procedure pdf_end_stream; {end a stream}
begin
    if pdf_compress_level > 0 then begin
        zip_write_state := zip_finish;
        pdf_flush;
    end
    else
        pdf_stream_length := pdf_offset - pdf_save_offset;
    pdf_print_ln("endstream");
    pdf_end_obj;
    pdf_begin_obj(pdf_last_length);
    pdf_print_int_ln(pdf_stream_length);
    pdf_end_obj;
end;

@ Basic printing procedures for \PDF{} output are very similiar to \TeX\ basic
printing ones but the output is going to \PDF{} buffer. Subroutines with
suffix |_ln| append a new-line character to the \PDF{} output.

@d pdf_new_line_char == 10 {new-line character for UNIX platforms}

@d pdf_print_nl == {output a new-line character to \PDF{} buffer}
    pdf_out(pdf_new_line_char) 

@d pdf_print_ln(#) == {print out a string to \PDF{} buffer followed by
a new-line character}
begin
    pdf_print(#);
    pdf_print_nl;
end

@d pdf_print_int_ln(#) == {print out an integer to \PDF{} buffer followed by
a new-line character}
begin
    pdf_print_int(#);
    pdf_print_nl;
end

@<Declare procedures that need to be declared forward...@>=
procedure pdf_print_octal(n:integer); {prints an integer in octal form to
\PDF{} buffer}
var k:0..23; {index to current digit; we assume that $|n|<10^{23}$}
begin
  k:=0;
  repeat dig[k]:=n mod 8; n:=n div 8; incr(k);
  until n=0;
  if k = 1 then begin
    pdf_out("0");
    pdf_out("0");
  end;
  if k = 2 then
    pdf_out("0");
  while k>0 do begin
    decr(k);
    pdf_out("0"+dig[k]);
  end;
end;

procedure pdf_print_char(c: integer); {print out a character to \PDF{}
buffer; the character will be printed in octal form when necessary}
begin
    case c of
    32,                        {space}
    10,                        {linefeed}
    13,                        {carriage return}
    9,                         {horizontal tab}
    8,                         {backspace}
    12,                        {formfeed}
    92,                        {backslash}
    40,                        {left parenthesis}
    41:                        {right parenthesis}
    begin
        pdf_out(92);           {output a backslash}
        pdf_print_octal(c);
    end;
    othercases pdf_out(c);
    endcases;
end;

procedure pdf_print(s: str_number); {print out a string to \PDF{} buffer}
var j: pool_pointer; {current character code position}
    c: integer;
begin
    j:=str_start[s];
    while j<str_start[s+1] do begin
       c := str_pool[j];
       pdf_out(c);
       incr(j);
    end;
end;

procedure literal(s: str_number; reset_origin, is_special, warn: boolean);
var j: pool_pointer; {current character code position}
begin
    j:=str_start[s];
    if is_special then begin
        if (length(s) <= 4) or
            (((str_pool[j]  <> "P") or 
             (str_pool[j+1] <> "D") or 
             (str_pool[j+2] <> "F")) and
            ((str_pool[j]   <> "p") or 
             (str_pool[j+1] <> "d") or 
             (str_pool[j+2] <> "f"))) or
            (str_pool[j+3]  <> ":") then begin
            if warn then
                print_nl("Non-PDF special ignored!");
            return;
        end;
        j := j + 4;
    end;
    if reset_origin then begin
        pdf_end_text;
        pdf_set_origin;
    end
    else begin
        pdf_end_string;
        pdf_print_nl;
    end;
    while j<str_start[s+1] do begin
       pdf_out(str_pool[j]);
       incr(j);
    end;
    pdf_print_nl;
end;

procedure pdf_print_int(n:integer); {print out a integer to \PDF{} buffer}
var k:0..23; {index to current digit; we assume that $|n|<10^{23}$}
m:integer; {used to negate |n| in possibly dangerous cases}
begin
  k:=0;
  if n<0 then
    begin pdf_out("-");
    if n>-100000000 then negate(n)
    else  begin m:=-1-n; n:=m div 10; m:=(m mod 10)+1; k:=1;
      if m<10 then dig[0]:=m
      else  begin dig[0]:=0; incr(n);
        end;
      end;
    end;
  repeat dig[k]:=n mod 10; n:=n div 10; incr(k);
  until n=0;
  pdf_room(k);
  while k>0 do begin
    decr(k);
    pdf_quick_out("0"+dig[k]);
  end;
end;

procedure pdf_print_two(n:integer); {prints two least significant digits in
decimal form to \PDF{} buffer}
begin n:=abs(n) mod 100; pdf_out("0"+(n div 10));
pdf_out("0"+(n mod 10));
end;

@ To print |scaled| value to PDF output we need some subroutines to ensure
accurary.

@d max_integer == @'17777777777

@<Glob...@>=
@!decimal_digits: integer; {number of decimal digits (|dd|)}
@!ten_pow: array[0..5] of integer; {$10^0..10^5$}
@!scaled_out: integer; {amount of |scaled| that was taken out in
|divide_scaled|}

@ @<Set init...@>=
ten_pow[0] := 1;
ten_pow[1] := 10;
ten_pow[2] := 100;
ten_pow[3] := 1000;
ten_pow[4] := 10000;
ten_pow[5] := 100000;

@ The following function divides |s| by |m|. |dd| is number of decimal digits.

@<Declare procedures that need to be declared forward...@>=
function divide_scaled(s, m: scaled; dd: integer): scaled; 
var q, r: scaled;
    sign, i: integer;
begin
    if s < 0 then begin
        sign := -1;
        s := -s;
    end
    else
        sign := 1;
    if (m < 0) or (m >= (max_integer div 10)) then begin
        arith_error := true;
        return;
    end;
    q := s div m;
    r := s mod m;
    for i := 1 to dd do begin
        q := 10*q + (10*r) div m;
        r := (10*r) mod m;
    end;
    if 2*r >= m then begin
        incr(q);
        r := r - m;
    end;
    scaled_out := sign*(s - (r div ten_pow[dd]));
    divide_scaled := sign*q;
end;

@ Next subroutines are needed for spscing control in PDF page description.

@p procedure add_char_width(s: scaled); {update |pdf_h| by |s|} 
var i: integer;
begin
    if divide_scaled(s, pdf_font_size[f], 3) > 0 then
        do_nothing; {to calculate |scale_out| only}
    pdf_h := pdf_h + scaled_out;
end;

procedure pdf_print_real(m, d: integer); {print $m/10^d$ as real}
var n: integer;
begin
    if m < 0 then begin
        pdf_out("-");
        m := -m;
    end;
    n := ten_pow[d];
    pdf_print_int(m div n);
    m := m mod n;
    if m > 0 then begin
        pdf_out(".");
        n := n div 10;
        while m < n do begin
            pdf_out("0");
            n := n div 10;
        end;
        while m mod 10 = 0 do
            m := m div 10;
        pdf_print_int(m);
    end;
end;

procedure pdf_print_bp(s: scaled); {print scaled as |bp|}
var m: scaled;
begin
{   (s/65536)*(7200/7227) = s/65781.76  }
    m := divide_scaled(s, pdf_1bp, decimal_digits);
    pdf_print_real(m, decimal_digits);
end;

procedure pdf_print_mag_bp(s: scaled); {take |mag| into account}
begin
    if (mag <> 1000) and (mag <> 0) then
        s := xn_over_d(s, mag, 1000);
    pdf_print_bp(s);
end;


@* \[32c] \PDF{} page description.

@d pdf_x(#) == ((#) - pdf_origin_h) {convert $x$-coordinate from \.{DVI} to
\PDF{}}
@d pdf_y(#) == (pdf_origin_v - (#)) {convert $y$-coordinate from \.{DVI} to
\PDF{}}
@d min_emthousandth == 10 {|em_thousandth|'s values less than this number will
not be displayed}

@<Glob...@>=
@!pdf_f: internal_font_number; {the current font in \PDF{} output page}
@!pdf_h: scaled; {current horizontal coordinate in \PDF{} output page}
@!pdf_v: scaled; {current vertical coordinate in \PDF{} output page}
@!pdf_last_h: scaled; {last horizontal coordinate in \PDF{} output page}
@!pdf_last_v: scaled; {last vertical coordinate in \PDF{} output page}  
@!pdf_origin_h: scaled; {current horizontal origin in \PDF{} output page}
@!pdf_origin_v: scaled; {current vertical origin in \PDF{} output page}  
@!pdf_1st_ws: integer; {amount of first word spacing while drawing a string;
for some reason it is not taken into account of the length of the string, so we
have to save it in order to adjust spacing when string drawing is finished}
@!pdf_doing_string: boolean; {we are writting string to \PDF{} file?}
@!pdf_doing_text: boolean; {we are writting text section to \PDF{} file?}
@!pdf_font_changed: boolean; {current font has been changed?}
@!min_bp_val: scaled; {scaled value corresponds to $10^{-dd}$ bp, where |dd|
is |decimal_digits|}
@!expand_ratio: integer;

@ Following procedures implement low-level subroutines to convert \TeX{}
internal structures to \PDF{} page description.

@p procedure pdf_set_origin; {set the origin to |cur_h|, |cur_v|}
begin
    if (abs(cur_h - pdf_origin_h) >= min_bp_val) or
        (abs(cur_v - pdf_origin_v) >= min_bp_val) then begin
        pdf_print("1 0 0 1 ");
        pdf_print_bp(cur_h - pdf_origin_h);
        pdf_origin_h := pdf_origin_h + scaled_out;
        pdf_out(" ");
        pdf_print_bp(pdf_origin_v - cur_v);
        pdf_origin_v := pdf_origin_v - scaled_out;
        pdf_print_ln(" cm");
    end;
    pdf_h := pdf_origin_h;
    pdf_last_h := pdf_origin_h;
    pdf_v := pdf_origin_v;
    pdf_last_v := pdf_origin_v;
end;

procedure pdf_end_string; {end the current string}
begin
    if pdf_doing_string then begin
        pdf_print(")]TJ");
        if pdf_1st_ws <> 0 then begin
            pdf_h := pdf_h - pdf_1st_ws;
            pdf_1st_ws := 0;
        end;
        pdf_doing_string := false;
    end;
end;

procedure pdf_moveto; {set the next starting point to |cur_h|, |cur_v|}
var v, s, m: scaled;
begin
    v := divide_scaled(pdf_last_v - cur_v, pdf_1bp, decimal_digits);
    pdf_v := pdf_last_v - scaled_out;
    pdf_out(" ");
    pdf_print_bp(cur_h - pdf_last_h);
    pdf_h := pdf_last_h + scaled_out;
    pdf_out(" ");
    pdf_print_real(v, decimal_digits);
    pdf_print(" Td");
    pdf_last_h := pdf_h;
    pdf_last_v := pdf_v;
end;

procedure pdf_begin_text; {begin a text section}
begin
    if not pdf_doing_text then begin
        pdf_set_origin;
        pdf_print_ln("BT");
        pdf_doing_text := true;
        pdf_f := null_font;
        pdf_1st_ws := 0;
        pdf_doing_string := false;
    end;
end;

procedure pdf_setfont; forward;

procedure pdf_begin_string; {begin to draw a string}
var b: boolean; {|b| is true only when we must adjust word spacing
at the beginning of string}
    s: integer;
    m: scaled;
begin
    pdf_begin_text;
    if f<>pdf_f then begin
        pdf_end_string;
        pdf_setfont;
    end;
    b := false;
    s := divide_scaled(cur_h - pdf_h, pdf_font_size[f], 3);
    if pdf_font_changed or (abs(cur_v - pdf_v) >= min_bp_val) or 
        (abs(s) >= @'100000) then begin
        pdf_end_string;
        pdf_moveto;
        pdf_font_changed := false;
        s := 0;
    end;
    if pdf_doing_string then begin
        if abs(s) >= min_emthousandth then
            pdf_out(")");
    end
    else begin
        pdf_out("[");
        if abs(s) < min_emthousandth then
            pdf_out("(")
        else
            b := true;
        pdf_doing_string := true;
    end;
    if abs(s) >= min_emthousandth then begin
        pdf_print_int(-s);
        if b then
            pdf_1st_ws := scaled_out;
        pdf_out("(");
        pdf_h := pdf_h + scaled_out;
    end;
end;

procedure pdf_end_text; {end a text section}
begin
    if pdf_doing_text then begin
        pdf_end_string;
        pdf_print_nl;
        pdf_print_ln("ET");
        pdf_doing_text := false;
    end;
end;

procedure pdf_set_rule(h, v, wd, ht: scaled); {output a rule to \PDF{}
file}
begin
    pdf_end_text;
    pdf_print_bp(pdf_x(h));
    pdf_out(" "); pdf_print_bp(pdf_y(v));
    pdf_out(" "); pdf_print_bp(wd);
    pdf_out(" "); pdf_print_bp(ht);
    pdf_print_ln(" re f");
{
    pdf_set_origin;
    pdf_print("n 0 0 m 0 "); pdf_print_bp(ht); pdf_print(" l ");
    pdf_print_bp(wd); pdf_out(" "); pdf_print_bp(ht); pdf_print(" l ");
    pdf_print_bp(wd); pdf_print_ln(" 0 l h f");
}
end;

procedure pdf_rectangle(left, top, right, bottom: scaled); {output a
rectangle specification to \PDF{} file}
begin
  pdf_print("/Rect [");
  pdf_print_mag_bp(pdf_x(left)); pdf_out(" ");
  pdf_print_mag_bp(pdf_y(bottom)); pdf_out(" ");
  pdf_print_mag_bp(pdf_x(right)); pdf_out(" ");
  pdf_print_mag_bp(pdf_y(top));
  pdf_print_ln("]");
end;

@* \[32d] The cross-reference table.

The cross-reference table |obj_tab| is an array of |obj_tab_size| of
|tab_entry|. Each entry contains four integer fields and represents an object
in \PDF{} file whose object number is the index of this entry in |obj_tab|.
Objects in |obj_tab| maybe linked into list; objects in a list have the same
type. 

The first field of |obj_entry| contains link to the next
object in |obj_tab| if this object in linked in a list. 

The second field contains information representing identifier of this object.
Tt is usally a number for most of object types, but it may be a string number
for named destination or named thread.

The third field and fouth ones may contain various types of data; their
interpretations are dependent on the type of object represented by this entry.
It is typically a pointer to auxiliary memory; but it can represent an integer
number as well.

@<Types...@>=
@!obj_entry = record@;@/
    int0, int1, int2, int3: integer;
end;

@ The array |head_tab| contains pointers holding heads of all linked lists
of entries in |obj_tab|. Some entries have a pointer which point to
auxiliary data of this object. It's typically for annotations, outlines,
XObject, etc.

Here we define all types of new whatsit nodes and the way how to access to
their data.

@d obj_info(#) == obj_tab[#].int0 {information representing identifier of this object}
@d obj_link(#) == obj_tab[#].int1 {link to the next entry in linked list}
@d obj_int(#) == obj_tab[#].int2 {accumulator for saving integer data}
@d obj_scaled == obj_int {accumulator for saving scaled data}
@d obj_offset == obj_int {byte offset of this object in \PDF{} output file}
@d obj_aux(#) == obj_tab[#].int3 {first auxiliary pointer}

@# {types of objects}
@d obj_type_others              == 0 {objects which are not linked in any list}
@d obj_type_page                == 1 {index of linked list of Page objects}
@d obj_type_pages               == 2 {index of linked list of Pages objects}
@d obj_type_font                == 3 {index of linked list of Fonts objects}
@d obj_type_outline             == 4 {index of linked list of outline objects}
@d obj_type_dest_num            == 5 {index of linked list of num destination objects}
@d obj_type_dest_name           == 6 {index of linked list of named destination objects}
@d obj_type_form                == 7 {index of linked list of XObject forms}
@d obj_type_thread_num          == 8 {index of linked list of num article threads}
@d obj_type_thread_name         == 9 {index of linked list of named article threads}
@d head_tab_max            == 9 {max index of |head_tab|}

@# {max number of kids for balanced trees}
@d pages_tree_kids_max     == 6 {max number of kids of Pages tree node}
@d name_tree_kids_max      == @'100000 {max number of kids of node of name tree for
name destinations}

@# {type of identifier of destinations/threads/annotations}
@d pdf_id_num              == 0 {num identifier}
@d pdf_id_name             == 1 {name identifier}
@d pdf_id_page             == 2 {page identifier (page number)}

@# {when a whatsit node representing annotation is created, words |1..3| are
width, height and depth of this annotation; after shipping out words |1..4|
are rectangle specification of annotation. For whatsit node representing
destination |pdf_left| and |pdf_top| are used for some types of destinations}

@# {coordinates of destinations/threads/annotations (in whatsit node)}
@d pdf_left(#)             == mem[# + 1].sc
@d pdf_top(#)              == mem[# + 2].sc
@d pdf_right(#)            == mem[# + 3].sc
@d pdf_bottom(#)           == mem[# + 4].sc

@# {dimesion of destinations/threads/annotations (in whatsit node)}
@d pdf_width(#)            == mem[# + 1].sc
@d pdf_height(#)           == mem[# + 2].sc
@d pdf_depth(#)            == mem[# + 3].sc

@# {data struture of |pdf_literal|}
@d pdf_literal_data(#)     == link(#+1) {reference to token list}
@d pdf_literal_direct(#)   == info(#+1) {will the literal embeded directly to the page contents}

@# {data struture of raw objects}
@d obj_obj_ptr             == obj_aux {pointer to corresponding |pdf_obj_node|}
@d pdf_obj_node_size       == 3 {size of whatsit node representing the raw object}
@d pdf_obj_data(#)         == info(# + 1) {pointer to token list}
@d pdf_obj_is_stream(#)    == link(# + 1) {is this object a stream?}
@d pdf_obj_obj_num(#)      == mem[# + 2].int {number of the raw object}

@# {data struture of annotations}
@d obj_annot_ptr           == obj_aux {pointer to corresponding whatsit node}
@d pdf_annot_node_size     == 7 {size of whatsit node representing annotation}
@d pdf_annot_data(#)       == info(# + 5) {raw data of general annotations}
@d pdf_annot_link_attr(#)  == info(# + 5) {attributes of link annotations}
@d pdf_annot_link_action(#)== link(# + 5) {pointer to action structure}
@d pdf_annot_obj_num(#)    == mem[# + 6].int {object number of corresponding object}

@# {types of actions}
@d pdf_action_page         == 0 {Goto action}
@d pdf_action_goto         == 1 {Goto action}
@d pdf_action_thread       == 2 {Thread action}
@d pdf_action_user         == 3 {user-defined action}

@# {data structure of actions}
@d pdf_action_struct_size  == 4 {size of action structure in |mem|}
@d pdf_action_type         == type {action type}
@d pdf_action_id_type      == subtype {identifier type}
@d pdf_action_name         == link {destination/thread name identifier}
@d pdf_action_num          == link {destination/thread num identifier} 
@d pdf_action_page_num     == link {page number for GotoPage action}
@d pdf_action_file(#)      == info(# + 1) {file name for external action}
@d pdf_action_new_window(#)== link(# + 1) {open a new window?}
@d pdf_action_obj_num(#)   == mem[# + 2].int {object number of local action}
@d pdf_action_page_tokens(#) == info(# + 3) {specification of goto page action}
@d pdf_action_user_tokens(#) == info(# + 3) {user-defined action string}
@d pdf_action_refcount(#)  == link(# + 3) {counter of references to this action}

@# {data structure of outlines; it's not able to write out outline entries
before all outline entries are defined, so memory allocated for outline
entries can't not be deallocated and will stay in memory. For this reason we
will store data of outline entries in |pdf_mem| instead of |mem|}

@d pdf_outline_struct_size  == 7 {size of memory in |pdf_mem| which
|obj_outline_ptr| points to}
@d obj_outline_count        == obj_info{count of all opened children}
@d obj_outline_ptr          == obj_aux {pointer to |pdf_mem|}
@d obj_outline_title(#)     == pdf_mem[obj_outline_ptr(#)]
@d obj_outline_parent(#)    == pdf_mem[obj_outline_ptr(#) + 1]
@d obj_outline_prev(#)      == pdf_mem[obj_outline_ptr(#) + 2]
@d obj_outline_next(#)      == pdf_mem[obj_outline_ptr(#) + 3]
@d obj_outline_first(#)     == pdf_mem[obj_outline_ptr(#) + 4]
@d obj_outline_last(#)      == pdf_mem[obj_outline_ptr(#) + 5]
@d obj_outline_action_obj_num(#) == pdf_mem[obj_outline_ptr(#) + 6] {object number of
action}

@# {types of destinations}
@d pdf_dest_xyz             == 0
@d pdf_dest_fit             == 1
@d pdf_dest_fit_h           == 2
@d pdf_dest_fit_v           == 3
@d pdf_dest_fitb            == 4
@d pdf_dest_fitb_h          == 5
@d pdf_dest_fitb_v          == 6

@# {data structure of destinations}
@d obj_dest_ptr             == obj_aux {pointer to |pdf_dest_node|}
@d pdf_dest_node_size       == 6 {size of whatsit node for destination; words
|1..2| are current position, word |3| is identifier type, subtype and
identifier of destination}
@d pdf_dest_type(#)          == type(# + 3) {type of destination}
@d pdf_dest_id_type(#)       == subtype(# + 3) {type of destination identifier}
@d pdf_dest_id_num(#)        == link(# + 3) {num identifier of destination}
@d pdf_dest_id_name(#)       == link(# + 3) {name identifier of destination}
@d pdf_dest_xyz_zoom(#)      == info(# + 4) {zoom factor for |destxyz| destination}
@d pdf_dest_obj_num(#)       == mem[# + 5].int {object number of corresponding
object}

@# {data structure of threads}
@d obj_thread_first          == obj_aux {pointer to the first bead of this
thread}
@d pdf_thread_node_size      == 2
@d pdf_thread_id_type(#)     == type(# + 1) {type of thread identifier}
@d pdf_thread_id_num(#)      == link(# + 1) {thread num identifier}
@d pdf_thread_id_name(#)     == link(# + 1) {thread name identifier}

@# {data structure of beads}
@d pdf_bead_rect_node_size  == 5 {size of memory in |mem| which
|obj_thread_rect| points to; the rectangle specification of bead is held here}
@d pdf_bead_struct_size     == 4 {size of memory in |pdf_mem| which |obj_bead_ptr| points to}
@d obj_bead_ptr             == obj_aux {pointer to |pdf_mem|}
@d obj_bead_rect(#)         == pdf_mem[obj_bead_ptr(#)] {pointer to |pdf_mem|;
after writting out the object number of array of rectangle specification will be
held here}
@d obj_bead_page(#)         == pdf_mem[obj_bead_ptr(#) + 1]
@d obj_bead_next(#)         == pdf_mem[obj_bead_ptr(#) + 2]
@d obj_bead_prev(#)         == pdf_mem[obj_bead_ptr(#) + 3]

@# {data structure of images; for |pdf_image_node| and |pdf_ref_form_node| the
identifier of XObject will be store in |obj_info| of coresponding object in
|obj_tab|}
@d pdf_image_node_size      == 5 {size of whatsit node for image}
@d pdf_image_info(#)        == mem[# + 4].int {pointer to hold data structures
in external \.{libpng} library}
@d obj_image_ptr            == obj_aux {pointer to image structure}

@# {data structure of forms}
@d pdf_ref_form_node_size   == 5 {size of whatsit node for form; word 1..3 are
dimensions}
@d pdf_form_obj_num(#)      == info(# + 4) {object number of coresponding
XObject in |obj_tab|; number larger than |max_halfword| is not allowed}
@d pdf_form_struct_size     == 4 {size of memory in |pdf_mem| which |obj_aux|
points to}
@d obj_form_ptr             == obj_aux
@d obj_form_width(#)        == pdf_mem[obj_form_ptr(#) + 0]
@d obj_form_height(#)       == pdf_mem[obj_form_ptr(#) + 1]
@d obj_form_depth(#)        == pdf_mem[obj_form_ptr(#) + 2]
@d obj_form_box(#)          == pdf_mem[obj_form_ptr(#) + 3] {this field holds
pointer to the corresponding box}

@<Constants...@>=
@!inf_obj_tab_size = 32000; {min size of the cross-reference table for \PDF{} output}
@!sup_obj_tab_size = 524288; {max size of the cross-reference table for \PDF{} output}

@ @<Glob...@>=
@!obj_tab_size:integer;
@!obj_tab:^obj_entry;
@!head_tab: array[1..head_tab_max] of integer;
@!obj_ptr: integer; {objects counter}
@!pdf_last_pages: integer; {pointer to most recently generated pages object}
@!pdf_last_page: integer; {pointer to most recently generated page object}
@!pdf_last_stream: integer; {pointer to most recently generated stream}
@!pdf_last_length: integer; {pointer to length of most recently generated stream}
@!pdf_stream_length: integer; {length of most recently generated stream}
@!pdf_image_b: integer; {is any black-white image in  most recently output page/form?}
@!pdf_image_c: integer; {is any color image in  most recently output page/form?}
@!pdf_image_i: integer; {is any indexed-color image in  most recently output page/form?}
@!pdf_text: integer; {is any text in most recently output page/form?}
@!tmp_i: integer; {needed for defining |pdf_append_list|}

@ @<Set init...@>=
obj_ptr := 0;
for k := 1 to head_tab_max do
    head_tab[k] := 0;

@ Here we implement subroutines for work with objects and related things.
Some of them are used in former parts too, so we need to declare them
forward.

@d append_list_end(#) == # := append_ptr(#, tmp_i); end
@d pdf_append_list(#) == begin tmp_i := #; append_list_end

@<Declare procedures that need to be declared forward...@>=
procedure append_dest_name(s: str_number; n: integer);
begin
    if dest_names_ptr = max_dest_names then
        overflow("number of destination names", max_dest_names);
    dest_names[dest_names_ptr].objname := s;
    dest_names[dest_names_ptr].obj_num := n;
    incr(dest_names_ptr);
end;

procedure pdf_create_obj(t, i: integer); {create an object with type |t| and
identifier |i|}
label done;
var p, q: integer;
begin
    if obj_ptr = obj_tab_size then
        overflow("indirect objects table size", obj_tab_size);
    incr(obj_ptr);
    obj_info(obj_ptr) := i;
    obj_int(obj_ptr) := 0;
    obj_aux(obj_ptr) := 0;
    if t = obj_type_page then begin
        p := head_tab[t];
        {find the right poition to insert newly created object}@/
        if (p = 0) or (obj_info(p) < i) then begin
            obj_link(obj_ptr) := p;
            head_tab[t] := obj_ptr; 
        end
        else begin
            while p <> 0 do begin
                if obj_info(p) < i then
                    goto done;
                q := p;
                p := obj_link(p);
            end;
done:
            obj_link(q) := obj_ptr;
            obj_link(obj_ptr) := p;
        end;
    end
    else if t <> obj_type_others then begin
        obj_link(obj_ptr) := head_tab[t];
        head_tab[t] := obj_ptr;
        if t = obj_type_dest_name then
            append_dest_name(obj_info(obj_ptr), obj_ptr);
    end;
end;

function pdf_lookup(t, i: integer; by_name: boolean): integer; {looks up
object with identifier |i| in linked list of type |t|; |by_name| indicates
whether |i| should be treated as a string number}
var p: integer;
begin
    pdf_lookup := 0;
    p := head_tab[t];
    if by_name then
        while p <> 0 do begin
            if str_eq_str(obj_info(p), i) then
            begin
                pdf_lookup := p;
                return;
            end;
            p := obj_link(p);
        end
    else
        while p <> 0 do begin
            if (obj_info(p) = i) then 
            begin
                pdf_lookup := p;
                return;
            end;
            p := obj_link(p);
        end;
end;

procedure pdf_begin_obj(i: integer); {begin a \PDF{} object}
begin
    obj_offset(i) := pdf_offset;
    pdf_print_int(i);
    pdf_print_ln(" 0 obj");
end;

procedure pdf_end_obj;
begin
    pdf_print_ln("endobj"); {end a \PDF{} object}
end;

procedure pdf_begin_dict(i: integer); {begin a \PDF{} dictionary object}
begin
    obj_offset(i) := pdf_offset;
    pdf_print_int(i);
    pdf_print_ln(" 0 obj <<");
end;

procedure pdf_end_dict; {end a \PDF{} object of type dictionary}
begin
    pdf_print_ln(">> endobj");
end;

procedure pdf_new_obj(t, i: integer); {begin to a new object}
begin
    pdf_create_obj(t, i);
    pdf_begin_obj(obj_ptr);
end;

procedure pdf_new_dict(t, i: integer); {begin a new object with type dictionary}
begin
    pdf_create_obj(t, i);
    pdf_begin_dict(obj_ptr);
end;

function append_ptr(p: pointer; i: integer): pointer; {appends a pointer with
info |i| to the end of linked list with head |p|}
var q: pointer;
begin
    append_ptr := p;
    fast_get_avail(q);
    info(q) := i;
    link(q) := null;
    if p = null then begin
        append_ptr := q;
        return;
    end;
    while link(p) <> null do
        p := link(p);
    link(p) := q;
end;

function pdf_lookup_list(p: pointer; i: integer): pointer; {looks up for pointer 
with info |i| in list |p|}
begin
    pdf_lookup_list := null;
    while p <> null do begin
        if info(p) = i then begin
            pdf_lookup_list := p;
            return;
        end;
        p := link(p);
    end;
end;

@ Subroutines to print out various \PDF{} objects

@p procedure pdf_print_offset(n: integer); {print out an entry of
cross-reference table to \PDF{} buffer}
var k:0..23; {index to current digit; we assume that $|n|<10^{23}$}
m:integer; {used to negate |n| in possibly dangerous cases}
begin
  k:=0;
  repeat dig[k]:=n mod 10; n:=n div 10; incr(k);
  until k = 10;
  pdf_room(k);
  while k>0 do begin
    decr(k);
    pdf_quick_out("0"+dig[k]);
  end;
end;

procedure pdf_int_entry(s: str_number; v: integer); {print out an entry in
dictionary with integer value to \PDF{} buffer}
begin
    pdf_out("/");
    pdf_print(s);
    pdf_out(" ");
    pdf_print_int(v);
end;

procedure pdf_int_entry_ln(s: str_number; v: integer);
begin
    pdf_int_entry(s, v);
    pdf_print_nl;
end;

procedure pdf_indirect(s: str_number; o: integer); {print out an indirect
entry in dictionary}
begin
    pdf_out("/");
    pdf_print(s);
    pdf_out(" ");
    pdf_print_int(o);
    pdf_print(" 0 R");
end;

procedure pdf_indirect_ln(s: str_number; o: integer);
begin
    pdf_indirect(s, o);
    pdf_print_nl;
end;

procedure pdf_print_str(s: str_number); {print out |s| as string in \PDF{}
output}
begin
    pdf_out("(");
    pdf_print(s);
    pdf_out(")");
end;

procedure pdf_print_str_ln(s: str_number); {print out |s| as string in \PDF{}
output}
begin
    pdf_print_str(s);
    pdf_print_nl;
end;

procedure pdf_str_entry(s, v: str_number); {print out an entry in
dictionary with string value to \PDF{} buffer}
begin
    if v = 0 then
        return;
    pdf_out("/");
    pdf_print(s);
    pdf_out(" ");
    pdf_print_str(v);
end;

procedure pdf_str_entry_ln(s, v: str_number);
begin
    if v = 0 then
        return;
    pdf_str_entry(s, v);
    pdf_print_nl;
end;

@* \[32e] Font processing.

As pdfTeX should also act as a back-end driver, it needs to support virtual
font too. Information about virtual font can be found in source of some
\.{DVI}-related programs.

Whenever we want to write out a character in a font to \PDF{} output, we
should check whether the used font is new font (has not been used yet),
virtual font or real font. The array |pdf_font_type| holds flag of each used
font. After initialization flag of each font is set to |new_font_type|.
The first time when a character of a font is written out, pdfTeX looks for
the corresponding virtual font. If the corresponding virtual font exists, then
the font type is set to |virtual_font_type|; otherwise it will be set to
|real_font_type|. |subst_font_type| indicates fonts that have been substituted 
during adjusting spacing. Such fonts are linked via the |pdf_font_link| array.

@d new_font_type = 0 {new font (has not been used yet)}
@d virtual_font_type = 1 {virtual font}
@d real_font_type = 2 {real font}
@d subst_font_type = 3 {substituted font}

@<Glob...@>=
@!pdf_font_type: ^eight_bits; {the type of font}
@!pdf_font_link: ^internal_font_number; {link to expanded fonts}
@!cur_font_stretch: integer;
@!pdf_max_font_stretch: ^integer; {limit of expansion}
@!cur_font_shrink: integer;
@!pdf_max_font_shrink: ^integer; {limit of expansion}
@!cur_font_step: integer;
@!pdf_font_step: ^integer;  {amount of one step}

@ The procedure |pdf_find_font| set the object number of a internal font if
needed. This procedure must be declared forward.

@<Declare procedures that need to be declared forward...@>=
procedure pdf_create_font_obj(f: internal_font_number);
label done;
var k: integer;
begin 
    if pdf_expand_font[f] <> 0 then begin
        k := font_ptr + 1;
        goto done;
    end;
    if pdf_font_map[f] = -1 then 
        pdf_font_map[f] := fmlookup(font_name[f]);
    k := font_base;
    while k <= font_ptr do begin
        if font_used[k] and (pdf_expand_font[k] = 0) and
           (pdf_font_map[f] >= 0) and (pdf_font_map[k] >= 0) and 
           str_eq_str(font_name[k], font_name[f]) then
            goto done;
        incr(k);
    end;
done:
    if k <= font_ptr then begin
        pdf_font_num[f] := pdf_font_num[k];
    end
    else begin
        pdf_create_obj(obj_type_font, f);
        pdf_font_num[f] := obj_ptr;
        font_used[f] := true;
    end;
    k := divide_scaled(font_size[f], pdf_1bp, decimal_digits);
    pdf_font_size[f] := scaled_out;
end;

@ The following functions are used to create an expanded font for 
interword spacing adjusting.

@p function read_expanded_font(f: internal_font_number; e: scaled): internal_font_number;
{read font |f| expanded by |e| thousandths into font memory}
label found;
var old_setting:0..max_selector; {holds |selector| setting}
    s: str_number; {font name}
    k: internal_font_number;
begin
    old_setting:=selector; selector:=new_string;
    print(font_name[f]);
    if e > 0 then
        print("+");
    print_int(e);
    selector:=old_setting;
    s := make_string;
    for k := font_base + 1 to font_ptr do
        if str_eq_str(font_name[k], s) then begin
            if s <> font_name[k] then begin
                flush_last_string(s);
                s := font_name[k];
            end;
            if (font_dsize[k] = font_dsize[f]) and
               (font_size[k] = font_size[f]) then
                goto found;
        end;
    k := read_font_info(null_cs, s, "", font_size[f]);
found:
    read_expanded_font := k;
end;

function new_ex_font(f: internal_font_number; e: scaled): internal_font_number;
var k: internal_font_number;
begin
    k := pdf_font_link[f];
    while k <> 0 do begin
        if pdf_expand_font[k] = e then begin
            new_ex_font := k;
            return;
        end;
        k := pdf_font_link[k];
    end;
    k := read_expanded_font(f, e);
    pdf_expand_font[k] := e;
    pdf_font_link[k] := pdf_font_link[f];
    pdf_font_link[f] := k;
    new_ex_font := k;
end;

function expand_font(f: internal_font_number; e: scaled): internal_font_number;
{look up for font |f| expanded by |e| thousandths}
var negative: boolean;
    s, l: scaled;
begin
    if pdf_font_link[f] = null_font then
        confusion("expand");
    if e < 0 then begin
        e := -e;
        negative := true;
    end
    else
        negative := false;
    if negative then
        l := -pdf_expand_font[pdf_max_font_shrink[f]]
    else
        l := pdf_expand_font[pdf_max_font_stretch[f]];
    s := pdf_font_step[f];
    if e mod s <> 0 then
        e := ((2*e + s) div (2*s))*s;
    if e > l then
        e :=  l - l mod s;
    if negative then
        e := -e;
    if e = 0 then begin
        expand_font := f;
        return;
    end;
    expand_font := new_ex_font(f, e);
end;

function char_stretch_amount(f: internal_font_number; c: integer): scaled;
var k: internal_font_number;
begin
    if (pdf_max_font_stretch[f] > 0) and (ef_code(c) > 0) then begin
        k := pdf_max_font_stretch[f];
        char_stretch_amount := divide_scaled(
            (char_width(k)(char_info(k)(c)) - char_width(f)(char_info(f)(c)))
            *ef_code(c), 1000, 0);
    end
    else
        char_stretch_amount := 0;
end;

function char_shrink_amount(f: internal_font_number; c: integer): scaled;
var k: internal_font_number;
begin
    if (pdf_max_font_shrink[f] > 0) and (ef_code(c) > 0) then begin
        k := pdf_max_font_shrink[f];
        char_shrink_amount := divide_scaled(
            (char_width(f)(char_info(f)(c)) - char_width(k)(char_info(k)(c)))
            *ef_code(c), 1000, 0);
    end
    else
        char_shrink_amount := 0;
end;

@ To set \PDF{} font we use procedure |pdf_setfont|. Here we need to find out
fonts with the same name, because \TeX\ can load the same font several times
for various sizes. For such fonts we define only one font resources. The
object number of font resources of each font is held in array |pdf_font_num|.
For partial downloading we also need to hold flags indicating which charaters
in particular font are used in array |pdf_char_used|.

@p procedure pdf_print_font_tag(f: internal_font_number);
begin
    if pdf_expand_font[f] = 0 then
        return;
    if pdf_expand_font[f] > 0 then
        pdf_out("+");
    pdf_print_int(pdf_expand_font[f]);
end;

procedure pdf_setfont;
var p: pointer;
begin
    p := pdf_lookup_list(pdf_font_list, f); {first look-up for font |f| in |pdf_font_list|}
    if p = null then begin {|f| not found in |pdf_font_list|}
        pdf_append_list(f)(pdf_font_list);
        if not font_used[f] then
            pdf_create_font_obj(f);
    end;
    pdf_begin_text;
    pdf_out("/");
    pdf_print(pdf_font_prefix_str);
    pdf_print_int(f);
    pdf_print_font_tag(f);
    pdf_out(" ");
    pdf_print_bp(font_size[f]);
    pdf_print(" Tf");
    pdf_f := f;
    pdf_font_changed := true;
end;

@ We need to hold information about used characters in each font for partial 
downloading.

@<Types...@>=
chars_in_fonts=array[0..31] of eight_bits;

@ @<Glob...@>=
@!pdf_char_used: ^chars_in_fonts;
@!pdf_font_size: ^scaled; {used size of font in \PDF{} file}
@!pdf_font_num: ^integer; {mapping between internal font number in \TeX\ and
    font name defined in resources in \PDF{} file}
@!pdf_font_map: ^integer; {index in table of font mappings}
@!pdf_expand_font: ^integer; 
@!pdf_font_list: pointer; {list of used fonts in current page}
@!last_tokens_string: str_number; {the number of the most recently string
created by |tokens_to_string|}

@ @<Set init...@>=
pdf_font_list := null;
last_tokens_string := 0;

@ The following code typesets a character to PDF output.

@<Output character |c| of font |f|@>=
if pdf_font_type[f] = new_font_type then
    do_vf;
if pdf_font_type[f] = virtual_font_type then
    do_vf_packet(c)
else begin
    pdf_begin_string;
    pdf_print_char(c);
    add_char_width(char_width(f)(char_info(f)(c)));
    pdf_set_char_used(f, c);
end

@ Here we implement reading information from \.{VF} file.

@d vf_max_packet_length = 10000 {max length of character packet in \.{VF} file}

@#
@d vf_error = 61 {label to go to when an error occur}
@d do_char = 70 {label to go to typesetting a character of virtual font}
@#
@d long_char = 242 {\.{VF} command for general character packet}
@d vf_id = 202 {identifies \.{VF} files}
@d put1=133 {typeset a character}

@#
@d vf_byte == getc(vf_file) {get a byte from\.{VF} file}
@d vf_packet(#) == vf_packet_start[vf_packet_base[#] + vf_packet_end
@d vf_packet_end(#) == #]

@#
@d bad_vf(#) == begin vf_err_str := #; goto vf_error; end {go out \.{VF}
processing with an error message}
@d four_cases(#) == #,#+1,#+2,#+3

@# 
@d tmp_b0 == tmp_w.qqqq.b0
@d tmp_b1 == tmp_w.qqqq.b1
@d tmp_b2 == tmp_w.qqqq.b2
@d tmp_b3 == tmp_w.qqqq.b3
@d tmp_int == tmp_w.int

@#
@d scaled3u == {convert |tmp_b1..tmp_b3| to an unsigned scaled dimension}
(((((tmp_b3*vf_z)div@'400)+(tmp_b2*vf_z))div@'400)+(tmp_b1*vf_z))div vf_beta
@d scaled4(#) == {convert |tmp_b0..tmp_b3| to a scaled dimension}
  #:=scaled3u;
  if tmp_b0>0 then if tmp_b0=255 then # := # - vf_alpha
@d scaled3(#) == {convert |tmp_b1..tmp_b3| to a scaled dimension}
  #:=scaled3u; @+ if tmp_b1>127 then # := # - vf_alpha
@d scaled2 == {convert |tmp_b2..tmp_b3| to a scaled dimension}
  if tmp_b2>127 then tmp_b1:=255 else tmp_b1:=0;
  scaled3
@d scaled1 == {convert |tmp_b3| to a scaled dimension}
  if tmp_b3>127 then tmp_b1:=255 else tmp_b1:=0;
  tmp_b2:=tmp_b1; scaled3

@<Glob...@>=
@!vf_packet_base: ^integer; {base addresses of character
packets from virtual fonts}
@!vf_default_font: ^internal_font_number; {default font in a \.{VF} file}
@!vf_local_font_num: ^internal_font_number; {number of local fonts in a \.{VF} file}
@!vf_packet_length: integer; {length of the current packet}
@!vf_file: byte_file;
@!vf_nf: internal_font_number; {the local fonts counter}
@!vf_e_fnts: ^integer; {external font numbers}
@!vf_i_fnts: ^internal_font_number; {corresponding internal font numbers}
@!tmp_w: memory_word; {accumulator}
@!vf_z: integer; {multiplier}
@!vf_alpha: integer; {correction for negative values}
@!vf_beta: 1..16; {divisor}

@ @<Set init...@>=
vf_nf := 0;

@ The |do_vf| procedure attempts to read the \.{VF} file for a font, and sets
|pdf_font_type[f]| to |real_font_type| if the \.{VF} file could not be found
or loaded, otherwise sets |pdf_font_type[f]| to |virtual_font_type|.  At this
time, |f| is the internal font number of the current \.{TFM} font.  To process
font definitions in virtual font we call |vf_def_font|.

@p procedure vf_replace_z;
begin
    vf_alpha:=16;
    while vf_z>=@'40000000 do begin
        vf_z:=vf_z div 2;
        vf_alpha:=vf_alpha+vf_alpha;
    end;
    vf_beta:=256 div vf_alpha;
    vf_alpha:=vf_alpha*vf_z;
end;

function vf_read(k: integer): integer; {read |k| bytes as an integer from \.{VF} file}
var i: integer;
begin
    i := 0;
    while k > 0 do begin
        i := i*256 + vf_byte;
        decr(k);
    end;
    vf_read := i;
end;

procedure vf_local_font_warning(k: internal_font_number; s: str_number);
{print a warning message if an error ocurrs during processing local fonts in
\.{VF} file}
begin
    print_nl(s);
    print(" in local font ");
    print(font_name[k]);
    print(" in virtual font ");
    print(font_name[f]);
    print(".vf ignored.");
end;

function vf_def_font: internal_font_number; {process a local font in \.{VF} file}
label found;
var k: internal_font_number;
    lf_name: str_number;
    ds, s: scaled;
    cs: four_quarters;
    c: integer;
begin
    cs.b0 := vf_byte; cs.b1 := vf_byte; cs.b2 := vf_byte; cs.b3 := vf_byte;
    tmp_b0 := vf_byte; tmp_b1 := vf_byte; tmp_b2 := vf_byte; tmp_b3 := vf_byte;
    scaled4(s);
    ds := vf_read(4) div @'20;
    tmp_b0 := vf_byte;
    tmp_b1 := vf_byte;
    while tmp_b0 > 0 do begin
        decr(tmp_b0);
        if vf_byte > 0 then
            do_nothing; {skip the font path}
    end;
    str_room(tmp_b1);
    while tmp_b1 > 0 do begin
        decr(tmp_b1);
        append_char(vf_byte);
    end;
    lf_name := make_string;
    for k:=font_base+1 to font_ptr do
        if str_eq_str(font_name[k], lf_name) then begin
            if  lf_name <> font_name[k] then begin
                flush_last_string(lf_name);
                lf_name := font_name[k];
            end;
            if (font_dsize[k] = ds) and (font_size[k] = s) and
               (pdf_expand_font[k] = 0) then
                goto found;
        end;
    k := read_font_info(null_cs, lf_name, "", s);
found:
    if k <> null_font then begin
        if ((cs.b0 <> 0) or (cs.b1 <> 0) or (cs.b2 <> 0) or (cs.b3 <> 0)) and
           ((font_check[k].b0 <> 0) or (font_check[k].b1 <> 0) or 
            (font_check[k].b2 <> 0) or (font_check[k].b3 <> 0)) and
           ((cs.b0 <> font_check[k].b0) or (cs.b1 <> font_check[k].b1) or
            (cs.b2 <> font_check[k].b2) or (cs.b3 <> font_check[k].b3)) then
            vf_local_font_warning(k, "checksum mismatch");
        if ds <> font_dsize[k] then
            vf_local_font_warning(k, "design size mismatch");
    end;
    if pdf_expand_font[f] <> 0 then
        pdf_expand_font[k] := pdf_expand_font[f];
    vf_def_font := k;
end;

procedure do_vf; {process \.{VF} file with font internal number |f|}
label vf_error;
var cmd, k, n: integer;
    cc, cmd_length: integer;
    tfm_width: scaled;
    vf_err_str, s: str_number;
    stack_level: vf_stack_index;
    save_vf_nf: internal_font_number;
begin
    stack_level := 0;
    pdf_font_type[f] := real_font_type;
    @<Open |vf_file|, return if not found@>;
    @<Process the preamble@>;@/
    @<Process the font definitions@>;@/
    @<Allocate memory for the new virtual font@>;@/
    while cmd <= long_char do begin@/
        @<Build a character packet@>;@/
    end;
    if cmd <> post then
        bad_vf("POST command expected");
    b_close(vf_file);
    pdf_font_type[f] := virtual_font_type;
    return;
vf_error:
    print_nl("Error in processing VF font (");
    print(font_name[f]);
    print(".vf): ");
    print(vf_err_str);
    print(", virtual font will be ignored");
    print_ln;
    b_close(vf_file);
    update_terminal;
end;

@ @<Open |vf_file|, return if not found@>=
pack_file_name(font_name[f], "", ".vf");
if not vf_b_open_in(vf_file) then
    return

@ @<Process the preamble@>=
if vf_byte <> pre then
    bad_vf("PRE command expected");
if vf_byte <> vf_id then
    bad_vf("wrong id byte");
cmd_length := vf_byte;
for k := 1 to cmd_length do
    tmp_int := vf_byte;
tmp_b0 := vf_byte; tmp_b1 := vf_byte; tmp_b2 := vf_byte; tmp_b3 := vf_byte;
if ((tmp_b0 <> 0) or (tmp_b1 <> 0) or (tmp_b2 <> 0) or (tmp_b3 <> 0)) and
   ((font_check[f].b0 <> 0) or (font_check[f].b1 <> 0) or 
    (font_check[f].b2 <> 0) or (font_check[f].b3 <> 0)) and
   ((tmp_b0 <> font_check[f].b0) or (tmp_b1 <> font_check[f].b1) or
    (tmp_b2 <> font_check[f].b2) or (tmp_b3 <> font_check[f].b3)) then begin
    print_nl("checksum mismatch in font ");
    print(font_name[f]);
    print(".vf ignored");
end;
if vf_read(4) div @'20 <> font_dsize[f] then begin
    print_nl("design size mismatch in font ");
    print(font_name[f]);
    print(".vf ignored");
end;
update_terminal;
vf_z := font_size[f];
vf_replace_z

@ @<Process the font definitions@>=
cmd := vf_byte;
save_vf_nf := vf_nf;
while (cmd >= fnt_def1) and (cmd <= fnt_def1 + 3) do begin
    vf_e_fnts[vf_nf] := vf_read(cmd - fnt_def1 + 1);
    vf_i_fnts[vf_nf] := vf_def_font;
    incr(vf_nf);
    cmd := vf_byte;
end;
vf_default_font[f] := save_vf_nf;
vf_local_font_num[f] := vf_nf - save_vf_nf;

@ @<Allocate memory for the new virtual font@>=
    vf_packet_base[f] := new_vf_packet(f)

@ @<Build a character packet@>=
if cmd = long_char then begin
    vf_packet_length := vf_read(4);
    cc := vf_read(4);
    if (cc < 0) or (cc > 255) then
        bad_vf("character code out of range");
    tmp_b0 := vf_byte; tmp_b1 := vf_byte; tmp_b2 := vf_byte; tmp_b3 := vf_byte;
    scaled4(tfm_width);
end
else begin
    vf_packet_length := cmd;
    cc := vf_byte;
    tmp_b1 := vf_byte; tmp_b2 := vf_byte; tmp_b3 := vf_byte;
    scaled3(tfm_width);
end;
if vf_packet_length < 0 then
    bad_vf("negative packet length");
if vf_packet_length > vf_max_packet_length then
    bad_vf("packet length too long");
if (tfm_width <> char_width(f)(orig_char_info(f)(cc))) then begin
    print_nl("character width mismatch in font ");
    print(font_name[f]);
    print(".vf ignored");
end;
str_room(vf_packet_length);
while vf_packet_length > 0 do begin
    cmd := vf_byte;
    decr(vf_packet_length);
    @<Cases of \.{DVI} commands that can appear in character packet@>;
    if cmd <> nop then
        append_char(cmd);
    vf_packet_length := vf_packet_length - cmd_length;
    while cmd_length > 0 do begin
        decr(cmd_length);
        append_char(vf_byte);
    end;
end;
if stack_level <> 0 then
    bad_vf("more PUSHs than POPs in character packet");
if vf_packet_length <> 0 then
    bad_vf("invalid packet length or DVI command in packet");
@<Store the packet being built@>;
cmd := vf_byte

@ @<Store the packet being built@>=
s := make_string;
storepacket(f, cc, s);
flush_last_string(s)

@ @<Cases of \.{DVI} commands that can appear in character packet@>=
if (cmd >= set_char_0) and (cmd <= set_char_0 + 127) then
    cmd_length := 0
else if ((fnt_num_0 <= cmd) and (cmd <= fnt_num_0 + 63)) or
        ((fnt1 <= cmd) and (cmd <= fnt1 + 3)) then begin
    if cmd >= fnt1 then begin
        k := vf_read(cmd - fnt1 + 1);
        vf_packet_length := vf_packet_length - (cmd - fnt1 + 1);
    end
    else
        k := cmd - fnt_num_0;
    if k >= 256 then
        bad_vf("too many local fonts");
    n := 0;
    while (n < vf_local_font_num[f]) and
          (vf_e_fnts[vf_default_font[f] + n] <> k) do
        incr(n);
    if n = vf_local_font_num[f] then
        bad_vf("undefined local font");
    if k <= 63 then
        append_char(fnt_num_0 + k)
    else begin
        append_char(fnt1);
        append_char(k);
    end;
    cmd_length := 0;
    cmd := nop;
end
else case cmd of
set_rule, put_rule: cmd_length := 8;
four_cases(set1):   cmd_length := cmd - set1 + 1;
four_cases(put1):   cmd_length := cmd - put1 + 1;
four_cases(right1): cmd_length := cmd - right1 + 1;
four_cases(w1):     cmd_length := cmd - w1 + 1;
four_cases(x1):     cmd_length := cmd - x1 + 1;
four_cases(down1):  cmd_length := cmd - down1 + 1;
four_cases(y1):     cmd_length := cmd - y1 + 1;
four_cases(z1):     cmd_length := cmd - z1 + 1;
four_cases(xxx1):  begin
    cmd_length := vf_read(cmd - xxx1 + 1);
    vf_packet_length := vf_packet_length - (cmd - xxx1 + 1);
    if cmd_length > vf_max_packet_length then
        bad_vf("packet length too long");
    if cmd_length < 0 then
        bad_vf("string of negative length");
    append_char(xxx1);
    append_char(cmd_length);
    cmd := nop; {|cmd| has been already stored above as |xxx1|}
end;
w0, x0, y0, z0, nop:
    cmd_length := 0;
push, pop:  begin
    cmd_length := 0;
    if cmd = push then
        if stack_level = vf_stack_size then
            overflow("virtual font stack size", vf_stack_size)
        else
            incr(stack_level)
    else
        if stack_level = 0 then
            bad_vf("more POPs than PUSHs in character")
        else
            decr(stack_level);
end;
othercases
    bad_vf("improver DVI command");
endcases

@ The |do_vf_packet| procedure is called in order to interpret the
character packet for a virtual character. Such a packet may contain the
instruction to typeset a character from the same or an other virtual
font; in such cases |do_vf_packet| calls itself recursively. The
recursion level, i.e., the number of times this has happened, is kept
in the global variable |vf_cur_s| and should not exceed |vf_max_recursion|.

@<Constants...@>=
@!vf_max_recursion=10; {\.{VF} files shouldn't recurse beyond this level}
@!vf_stack_size=100; {\.{DVI} files shouldn't |push| beyond this depth}

@ @<Types...@>=
@!vf_stack_index=0..vf_stack_size; {an index into the stack}
@!vf_stack_record=record
    stack_h, stack_v, stack_w, stack_x, stack_y, stack_z: scaled;
end;

@ @<Glob...@>=
@!vf_cur_s: 0..vf_max_recursion; {current recursion level}
@!vf_stack: array [vf_stack_index] of vf_stack_record;
@!vf_stack_ptr: vf_stack_index; {pointer into |vf_stack|}

@ @<Set init...@>=
vf_cur_s := 0;
vf_stack_ptr := 0;

@ Some functions for processing character packets.

@p function packet_read(k: integer): integer; {read |k| bytes as an integer from
character packet}
var i: integer;
begin
    i := 0;
    while k > 0 do begin
        i := i*256 + packet_byte;
        decr(k);
    end;
    packet_read := i;
end;

function packet_scaled(k: integer): integer; {get |k| bytes from packet as a
scaled}
var s: scaled;
begin
    case k of
    1: begin 
        tmp_b3 := packet_byte;
        scaled1(s);
    end;
    2: begin 
        tmp_b2 := packet_byte;
        tmp_b3 := packet_byte;
        scaled2(s);
    end;
    3: begin 
        tmp_b1 := packet_byte;
        tmp_b2 := packet_byte;
        tmp_b3 := packet_byte;
        scaled3(s);
    end;
    4: begin 
        tmp_b0 := packet_byte;
        tmp_b1 := packet_byte;
        tmp_b2 := packet_byte;
        tmp_b3 := packet_byte;
        scaled4(s);
    end;
    endcases;
    packet_scaled := s;
end;

procedure do_vf_packet(c: eight_bits); {typeset the \.{DVI} commands in the
character packet for character |c| in current font |f|}
label do_char, char_done, continue;
var save_packet_ptr, save_packet_length: pool_pointer;
    save_vf, k, n: internal_font_number;
    base_line, save_h, save_v: scaled;
    cmd: integer;
    char_move: boolean;
    w, x, y, z: scaled;
    s: str_number;
begin
    incr(vf_cur_s);
    if vf_cur_s > vf_max_recursion then
        overflow("max level recursion", vf_max_recursion);
    push_packet_state;
    start_packet(f, c);
    vf_z := font_size[f];
    vf_replace_z;
    save_vf := f;
    f := vf_i_fnts[vf_default_font[save_vf]];
    save_v := cur_v;
    save_h := cur_h;
    w := 0; x := 0; y := 0; z := 0;
    while vf_packet_length > 0 do begin
        cmd := packet_byte;
        @<Do typesetting the \.{DVI} commands in virtual character packet@>;
continue:
    end;
    cur_h := save_h;
    cur_v := save_v;
    pop_packet_state;
    vf_z := font_size[f];
    vf_replace_z;
    decr(vf_cur_s);
end;

@ @<Do typesetting the \.{DVI} commands in virtual character packet@>=
if (cmd >= set_char_0) and (cmd <= set_char_0 + 127)  then begin
    if not ((font_bc[f] <= cmd) and (cmd <= font_ec[f])) then
        char_warning(f, cmd);
    c := cmd;
    char_move := true;
    goto do_char;
end 
else if ((fnt_num_0 <= cmd) and (cmd <= fnt_num_0 + 63)) or (cmd = fnt1) then begin
    if cmd = fnt1 then
        k := packet_byte
    else
        k := cmd - fnt_num_0;
    n := 0;
    while (n < vf_local_font_num[save_vf]) and
          (vf_e_fnts[vf_default_font[save_vf] + n] <> k) do
        incr(n);
    if (n = vf_local_font_num[save_vf]) then
        f := null_font
    else
        f := vf_i_fnts[vf_default_font[save_vf] + n];
end
else case cmd of
push: begin
    vf_stack[vf_stack_ptr].stack_h := cur_h;
    vf_stack[vf_stack_ptr].stack_v := cur_v;
    vf_stack[vf_stack_ptr].stack_w := w;
    vf_stack[vf_stack_ptr].stack_x := x;
    vf_stack[vf_stack_ptr].stack_y := y;
    vf_stack[vf_stack_ptr].stack_z := z;
    incr(vf_stack_ptr);
end;
pop: begin
    decr(vf_stack_ptr);
    cur_h := vf_stack[vf_stack_ptr].stack_h;
    cur_v := vf_stack[vf_stack_ptr].stack_v;
    w := vf_stack[vf_stack_ptr].stack_w;
    x := vf_stack[vf_stack_ptr].stack_x;
    y := vf_stack[vf_stack_ptr].stack_y;
    z := vf_stack[vf_stack_ptr].stack_z;
end;
four_cases(set1), four_cases(put1): begin
    if (set1 <= cmd) and (cmd <= set1 + 3) then begin
        tmp_int := packet_read(cmd - set1 + 1);
        char_move := true;
    end
    else begin
        tmp_int := packet_read(cmd - put1 + 1);
        char_move := false;
    end;
    if not ((font_bc[f] <= tmp_int) and (tmp_int <= font_ec[f])) then
        char_warning(f, tmp_int);
    c := tmp_int;
    goto do_char;
end;
set_rule, put_rule: begin
    rule_ht := packet_scaled(4);
    rule_wd := packet_scaled(4);
    if (rule_wd > 0) and (rule_ht > 0) then begin
        pdf_set_rule(cur_h, cur_v, rule_wd, rule_ht);
        if cmd = set_rule then
            cur_h := cur_h + rule_wd;
    end;
end;
four_cases(right1):
    cur_h := cur_h + packet_scaled(cmd - right1 + 1);
w0, four_cases(w1): begin
    if cmd > w0 then
        w := packet_scaled(cmd - w0);
    cur_h := cur_h + w;
end;
x0, four_cases(x1): begin
    if cmd > x0 then
        x := packet_scaled(cmd - x0);
    cur_h := cur_h + x;
end;
four_cases(down1):
    cur_v := cur_v + packet_scaled(cmd - down1 + 1);
y0, four_cases(y1): begin
    if cmd > y0 then
        y := packet_scaled(cmd - y0);
    cur_v := cur_v + y;
end;
z0, four_cases(z1): begin
    if cmd > z0 then
        z := packet_scaled(cmd - z0);
    cur_v := cur_v + z;
end;
four_cases(xxx1):  begin
    tmp_int := packet_read(cmd - xxx1 + 1);
    str_room(tmp_int);
    while tmp_int > 0 do begin
        decr(tmp_int);
        append_char(packet_byte);
    end;
    s := make_string;
    literal(s, true, true, false);
    flush_last_string(s);
end;
endcases;
goto continue;
do_char:
@<Output character |c| of font |f|@>;
if char_move then
    cur_h := cur_h + char_width(f)(orig_char_info(f)(c))


@* \[32f] PDF shipping out.
To ship out a \TeX\ box to \PDF{} page description we need to implement
|pdf_hlist_out|, |pdf_vlist_out| and |pdf_ship_out|, which are equivalent to
the \TeX' original |hlist_out|, |vlist_out| and |ship_out| resp. But first we
need to declare some procedures needed in |pdf_hlist_out| and |pdf_vlist_out|.

@ @<Glob...@>=
@!pdf_font_prefix_str: str_number; {string holding name prefix for fonts}
@!pdf_form_prefix_str: str_number; {string holding name prefix for forms}
@!pdf_image_prefix_str: str_number; {string holding name prefix for images}

@ @<Set init...@>=
pdf_font_prefix_str := "F";
pdf_form_prefix_str := "Fm";
pdf_image_prefix_str := "Im";

@ @<Declare procedures needed in |pdf_hlist_out|, |pdf_vlist_out|@>=
procedure pdf_out_literal(p:pointer); 
var old_setting:0..max_selector; {holds print |selector|}
    s: str_number;
begin
    old_setting:=selector; selector:=new_string;
    show_token_list(link(pdf_literal_data(p)),null,pool_size-pool_ptr);
    selector:=old_setting;
    s := make_string;
    if pdf_literal_direct(p) = 1 then
        literal(s, false, false, false)
    else
        literal(s, true, false, false);
    flush_last_string(s);
end;

procedure pdf_special(p: pointer);
var old_setting:0..max_selector; {holds print |selector|}
    s: str_number;
begin
    old_setting:=selector; selector:=new_string;
    show_token_list(link(write_tokens(p)),null,pool_size-pool_ptr);
    selector:=old_setting;
    s := make_string;
    literal(s, true, true, true);
    flush_last_string(s);
end;

procedure pdf_error(t, p: str_number);
begin
    print_nl("pdfTeX error (");
    print(t); print("): "); print(p);
    succumb;
end;

procedure pdf_warning(t, p: str_number);
begin
    print_nl("Warning (");
    print(t); print("): "); print(p);
end;

function tokens_to_string(p: pointer): str_number; {return a string from tokens
list}
begin
    old_setting:=selector; selector:=new_string;
    show_token_list(link(p),null,pool_size-pool_ptr);
    selector:=old_setting;
    last_tokens_string := make_string;
    tokens_to_string := last_tokens_string;
end;

procedure pdf_print_toks(p: pointer); {print tokens list |p|}
begin
    pdf_print_ln(tokens_to_string(p));
    flush_last_string(last_tokens_string);
end;

procedure pdf_print_image_attr;
begin
    if pdf_image_attr <> null then
        pdf_print_toks(pdf_image_attr);
end;

procedure pdf_print_font_id(n: integer; add_name: boolean);
begin
    if add_name and pdf_include_form_resources then
        append_resources_name(pdf_font_prefix_str, n);
    pdf_out("/");
    pdf_print(pdf_font_prefix_str);
    pdf_print_int(n);
    pdf_print_font_tag(n);
    pdf_out(" ");
    pdf_print_int(pdf_font_num[n]);
    pdf_print(" 0 R ");
end;

procedure pdf_print_form_id(n: integer; add_name: boolean);
begin
    if add_name and pdf_include_form_resources then
        append_resources_name(pdf_form_prefix_str, obj_info(n));
    pdf_out("/");
    pdf_print(pdf_form_prefix_str);
    pdf_print_int(obj_info(n));
    pdf_out(" ");
    pdf_print_int(n);
    pdf_print(" 0 R ");
end;

procedure pdf_print_image_id(n: integer; add_name: boolean);
begin
    if add_name and pdf_include_form_resources then
        append_resources_name(pdf_image_prefix_str, obj_info(n));
    pdf_out("/");
    pdf_print(pdf_image_prefix_str);
    pdf_print_int(obj_info(n));
    pdf_out(" ");
    pdf_print_int(n);
    pdf_print(" 0 R ");
end;

procedure pdf_print_raw_obj(p: pointer); {print a node representing raw PDF object}
var s: str_number;
    n: integer; 
begin
    n := pdf_obj_obj_num(p);
    s := tokens_to_string(pdf_obj_data(p));
    if pdf_obj_is_stream(p) = 1 then begin
        pdf_begin_dict(n);
        pdf_print("/Length ");
        pdf_print_int_ln(length(s) + 1);
        pdf_print_ln(">>");
        pdf_print_ln("stream");
        pdf_print_ln(s);
        pdf_print_ln("endstream");
    end
    else begin
        pdf_begin_obj(n);
        pdf_print_ln(s);
    end;
    pdf_end_obj;
    flush_last_string(s);
end;

@ Similiar to |vlist_out|, |pdf_vlist_out| needs to be declared forward

@p procedure@?pdf_vlist_out; forward;

@ The implementation of procedure |pdf_hlist_out| is similiar to |hlist_out|

@p @t\4@>@<Declare procedures needed in |pdf_hlist_out|, |pdf_vlist_out|@>@t@>@/

procedure pdf_hlist_out; {output an |hlist_node| box}
label reswitch, move_past, fin_rule, next_p, found, continue, char_done;
var base_line: scaled; {the baseline coordinate for this box}
@!left_edge: scaled; {the left coordinate for this box}
@!save_h: scaled; {what |cur_h| should pop to}
@!this_box: pointer; {pointer to containing box}
@!g_order: glue_ord; {applicable order of infinity for glue}
@!g_sign: normal..shrinking; {selects type of glue}
@!p:pointer; {current position in the hlist}
@!leader_box:pointer; {the leader box being replicated}
@!leader_wd:scaled; {width of leader box being replicated}
@!lx:scaled; {extra space between leader boxes}
@!outer_doing_leaders:boolean; {were we doing leaders?}
@!edge:scaled; {left edge of sub-box, or right edge of leader space}
begin this_box:=temp_ptr; g_order:=glue_order(this_box);
g_sign:=glue_sign(this_box); p:=list_ptr(this_box);
incr(cur_s);
base_line:=cur_v; left_edge:=cur_h;
@<Generate the \PDF{} mark (if any) for the current box in |pdf_hlist_out|@>;
while p<>null do
    @<Output node |p| for |pdf_hlist_out| and move to the next node,
    maintaining the condition |cur_v=base_line|@>;
decr(cur_s);
end;

@ @<Output node |p| for |pdf_hlist_out|...@>=
reswitch: if is_char_node(p) then
  begin
  repeat f:=font(p); c:=character(p);
  @<Output character |c| of font |f|@>;
  cur_h:=cur_h+char_width(f)(orig_char_info(f)(c));
continue:
  p:=link(p);
  until not is_char_node(p);
  end
else @<Output the non-|char_node| |p| for |pdf_hlist_out|
    and move to the next node@>

@ @<Output the non-|char_node| |p| for |pdf_hlist_out|...@>=
begin case type(p) of
hlist_node,vlist_node:@<(in |pdf_hlist_out|)Output a box in an hlist@>;
rule_node: begin rule_ht:=height(p); rule_dp:=depth(p); rule_wd:=width(p);
  goto fin_rule;
  end;
whatsit_node: @<Output the whatsit node |p| in |pdf_hlist_out|@>;
glue_node: @<(in |pdf_hlist_out|)Move right or output leaders@>;
kern_node,math_node:cur_h:=cur_h+width(p);
ligature_node: @<Make node |p| look like a |char_node| and |goto reswitch|@>;
othercases do_nothing
endcases;@/
goto next_p;
fin_rule: @<(in |pdf_hlist_out|)Output a rule in an hlist@>;
move_past: cur_h:=cur_h+rule_wd;
next_p:p:=link(p);
end

@ @<(in |pdf_hlist_out|)Output a box in an hlist@>=
if list_ptr(p)=null then cur_h:=cur_h+width(p)
else  begin
  cur_v:=base_line+shift_amount(p); {shift the box down}
  temp_ptr:=p; edge:=cur_h;
  if type(p)=vlist_node then pdf_vlist_out@+else pdf_hlist_out;
  cur_h:=edge+width(p); cur_v:=base_line;
  end

@ @<(in |pdf_hlist_out|)Output a rule in an hlist@>=
if is_running(rule_ht) then rule_ht:=height(this_box);
if is_running(rule_dp) then rule_dp:=depth(this_box);
rule_ht:=rule_ht+rule_dp; {this is the rule thickness}
if (rule_ht>0)and(rule_wd>0) then {we don't output empty rules}
  begin cur_v:=base_line+rule_dp;
  pdf_set_rule(cur_h, cur_v, rule_wd, rule_ht);
  cur_v:=base_line;
  end

@ @<(in |pdf_hlist_out|)Move right or output leaders@>=
begin g:=glue_ptr(p); rule_wd:=width(g);
if g_sign<>normal then
  begin if g_sign=stretching then
    begin if stretch_order(g)=g_order then
      rule_wd:=rule_wd+round(float(glue_set(this_box))*stretch(g));
@^real multiplication@>
    end
  else  begin if shrink_order(g)=g_order then
      rule_wd:=rule_wd-round(float(glue_set(this_box))*shrink(g));
    end;
  end;
if subtype(p)>=a_leaders then
  @<(in |pdf_hlist_out|)Output leaders in an hlist, |goto fin_rule| if a rule
    or to |next_p| if done@>;
goto move_past;
end

@ @<(in |pdf_hlist_out|)Output leaders in an hlist...@>=
begin leader_box:=leader_ptr(p);
if type(leader_box)=rule_node then
  begin rule_ht:=height(leader_box); rule_dp:=depth(leader_box);
  goto fin_rule;
  end;
leader_wd:=width(leader_box);
if (leader_wd>0)and(rule_wd>0) then
  begin rule_wd:=rule_wd+10; {compensate for floating-point rounding}
  edge:=cur_h+rule_wd; lx:=0;
  @<(in |pdf_hlist_out|)Let |cur_h| be the position of the first box, and set |leader_wd+lx|
    to the spacing between corresponding parts of boxes@>;
  while cur_h+leader_wd<=edge do
    @<(in |pdf_hlist_out|)Output a leader box at |cur_h|,
      then advance |cur_h| by |leader_wd+lx|@>;
  cur_h:=edge-10; goto next_p;
  end;
end

@ @<(in |pdf_hlist_out|)Let |cur_h| be the position of the first box, ...@>=
if subtype(p)=a_leaders then
  begin save_h:=cur_h;
  cur_h:=left_edge+leader_wd*((cur_h-left_edge)div leader_wd);
  if cur_h<save_h then cur_h:=cur_h+leader_wd;
  end
else  begin lq:=rule_wd div leader_wd; {the number of box copies}
  lr:=rule_wd mod leader_wd; {the remaining space}
  if subtype(p)=c_leaders then cur_h:=cur_h+(lr div 2)
  else  begin lx:=(2*lr+lq+1) div (2*lq+2); {round|(lr/(lq+1))|}
    cur_h:=cur_h+((lr-(lq-1)*lx) div 2);
    end;
  end

@ @<(in |pdf_hlist_out|)Output a leader box at |cur_h|, ...@>=
begin cur_v:=base_line+shift_amount(leader_box);@/
save_h:=cur_h; temp_ptr:=leader_box;
outer_doing_leaders:=doing_leaders; doing_leaders:=true;
if type(leader_box)=vlist_node then pdf_vlist_out@+else pdf_hlist_out;
doing_leaders:=outer_doing_leaders;
cur_v:=base_line;
cur_h:=save_h+leader_wd+lx;
end

@ The |pdf_vlist_out| routine is similar to |pdf_hlist_out|, but a bit simpler.
@p procedure pdf_vlist_out; {output a |pdf_vlist_node| box}
label move_past, fin_rule, next_p;
var left_edge: scaled; {the left coordinate for this box}
@!top_edge: scaled; {the top coordinate for this box}
@!save_v: scaled; {what |cur_v| should pop to}
@!this_box: pointer; {pointer to containing box}
@!g_order: glue_ord; {applicable order of infinity for glue}
@!g_sign: normal..shrinking; {selects type of glue}
@!p:pointer; {current position in the vlist}
@!leader_box:pointer; {the leader box being replicated}
@!leader_ht:scaled; {height of leader box being replicated}
@!lx:scaled; {extra space between leader boxes}
@!outer_doing_leaders:boolean; {were we doing leaders?}
@!edge:scaled; {bottom boundary of leader space}
begin this_box:=temp_ptr; g_order:=glue_order(this_box);
g_sign:=glue_sign(this_box); p:=list_ptr(this_box);
incr(cur_s);
left_edge:=cur_h; cur_v:=cur_v-height(this_box); top_edge:=cur_v;
@<Generate the \PDF{} mark (if any) for the current box in |pdf_vlist_out|@>;
while p<>null do
    @<Output node |p| for |pdf_vlist_out| and move to the next node,
    maintaining the condition |cur_h=left_edge|@>;
decr(cur_s);
end;

@ @<Output node |p| for |pdf_vlist_out|...@>=
begin if is_char_node(p) then confusion("pdfvlistout")
@:this can't happen pdfvlistout}{\quad pdfvlistout@>
else @<Output the non-|char_node| |p| for |pdf_vlist_out|@>;
next_p:p:=link(p);
end

@ @<Output the non-|char_node| |p| for |pdf_vlist_out|@>=
begin case type(p) of
hlist_node,vlist_node:@<(in |pdf_vlist_out|)Output a box in a vlist@>;
rule_node: begin rule_ht:=height(p); rule_dp:=depth(p); rule_wd:=width(p);
  goto fin_rule;
  end;
whatsit_node: @<Output the whatsit node |p| in |pdf_vlist_out|@>;
glue_node: @<(in |pdf_vlist_out|)Move down or output leaders@>;
kern_node:cur_v:=cur_v+width(p);
othercases do_nothing
endcases;@/
goto next_p;
fin_rule: @<(in |pdf_vlist_out|)Output a rule in a vlist, |goto next_p|@>;
move_past: cur_v:=cur_v+rule_ht;
end

@ @<(in |pdf_vlist_out|)Output a box in a vlist@>=
if list_ptr(p)=null then cur_v:=cur_v+height(p)+depth(p)
else  begin cur_v:=cur_v+height(p); save_v:=cur_v;
  cur_h:=left_edge+shift_amount(p); {shift the box right}
  temp_ptr:=p;
  if type(p)=vlist_node then pdf_vlist_out@+else pdf_hlist_out;
  cur_v:=save_v+depth(p); cur_h:=left_edge;
  end

@ @<(in |pdf_vlist_out|)Output a rule in a vlist...@>=
if is_running(rule_wd) then rule_wd:=width(this_box);
rule_ht:=rule_ht+rule_dp; {this is the rule thickness}
cur_v:=cur_v+rule_ht;
if (rule_ht>0)and(rule_wd>0) then {we don't output empty rules}
  pdf_set_rule(cur_h, cur_v, rule_wd, rule_ht);
goto next_p

@ @<(in |pdf_vlist_out|)Move down or output leaders@>=
begin g:=glue_ptr(p); rule_ht:=width(g);
if g_sign<>normal then
  begin if g_sign=stretching then
    begin if stretch_order(g)=g_order then
      rule_ht:=rule_ht+round(float(glue_set(this_box))*stretch(g));
@^real multiplication@>
    end
  else  begin if shrink_order(g)=g_order then
      rule_ht:=rule_ht-round(float(glue_set(this_box))*shrink(g));
    end;
  end;
if subtype(p)>=a_leaders then
  @<(in |pdf_vlist_out|)Output leaders in a vlist, |goto fin_rule| if a rule
    or to |next_p| if done@>;
goto move_past;
end

@ @<(in |pdf_vlist_out|)Output leaders in a vlist...@>=
begin leader_box:=leader_ptr(p);
if type(leader_box)=rule_node then
  begin rule_wd:=width(leader_box); rule_dp:=0;
  goto fin_rule;
  end;
leader_ht:=height(leader_box)+depth(leader_box);
if (leader_ht>0)and(rule_ht>0) then
  begin rule_ht:=rule_ht+10; {compensate for floating-point rounding}
  edge:=cur_v+rule_ht; lx:=0;
  @<(in |pdf_vlist_out|)Let |cur_v| be the position of the first box, and set |leader_ht+lx|
    to the spacing between corresponding parts of boxes@>;
  while cur_v+leader_ht<=edge do
    @<(in |pdf_vlist_out|)Output a leader box at |cur_v|,
      then advance |cur_v| by |leader_ht+lx|@>;
  cur_v:=edge-10; goto next_p;
  end;
end

@ @<(in |pdf_vlist_out|)Let |cur_v| be the position of the first box, ...@>=
if subtype(p)=a_leaders then
  begin save_v:=cur_v;
  cur_v:=top_edge+leader_ht*((cur_v-top_edge)div leader_ht);
  if cur_v<save_v then cur_v:=cur_v+leader_ht;
  end
else  begin lq:=rule_ht div leader_ht; {the number of box copies}
  lr:=rule_ht mod leader_ht; {the remaining space}
  if subtype(p)=c_leaders then cur_v:=cur_v+(lr div 2)
  else  begin lx:=(2*lr+lq+1) div (2*lq+2); {round|(lr/(lq+1))|}
    cur_v:=cur_v+((lr-(lq-1)*lx) div 2);
    end;
  end

@ @<(in |pdf_vlist_out|)Output a leader box at |cur_v|, ...@>=
begin cur_h:=left_edge+shift_amount(leader_box);@/
cur_v:=cur_v+height(leader_box); save_v:=cur_v;
temp_ptr:=leader_box;
outer_doing_leaders:=doing_leaders; doing_leaders:=true;
if type(leader_box)=vlist_node then pdf_vlist_out@+else pdf_hlist_out;
doing_leaders:=outer_doing_leaders;
cur_h:=left_edge;
cur_v:=save_v-height(leader_box)+leader_ht+lx;
end

@ |pdf_ship_out| is used instead of |ship_out| to shipout a box to \PDF{}
output. If |shipping_page| is not set then the output will be a Form object,
otherwise it will be a Page object.

@p procedure pdf_ship_out(p: pointer; shipping_page: boolean); {output the box |p|}
label done, done1;
var i,j,k:integer; {general purpose accumulators}
r: integer; {accumulator to copy node for pending link annotation}
save_font_list: pointer; {to save |pdf_font_list| during flushing pending forms}
save_image_list: pointer; {to save |pdf_image_list| during flushing pending forms}
save_form_list: pointer; {to save |pdf_form_list| during flushing pending forms}
@!pdf_last_resources: integer; {pointer to most recently generated Resources object}
@!s:str_number;
@!old_setting:0..max_selector; {saved |selector| setting}
begin if tracing_output>0 then
  begin print_nl(""); print_ln;
  print("Completed box being shipped out");
@.Completed box...@>
  end;
if shipping_page then begin
    if term_offset>max_print_line-9 then print_ln
    else if (term_offset>0)or(file_offset>0) then print_char(" ");
    print_char("["); j:=9;
    while (count(j)=0)and(j>0) do decr(j);
    for k:=0 to j do
      begin print_int(count(k));
      if k<j then print_char(".");
      end;
    update_terminal;
end;
if tracing_output>0 then
  begin if shipping_page then print_char("]");
  begin_diagnostic; show_box(p); end_diagnostic(true);
  end;
@<(in |pdf_ship_out|)Ship box |p| out@>;
if (tracing_output<=0) and shipping_page then print_char("]");
dead_cycles:=0;
update_terminal; {progress report}
@<(in |pdf_ship_out|)Flush the box from memory, showing statistics if requested@>;
end;

@ @<(in |pdf_ship_out|)Flush the box from memory, showing statistics if requested@>=
@!stat if tracing_stats>1 then
  begin print_nl("Memory usage before: ");
@.Memory usage...@>
  print_int(var_used); print_char("&");
  print_int(dyn_used); print_char(";");
  end;
tats@/
flush_node_list(p);
@!stat if tracing_stats>1 then
  begin print(" after: ");
  print_int(var_used); print_char("&");
  print_int(dyn_used); print("; still untouched: ");
  print_int(hi_mem_min-lo_mem_max-1); print_ln;
  end;
tats

@ @<(in |pdf_ship_out|)Ship box |p| out@>=
@<(in |pdf_ship_out|)Update the values of |max_h| and |max_v|; but if the page is too large,
  |goto done|@>;
@<Initialize variables as |pdf_ship_out| begins@>;
if type(p)=vlist_node then pdf_vlist_out@+else pdf_hlist_out;
if shipping_page then
    incr(total_pages);
cur_s:=-1;
@<Finish shipping@>;
done:

@ @<Initialize variables as |pdf_ship_out| begins@>=
temp_ptr:=p;
zip_write_state := no_zip;
pdf_image_b := 0;
pdf_image_c := 0;
pdf_image_i := 0;
pdf_text := 0;
ensure_pdf_open;
pdf_create_obj(obj_type_others, 0);
pdf_last_resources := obj_ptr;
if not shipping_page then begin
    pdf_form_width := width(p);
    pdf_form_height := height(p);
    pdf_form_depth := depth(p);
    pdf_begin_dict(cur_form);
    pdf_last_stream := cur_form;
    cur_v := height(p);
    cur_h := 0;
    pdf_origin_h := 0;
    pdf_origin_v := pdf_form_height + pdf_form_depth;
end
else begin
    @<Calculate page dimensions and margins@>;
    pdf_last_page := get_obj(obj_type_page, total_pages + 1, false);
    obj_int(pdf_last_page) := 1; {mark that this page is valid}
    pdf_new_dict(obj_type_others, 0);
    pdf_last_stream := obj_ptr;
    cur_h := cur_h_offset;
    cur_v := height(p) + cur_v_offset;
    pdf_origin_h := 0;
    pdf_origin_v := cur_page_height;
end;
if not shipping_page then begin
    @<Write out Form stream header@>;
end;
@<Start stream of page/form contents@>

@ @<Calculate page dimensions and margins@>=
prepare_mag;
cur_h_offset := pdf_h_origin + h_offset;
cur_v_offset := pdf_v_origin + v_offset;
if pdf_page_width <> 0 then
    cur_page_width := pdf_page_width
else
    cur_page_width := width(p) + 2*cur_h_offset;
if pdf_page_height <> 0 then
    cur_page_height := pdf_page_height
else
    cur_page_height := height(p) + depth(p) + 2*cur_v_offset

@ Here we write out the header for Form.

@<Write out Form stream header@>=
pdf_print_ln("/Type /XObject");
pdf_print_ln("/Subtype /Form");
if pdf_form_attr <> null then
    pdf_print_toks(pdf_form_attr);
pdf_print("/BBox [");
pdf_print("-1 -1 ");
pdf_print_bp(pdf_form_width + pdf_1bp); pdf_out(" ");
pdf_print_bp(pdf_form_height + pdf_form_depth + pdf_1bp); pdf_print_ln("]");
pdf_print_ln("/FormType 1");
pdf_print_ln("/Matrix [1 0 0 1 0 0]");
pdf_indirect_ln("Resources", pdf_last_resources)

@ @<Start stream of page/form contents@>=
pdf_begin_stream;
if shipping_page then begin
    @<Adjust tranformation matrix for the magnification ratio@>;
end

@ @<Adjust tranformation matrix for the magnification ratio@>=
prepare_mag;
if (mag <> 1000) and (mag <> 0) then begin
    pdf_print_real(mag, 3);
    pdf_print(" 0 0 ");
    pdf_print_real(mag, 3);
    pdf_print_ln(" 0 0 cm");
end

@ @<(in |pdf_ship_out|)Update the values of |max_h| and |max_v|; but if the page is too large...@>=
if (height(p)>max_dimen)or@|(depth(p)>max_dimen)or@|
   (height(p)+depth(p)+v_offset>max_dimen)or@|
   (width(p)+h_offset>max_dimen) then
  begin print_err("Huge page cannot be shipped out");
@.Huge page...@>
  help2("The page just created is more than 18 feet tall or")@/
   ("more than 18 feet wide, so I suspect something went wrong.");
  error;
  if tracing_output<=0 then
    begin begin_diagnostic;
    print_nl("The following box has been deleted:");
@.The following...deleted@>
    show_box(p);
    end_diagnostic(true);
    end;
  goto done;
  end;
if height(p)+depth(p)+v_offset>max_v then max_v:=height(p)+depth(p)+v_offset;
if width(p)+h_offset>max_h then max_h:=width(p)+h_offset

@ @<Finish shipping@>=
@<Finish stream of page/form contents@>;
if shipping_page then begin
    @<Write out page object@>;
end;
@<Flush out pending images@>;
@<Flush out pending forms@>;
if shipping_page then begin
    @<Flush out pending \PDF{} marks except outline entries and article threads@>;
end;
@<Write out resources dictionary@>;
@<Reset all resource lists@>;
if shipping_page then begin
    @<Reset all mark lists@>;
end

@ @<Finish stream of page/form contents@>=
pdf_end_text;
pdf_end_stream

@ @<Write out resources dictionary@>=
pdf_begin_dict(pdf_last_resources);
@<Generate font resources@>;
@<Generate XObject resources@>;
@<Generate ProcSet@>;
@<Generate other resources@>;
pdf_end_dict

@ In the end of shipping out a page we reset all the lists holding objects 
have been created during the page shipping.

@d reset_list(#) == {flush a single-word linked list and reset the head
pointer}
if # <> null then begin
    flush_list(#);
    # := null;
end

@d test_extra_list(#) == 
((pdf_include_form_resources > 0) and shipping_page and (# <> null))

@d test_lists(#) == (# <> null) or test_extra_list

@<Reset all resource lists@>=
if pdf_include_form_resources > 0 then begin
    if shipping_page then begin
        reset_list(pdf_form_font_list);
        reset_list(pdf_form_image_list);
        reset_list(pdf_form_form_list);
    end
    else begin
        @<Append form resources lists@>;
    end;
end;
reset_list(pdf_font_list);
reset_list(pdf_image_list);
reset_list(pdf_form_list);
if (pdf_include_form_resources > 0) and shipping_page then
    delete_resources_names

@ @<Reset all mark lists@>=
reset_list(pdf_obj_list);
reset_list(pdf_annot_list);
reset_list(pdf_annot_link_list);
reset_list(pdf_dest_list);
reset_list(pdf_bead_list)

@ @<Append form resources lists@>=
k := pdf_font_list;
while k <> null do begin
    if pdf_lookup_list(pdf_form_font_list, info(k)) = null then
        pdf_append_list(info(k))(pdf_form_font_list);
    k := link(k);
end;
k := pdf_form_list;
while k <> null do begin
    if pdf_lookup_list(pdf_form_form_list, info(k)) = null then
        pdf_append_list(info(k))(pdf_form_form_list);
    k := link(k);
end;
k := pdf_image_list;
while k <> null do begin
    if pdf_lookup_list(pdf_form_image_list, info(k)) = null then
        pdf_append_list(info(k))(pdf_form_image_list);
    k := link(k);
end

@ @<Generate font resources@>=
if test_lists(pdf_font_list)(pdf_form_font_list) then begin
    pdf_print("/Font << ");
    k := pdf_font_list;
    while k <> null do begin
        pdf_print_font_id(info(k), true);
        k := link(k);
    end;
    if test_extra_list(pdf_form_font_list) then begin
        k := pdf_form_font_list;
        while k <> null do begin
            if pdf_lookup_list(pdf_font_list, info(k)) = null then
                pdf_print_font_id(info(k), false);
            k := link(k);
        end;
    end;
    if pdf_include_form_resources > 0 then
        print_extra_fonts;
    pdf_print_ln(">>");
    pdf_text := 1;
end

@ @<Generate XObject resources@>=
if (test_lists(pdf_form_list)(pdf_form_form_list)) or
    (test_lists(pdf_image_list)(pdf_form_image_list)) then begin
    pdf_print("/XObject << ");
    k := pdf_form_list;
    while k <> null do begin
        pdf_print_form_id(info(k), true);
        k := link(k);
    end;
    if test_extra_list(pdf_form_form_list) then begin
        k := pdf_form_form_list;
        while k <> null do begin
            if pdf_lookup_list(pdf_form_list, info(k)) = null then
                pdf_print_form_id(info(k), false);
            k := link(k);
        end;
    end;
    k := pdf_image_list;
    while k <> null do begin
        pdf_print_image_id(info(k), true);
        k := link(k);
    end;
    if test_extra_list(pdf_form_image_list) then begin
        k := pdf_form_image_list;
        while k <> null do begin
            if pdf_lookup_list(pdf_image_list, info(k)) = null then
                pdf_print_image_id(info(k), false);
            k := link(k);
        end;
    end;
    if pdf_include_form_resources > 0 then
        print_extra_xobjects;
    pdf_print_ln(">>");
end

@ @<Generate ProcSet@>=
pdf_print("/ProcSet [ /PDF");
if pdf_text = 1 then
    pdf_print(" /Text");
if pdf_image_b = 1 then
    pdf_print(" /ImageB");
if pdf_image_c = 1 then
    pdf_print(" /ImageC");
if pdf_image_i = 1 then
    pdf_print(" /ImageI");
pdf_print_ln(" ]")

@ @<Generate other resources@>=
if pdf_include_form_resources > 0 then
    print_other_resources

@ @<Write out page object@>=
pdf_begin_dict(pdf_last_page);
pdf_print_ln("/Type /Page");
pdf_indirect_ln("Contents", pdf_last_stream);
pdf_indirect_ln("Resources", pdf_last_resources);
pdf_print("/MediaBox [0 0 ");
pdf_print_mag_bp(cur_page_width); pdf_out(" ");
pdf_print_mag_bp(cur_page_height);
pdf_print_ln("]");
if pdf_page_attr <> null then
    pdf_print_toks(pdf_page_attr);
@<Generate parent pages object@>;
@<Generate array of annotations or beads in page@>;
pdf_end_dict

@ @<Generate parent pages object@>=
if total_pages mod pages_tree_kids_max = 1 then begin
    pdf_create_obj(obj_type_pages, pages_tree_kids_max);
    pdf_last_pages := obj_ptr;
end;
pdf_indirect_ln("Parent", pdf_last_pages)

@ @<Generate array of annotations or beads in page@>=
if (pdf_annot_list <> null) or (pdf_annot_link_list <> null) then begin
    pdf_print("/Annots [ ");
    k := pdf_annot_list;
    while k <> null do begin
        pdf_print_int(info(k));
        pdf_print(" 0 R ");
        k := link(k);
    end;
    k := pdf_annot_link_list;
    while k <> null do begin
        pdf_print_int(info(k));
        pdf_print(" 0 R ");
        k := link(k);
    end;
    pdf_print_ln("]");
end;
if pdf_bead_list <> null then begin
    k := pdf_bead_list;
    pdf_print("/B [ ");
    while k <> null do begin
        pdf_print_int(info(k));
        pdf_print(" 0 R ");
        k := link(k);
    end;
    pdf_print_ln("]");
end

@ @<Flush out pending images@>=
if pdf_image_list <> null then begin
    k := pdf_image_list;
    while k <> null do begin
        i := obj_image_ptr(info(k));
        write_img(info(k), pdf_image_info(i));
        k := link(k);
    end;
end

@ We need to reset and restore the resources lists during shipping out forms

@d pdf_reset_lists ==
    save_font_list := pdf_font_list;
    save_image_list := pdf_image_list;
    save_form_list := pdf_form_list;
    pdf_font_list := null;
    pdf_form_list := null;
    pdf_image_list := null

@d pdf_restore_lists == 
    pdf_font_list := save_font_list;
    pdf_form_list := save_form_list;
    pdf_image_list := save_image_list

@<Flush out pending forms@>=
if pdf_form_list <> null then begin
    k := pdf_form_list;
    while k <> null do begin
        i := info(k);
        if obj_form_box(i) <> null then begin
            cur_form := i;
            pdf_reset_lists;
            pdf_ship_out(obj_form_box(i), false);
            pdf_restore_lists;
            obj_form_box(i) := null;
        end;
        k := link(k);
    end;
end

@ @<Flush out pending \PDF{} marks...@>=
pdf_origin_h := 0;
pdf_origin_v := cur_page_height;
@<Flush out \PDF{} raw objects@>;
@<Flush out \PDF{} annotations@>;
@<Flush out \PDF{} link annotations@>;
@<Flush out \PDF{} mark destinations@>;
@<Flush out \PDF{} bead rectangle specifications@>

@ @<Flush out \PDF{} raw objects@>=
if pdf_obj_list <> null then begin
    k := pdf_obj_list;
    while k <> null do begin
        pdf_print_raw_obj(obj_obj_ptr(info(k)));
        k := link(k);
    end;
end

@ @<Flush out \PDF{} annotations@>=
if pdf_annot_list <> null then begin
    k := pdf_annot_list;
    while k <> null do begin
        i := obj_annot_ptr(info(k)); {|i| points to |pdf_annot_node|}
        pdf_begin_dict(info(k));
        pdf_print_ln("/Type /Annot");
        pdf_print_toks(pdf_annot_data(i));
        pdf_rectangle(pdf_left(i), pdf_top(i), pdf_right(i), pdf_bottom(i));
        pdf_end_dict;
        k := link(k);
    end;
end

@ For pending list of link annotations we should be careful that
there may be a multi-page annotation. If yes then we must hold a copy of
node corresponding to |last_annot| in |pdf_pending_annot_link| before
destroying the list.

@<Flush out \PDF{} link annotations@>=
if pdf_annot_link_list <> null then begin
    @<Write out \PDF{} link annotations@>;
    @<Free \PDF{} link annotations@>
end

@ @<Write out \PDF{} link annotations@>=
k := pdf_annot_link_list;
while k <> null do begin
    i := obj_annot_ptr(info(k));
    pdf_begin_dict(info(k));
    pdf_print_ln("/Type /Annot");
    if pdf_annot_link_attr(i) <> null then
        pdf_print_toks(pdf_annot_link_attr(i));
    pdf_rectangle(pdf_left(i), pdf_top(i), pdf_right(i), pdf_bottom(i));
    if pdf_action_type(pdf_annot_link_action(i)) <> pdf_action_user
    then begin
        pdf_print_ln("/Subtype /Link");
        pdf_print("/A ");
    end;
    write_action(pdf_annot_link_action(i));
    pdf_end_dict;
    k := link(k);
end

@ @<Free \PDF{} link annotations@>=
if link_box_depth <> -1 then {multi-page link annotation, copy the pending
annotation to |pdf_pending_annot_link|}
begin
    i := obj_annot_ptr(last_annot);
    copy_annot_link_node(i); {make a copy of |i| and store it in |r|}
    pdf_pending_annot_link := r;
    info(r) := null; {this is not a whatsit node}
end;
k := pdf_annot_link_list;
while k <> null do begin
    i := obj_annot_ptr(info(k));
    if info(i) = null then {not a whatsit node, so it must be destroyed here}
        delete_annot_link_node(i);
    k := link(k);
end;

@ @<Flush out \PDF{} mark destinations@>=
if pdf_dest_list <> null then begin
    k := pdf_dest_list;
    while k <> null do begin
        i := obj_dest_ptr(info(k));
        if pdf_dest_id_type(i) = pdf_id_num then
            pdf_begin_obj(info(k))
        else begin
            pdf_begin_dict(info(k));
            pdf_print("/D ");
        end;
        pdf_out("["); pdf_print_int(pdf_last_page); pdf_print(" 0 R ");
        case pdf_dest_type(i) of
        pdf_dest_xyz: begin
            pdf_print("/XYZ ");
            pdf_print_mag_bp(pdf_x(pdf_left(i))); pdf_out(" ");
            pdf_print_mag_bp(pdf_y(pdf_top(i))); pdf_out(" ");
            if pdf_dest_xyz_zoom(i) = null then
                pdf_print("null")
            else begin
                pdf_print_int(pdf_dest_xyz_zoom(i) div 1000);
                pdf_out(".");
                pdf_print_int((pdf_dest_xyz_zoom(i) mod 1000));
            end;
        end;
        pdf_dest_fit:
            pdf_print("/Fit");
        pdf_dest_fit_h: begin
            pdf_print("/FitH ");
            pdf_print_mag_bp(pdf_y(pdf_top(i)));
        end;
        pdf_dest_fit_v: begin
            pdf_print("/FitV ");
            pdf_print_mag_bp(pdf_x(pdf_left(i)));
        end;
        pdf_dest_fitb:
            pdf_print("/FitB");
        pdf_dest_fitb_h: begin
            pdf_print("/FitBH ");
            pdf_print_mag_bp(pdf_y(pdf_top(i)));
        end;
        pdf_dest_fitb_v: begin
            pdf_print("/FitBV ");
            pdf_print_mag_bp(pdf_x(pdf_left(i)));
        end;
        endcases;
        pdf_print_ln("]");
        if pdf_dest_id_type(i) = pdf_id_num then
            pdf_end_obj
        else
            pdf_end_dict;
        k := link(k);
    end;
end

@ @<Flush out \PDF{} bead rectangle specifications@>=
if pdf_bead_list <> null then begin
    k := pdf_bead_list;
    while k <> null do begin
        pdf_new_obj(obj_type_others, 0);
        i := obj_bead_rect(info(k));
        pdf_out("[");
        pdf_print_mag_bp(pdf_x(pdf_left(i) - pdf_thread_hoffset));
        pdf_out(" ");
        pdf_print_mag_bp(pdf_y(pdf_bottom(i) + pdf_thread_voffset));
        pdf_out(" ");
        pdf_print_mag_bp(pdf_x(pdf_right(i) + pdf_thread_hoffset));
        pdf_out(" ");
        pdf_print_mag_bp(pdf_y(pdf_top(i) - pdf_thread_voffset));
        pdf_print_ln("]");
        pdf_end_obj;
        obj_bead_rect(info(k)) := obj_ptr;
        free_node(i, pdf_bead_rect_node_size);
        k := link(k);
    end;
end

@ In the end we must flush \PDF{} objects that cannot be written out
immediately after shipping out pages.

@ @<Output outlines@>=
if first_outline <> 0 then begin
    pdf_new_dict(obj_type_others, 0);
    outlines := obj_ptr;
    l := first_outline; k := 0;
    repeat
        incr(k);
        a := open_subentries(l);
        if obj_outline_count(l) > 0 then
            k := k + a;
        obj_outline_parent(l) := obj_ptr;
        l := obj_outline_next(l);
    until l = 0;
    pdf_print_ln("/Type /Outlines");
    pdf_indirect_ln("First", first_outline);
    pdf_indirect_ln("Last", last_outline);
    pdf_int_entry_ln("Count", k);
    pdf_end_dict;
    @<Output \PDF{} outline entries@>;
end
else
    outlines := 0

@ @<Output \PDF{} outline entries@>=
k := head_tab[obj_type_outline];
while k <> 0 do begin
    if obj_outline_parent(k) = parent_outline then begin
        if obj_outline_prev(k) = 0 then
            first_outline := k;
        if obj_outline_next(k) = 0 then
            last_outline := k;
    end;
    pdf_begin_dict(k);
    pdf_indirect_ln("Title", obj_outline_title(k));
    pdf_indirect_ln("A", obj_outline_action_obj_num(k));
    if obj_outline_parent(k) <> 0 then
        pdf_indirect_ln("Parent", obj_outline_parent(k));
    if obj_outline_prev(k) <> 0 then
        pdf_indirect_ln("Prev", obj_outline_prev(k));
    if obj_outline_next(k) <> 0 then
        pdf_indirect_ln("Next", obj_outline_next(k));
    if obj_outline_first(k) <> 0 then
        pdf_indirect_ln("First", obj_outline_first(k));
    if obj_outline_last(k) <> 0 then
        pdf_indirect_ln("Last", obj_outline_last(k));
    if obj_outline_count(k) <> 0 then
        pdf_int_entry_ln("Count", obj_outline_count(k));
    pdf_end_dict;
    k := obj_link(k);
end

@ @<Output article threads@>=
if (head_tab[obj_type_thread_num] <> 0) or (head_tab[obj_type_thread_name] <> 0) then begin
    pdf_new_obj(obj_type_others, 0);
    pdf_out("[");
    k := head_tab[obj_type_thread_num];
    while k <> 0 do begin
        pdf_print_int(k);
        pdf_print(" 0 R ");
        k := obj_link(k);
    end;
    k := head_tab[obj_type_thread_name];
    while k <> 0 do begin
        pdf_print_int(k);
        pdf_print(" 0 R ");
        k := obj_link(k);
    end;
    pdf_remove_last_space;
    pdf_print_ln("]");
    pdf_end_obj;
    threads := obj_ptr;
    k := head_tab[obj_type_thread_num];
    while k <> 0 do begin
        if obj_thread_first(k) = 0 then begin
            print_err("Thread ");
            print_int(obj_info(k));
            print(" has been refered to but not created");
            print_ln;
        end
        else begin
            pdf_save_offset := pdf_offset; {we can't use |pdf_begin_dict|
            because it rewrites the second memory word of this entry}
            pdf_print_int(k);
            pdf_print_ln(" 0 obj <<");
            pdf_indirect_ln("F", obj_thread_first(k));
            pdf_end_dict;
            @<Output the bead list of this thread@>;
            obj_offset(k) := pdf_save_offset; {write the byte offset after we
            write out all data}
        end;
        k := obj_link(k);
    end;
    k := head_tab[obj_type_thread_name];
    while k <> 0 do begin
        if obj_thread_first(k) = 0 then begin
            print_err("Thread ");
            print(obj_info(k));
            print(" has been refered to but not created");
            print_ln;
        end
        else begin
            pdf_save_offset := pdf_offset; {we can't use |pdf_begin_dict|
            because it rewrites the second memory word of this entry}
            pdf_print_int(k);
            pdf_print_ln(" 0 obj <<");
            pdf_print("/I << /Title "); 
            pdf_print_str(obj_info(k));
            pdf_print_ln(" >>");
            pdf_indirect_ln("F", obj_thread_first(k));
            pdf_end_dict;
            @<Output the bead list of this thread@>;
            obj_offset(k) := pdf_save_offset; {write the byte offset after we
            write out all data}
        end;
        k := obj_link(k);
    end;
end
else
    threads := 0

@ @<Output the bead list of this thread@>=
l := obj_thread_first(k);
a := l;
repeat
    pdf_begin_dict(l);
    if l = a then
        pdf_indirect_ln("T", k);
    pdf_indirect_ln("V", obj_bead_prev(l));
    pdf_indirect_ln("N", obj_bead_next(l));
    pdf_indirect_ln("P", obj_bead_page(l));
    pdf_indirect_ln("R", obj_bead_rect(l));
    pdf_end_dict;
    l := obj_bead_next(l);
until l = a

@ Now we are ready to declare our new procedure |ship_out|.  It will call
|pdf_ship_out| if integer parametr |pdf_output| is positive; otherwise it
will call |dvi_ship_out|, which is the \TeX\ original |ship_out|. When 
shipping the first page we also set some parameters.

@p procedure ship_out(p:pointer); {output the box |p|}
begin
    if (total_pages > 0) and (set_pdf_output <> pdf_output) then
        pdf_error("setup", "\pdfoutput cannot be changed after shipping out the first page");
    if pdf_output > 0 then begin
        if total_pages = 0 then begin
            @<Initialize variables for \.{PDF} output@>;
        end;
        pdf_ship_out(p, true);
    end
    else
        dvi_ship_out(p);
end;

@ @<Initialize variables for \.{PDF} output@>=
set_pdf_output := pdf_output;
if (mag <> 1000) and (mag <> 0) then
    adjust_cfg_dimens(mag);
if pdf_h_origin = 0 then
    pdf_h_origin := cfg_h_origin;
if pdf_v_origin = 0 then
    pdf_v_origin := cfg_v_origin;
if pdf_page_width = 0 then
    pdf_page_width := cfg_page_width;
if pdf_page_height = 0 then
    pdf_page_height := cfg_page_height;
if pdf_font_prefix <> null then
    pdf_font_prefix_str := tokens_to_string(pdf_font_prefix);
if pdf_form_prefix <> null then
    pdf_form_prefix_str := tokens_to_string(pdf_form_prefix);
if pdf_image_prefix <> null then
    pdf_image_prefix_str := tokens_to_string(pdf_image_prefix)

@ Finishing the \PDF{} output file.

The following procedures sort the table of destination names
@p function str_less_str(s1, s2: str_number): boolean; {compare two strings}
var j1, j2: pool_pointer;
    l, i: integer;
begin
    j1 := str_start[s1];
    j2 := str_start[s2];
    if length(s1) < length(s2) then
        l := length(s1)
    else
        l := length(s2);
    i := 0;
    while (i < l) and (str_pool[j1 + i] = str_pool[j2 + i]) do
        incr(i);
    if ((i < l) and (str_pool[j1 + i] < str_pool[j2 + i])) or
        ((i = l) and (length(s1) < length(s2))) then
        str_less_str := true
    else
        str_less_str := false;
end;

procedure sort_dest_names(l, r: integer); {sorts |dest_names| by names}
var i, j: integer;
    s: str_number;
    x, y: integer;
    e: dest_name_entry;
begin
    i := l;
    j := r;
    s := dest_names[(l + r) div 2].objname;
    repeat
        while str_less_str(dest_names[i].objname, s) do
            incr(i);
        while str_less_str(s, dest_names[j].objname) do
            decr(j);
        if i <= j then begin
            e := dest_names[i];
            dest_names[i] := dest_names[j];
            dest_names[j] := e;
            incr(i);
            decr(j);
        end;
    until i > j;
    if l < j then
        sort_dest_names(l, j);
    if i < r then
        sort_dest_names(i, r);
end;

@  Now the finish of \PDF{} output file. At this moment all Page object
are already written completly to \PDF{} output file.

@<Finish the \PDF{} file@>=
if total_pages=0 then print_nl("No pages of output.")
@.No pages of output@>
else begin
    pdf_flush; {to make sure that the output file name has been already
    created}
    if total_pages mod pages_tree_kids_max <> 0 then
        obj_info(pdf_last_pages) := total_pages mod pages_tree_kids_max;
    {last pages object may have less than |pages_tree_kids_max| chilrend}
    @<Reverse the linked list of Page and Pages objects@>;
    @<Output fonts definition@>;
    @<Output pages tree@>;
    @<Output outlines@>;
    @<Output name tree@>;
    @<Output article threads@>;
    @<Output the catalog object@>;
    @<Output the info object@>;
    @<Output the |obj_tab|@>;
    @<Output the trailer@>;
    pdf_flush;
    print_nl("Output written on "); slow_print(output_file_name);
  @.Output written on x@>
    print(" ("); print_int(total_pages); print(" page");
    if total_pages<>1 then print_char("s");
    print(", "); print_int(pdf_offset); print(" bytes).");
    libpdffinish;
    b_close(pdf_file);
end

@ @<Reverse the linked list of Page and Pages objects@>=
@<Remove invalid Page objects@>;
k := head_tab[obj_type_page];
l := 0;
repeat
    i := obj_link(k);
    obj_link(k) := l;
    l := k;
    k := i;
until k = 0;
head_tab[obj_type_page] := l;
k := head_tab[obj_type_pages];
l := 0;
repeat
    i := obj_link(k);
    obj_link(k) := l;
    l := k;
    k := i;
until k = 0;
head_tab[obj_type_pages] := l

@ @<Remove invalid Page objects@>=
k := head_tab[obj_type_page];
while obj_int(k) = 0 do begin
    print_err("Page ");
    print_int(obj_info(k));
    print(" has been refered to but not created");
    print_ln;
    k := obj_link(k);
end;
head_tab[obj_type_page] := k

@ @<Output fonts definition@>=
for k := font_base + 1 to font_ptr do begin
    if (not font_used[k]) and (pdf_font_num[k] > 0) then begin
        i := font_base;
        repeat
            incr(i);
        until font_used[i] and (pdf_font_num[i] = pdf_font_num[k]);
        for j := 0 to 255 do
            if pdf_is_char_used(k, j) then
                pdf_set_char_used(i, j);
    end;
end;
k := head_tab[obj_type_font];
while k <> 0 do begin
    f := obj_info(k);
    do_pdf_font(k, f);
    k := obj_link(k);
end

@ We will generate in each single step the parents of all Pages/Page objects in
the previous level. These new generated Pages object will create a new level of
Pages tree. We will repeat this until search only one Pages object. This one
will be the Root object.

@<Output pages tree@>=
a := obj_ptr + 1; {all Pages object whose childrend are not Page objects
should have index greater than |a|}
l := head_tab[obj_type_pages]; {|l| is the index of current Pages object which is
being output}
k := head_tab[obj_type_page]; {|k| is the index of current child of |l|}
if obj_link(l) = 0 then
    is_root := true {only Pages object; total pages is not greater than
    |pages_tree_kids_max|}
else
    is_root := false;
b := obj_ptr + 1; {to init |c| in next step}
repeat
    i := 0; {counter of Pages object in current level}
    c := b; {first Pages object in previous level}
    b := obj_ptr + 1; {succcesor of last created object}
    repeat
        if not is_root then begin
            if i mod pages_tree_kids_max = 0 then begin {create a new Pages object for next level}
                pdf_create_obj(obj_type_others, 0);
                pdf_last_pages := obj_ptr;
                obj_info(pdf_last_pages) := obj_info(l);
            end
            else
                obj_info(pdf_last_pages) := obj_info(pdf_last_pages) +
                    obj_info(l);
        end;
        @<Output the current Pages object in this level@>;
        incr(i);
        if l < a  then
            l := obj_link(l)
        else
            incr(l);
    until (l = 0) or (l = b);
    if l = 0 then
        l := a;
    if b = obj_ptr then
        is_root := true;
until false;
done:

@ @<Output the current Pages object in this level@>=
pdf_begin_dict(l);
pdf_print_ln("/Type /Pages");
pdf_int_entry_ln("Count", obj_info(l));
if not is_root then
    pdf_indirect_ln("Parent", pdf_last_pages);
pdf_print("/Kids [");
j := 0;
repeat
    pdf_print_int(k);
    pdf_print(" 0 R ");
    if k < a then {the next Pages/Page object must be |obj_link(k)|}
        k := obj_link(k)
    else {|k >= a|, the next Pages object is |k + 1|}
        incr(k);
    incr(j);
until ((l < a) and (j = obj_info(l))) or
    (k = 0) or (k = c) or
    (j = pages_tree_kids_max);
pdf_remove_last_space;
pdf_print_ln("]");
if k = 0 then begin
    if head_tab[obj_type_pages] <> 0 then begin {we are in Page objects list}
        k := head_tab[obj_type_pages];
        head_tab[obj_type_pages] := 0;
    end
    else {we are in Pages objects list}
        k := a;
end;
if is_root and (pdf_pages_attr <> null) then
    pdf_print_toks(pdf_pages_attr);
pdf_end_dict;
if is_root then
    goto done

@ The name tree is very similiar to Pages tree so its construction should be
certain from Pages tree construction. For intermediate node |obj_info| will be
the first name and |obj_link| will be the last name in \.{\\Limits} array.
Note that |dest_names_ptr| will be less than |obj_ptr|, so we test if
|k < dest_names_ptr| then |k| is index of leaf in |dest_names|; else
|k| will be index in |obj_tab| of some intermediate node.

@<Output name tree@>=
if dest_names_ptr = 0 then begin
    dests := 0;
    goto done1;
end;
sort_dest_names(0, dest_names_ptr - 1);
a := obj_ptr + 1; {first intermediate node of name tree}
l := a; {index of node being output}
k := 0; {index of current child of |l|; if |k < dest_names_ptr| then this is
pointer to |dest_names| array; otherwise it is the pointer to |obj_tab|
(object number) }
repeat
    c := obj_ptr + 1; {first node in current level}
    repeat
        pdf_create_obj(obj_type_others, 0); {create a new node for next level}
        @<Output the current node in this level@>;
        incr(l);
        incr(i);
    until k = c;
until false;
done1:
if (dests <> 0) or (pdf_names_toks <> null) then begin
    pdf_new_dict(obj_type_others, 0);
    if (dests <> 0) then
        pdf_indirect_ln("Dests", dests);
    if pdf_names_toks <> null then begin
        pdf_print_toks(pdf_names_toks);
        delete_token_ref(pdf_names_toks);
    end;
    pdf_end_dict;
    names_tree := obj_ptr;
end
else
    names_tree := 0

@ @<Output the current node in this level@>=
pdf_begin_dict(l);
j := 0;
if k < dest_names_ptr then begin
    obj_info(l) := dest_names[k].objname;
    pdf_print("/Names [");
    repeat
        pdf_print_str(dest_names[k].objname);
        pdf_out(" ");
        pdf_print_int(dest_names[k].obj_num);
        pdf_print(" 0 R ");
        incr(j);
        incr(k);
    until (j = name_tree_kids_max) or (k = dest_names_ptr);
    pdf_remove_last_space;
    pdf_print_ln("]");
    obj_link(l) := dest_names[k - 1].objname;
    if k = dest_names_ptr then
        k := a;
end
else begin
    obj_info(l) := obj_info(k);
    pdf_print("/Kids [");
    repeat
        pdf_print_int(k);
        pdf_print(" 0 R ");
        incr(j);
        incr(k);
    until (j = name_tree_kids_max) or (k = c);
    pdf_remove_last_space;
    pdf_print_ln("]");
    obj_link(l) := obj_link(k - 1);
end;
if (l > k) or (l = a) then begin
    pdf_print("/Limits [");
    pdf_print_str(obj_info(l));
    pdf_out(" ");
    pdf_print_str(obj_link(l));
    pdf_print_ln("]");
    pdf_end_dict;
end
else begin
    pdf_end_dict;
    dests := l;
    goto done1;
end

@ @<Output the catalog object@>=
pdf_new_dict(obj_type_others, 0);
root := obj_ptr;
pdf_print_ln("/Type /Catalog");
pdf_indirect_ln("Pages", pdf_last_pages);
if threads <> 0 then
    pdf_indirect_ln("Threads", threads);
if outlines <> 0 then
    pdf_indirect_ln("Outlines", outlines);
if names_tree <> 0 then
    pdf_indirect_ln("Names", names_tree);
if pdf_catalog_toks <> null then begin
    pdf_print_toks(pdf_catalog_toks);
    delete_token_ref(pdf_catalog_toks);
end;
if pdf_catalog_openaction <> 0 then
    pdf_indirect_ln("OpenAction", pdf_catalog_openaction);
pdf_end_dict

@ @<Output the info object@>=
pdf_new_dict(obj_type_others, 0);
if pdf_info_toks <> null then begin
    pdf_print_toks(pdf_info_toks);
    delete_token_ref(pdf_info_toks);
end;
pdf_str_entry_ln("Creator", "TeX");
pdf_print("/Producer (pdfTeX-");
pdf_print_int(pdftex_version div 100);
pdf_out(".");
pdf_print_int(pdftex_version mod 100);
pdf_print(pdftex_revision);
pdf_print_ln(")");
pdf_print("/CreationDate (D:");
pdf_print_int(year);
pdf_print_two(month);
pdf_print_two(day);
pdf_print_two(time div 60);
pdf_print_two(time mod 60);
pdf_print_ln("00)");
pdf_end_dict

@ @<Output the |obj_tab|@>=
pdf_save_offset := pdf_offset;
pdf_print_ln("xref");
pdf_print("0 "); pdf_print_int_ln(obj_ptr + 1);
pdf_print_ln("0000000000 65535 f ");
for k := 1 to obj_ptr do begin
    pdf_print_offset(obj_offset(k));
    pdf_print_ln(" 00000 n ");@/
    {{\bf Important note:} the last space after \.{`n'} and \.{`f'} is required
    for single end-of-line character}
end

@ @<Output the trailer@>=
pdf_print_ln("trailer");
pdf_print_ln("<<");
pdf_int_entry_ln("Size", obj_ptr + 1);
pdf_indirect_ln("Root", root);
pdf_indirect_ln("Info", obj_ptr);
pdf_print_ln(">>");
pdf_print_ln("startxref");
pdf_print_int_ln(pdf_save_offset);
pdf_print_ln("%%EOF")

@* \[33] Packaging.
@z

% Adjust hboxes produced by breaking paragraphs into lines

@x [33.644] - HZ
@d natural==0,additional {shorthand for parameters to |hpack| and |vpack|}
@y
@d natural==0,additional {shorthand for parameters to |hpack| and |vpack|}
@d cal_expand_ratio == 2 {calculate amount for expanding fonts after breaking 
paragraphs into lines}
@d adjust_expand_ratio == 3  {adjust amount of stretching/shrinking that have 
been taken into account while breaking paragraphs into lines}
@d subst_font        == 4


@d add_font_stretch_end(#) == char_stretch_amount(f, #)
@d add_font_stretch(#) == # := # + add_font_stretch_end

@d add_font_shrink_end(#) == char_shrink_amount(f, #)
@d add_font_shrink(#) == # := # + add_font_shrink_end

@d sub_font_stretch_end(#) == char_stretch_amount(f, #)
@d sub_font_stretch(#) == # := # - sub_font_stretch_end

@d sub_font_shrink_end(#) == char_shrink_amount(f, #)
@d sub_font_shrink(#) == # := # - sub_font_shrink_end

@d do_subst_font(#) ==
    if (character(#) < font_bc[f]) or (character(#) > font_ec[f]) or
        (not char_exists(char_info(f)(character(#)))) then
        char_warning(f, character(#))
    else if (pdf_max_font_stretch[f] <> null_font) and (expand_ratio > 0) then
        font(#) := expand_font(f, divide_scaled(expand_ratio*
            pdf_expand_font[pdf_max_font_stretch[f]]*
            ef_code(character(#)), 1000000, 0))
    else if (pdf_max_font_shrink[f] <> null_font) and (expand_ratio < 0) then
        font(#) := expand_font(f, -divide_scaled(expand_ratio*
            pdf_expand_font[pdf_max_font_shrink[f]]*
            ef_code(character(#)), 1000000, 0))
@z

@x [33.649] - HZ
@!f:internal_font_number; {the font in a |char_node|}
@y
@!f:internal_font_number; {the font in a |char_node|}
@!total_font_stretch: scaled; {sum of stretch amount of characters}
@!total_font_shrink: scaled; {sum of shrink amount of characters}
@!z: scaled; {accumulator}
@!save_p: pointer;
@!save_w: scaled;
@z

@x [33.649] - HZ
h:=0; @<Clear dimensions to zero@>;
@y
h:=0; @<Clear dimensions to zero@>;
if (m = cal_expand_ratio) or (m = adjust_expand_ratio) then begin
    total_font_stretch := 0;
    total_font_shrink := 0;
    save_p := p;
    save_w := w;
    expand_ratio := 0;
end;
@z

@x [33.649] - HZ
exit: hpack:=r;
@y
exit: 
if ((m = cal_expand_ratio) or (m = adjust_expand_ratio)) and (expand_ratio <> 0) then begin
    if abs(expand_ratio) > 1000 then
        if expand_ratio > 0 then
            expand_ratio := 1000
        else
            expand_ratio := -1000;
    free_node(r, box_node_size);
    r := hpack(save_p, save_w, subst_font);
end;
hpack:=r;
@z

@x [33.651] - HZ
  ligature_node: @<Make node |p| look like a |char_node|
    and |goto reswitch|@>;
@y
  ligature_node: begin
      if m = subst_font then begin
          f:=font(lig_char(p));
          do_subst_font(lig_char(p));
      end;
      @<Make node |p| look like a |char_node| and |goto reswitch|@>;
  end;
@z

@x [33.654] - HZ
begin f:=font(p); i:=char_info(f)(character(p)); hd:=height_depth(i);
@y
begin
if m >= cal_expand_ratio then begin
    f:=font(p);
    case m of 
    cal_expand_ratio: begin
        total_font_stretch := total_font_stretch + char_stretch_amount(f, character(p));
        total_font_shrink := total_font_shrink + char_shrink_amount(f, character(p));
    end;
    adjust_expand_ratio: begin
        z := char_stretch_amount(f, character(p));
        total_font_stretch := total_font_stretch + z;
        total_stretch[normal] := total_stretch[normal] + z;
        z := char_shrink_amount(f, character(p));
        total_font_shrink := total_font_shrink + z;
        total_shrink[normal] := total_shrink[normal] + z;
    end;
    subst_font:
        do_subst_font(p);
    endcases;
end;
f:=font(p); i:=char_info(f)(character(p)); hd:=height_depth(i);
@z

@x [33.658] - HZ
@ @<Determine horizontal glue stretch setting...@>=
begin @<Determine the stretch order@>;
@y
@ If |hpack| is called with |m=cal_expand_ratio| or |m=adjust_expand_ratio| 
we calculate |expand_ratio| and return without checking overfull or underfull

@<Determine horizontal glue stretch setting...@>=
begin @<Determine the stretch order@>;
if (m = cal_expand_ratio) or (m = adjust_expand_ratio) then begin
    if (o = normal) and (total_stretch[o]<>0) and (total_font_stretch > 0) then
        expand_ratio := divide_scaled(x, total_font_stretch, 3);
    if expand_ratio <> 0 then
        return;
end;
@z

@x [33.664] - HZ
@ @<Determine horizontal glue shrink setting...@>=
begin @<Determine the shrink order@>;
@y
@ @<Determine horizontal glue shrink setting...@>=
begin @<Determine the shrink order@>;
if (m = cal_expand_ratio) or (m = adjust_expand_ratio) then begin
    if (o = normal) and (total_shrink[o]<>0) and (total_font_shrink > 0) then
        expand_ratio := divide_scaled(x, total_font_shrink, 3);
    if expand_ratio <> 0 then
        return;
end;
@z

% Incorporate font expansion into algorithm breaking paragraphs into lines

@x [38.839] - HZ
The value of $l_0$ need not be computed, since |line_break| will put
it into the global variable |disc_width| before calling |try_break|.

@<Glob...@>=
@!disc_width:scaled; {the length of discretionary material preceding a break}
@y
The value of $l_0$ need not be computed, since |line_break| will put
it into the global variable |disc_width| before calling |try_break|.

@d add_by_disc_width(#) ==
#[1]:=#[1]+disc_width[1];
#[2]:=#[2]+disc_width[2];
#[6]:=#[6]+disc_width[6]

@d substract_by_disc_width(#) ==
#[1]:=#[1]-disc_width[1];
#[2]:=#[2]-disc_width[2];
#[6]:=#[6]-disc_width[6]

@<Glob...@>=
@!disc_width: array [1..6] of scaled; {the length of discretionary material preceding a break}
@z

@x [38.840] - HZ
break_width[1]:=break_width[1]+disc_width;
@y
add_by_disc_width(break_width);
@z

@x [38.841] - HZ
@<Subtract the width of node |v|...@>=
if is_char_node(v) then
  begin f:=font(v);
  break_width[1]:=break_width[1]-char_width(f)(char_info(f)(character(v)));
  end
else  case type(v) of
  ligature_node: begin f:=font(lig_char(v));@/
    break_width[1]:=@|break_width[1]-
      char_width(f)(char_info(f)(character(lig_char(v))));
    end;
@y
@d subtract_break_width(#) == begin
    f := font(#);
    if pdf_adjust_spacing = 2 then begin
        sub_font_stretch(break_width[2])(character(#));
        sub_font_shrink(break_width[6])(character(#));
    end;
    break_width[1] := break_width[1] - char_width(f)(char_info(f)(character(#)));
end

@d add_break_width(#) == begin
    f := font(#);
    if pdf_adjust_spacing = 2 then begin
        add_font_stretch(break_width[2])(character(#));
        add_font_shrink(break_width[6])(character(#));
    end;
    break_width[1] := break_width[1] + char_width(f)(char_info(f)(character(#)));
end

@d add_act_width(#) == begin
    f := font(#);
    if pdf_adjust_spacing = 2 then begin
        add_font_stretch(active_width[2])(character(#));
        add_font_shrink(active_width[6])(character(#));
    end;
    active_width[1] := active_width[1] + char_width(f)(char_info(f)(character(#)));
end

@d add_disc_width(#) == begin
    f := font(#);
    if pdf_adjust_spacing = 2 then begin
        add_font_stretch(disc_width[2])(character(#));
        add_font_shrink(disc_width[6])(character(#));
    end;
    disc_width[1] := disc_width[1] + char_width(f)(char_info(f)(character(#)));
end

@<Subtract the width of node |v|...@>=
if is_char_node(v) then
    subtract_break_width(v)
else  case type(v) of
  ligature_node: 
    subtract_break_width(lig_char(v));
@z

@x [38.842] - HZ
@ @<Add the width of node |s| to |b...@>=
if is_char_node(s) then
  begin f:=font(s);
  break_width[1]:=@|break_width[1]+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
    break_width[1]:=break_width[1]+
      char_width(f)(char_info(f)(character(lig_char(s))));
    end;
@y
@ @<Add the width of node |s| to |b...@>=
if is_char_node(s) then
    add_break_width(s)
else  case type(s) of
  ligature_node: 
    add_break_width(lig_char(s));
@z

@x [39.866] - HZ
ligature_node: begin f:=font(lig_char(cur_p));
  act_width:=act_width+char_width(f)(char_info(f)(character(lig_char(cur_p))));
  end;
@y
ligature_node:
  add_act_width(lig_char(cur_p));
@z

@x [39.867] - HZ
repeat f:=font(cur_p);
act_width:=act_width+char_width(f)(char_info(f)(character(cur_p)));
@y
repeat
    add_act_width(cur_p);
@z

@x [39.869] - HZ
begin s:=pre_break(cur_p); disc_width:=0;
@y
begin s:=pre_break(cur_p);
disc_width[1]:=0;
disc_width[2]:=0;
disc_width[6]:=0;
@z

@x [39.869] - HZ
  act_width:=act_width+disc_width;
  try_break(hyphen_penalty,hyphenated);
  act_width:=act_width-disc_width;
@y
  add_by_disc_width(active_width);
  try_break(hyphen_penalty,hyphenated);
  substract_by_disc_width(active_width);
@z

@x [39.870] - HZ
@ @<Add the width of node |s| to |disc_width|@>=
if is_char_node(s) then
  begin f:=font(s);
  disc_width:=disc_width+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
    disc_width:=disc_width+
      char_width(f)(char_info(f)(character(lig_char(s))));
    end;
  hlist_node,vlist_node,rule_node,kern_node:
    disc_width:=disc_width+width(s);
@y
@ @<Add the width of node |s| to |disc_width|@>=
if is_char_node(s) then
    add_disc_width(s)
else  case type(s) of
  ligature_node: 
    add_disc_width(lig_char(s));
  hlist_node,vlist_node,rule_node,kern_node:
    disc_width[1]:=disc_width[1]+width(s);
@z

@x [39.871] - HZ
@ @<Add the width of node |s| to |act_width|@>=
if is_char_node(s) then
  begin f:=font(s);
  act_width:=act_width+char_width(f)(char_info(f)(character(s)));
  end
else  case type(s) of
  ligature_node: begin f:=font(lig_char(s));
    act_width:=act_width+
      char_width(f)(char_info(f)(character(lig_char(s))));
    end;
@y
@ @<Add the width of node |s| to |act_width|@>=
if is_char_node(s) then
    add_act_width(s)
else  case type(s) of
  ligature_node: 
    add_act_width(lig_char(s));
@z

@x [39.889] - HZ
adjust_tail:=adjust_head; just_box:=hpack(q,cur_width,exactly);
@y
adjust_tail:=adjust_head;
if pdf_adjust_spacing = 2 then
    just_box:=hpack(q,cur_width,adjust_expand_ratio)
else if pdf_adjust_spacing = 1 then
    just_box:=hpack(q,cur_width,cal_expand_ratio)
else
    just_box:=hpack(q,cur_width,exactly);
@z

@x [49.12??] - HZ
primitive("sfcode",def_code,sf_code_base);
@!@:sf_code_}{\.{\\sfcode} primitive@>
@y
primitive("sfcode",def_code,sf_code_base);
@!@:sf_code_}{\.{\\sfcode} primitive@>
primitive("efcode",def_code,ef_code_base);
@!@:ef_code_}{\.{\\efcode} primitive@>
@z

@x [49.12??] - HZ
  else if chr_code=sf_code_base then print_esc("sfcode")
@y
  else if chr_code=sf_code_base then print_esc("sfcode")
  else if chr_code=ef_code_base then print_esc("efcode")
@z

@x [49.12??]- HZ
else if cur_chr=sf_code_base then n:=@'77777
@y
else if cur_chr=sf_code_base then n:=@'77777
else if cur_chr=ef_code_base then n:=@'77777
@z

% Check for expandable fonts
@x [49.1257] - HZ
procedure new_font(@!a:small_number);
@y
procedure check_expand;
label reswitch;
begin
    cur_font_shrink := 0;
    cur_font_stretch := 0;
    cur_font_step := 0;
reswitch:
    if scan_keyword("stretch") then begin 
        scan_int;
        cur_font_stretch := cur_val;
        goto reswitch;
    end;
    if scan_keyword("shrink") then begin 
        scan_int;
        cur_font_shrink := cur_val;
        goto reswitch;
    end;
    if scan_keyword("step") then begin 
        scan_int;
        cur_font_step := cur_val;
        goto reswitch;
    end;
    if (cur_font_stretch = 0) and (cur_font_shrink = 0) and (cur_font_step = 0) then
        return;
    if (cur_font_shrink < 0) or (cur_font_shrink > 1000) or
       (cur_font_stretch < 0) or (cur_font_stretch > 1000) or
       (cur_font_step < 0) or (cur_font_step > 1000) or
       ((cur_font_stretch > 0) and (cur_font_step > cur_font_stretch)) or
       ((cur_font_shrink > 0) and (cur_font_step > cur_font_shrink)) then
    begin
        pdf_error("expand", "invalid value of font expansion");
        cur_font_shrink := 0;
        cur_font_stretch := 0;
        cur_font_step := 0;
        return;
    end;
    if ((cur_font_shrink > 0) or (cur_font_stretch > 0)) then begin
        if (cur_font_step = 0) then
            cur_font_step := 5;
        cur_font_stretch := cur_font_stretch - cur_font_stretch mod cur_font_step;
        cur_font_shrink := cur_font_shrink - cur_font_shrink mod cur_font_step;
    end
    else if cur_font_step > 0 then
        cur_font_step := 0;
end;

procedure new_font(@!a:small_number);
@z

@x [49.1257] - HZ
@<Scan the font size specification@>;
@<If this font has already been loaded, set |f| to the internal
  font number and |goto common_ending|@>;
f:=read_font_info(u,cur_name,cur_area,s);
common_ending: equiv(u):=f; eqtb[font_id_base+f]:=eqtb[u]; font_id_text(f):=t;
@y
@<Scan the font size specification@>;
check_expand;
@<If this font has already been loaded, set |f| to the internal
  font number and |goto common_ending|@>;
f:=read_font_info(u,cur_name,cur_area,s);
if cur_font_step > 0 then begin
    if cur_font_stretch > 0 then
        pdf_max_font_stretch[f] := new_ex_font(f, cur_font_stretch);
    if cur_font_shrink > 0 then
        pdf_max_font_shrink[f] := new_ex_font(f, -cur_font_shrink);
    pdf_font_step[f] := cur_font_step;
end;
common_ending: equiv(u):=f; eqtb[font_id_base+f]:=eqtb[u]; font_id_text(f):=t;
@z

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Print new line before termination; switch to editor if
% necessary.
% Declare the necessary variables for finishing PDF file
% Close PDF output if necessary
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
@x [51.1333]
procedure close_files_and_terminate;
var k:integer; {all-purpose index}
begin @<Finish the extensions@>;
@!stat if tracing_stats>0 then @<Output statistics about this job@>;@;@+tats@/
wake_up_terminal; @<Finish the \.{DVI} file@>;
@y
procedure close_files_and_terminate;
label done, done1;
var a, b, c, i, j, k, l:integer; {all-purpose index}
    is_root: boolean; {|pdf_last_pages| is root of Pages tree?}
    root, outlines, threads, names_tree, dests: integer;
begin @<Finish the extensions@>;
@!stat if tracing_stats>0 then @<Output statistics about this job@>;@;@+tats@/
wake_up_terminal;
if pdf_output > 0 then begin
    if history = fatal_error_stop then
        print_err(" ==> Fatal error occurred, the output PDF file not finished!")
    else begin
        @<Finish the \PDF{} file@>;
    end;
end
else begin
    @<Finish the \.{DVI} file@>;
end;
@z

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Read config file before input
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
@x [51.1337]
@<Initialize the print |selector|...@>;
if (loc<limit)and(cat_code(buffer[loc])<>escape) then start_input;
  {\.{\\input} assumed}
@y
@<Initialize the print |selector|...@>;
if (loc<limit)and(cat_code(buffer[loc])<>escape) then start_input;
  {\.{\\input} assumed}
@<Read values from config file if necessary@>;
@z


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Whatsit extensions for PDF output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
@x [53.1344]
@d immediate_code=4 {command modifier for \.{\\immediate}}
@d set_language_code=5 {command modifier for \.{\\setlanguage}}
@y
@d immediate_code=4 {command modifier for \.{\\immediate}}
@d set_language_code=5 {command modifier for \.{\\setlanguage}}
@d pdf_literal_node        == 6
@d pdf_obj_node            == 7
@d pdf_annot_node          == 8
@d pdf_annot_link_node     == 9
@d pdf_end_link_node       == 10
@d pdf_outline_code        == 11
@d pdf_dest_node           == 12
@d pdf_thread_node         == 13
@d pdf_end_thread_node     == 14
@d pdf_image_node          == 15
@d pdf_form_code           == 16
@d pdf_ref_form_node       == 17
@d pdf_info_code           == 18
@d pdf_catalog_code        == 19
@d pdf_names_code          == 20
@z

@x [53.1344]
primitive("setlanguage",extension,set_language_code);@/
@!@:set_language_}{\.{\\setlanguage} primitive@>
@y
primitive("setlanguage",extension,set_language_code);@/
@!@:set_language_}{\.{\\setlanguage} primitive@>
primitive("pdfliteral",extension,pdf_literal_node);@/
@!@:pdf_literal_}{\.{\\pdfliteral} primitive@>
primitive("pdfobj",extension,pdf_obj_node);@/
@!@:pdf_obj_}{\.{\\pdfobj} primitive@>
primitive("pdfannot",extension,pdf_annot_node);@/
@!@:pdf_annot_}{\.{\\pdfannot} primitive@>
primitive("pdfannotlink",extension,pdf_annot_link_node);@/
@!@:pdf_annot_link_}{\.{\\pdfannotlink} primitive@>
primitive("pdfendlink",extension,pdf_end_link_node);@/
@!@:pdf_end_annot_}{\.{\\pdfendlink} primitive@>
primitive("pdfoutline",extension,pdf_outline_code);@/
@!@:pdf_outline_}{\.{\\pdfoutline} primitive@>
primitive("pdfdest",extension,pdf_dest_node);@/
@!@:pdf_dest_}{\.{\\pdfdest} primitive@>
primitive("pdfthread",extension,pdf_thread_node);@/
@!@:pdf_thread_}{\.{\\pdfthread} primitive@>
primitive("pdfendthread",extension,pdf_end_thread_node);@/
@!@:pdf_end_thread_}{\.{\\pdfendthread} primitive@>
primitive("pdfimage",extension,pdf_image_node);@/
@!@:pdf_image_}{\.{\\pdfimage} primitive@>
primitive("pdfform",extension,pdf_form_code);@/
@!@:pdf_form_}{\.{\\pdfform} primitive@>
primitive("pdfrefform",extension,pdf_ref_form_node);@/
@!@:pdf_ref_form_}{\.{\\pdfrefform} primitive@>
primitive("pdfinfo",extension,pdf_info_code);@/
@!@:pdf_info_}{\.{\\pdfinfo} primitive@>
primitive("pdfcatalog",extension,pdf_catalog_code);@/
@!@:pdf_catalog_}{\.{\\pdfcatalog} primitive@>
primitive("pdfnames",extension,pdf_names_code);@/
@!@:pdf_names_}{\.{\\pdfnames} primitive@>
@z

@x [53.1346]
  set_language_code:print_esc("setlanguage");
  othercases print("[unknown extension!]")
@y
  set_language_code: print_esc("setlanguage");
  pdf_literal_node: print_esc("pdfliteral");
  pdf_obj_node: print_esc("pdfobj");
  pdf_annot_node: print_esc("pdfannot");
  pdf_annot_link_node: print_esc("pdfannotlink");
  pdf_end_link_node: print_esc("pdfendlink");
  pdf_outline_code: print_esc("pdfoutline");
  pdf_dest_node: print_esc("pdfdest");
  pdf_thread_node: print_esc("pdfthread");
  pdf_end_thread_node: print_esc("pdfendthread");
  pdf_image_node: print_esc("pdfimage");
  pdf_form_code: print_esc("pdfform");
  pdf_ref_form_node: print_esc("pdfrefform");
  pdf_info_code: print_esc("pdfinfo");
  pdf_catalog_code: print_esc("pdfcatalog");
  pdf_names_code: print_esc("pdfnames");
  othercases print("[unknown extension!]")
@z

@x [53.1348]
set_language_code:@<Implement \.{\\setlanguage}@>;
othercases confusion("ext1")
@y
set_language_code: @<Implement \.{\\setlanguage}@>;
pdf_literal_node: @<Implement \.{\\pdfliteral}@>;
pdf_obj_node: @<Implement \.{\\pdfobj}@>;
pdf_annot_node: @<Implement \.{\\pdfannot}@>;
pdf_annot_link_node: @<Implement \.{\\pdfannotlink}@>;
pdf_end_link_node: @<Implement \.{\\pdfendlink}@>;
pdf_outline_code: @<Implement \.{\\pdfoutline}@>;
pdf_dest_node: @<Implement \.{\\pdfdest}@>;
pdf_thread_node: @<Implement \.{\\pdfthread}@>;
pdf_end_thread_node: @<Implement \.{\\pdfendthread}@>;
pdf_image_node: @<Implement \.{\\pdfimage}@>;
pdf_form_code: @<Implement \.{\\pdfform}@>;
pdf_ref_form_node: @<Implement \.{\\pdfrefform}@>;
pdf_info_code: @<Implement \.{\\pdfinfo}@>;
pdf_catalog_code: @<Implement \.{\\pdfcatalog}@>;
pdf_names_code: @<Implement \.{\\pdfnames}@>;
othercases confusion("ext1")
@z

@x [53.1354]
@<Implement \.{\\special}@>=
begin new_whatsit(special_node,write_node_size); write_stream(tail):=null;
p:=scan_toks(false,true); write_tokens(tail):=def_ref;
end
@y
@<Implement \.{\\special}@>=
begin new_whatsit(special_node,write_node_size); write_stream(tail):=null;
p:=scan_toks(false,true); write_tokens(tail):=def_ref;
end

@ The following subroutines are needed for pdfTeX extension


@d add_action_ref(#) == incr(pdf_action_refcount(#)) {increase count of
references to this action}

@d delete_action_ref(#) == {decrease count of references to this
action; free it if there is no reference to this action}
begin
    if pdf_action_refcount(#) = null then begin
        if pdf_action_type(#) = pdf_action_user then
            delete_token_ref(pdf_action_user_tokens(#))
        else begin
            if pdf_action_file(#) <> null then
                delete_token_ref(pdf_action_file(#));
            if pdf_action_type(#) = pdf_action_page then
                delete_token_ref(pdf_action_page_tokens(#))
            else if pdf_action_id_type(#) = pdf_id_name then
                delete_token_ref(pdf_action_name(#));
        end;
        free_node(#, pdf_action_struct_size);
    end
    else
        decr(pdf_action_refcount(#));
end

@d copy_annot_link_node(#) == {make a copy of node representing link annotation;
result is stored as |r|}
begin
    r := get_node(pdf_annot_node_size);
    pdf_height(r) := pdf_height(#);
    pdf_depth(r)  := pdf_depth(#);
    pdf_width(r)  := pdf_width(#);
    pdf_annot_link_attr(r) := pdf_annot_link_attr(#);
    if pdf_annot_link_attr(r) <> null then
        add_token_ref(pdf_annot_link_attr(r));
    pdf_annot_link_action(r) := pdf_annot_link_action(#);
    add_action_ref(pdf_annot_link_action(r));
end

@d delete_annot_link_node(#) == {delete a node representing link annotation}
begin
    if pdf_annot_link_attr(#) <> null then
        delete_token_ref(pdf_annot_link_attr(#));
    delete_action_ref(pdf_annot_link_action(#));
    free_node(#, pdf_annot_node_size);
end

@<Declare procedures needed in |do_ext...@>=
procedure new_annot_whatsit(w, s: small_number); {create a new whatsit node for
annotation}
label restart, reswitch;
var p: pointer;
begin
    new_whatsit(w, s);
    pdf_width(tail) := null_flag;
    pdf_depth(tail) := null_flag;
    pdf_height(tail) := null_flag;
    if w = pdf_annot_link_node then
        pdf_annot_link_attr(tail) := null;
reswitch:
    if scan_keyword("width") then
      begin scan_normal_dimen; pdf_width(tail):=cur_val; goto reswitch;
      end;
    if scan_keyword("height") then
      begin scan_normal_dimen; pdf_height(tail):=cur_val; goto reswitch;
      end;
    if scan_keyword("depth") then
      begin scan_normal_dimen; pdf_depth(tail):=cur_val; goto reswitch;
      end;
    if (w = pdf_annot_link_node) and scan_keyword("attr") then begin
        p := scan_toks(false, true);
        pdf_annot_link_attr(tail) := def_ref;
    end;
end;

function outline_list_count(p: pointer): integer; {return number of outline
entries in the same level with |p|}
var k: integer;
begin
    k := 1;
    while obj_outline_prev(p) <> 0 do begin
        incr(k);
        p := obj_outline_prev(p);
    end;
    outline_list_count := k;
end;

@ The following subroutines are needed in |pdf_ship_out|.

@<Declare procedures needed in |pdf_hlist_out|, |pdf_vlist_out|@>=
function get_obj(t, i: integer; by_name: boolean): integer; {finds object with
identifier |i| and type |t|. |by_name| indicates whether |i| should be treated
as string number. If no such object exists then create it}
label done;
var p, r: integer;
    s: str_number;
begin
    p := head_tab[t];
    r := 0;
    if by_name then begin
        s := tokens_to_string(i);
        while p <> 0 do begin
            if str_eq_str(obj_info(p), s) then begin
                flush_last_string(s);
                r := p;
                goto done;
            end;
            p := obj_link(p);
        end;
    end
    else while p <> 0 do begin
        if  obj_info(p) = i then begin
            r := p;
            goto done;
        end;
        p := obj_link(p);
    end;
done:
    if r = 0 then begin
        if by_name then
            pdf_create_obj(t, s)
        else
            pdf_create_obj(t, i);
        r := obj_ptr;
        if (t = obj_type_dest_name) or (t = obj_type_dest_num) then
            obj_dest_ptr(r) := null;
    end;
    get_obj := r;
end;

function scan_action: pointer; {read an action specification}
var p, t: integer;
    s: str_number;
begin
    p := get_node(pdf_action_struct_size);
    scan_action := p;
    pdf_action_file(p) := null;
    pdf_action_refcount(p) := null;
    if scan_keyword("user") then
        pdf_action_type(p) := pdf_action_user
    else if scan_keyword("goto") then
        pdf_action_type(p) := pdf_action_goto
    else if scan_keyword("thread") then
        pdf_action_type(p) := pdf_action_thread
    else
        pdf_error("ext1", "you must specify action type");
    if pdf_action_type(p) = pdf_action_user then begin
        t := scan_toks(false, true);
        pdf_action_user_tokens(p) := def_ref;
        return;
    end;
    if scan_keyword("file") then begin
        t := scan_toks(false, true);
        pdf_action_file(p) := def_ref;
    end;
    if scan_keyword("page") then begin
        if pdf_action_type(p) <> pdf_action_goto then
            pdf_error("ext1", "you can use `page' with GoTo action only");
        pdf_action_type(p) := pdf_action_page;
        scan_int;
        if cur_val <= 0 then
            pdf_error("ext1", "page number must be positive");
        pdf_action_page_num(p) := cur_val;
        t := scan_toks(false, true);
        pdf_action_page_tokens(p) := def_ref;
        if pdf_action_file(p) = null then
            pdf_action_obj_num(p) := 
                get_obj(obj_type_page, pdf_action_page_num(p), false);
        return;
    end
    else if scan_keyword("name") then begin
        pdf_action_id_type(p) := pdf_id_name;
        t := scan_toks(false, true);
        pdf_action_name(p) := def_ref;
    end
    else if scan_keyword("num") then begin
        if (pdf_action_type(p) = pdf_action_goto) and 
            (pdf_action_file(p) <> null) then
            pdf_error("ext1", 
                "you can't use both `file' and `num' with `goto' option");
        pdf_action_id_type(p) := pdf_id_num;
        scan_int;
        if cur_val <= 0 then
            pdf_error("ext1", "num identifier must be positive");
        pdf_action_num(p) := cur_val;
    end
    else
        pdf_error("ext1", "identifier type missing");
    if scan_keyword("newwindow") then
        pdf_action_new_window(p) := 1
    else if scan_keyword("nonewwindow") then
        pdf_action_new_window(p) := 2
    else
        pdf_action_new_window(p) := 0;
    if (pdf_action_new_window(p) > 0) and
       ((pdf_action_type(p) <> pdf_action_goto) or
        (pdf_action_file(p) = null)) then
           pdf_error("ext1", 
               "you must use `newwindow' with both `goto' and `file' option");
    if pdf_action_file(p) = null then
        if pdf_action_type(p) = pdf_action_goto then
            if pdf_action_id_type(p) = pdf_id_num then
                pdf_action_obj_num(p) := 
                    get_obj(obj_type_dest_num, pdf_action_num(p), false)
            else
                pdf_action_obj_num(p) := 
                    get_obj(obj_type_dest_name, pdf_action_name(p), true)
        else
            if pdf_action_id_type(p) = pdf_id_num then
                pdf_action_obj_num(p) := 
                    get_obj(obj_type_thread_num, pdf_action_num(p), false)
            else
                pdf_action_obj_num(p) := 
                    get_obj(obj_type_thread_name, pdf_action_name(p), true);
end;

procedure write_action(p: pointer); {write an action specification}
begin
    if pdf_action_type(p) = pdf_action_user then begin
        pdf_print_toks(pdf_action_user_tokens(p));
        flush_last_string(last_tokens_string);
        return;
    end;
    pdf_print("<< ");
    if pdf_action_file(p) <> null then begin
        pdf_str_entry("F", tokens_to_string(pdf_action_file(p)));
        flush_last_string(last_tokens_string);
        pdf_out(" ");
        if pdf_action_new_window(p) > 0 then begin
            pdf_print("/NewWindow ");
            if pdf_action_new_window(p) = 1 then
                pdf_print("true ")
            else
                pdf_print("false ");
        end;
    end;
    if pdf_action_type(p) = pdf_action_thread then
        pdf_print("/S /Thread ")
    else 
        if pdf_action_file(p) = null then
            pdf_print("/S /GoTo ")
        else
            pdf_print("/S /GoToR ");
    case pdf_action_type(p) of
    pdf_action_goto: begin
        if pdf_action_id_type(p) = pdf_id_num then
            pdf_indirect("D", pdf_action_obj_num(p))
        else begin
            pdf_str_entry("D", tokens_to_string(pdf_action_name(p)));
            flush_last_string(last_tokens_string);
        end;
    end;
    pdf_action_page: begin
        pdf_print("/D [");
        if pdf_action_file(p) = null then begin
            pdf_print_int(pdf_action_obj_num(p));
            pdf_print(" 0 R");
        end
        else
            pdf_print_int(pdf_action_num(p) - 1);
        pdf_out(" ");
        pdf_print(tokens_to_string(pdf_action_page_tokens(p)));
        flush_last_string(last_tokens_string);
        pdf_out("]");
    end;
    pdf_action_thread: begin
        if pdf_action_file(p) = null then 
            pdf_indirect("D", pdf_action_obj_num(p))
        else begin
            if pdf_action_id_type(p) = pdf_id_name then begin
                pdf_str_entry("D", tokens_to_string(pdf_action_name(p)));
                flush_last_string(last_tokens_string);
            end
            else
                pdf_int_entry("D", pdf_action_num(p));
        end;
    end;
    endcases;
    pdf_print_ln(" >>");
end;

@ Now we can implement pdfTeX new commands.

@<Implement \.{\\pdfliteral}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfliteral used when \pdfoutput is not set");
    new_whatsit(pdf_literal_node, write_node_size);
    if scan_keyword("direct") then
        pdf_literal_direct(tail) := 1
    else
        pdf_literal_direct(tail) := 0;
    p := scan_toks(false, true);
    pdf_literal_data(tail) := def_ref;
end

@ @<Implement \.{\\pdfobj}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfobj used when \pdfoutput is not set");
    pdf_create_obj(obj_type_others, 0);
    new_whatsit(pdf_obj_node, pdf_obj_node_size);
    pdf_obj_obj_num(tail) := obj_ptr;
    obj_obj_ptr(obj_ptr) := tail;
    if scan_keyword("stream") then 
        pdf_obj_is_stream(tail) := 1
    else
        pdf_obj_is_stream(tail) := 0;
    p := scan_toks(false, true);
    pdf_obj_data(tail) := def_ref;
    pdf_last_obj := obj_ptr;
end

@ @<Implement \.{\\pdfannot}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfannot used when \pdfoutput is not set");
    pdf_create_obj(obj_type_others, 0);
    new_annot_whatsit(pdf_annot_node, pdf_annot_node_size);
    pdf_annot_obj_num(tail) := obj_ptr;
    p := scan_toks(false, true);
    pdf_annot_data(tail) := def_ref;
    pdf_last_annot := obj_ptr;
end

@ @<Implement \.{\\pdfannotlink}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfannotlink used when \pdfoutput is not set");
    if abs(mode)<>hmode then
        pdf_error("ext1", "\pdfannotlink can be used in horizontal mode only");
    new_annot_whatsit(pdf_annot_link_node, pdf_annot_node_size);
    pdf_annot_link_action(tail) := scan_action;
end

@ @<Implement \.{\\pdfendlink}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfendlink used when \pdfoutput is not set");
    if abs(mode)<>hmode then
        pdf_error("ext1", "\pdfendlink can be used in horizontal mode only");
    new_whatsit(pdf_end_link_node, small_node_size);
end

@ @<Implement \.{\\pdfoutline}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfoutline used when \pdfoutput is not set");
    p := scan_action;
    if scan_keyword("count") then begin
        scan_int;
        i := cur_val;
    end
    else
        i := 0;
    q := scan_toks(false, true);
    pdf_new_obj(obj_type_others, 0);
    write_action(p);
    pdf_end_obj;
    delete_action_ref(p);
    pdf_create_obj(obj_type_outline, 0);
    k := obj_ptr;
    obj_outline_ptr(k) := pdf_get_mem(pdf_outline_struct_size);
    obj_outline_action_obj_num(k) := obj_ptr - 1;
    obj_outline_count(k) := i;
    pdf_new_obj(obj_type_others, 0);
    pdf_print_str_ln(tokens_to_string(def_ref));
    flush_last_string(last_tokens_string);
    delete_token_ref(def_ref);
    pdf_end_obj;
    obj_outline_title(k) := obj_ptr;
    obj_outline_prev(k) := 0;
    obj_outline_next(k) := 0;
    obj_outline_first(k) := 0;
    obj_outline_last(k) := 0;
    obj_outline_parent(k) := parent_outline;
    if first_outline = 0 then
        first_outline :=  k;
    if last_outline = 0 then begin
        if parent_outline <> 0 then
            obj_outline_first(parent_outline) := k;
    end
    else begin
        obj_outline_next(last_outline) := k;
        obj_outline_prev(k) := last_outline;
    end;
    last_outline := k;
    if obj_outline_count(k) <> 0 then begin
        parent_outline := k;
        last_outline := 0;
    end
    else if (parent_outline <> 0) and
    (outline_list_count(k) = abs(obj_outline_count(parent_outline))) then
    begin
        j := last_outline;
        repeat
            obj_outline_last(parent_outline) := j;
            j := parent_outline;
            parent_outline := obj_outline_parent(parent_outline);
        until (parent_outline = 0) or
        (outline_list_count(j) < abs(obj_outline_count(parent_outline)));
        if parent_outline = 0 then
            last_outline := first_outline
        else
            last_outline := obj_outline_first(parent_outline);
        while obj_outline_next(last_outline) <> 0 do
            last_outline := obj_outline_next(last_outline);
    end;
end

@ Notice that |scan_keyword| doesn't care if two words have same prefix; so
we should be careful when scan keywords with same prefix. The main rule: if
there are two or more keywords with the same prefix, then always test in
order from the longest one to the shortest one.

@<Implement \.{\\pdfdest}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfdest used when \pdfoutput is not set");
    new_whatsit(pdf_dest_node, pdf_dest_node_size);
    if scan_keyword("num") then begin
        pdf_dest_id_type(tail) := pdf_id_num;
        scan_int;
        if cur_val <= 0 then
            pdf_error("ext1", "num identifier must be positive");
        if cur_val > max_halfword then
            pdf_error("ext1", "number too large");
        pdf_dest_id_num(tail) := cur_val;
    end
    else if scan_keyword("name") then begin
        pdf_dest_id_type(tail) := pdf_id_name;
        p := scan_toks(false, true);
        pdf_dest_id_name(tail) := def_ref;
    end
    else
        pdf_error("ext1", "you must specify identifier type");
    if pdf_dest_id_type(tail) = pdf_id_num then
        k := get_obj(obj_type_dest_num, pdf_dest_id_num(tail), false)
    else
        k := get_obj(obj_type_dest_name, pdf_dest_id_name(tail), true);
    if obj_dest_ptr(k) <> null then begin
        pdf_warning("ext1", "destination with the same identifier (");
        if pdf_dest_id_type(tail) = pdf_id_num then begin
            print("num");
            print_int(pdf_dest_id_num(tail));
        end
        else begin
            print("name");
            print_mark(pdf_dest_id_name(tail));
        end;
        print(") already exists!");
        show_context;
    end;
    pdf_dest_obj_num(tail) := k;
    if scan_keyword("xyz") then begin
        pdf_dest_type(tail) := pdf_dest_xyz;
        if scan_keyword("zoom") then begin
            scan_int;
            if cur_val > max_halfword then
                pdf_error("ext1", "number too large");
            pdf_dest_xyz_zoom(tail) := cur_val;
        end
        else
            pdf_dest_xyz_zoom(tail) := null;
    end
    else if scan_keyword("fitbh") then
        pdf_dest_type(tail) := pdf_dest_fitb_h
    else if scan_keyword("fitbv") then
        pdf_dest_type(tail) := pdf_dest_fitb_v
    else if scan_keyword("fitb") then
        pdf_dest_type(tail) := pdf_dest_fitb
    else if scan_keyword("fith") then
        pdf_dest_type(tail) := pdf_dest_fit_h
    else if scan_keyword("fitv") then
        pdf_dest_type(tail) := pdf_dest_fit_v
    else if scan_keyword("fit") then
        pdf_dest_type(tail) := pdf_dest_fit
    else
        pdf_error("ext1", "you must specify destination type");
end

@ @<Implement \.{\\pdfthread}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfthread used when \pdfoutput is not set");
    new_whatsit(pdf_thread_node, pdf_thread_node_size);
    if scan_keyword("num") then begin
        pdf_thread_id_type(tail) := pdf_id_num;
        scan_int;
        if cur_val <= 0 then
            pdf_error("ext1", "num identifier must be positive");
        if cur_val > max_halfword then
            pdf_error("ext1", "number too large");
        pdf_thread_id_num(tail) := cur_val;
    end
    else if scan_keyword("name") then begin
        pdf_thread_id_type(tail) := pdf_id_name;
        p := scan_toks(false, true);
        pdf_thread_id_name(tail) := def_ref;
    end
    else
        pdf_error("ext1", "you must specify identifier type");
    if pdf_thread_id_type(tail) = pdf_id_num then
        last_thread := get_obj(obj_type_thread_num, pdf_thread_id_num(tail), false)
    else
        last_thread := get_obj(obj_type_thread_name, pdf_thread_id_name(tail), true);
end

@ @<Implement \.{\\pdfendthread}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfendthread used when \pdfoutput is not set");
    new_whatsit(pdf_end_thread_node, small_node_size);
end

@ For the \.{\\pdfimage} primitive we need extract immediately information
about image as width and height. All images in page will be written after the
content of page is shipped out.  |pdf_image_list| and |pdf_form_list| hold the
list of images and forms in the current page. 

@<Declare procedures needed in |do_ext...@>=
procedure scale_image(p: pointer); {scale the size of box containing image
to indeed size of image}
var x, y, xr, yr: integer; {size and resolution of image}
    w, h: scaled; {indeed size corresponds to image resolution}
    a, b: scaled; {accumulators}
begin
    x := image_width(pdf_image_info(p));
    y := image_height(pdf_image_info(p));
    xr := image_x_res(pdf_image_info(p));
    yr := image_y_res(pdf_image_info(p));
    if (x <= 0) or (y <= 0) or (xr < 0) or (yr < 0) or 
        (x > 65536) or (y > 65536) or (xr > 65536) or (yr > 65536) then
        pdf_error("ext1", "invalid image dimension");
    if (pdf_image_resolution > 0) and ((xr = 0) or (yr = 0)) then begin
        xr := pdf_image_resolution;
        yr := pdf_image_resolution;
    end;
    if (xr > 0) and (yr > 0) then begin
        {calculates |w = (x*unity*7227)/(xr*100)|}
        a := xn_over_d(unity, x, xr); b := xn_over_d(7227, remainder, xr);
        w := xn_over_d(a, 7227, 100) + x_over_n(b, 100);
        a := xn_over_d(unity, y, yr); b := xn_over_d(7227, remainder, yr);
        h := xn_over_d(a, 7227, 100) + x_over_n(b, 100);
    end
    else begin
        w := xn_over_d(unity*x, 7227, 7200);
        h := xn_over_d(unity*y, 7227, 7200);
    end;
    if is_running(pdf_width(p)) and is_running(pdf_height(p)) and
        is_running(pdf_depth(p)) then begin
        pdf_width(p) := w;
        pdf_height(p) := h;
        pdf_depth(p) := 0;
    end
    else if is_running(pdf_width(p)) then begin
        {image depth or height is explicitly specified}
        if is_running(pdf_height(p)) then begin
            {image depth is explicitly specified}
            pdf_width(p) := xn_over_d(h, x, y);
            pdf_height(p) := h - pdf_depth(p);
        end
        else if is_running(pdf_depth(p)) then begin
            {image height is explicitly specified}
            pdf_width(p) := xn_over_d(pdf_height(p), x, y);
            pdf_depth(p) := 0;
        end
        else begin
            {both image depth and height are explicitly specified}
            pdf_width(p) := xn_over_d(pdf_height(p) + pdf_depth(p), x, y);
        end;
    end
    else begin
        {image width is explicitly specified}
        if is_running(pdf_height(p)) and is_running(pdf_depth(p)) then begin
            {both image depth and height are not specified}
            pdf_height(p) := xn_over_d(pdf_width(p), y, x);
            pdf_depth(p) := 0;
        end
        else if is_running(pdf_height(p)) then begin
            {image depth is explicitly specified}
            pdf_height(p) := xn_over_d(pdf_width(p), y, x) - pdf_depth(p);
        end
        else if is_running(pdf_depth(p)) then begin
            {image height is explicitly specified}
            pdf_depth(p) := 0;
        end
        else begin
            {both image depth and height are explicitly specified}
            do_nothing;
        end;
    end;
end;

procedure append_image; {append an whatsit node for image to current list}
label reswitch;
var p: pointer;
begin
    new_whatsit(pdf_image_node, pdf_image_node_size);
    pdf_width(tail) := null_flag;
    pdf_depth(tail) := null_flag;
    pdf_height(tail) := null_flag;
reswitch:
    if scan_keyword("width") then
      begin scan_normal_dimen; pdf_width(tail):=cur_val; goto reswitch;
      end;
    if scan_keyword("height") then
      begin scan_normal_dimen; pdf_height(tail):=cur_val; goto reswitch;
      end;
    if scan_keyword("depth") then
      begin scan_normal_dimen; pdf_depth(tail):=cur_val; goto reswitch;
      end;
    p := scan_toks(false, true);
    cur_name := tokens_to_string(def_ref);
    cur_area := "";
    cur_ext := "";
    pack_cur_name;
    pdf_image_info(tail) := read_img;
    scale_image(tail);
end;

procedure append_form; {append an whatsit node for form to current list}
var r: integer;
begin
    new_whatsit(pdf_ref_form_node, pdf_ref_form_node_size);
    scan_int;
    r := head_tab[obj_type_form];
    while (r <> 0) and (r <> cur_val) do
        r := obj_link(r);
    if r = null then
        pdf_error("ext1", "cannot find form");
    if cur_val > max_halfword then
        pdf_error("ext1", "number too large");
    pdf_form_obj_num(tail) := cur_val;
    pdf_width(tail) := obj_form_width(cur_val);
    pdf_height(tail) := obj_form_height(cur_val);
    pdf_depth(tail) := obj_form_depth(cur_val);
end;

@ @<Implement \.{\\pdfimage}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfimage used when \pdfoutput is not set");
    append_image;
end

@ @<Implement \.{\\pdfform}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfform used when \pdfoutput is not set");
    scan_int;
    if box(cur_val) = null then begin
        print_err("You have tried to use a void box in a form.");
        help1("I'm discarding it.");
        error;
    end
    else begin
        incr(form_ptr);
        pdf_create_obj(obj_type_form, form_ptr);
        pdf_last_form := obj_ptr;
        obj_form_ptr(obj_ptr) := pdf_get_mem(pdf_form_struct_size);
        obj_form_width(obj_ptr) := width(box(cur_val));
        obj_form_height(obj_ptr) := height(box(cur_val));
        obj_form_depth(obj_ptr) := depth(box(cur_val));
        obj_form_box(obj_ptr) := box(cur_val); {save pointer to the box}
        box(cur_val) := null;
    end;
end

@ @<Implement \.{\\pdfrefform}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfrefform used when \pdfoutput is not set");
    append_form;
end


@ To implement primitives as \.{\\pdfinfo}, \.{\\pdfcatalog} or
\.{\\pdfnamesobj} we need to concatenate tokens lists.

@<Declare procedures needed in |do_ext...@>=
function concat_tokens(q, r: pointer): pointer; {concat |q| and |r| and
returns the result tokens list}
var p: pointer;
begin
    if q = null then begin
        concat_tokens := r;
        return;
    end;
    p := q;
    while link(p) <> null do
        p := link(p);
    link(p) := link(r);
    free_avail(r);
    concat_tokens := q;
end;

@ @<Implement \.{\\pdfinfo}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfinfo used when \pdfoutput is not set");
    p := scan_toks(false, true);
    pdf_info_toks := concat_tokens(pdf_info_toks, def_ref);
end

@ @<Implement \.{\\pdfcatalog}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\catalog used when \pdfoutput is not set");
    p := scan_toks(false, true);
    pdf_catalog_toks := concat_tokens(pdf_catalog_toks, def_ref);
    if scan_keyword("openaction") then
        if pdf_catalog_openaction <> 0 then
            pdf_error("ext1", "duplicate of openaction")
        else begin
            p := scan_action;
            pdf_new_obj(obj_type_others, 0);
            write_action(p);
            pdf_end_obj;
            delete_action_ref(p);
            pdf_catalog_openaction := obj_ptr;
        end;
end

@ @<Implement \.{\\pdfnames}@>=
begin
    if pdf_output <= 0 then
        pdf_error("ext1", "\pdfnames used when \pdfoutput is not set");
    p := scan_toks(false, true);
    pdf_names_toks := concat_tokens(pdf_names_toks, def_ref);
end
@z

@x [53.1356]
othercases print("whatsit?")
@y
pdf_literal_node: begin
    print_esc("pdfliteral");
    if pdf_literal_direct(p) > 0 then
        print(" direct");
    print_mark(pdf_literal_data(p));
end;
pdf_obj_node: begin
    print_esc("pdfobj");
    if pdf_obj_is_stream(p) = 1 then
        print(" stream");
    print_mark(pdf_obj_data(p));
end;
pdf_annot_node: begin
    print_esc("pdfannot");
    print("("); print_rule_dimen(pdf_height(p)); print_char("+");
    print_rule_dimen(pdf_depth(p)); print(")x");
    print_rule_dimen(pdf_width(p));
    print_mark(pdf_annot_data(p));
end;
pdf_annot_link_node: begin
    print_esc("pdfannotlink");
    print("("); print_rule_dimen(pdf_height(p)); print_char("+");
    print_rule_dimen(pdf_depth(p)); print(")x");
    print_rule_dimen(pdf_width(p));
    if pdf_annot_link_attr(p) <> null then begin
        print(" attr");
        print_mark(pdf_annot_link_attr(p));
    end;
    print(" action");
    if pdf_action_type(p) = pdf_action_user then begin
        print(" user");
        print_mark(pdf_action_user_tokens(p));
        return;
    end;
    if pdf_action_file(p) <> null then begin
        print(" file");
        print_mark(pdf_action_file(p));
    end;
    case pdf_action_type(p) of
    pdf_action_goto: begin
        if pdf_action_id_type(p) = pdf_id_num then begin
            print(" goto num");
            print_int(pdf_action_obj_num(p))
        end
        else begin
            print(" goto name");
            print_mark(pdf_action_name(p));
        end;
    end;
    pdf_action_page: begin
        print(" page");
        print_int(pdf_action_obj_num(p));
        print_mark(pdf_action_page_tokens(p));
    end;
    pdf_action_thread: begin
        if pdf_action_id_type(p) = pdf_id_num then begin
            print(" thread num");
            print_int(pdf_action_obj_num(p))
        end
        else begin
            print(" thread name");
            print_mark(pdf_action_name(p));
        end;
    end;
    endcases;
end;
pdf_end_link_node: print_esc("pdfendlink");
pdf_dest_node: begin
    print_esc("pdfdest");
    if pdf_dest_id_type(p) = pdf_id_num then begin
        print(" num");
        print_int(pdf_dest_id_num(p));
    end
    else begin
        print(" name");
        print_mark(pdf_dest_id_name(p));
    end;
    print(" ");
    case pdf_dest_type(tail) of
    pdf_dest_xyz: begin
        print("xyz");
        if pdf_dest_xyz_zoom(p) <> null then begin
            print(" zoom");
            print_int(pdf_dest_xyz_zoom(p));
        end;
    end;
    pdf_dest_fitb_h: print("fitbh");
    pdf_dest_fitb_v: print("fitbv");
    pdf_dest_fitb: print("fitb");
    pdf_dest_fit_h: print("fith");
    pdf_dest_fit_v: print("fitv");
    pdf_dest_fit: print("fit");
    endcases;
end;
pdf_thread_node: begin
    print_esc("pdfthread");
    if pdf_thread_id_type(p) = pdf_id_num then begin
        print(" num");
        print_int(pdf_thread_id_num(p));
    end
    else begin
        print(" name");
        print_mark(pdf_thread_id_name(p));
    end;
end;
pdf_end_thread_node: print_esc("pdfendthread");
pdf_image_node: begin
    print_esc("pdfimage");
    print("("); print_scaled(pdf_height(p)); print_char("+");
    print_scaled(pdf_depth(p)); print(")x");
    print_scaled(pdf_width(p));
end;
pdf_ref_form_node: begin
    print_esc("pdfrefform");
    print_int(pdf_form_obj_num(p));
    print("("); print_scaled(pdf_height(p)); print_char("+");
    print_scaled(pdf_depth(p)); print(")x");
    print_scaled(pdf_width(p));
end;
othercases print("whatsit?")
@z

@x [53.1357]
othercases confusion("ext2")
@y
pdf_literal_node: begin
    r := get_node(write_node_size);
    add_token_ref(pdf_literal_data(p));
    words := write_node_size;
end;
pdf_obj_node: begin
    r := get_node(pdf_obj_node_size);
    add_token_ref(pdf_obj_data(p));
    words := pdf_obj_node_size;
end;
pdf_annot_node: begin
    r := get_node(pdf_annot_node_size);
    add_token_ref(pdf_annot_data(p));
    words := pdf_annot_node_size;
end;
pdf_annot_link_node:
    copy_annot_link_node(p);
pdf_end_link_node:
    r := get_node(small_node_size);
pdf_dest_node: begin
    r := get_node(pdf_dest_node_size);
    if pdf_dest_id_type(p) = pdf_id_name then
        add_token_ref(pdf_dest_id_name(p));
    words := pdf_dest_node_size;
end;
pdf_thread_node: begin
    r := get_node(pdf_thread_node_size);
    if pdf_thread_id_type(p) = pdf_id_name then
        add_token_ref(pdf_thread_id_name(p));
    words := pdf_thread_node_size;
end;
pdf_end_thread_node: begin
    r := get_node(small_node_size);
    words := small_node_size;
end;
pdf_image_node: begin
    r := get_node(pdf_image_node_size);
    add_image_ref(pdf_image_info(p));
    words := pdf_image_node_size;
end;
pdf_ref_form_node: begin
    r := get_node(pdf_ref_form_node_size);
    words := pdf_ref_form_node_size;
end;
othercases confusion("ext2")
@z

@x [53.1358]
othercases confusion("ext3")
@y
pdf_literal_node: begin
    delete_token_ref(pdf_literal_data(p));
    free_node(p, write_node_size);
end;
pdf_obj_node: begin
    delete_token_ref(pdf_obj_data(p));
    free_node(p, pdf_obj_node_size);
end;
pdf_annot_node: begin
    delete_token_ref(pdf_annot_data(p));
    free_node(p, pdf_annot_node_size);
end;
pdf_annot_link_node: 
    delete_annot_link_node(p);
pdf_end_link_node:
    free_node(p, small_node_size);
pdf_dest_node: begin
    if pdf_dest_id_type(p) = pdf_id_name then
        delete_token_ref(pdf_dest_id_name(p));
    free_node(p, pdf_dest_node_size);
end;
pdf_thread_node: begin
    if pdf_thread_id_type(p) = pdf_id_name then
        delete_token_ref(pdf_thread_id_name(p));
    free_node(p, pdf_thread_node_size);
end;
pdf_end_thread_node:
    free_node(p, small_node_size);
pdf_image_node: begin
    delete_image_ref(pdf_image_info(p));
    free_node(p, pdf_image_node_size);
end;
pdf_ref_form_node:
    free_node(p, pdf_ref_form_node_size);
othercases confusion("ext3")
@z

@x [53.1359]
@ @<Incorporate a whatsit node into a vbox@>=do_nothing
@y
@ @<Incorporate a whatsit node into a vbox@>=
if (subtype(p) = pdf_image_node) or (subtype(p) = pdf_ref_form_node) then
begin x:=x+d+pdf_height(p); d:=pdf_depth(p);
s:=0;
if pdf_width(p)+s>w then w:=pdf_width(p)+s;
end
@z

@x [53.1360]
@ @<Incorporate a whatsit node into an hbox@>=do_nothing
@y
@ @<Incorporate a whatsit node into an hbox@>=
if (subtype(p) = pdf_image_node) or (subtype(p) = pdf_ref_form_node) then
begin x:=x+pdf_width(p);
s:=0;
if pdf_height(p)-s>h then h:=pdf_height(p)-s;
if pdf_depth(p)+s>d then d:=pdf_depth(p)+s;
end
@z

@x [53.1361]
@ @<Let |d| be the width of the whatsit |p|@>=d:=0
@y
@ @<Let |d| be the width of the whatsit |p|@>=
if (subtype(p) = pdf_image_node) or (subtype(p) = pdf_ref_form_node) then
    d := pdf_width(p)
else
    d := 0
@z

@x [53.1362]
@<Advance \(p)past a whatsit node in the \(l)|line_break| loop@>=@+
adv_past(cur_p)
@y
@<Advance \(p)past a whatsit node in the \(l)|line_break| loop@>=@+
begin
adv_past(cur_p);
if (subtype(cur_p) = pdf_image_node) or (subtype(cur_p) = pdf_ref_form_node) then
    act_width:=act_width+pdf_width(cur_p);
end
@z

@x [53.1364]
@ @<Prepare to move whatsit |p| to the current page, then |goto contribute|@>=
goto contribute
@y
@ @<Prepare to move whatsit |p| to the current page, then |goto contribute|@>=
begin
  if (subtype(p) = pdf_image_node) or (subtype(p) = pdf_ref_form_node) then
  begin page_total:=page_total+page_depth+pdf_height(p);
  page_depth:=pdf_depth(p);
  end;
  goto contribute;
end
@z

@x [53.1365]
@ @<Process whatsit |p| in |vert_break| loop, |goto not_found|@>=
goto not_found
@y
@ @<Process whatsit |p| in |vert_break| loop, |goto not_found|@>=
begin
  if (subtype(p) = pdf_image_node) or (subtype(p) = pdf_ref_form_node) then
  begin cur_height:=cur_height+prev_dp+pdf_height(p); prev_dp:=pdf_depth(p);
  end;
  goto not_found;
end
@z

@x [53.1375]
@<Implement \.{\\immediate}@>=
begin get_x_token;
if (cur_cmd=extension)and(cur_chr<=close_node) then
  begin p:=tail; do_extension; {append a whatsit node}
  out_what(tail); {do the action immediately}
  flush_node_list(tail); tail:=p; link(p):=null;
  end
else back_input;
end
@y
@<Implement \.{\\immediate}@>=
begin get_x_token;
if (cur_cmd=extension)and(cur_chr<=close_node) then
  begin p:=tail; do_extension; {append a whatsit node}
  out_what(tail); {do the action immediately}
  flush_node_list(tail); tail:=p; link(p):=null;
  end
else if (cur_cmd = extension) and (cur_chr = pdf_form_code) then begin
    do_extension;
    cur_form := pdf_last_form;
    pdf_ship_out(obj_form_box(pdf_last_form), false);
    obj_form_box(pdf_last_form) := null;
end
else if (cur_cmd = extension) and (cur_chr = pdf_obj_node) then begin
    p:=tail; do_extension;
    pdf_print_raw_obj(tail);
    flush_node_list(tail); tail:=p; link(p):=null;
end
else
    back_input;
end
@z

@x [53.1378]
@ @<Finish the extensions@>=
for k:=0 to 15 do if write_open[k] then a_close(write_file[k])
@y
@ @<Finish the extensions@>=
for k:=0 to 15 do if write_open[k] then a_close(write_file[k])

@ Shiping out \PDF{} mark.

@<Constants...@>=
@!max_dest_names=20000; {maximum number of names in name tree of \PDF{}
output file}

@ @<Types...@>=
dest_name_entry = record
    objname: str_number; {destination name}
    obj_num: integer; {destination object number}
end;

@ @<Glob...@>=
@!set_pdf_output: integer; {turn on \PDF{} output if positive}
@!pdf_include_form_resources: integer; {the default resolution of images}
@!cur_page_width: scaled; {width of page being shipped}
@!cur_page_height: scaled; {height of page being shipped}
@!cur_h_offset: scaled; {horizontal offset of page being shipped}
@!cur_v_offset: scaled; {vertical offset of page being shipped}
@!link_box_depth: integer; {depth of nesting of box containing link annotation}
@!thread_box_depth: integer; {depth of nesting of box containing article thread}
@!last_annot: integer; {pointer to the last link annotation}
@!last_thread: integer; {pointer to the last thread}
@!last_bead: integer; {pointer to the last bead}
@!pdf_link_ht, pdf_link_dp, pdf_link_wd: scaled; {dimensions of the last link
annotation}
@!pdf_obj_list: pointer; {list of objects in the current page}
@!pdf_annot_list: pointer; {list of annotations in the current page}
@!pdf_annot_link_list: pointer; {list of link annotations in the current page}
@!pdf_dest_list: pointer; {list of destinations in the current page}
@!pdf_bead_list: pointer; {list of thread beads in the current page}
@!pdf_image_list: pointer; {list of images in the current page}
@!pdf_form_list: pointer; {list of forms in the current page}
@!pdf_form_font_list: pointer; {list of fonts in XObject forms}
@!pdf_form_image_list: pointer; {list of images in XObject forms}
@!pdf_form_form_list: pointer; {list of forms in XObject forms}
@!pdf_pending_annot_link: pointer; {pointer to copy of last link annotation in
previous page}
@!image_ptr: integer; {counter of images}
@!form_ptr: integer; {counter of forms}
@!cur_form: integer; {the form being output}
@!first_outline, last_outline, parent_outline: integer;
@!pdf_form_width,
@!pdf_form_height,
@!pdf_form_depth: scaled; {dimension of of current form}
@!pdf_info_toks: pointer; {additional keys of Info dictionary}
@!pdf_catalog_toks: pointer; {additional keys of Catalog dictionary}
@!pdf_catalog_openaction: integer;
@!pdf_names_toks: pointer; {additional keys of Names dictionary}
@!dest_names_ptr: integer; {first unused position in |dest_names|}
@!dest_names: array[0..max_dest_names] of dest_name_entry;
@!image_orig_x, image_orig_y: integer; {origin of cropped pdf images}

@ @<Set init...@>=
pdf_include_form_resources := 1;
link_box_depth := -1;
thread_box_depth := -1;
first_outline:= 0;
last_outline:= 0;
parent_outline:= 0;
pdf_obj_list := null;
pdf_annot_list := null;
pdf_annot_link_list := null;
pdf_dest_list := null;
pdf_image_list := null;
pdf_bead_list := null;
pdf_form_list := null;
pdf_form_font_list := null;
pdf_form_image_list := null;
pdf_form_form_list := null;
pdf_pending_annot_link := null;
image_ptr := 0;
form_ptr := 0;
dest_names_ptr := 0;
pdf_info_toks := null;
pdf_catalog_toks := null;
pdf_names_toks := null;
pdf_catalog_openaction := 0;

@ The following procedures are needed for handling whatsit nodes for
pdfTeX.

@<Declare procedures needed in |pdf_hlist_out|, |pdf_vlist_out|@>=
procedure do_annot(p: pointer);
var w, h: scaled; {size of annotation}
begin
    if is_running(pdf_width(p)) then
        w := 0
    else
        w := pdf_width(p);
    if is_running(pdf_height(p)) then
        h := 0
    else
        h := pdf_height(p);
    if not is_running(pdf_depth(p)) then
        h := h + pdf_depth(p);
    pdf_left(p) := cur_h;
    pdf_top(p) := cur_v;
    pdf_right(p) := pdf_left(p) + w;
    pdf_bottom(p) := pdf_top(p) + h;
    obj_annot_ptr(pdf_annot_obj_num(p)) := p;
    pdf_append_list(pdf_annot_obj_num(p))(pdf_annot_list);
end;

procedure hbox_link_rect(p: pointer; v, h: scaled);
var l: integer; {pointer to whatsit node of last link annotation}
begin
    l := obj_annot_ptr(last_annot);
    pdf_left(l) := cur_h;
    if is_running(pdf_link_dp) then
        pdf_bottom(l) := v + depth(p)
    else
        pdf_bottom(l) := v + pdf_link_dp;
    if is_running(pdf_link_ht) then
        pdf_top(l) := v - height(p)
    else
        pdf_top(l) := v - pdf_link_ht;
    if is_running(pdf_link_wd) then
        pdf_right(l) := h + width(p)
    else
        pdf_right(l) := cur_h + pdf_link_wd;
end;

procedure do_annot_link(p, this_box: pointer; v, h: scaled);
begin
    if link_box_depth <> -1 then
        pdf_error("ext4", "link annotations can't be nested");
    if type(this_box) <> hlist_node then
        pdf_error("ext4", "link annotations can be inside hbox only");
    pdf_link_ht := pdf_height(p);
    pdf_link_dp := pdf_depth(p);
    pdf_link_wd := pdf_width(p);
    pdf_create_obj(obj_type_others, 0);
    obj_annot_ptr(obj_ptr) := p;
    link_box_depth := cur_s;
    last_annot := obj_ptr;
    hbox_link_rect(this_box, v, h);
    pdf_append_list(last_annot)(pdf_annot_link_list);
end;

procedure end_link(this_box: pointer);
begin
    if link_box_depth <> cur_s then
        pdf_error("ext4", "link ends in different nesting level than its start");
    if type(this_box) <> hlist_node then
        pdf_error("ext4", "link annotations can be inside hbox only");
    if is_running(pdf_link_wd) then
        pdf_right(obj_annot_ptr(last_annot)) := cur_h;
    link_box_depth := -1;
end;

@ For ``running'' annotations we must append a new node when the end of
annotation is in other box than its start. The new created node is identical to
corresponding whatsit node representing the start of annotation,  but its
|link| field is |null|. We don't free nodes created in this way in
|flush_node_list|, as for multi-page annotations it can cause troubles.

@<Declare procedures needed in |pdf_hlist_out|, |pdf_vlist_out|@>=
procedure append_annot_link(this_box: pointer; v, h: scaled); {append a new
|pdf_annot_link_node| to |pdf_annot_link_list| and update |last_annot|}
var p, r: integer;
begin
    if pdf_pending_annot_link <> null then {there is a multi-page link annotation}
    begin
        r := pdf_pending_annot_link;
        pdf_pending_annot_link := null;
    end
    else begin
        p := obj_annot_ptr(last_annot);
        copy_annot_link_node(p);
        info(r) := null; {this is not a whatsit node}
    end;
    pdf_create_obj(obj_type_others, 0);
    obj_annot_ptr(obj_ptr) := r;
    last_annot := obj_ptr;
    if type(this_box) <> hlist_node then
        pdf_error("ext4", "link annotations can be inside hbox only");
    hbox_link_rect(this_box, v, h);
    pdf_append_list(last_annot)(pdf_annot_link_list);
end;

procedure append_bead(this_box: pointer; v, h: scaled);
var b, q, r: integer;
begin
    r := get_node(pdf_bead_rect_node_size);
    pdf_left(r) := cur_h;
    pdf_top(r) := cur_v;
    pdf_right(r) := h + width(this_box);
    pdf_bottom(r) := v + height(this_box) + depth(this_box);
    q := pdf_get_mem(pdf_bead_struct_size);
    pdf_create_obj(obj_type_others, 0);
    b := obj_ptr;
    obj_bead_ptr(b) := q;
    obj_bead_page(b) := pdf_last_page;
    obj_bead_rect(b) := r;
    if obj_thread_first(last_thread) = 0 then begin
        obj_thread_first(last_thread) := b;
        obj_bead_next(b) := b;
        obj_bead_prev(b) := b;
    end
    else begin
        q := obj_thread_first(last_thread);
        r := obj_bead_prev(q);
        obj_bead_prev(b) := r;
        obj_bead_next(b) := q;
        obj_bead_prev(q) := b;
        obj_bead_next(r) := b;
    end;
    last_bead := b;
    pdf_append_list(b)(pdf_bead_list);
end;

procedure do_thread(p, this_box: integer; v, h: scaled);
begin
    if thread_box_depth <> -1 then
        pdf_error("ext4", "article threads can't be nested");
    thread_box_depth := cur_s;
    append_bead(this_box, v, h);
end;

procedure end_thread;
begin
    if thread_box_depth <> cur_s then
        pdf_error("ext4", "thread ends in different nesting level than its start");
    pdf_bottom(obj_bead_rect(last_bead)) := cur_v;
    thread_box_depth := -1;
end;

function open_subentries(p: pointer): integer;
var k, c: integer;
    l, r: integer;
begin
    k := 0;
    if obj_outline_first(p) <> 0 then begin
        l := obj_outline_first(p);
        repeat
            incr(k);
            c := open_subentries(l);
            if obj_outline_count(l) > 0 then
                k := k + c;
            obj_outline_parent(l) := p;
            r := obj_outline_next(l);
            if r = 0 then
                obj_outline_last(p) := l;
            l := r;
        until l = 0;
    end;
    if obj_outline_count(p) > 0 then
        obj_outline_count(p) := k
    else
        obj_outline_count(p) := -k;
    open_subentries := k;
end;

procedure do_dest(p, this_box: pointer);
begin
    case pdf_dest_type(p) of
    pdf_dest_xyz: begin
        pdf_left(p) := cur_h;
        pdf_top(p) := cur_v;
    end;
    pdf_dest_fit_h,
    pdf_dest_fitb_h:
        pdf_top(p) := cur_v;
    pdf_dest_fit_v,
    pdf_dest_fitb_v:
        pdf_left(p) := cur_h;
    pdf_dest_fit,
    pdf_dest_fitb:
        do_nothing;
    endcases;
    obj_dest_ptr(pdf_dest_obj_num(p)) := p;
    pdf_append_list(pdf_dest_obj_num(p))(pdf_dest_list);
end;

procedure out_image(p: pointer);
begin
    pdf_end_text;
    pdf_print_ln("q");
    if not is_pdf_image(pdf_image_info(p)) then begin
        pdf_print_bp(pdf_width(p)); pdf_print(" 0 0 ");
        pdf_print_bp(pdf_height(p) + pdf_depth(p)); pdf_out(" ");
        pdf_print_bp(pdf_x(cur_h)); pdf_out(" ");
        pdf_print_bp(pdf_y(cur_v));
    end
    else begin
        pdf_print_bp(x_over_n(pdf_width(p), image_width(pdf_image_info(p))));
        pdf_print(" 0 0 ");
        pdf_print_bp(x_over_n(pdf_height(p) + pdf_depth(p),
                     image_height(pdf_image_info(p))));
        pdf_out(" ");
        pdf_print_bp(pdf_x(cur_h) -
           xn_over_d(pdf_width(p),
               epdf_orig_x(pdf_image_info(p)),
               image_width(pdf_image_info(p))));
        pdf_out(" ");
        pdf_print_bp(pdf_y(cur_v) -
           xn_over_d(pdf_height(p) + pdf_depth(p),
               epdf_orig_y(pdf_image_info(p)),
               image_height(pdf_image_info(p))));
    end;
    pdf_print_ln(" cm");
    incr(image_ptr);
    pdf_create_obj(obj_type_others, image_ptr);
    obj_image_ptr(obj_ptr) := p;
    pdf_append_list(obj_ptr)(pdf_image_list);
    pdf_out("/");
    pdf_print(pdf_image_prefix_str);
    pdf_print_int(image_ptr);
    pdf_print_ln(" Do");
    pdf_print_ln("Q");
end;

procedure out_form(p: pointer);
begin
    pdf_end_text;
    pdf_print_ln("q");
    if pdf_lookup_list(pdf_form_list, pdf_form_obj_num(p)) = null then
        pdf_append_list(pdf_form_obj_num(p))(pdf_form_list);
    cur_v := cur_v + obj_form_depth(pdf_form_obj_num(p));
    pdf_print("1 0 0 1 ");
    pdf_print_bp(pdf_x(cur_h)); pdf_out(" ");
    pdf_print_bp(pdf_y(cur_v));
    pdf_print_ln(" cm");
    pdf_out("/");
    pdf_print(pdf_form_prefix_str);
    pdf_print_int(obj_info(pdf_form_obj_num(p)));
    pdf_print_ln(" Do");
    pdf_print_ln("Q");
end;

@ @<Output the whatsit node |p| in |pdf_vlist_out|@>=
case subtype(p) of
pdf_obj_node,
pdf_annot_node,
pdf_end_link_node,
pdf_dest_node,
pdf_thread_node,
pdf_end_thread_node:
    case subtype(p) of
        pdf_obj_node: begin
            obj_obj_ptr(pdf_obj_obj_num(p)) := p;
{we need to do this step, because |p| might be copied from other node}
            pdf_append_list(pdf_obj_obj_num(p))(pdf_obj_list);
        end;
        pdf_annot_node:
            do_annot(p);
        pdf_dest_node:
            do_dest(p, this_box);
        pdf_thread_node:
            do_thread(p, this_box, top_edge, left_edge);
        pdf_end_thread_node:
            end_thread;
    endcases;
pdf_literal_node:
    pdf_out_literal(p);
pdf_ref_form_node:
    @<Output a Form node in a vlist@>;
pdf_image_node:
    @<Output a Image node in a vlist@>;
special_node:
    pdf_special(p);
othercases out_what(p);
endcases

@ @<Output a Image node in a vlist@>=
begin cur_v:=cur_v+pdf_height(p)+pdf_depth(p); save_v:=cur_v;
  cur_h:=left_edge;
  out_image(p);
  cur_v:=save_v; cur_h:=left_edge;
  end

@ @<Output a Form node in a vlist@>=
begin cur_v:=cur_v+pdf_height(p); save_v:=cur_v;
  cur_h:=left_edge;
  out_form(p);
  cur_v:=save_v+pdf_depth(p); cur_h:=left_edge;
  end

@ @<Output the whatsit node |p| in |pdf_hlist_out|@>=
case subtype(p) of
pdf_obj_node,
pdf_annot_node,
pdf_annot_link_node,
pdf_end_link_node,
pdf_dest_node,
pdf_thread_node,
pdf_end_thread_node:
    case subtype(p) of
        pdf_obj_node: begin
            obj_obj_ptr(pdf_obj_obj_num(p)) := p;
{we need to do this step, because |p| might be copied from other node}
            pdf_append_list(pdf_obj_obj_num(p))(pdf_obj_list);
        end;
        pdf_annot_node:
            do_annot(p);
        pdf_annot_link_node:
            do_annot_link(p, this_box, base_line, left_edge);
        pdf_end_link_node:
            end_link(this_box);
        pdf_dest_node:
            do_dest(p, this_box);
        pdf_thread_node:
            do_thread(p, this_box, base_line, left_edge);
        pdf_end_thread_node:
            end_thread;
    endcases;
pdf_literal_node:
    pdf_out_literal(p);
pdf_image_node:
    @<Output a Image node in a hlist@>;
pdf_ref_form_node:
    @<Output a Form node in a hlist@>;
special_node:
    pdf_special(p);
othercases out_what(p);
endcases

@ @<Output a Image node in a hlist@>=
begin
  cur_v:=base_line+pdf_depth(p);
  edge:=cur_h;
  out_image(p);
  cur_h:=edge+pdf_width(p); cur_v:=base_line;
  end

@ @<Output a Form node in a hlist@>=
begin
  cur_v:=base_line;
  edge:=cur_h;
  out_form(p);
  cur_h:=edge+pdf_width(p); cur_v:=base_line;
  end

@ @<Generate the \PDF{} mark (if any) for the current box in |pdf_vlist_out|@>=
if (thread_box_depth <> -1) and (thread_box_depth = cur_s) then
    append_bead(this_box, top_edge, left_edge)

@ @<Generate the \PDF{} mark (if any) for the current box in |pdf_hlist_out|@>=
if (link_box_depth <> -1) and (link_box_depth = cur_s) and
    is_running(pdf_link_wd) then
    append_annot_link(this_box, base_line, left_edge);
if (thread_box_depth <> -1) and (thread_box_depth = cur_s) then
    append_bead(this_box, base_line, left_edge)
@z
