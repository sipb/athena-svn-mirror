% Start of enscript.pro -- prolog for text file translator
% Copyright (c) 1984,1985,1987 Adobe Systems Incorporated. All Rights Reserved. 
% GOVERNMENT END USERS: See Notice file in TranScript library directory
% -- probably /usr/lib/ps/Notice
% RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/lib/enscript.pro,v 1.1.1.1 1996-10-07 20:25:31 ghudson Exp $
/$enscript 50 dict def $enscript begin
/S/show load def
/bd { bind def } bind def
/X{exch 0 rmoveto S} bd
/Y{exch 0 exch rmoveto S} bd
/B{3 1 roll moveto S} bd
/F{$fd exch get setfont} def
/U{1440 mul}def
/UP{U 72 div}def
/StartPage{/svpg save def .05 dup scale} bd
/EndPage{svpg restore showpage} bd
/DoPreFeed{/statusdict where{pop
 statusdict/prefeed known{statusdict exch/prefeed exch put 0}if}if pop} bd
/SetUpFonts
 {dup/$fd exch array def{findfont reef exch scalefont $fd 3 1 roll
 put}repeat}def 
/InitGaudy{/Columns exch def
 /ftD /Times-Bold findfont 12 UP scalefont def
 /ftF /Times-Roman findfont 14 UP scalefont def
 /ftP /Helvetica-Bold findfont 30 UP scalefont def} bd
/LB{/pts exch UP def /charcolor exch def /boxcolor exch def /font exch def
 /label exch def /dy exch def /dx exch def /lly exch def /llx exch def
 gsave boxcolor setgray
 llx lly moveto dx 0 rlineto 0 dy rlineto dx neg 0 rlineto closepath fill
 /lines label length def
 /yp lly dy add dy lines pts mul sub 2 div sub pts .85 mul sub def
 font setfont charcolor setgray
 label {dup stringwidth pop 2 div llx dx 2 div add exch sub yp moveto show
   /yp yp pts sub def}forall grestore} bd
/Gaudy{/plength exch def /BarLength exch def /hy exch def /hx exch def
 /Page exch def /Date exch def  /File exch def /Comment exch def
 hx hy  BarLength 144 sub .25 U [File] ftF .97 0 14 LB
 hx hy 360 add BarLength 144 sub .25 U [Comment] ftF 1 0 14 LB
 hx hy 1 U .5 U Date ftD .7 0 12 LB
 BarLength 1080 sub hy 1 U .5 U [Page] ftP .7 1 30 LB
 2 1 Columns {1 sub plength Columns div mul hy moveto 0 0 hy sub rlineto
 stroke}for } bd
/Landscape { 90 rotate 0 exch translate } bd
