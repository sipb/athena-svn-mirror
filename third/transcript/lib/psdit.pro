% psdit.pro -- prolog for ditroff translator
% Copyright (c) 1985,1987,1992 Adobe Systems Incorporated. All Rights
% Reserved.  
% GOVERNMENT END USERS: See Notice file in TranScript library directory
% -- probably /usr/lib/ps/Notice
% RCS: $Header: /afs/dev.mit.edu/source/repository/third/transcript/lib/psdit.pro,v 1.1.1.1 1996-10-07 20:25:31 ghudson Exp $
/$DITroff 180 dict def $DITroff begin

% Psfig additions
/DocumentInitState [ matrix currentmatrix currentlinewidth currentlinecap
currentlinejoin currentdash currentgray currentmiterlimit ] cvx def

/startFig {
	/SavedState save def
	userdict maxlength dict begin
	currentpoint transform

	DocumentInitState setmiterlimit setgray setdash setlinejoin setlinecap
		setlinewidth setmatrix

	itransform moveto

	/ury exch def
	/urx exch def
	/lly exch def
	/llx exch def
	/y exch 72 mul resolution div def
	/x exch 72 mul resolution div def
	
	currentpoint /cy exch def /cx exch def

	/sx x urx llx sub div def 	% scaling for x
	/sy y ury lly sub div def	% scaling for y

	sx sy scale			% scale by (sx,sy)

	cx sx div llx sub
	cy sy div ury sub translate
	
	/DefFigCTM matrix currentmatrix def

	/initmatrix {
		DefFigCTM setmatrix
	} def
	/defaultmatrix {
		DefFigCTM exch copy
	} def

	/initgraphics {
		DocumentInitState setmiterlimit setgray setdash 
			setlinejoin setlinecap setlinewidth setmatrix
		DefFigCTM setmatrix
	} def

	/showpage {
		initgraphics
	} def

} def
% Args are llx lly urx ury (in figure coordinates)
/clipFig {
	currentpoint 6 2 roll
	newpath 4 copy
	4 2 roll moveto
	6 -1 roll exch lineto
	exch lineto
	exch lineto
	closepath clip
	newpath
	moveto
} def
% doclip, if called, will always be just after a `startfig'
/doclip { llx lly urx ury clipFig } def
/endFig {
	end SavedState restore
} def
/globalstart {
	% Push details about the enviornment on the stack.
	fontnum fontsize fontslant fontheight firstpage 
	mh my resolution slotno currentpoint 
	pagesave restore gsave 
} def
/globalend {
	grestore moveto
	/slotno exch def /resolution exch def /my exch def
	/mh exch def /firstpage exch def /fontheight exch def
	/fontslant exch def /fontsize exch def /fontnum exch def
	F
	/pagesave save def
} def

%% end XMOD additions
/fontnum 1 def /fontsize 10 def /fontheight 10 def /fontslant 0 def
/xi {/fontnum 1 def /fontsize 10 def /fontheight 10 def /fontslant 0 def F
  }def
/ps {/pagesave save def 72 mul 0 exch translate 72 resolution div dup neg
      scale 0 0 moveto } def
/PB{save /psv exch def currentpoint translate
  resolution 72 div dup neg scale 0 0 moveto}def
/PE{psv restore}def
/m1 matrix def /m2 matrix def /m3 matrix def /oldmat matrix def
/tan{dup sin exch cos div}bind def
/point{resolution 72 div mul}bind def
/dround	{transform round exch round exch itransform}bind def
/xT{/devname exch def}def
/xr{/mh exch def /my exch def /resolution exch def}def
/xp{}def
/xs{docsave restore end}def
/xt{}def
/xf{/fontname exch def /slotno exch def fontnames slotno get fontname eq not
 {fonts slotno fontname findfont put fontnames slotno fontname put}if}def
/xH{/fontheight exch def F}bind def
/xS{/fontslant exch def F}bind def
/s{/fontsize exch def /fontheight fontsize def F}bind def
/f{/fontnum exch def F}bind def
/F{fontheight 0 le {/fontheight fontsize def}if
   fonts fontnum get fontsize point 0 0 fontheight point neg 0 0 m1 astore
   fontslant 0 ne{1 0 fontslant neg tan 1 0 0 m2 astore m3 concatmatrix}if
   makefont setfont .04 fontsize point mul 0 dround pop setlinewidth}bind def
/X{exch currentpoint exch pop moveto show}bind def
/N{3 1 roll moveto show}bind def
/Y{exch currentpoint pop exch moveto show}bind def
/S /show load def
/ditpush{}def/ditpop{}def
/AX{3 -1 roll currentpoint exch pop moveto 0 exch ashow}bind def
/AN{4 2 roll moveto 0 exch ashow}bind def
/AY{3 -1 roll currentpoint pop exch moveto 0 exch ashow}bind def
/AS{0 exch ashow}bind def
/MX{currentpoint exch pop moveto}bind def
/MY{currentpoint pop exch moveto}bind def
/MXY /moveto load def
/cb{pop}def	% action on unknown char -- nothing for now
/n{}def/w{}def
/p{pop pagesave restore showpage}def
/abspoint{currentpoint exch pop add exch currentpoint pop add exch}def
/dstroke{currentpoint stroke moveto}bind def
/Dl{2 copy gsave rlineto stroke grestore rmoveto}bind def
/arcellipse{oldmat currentmatrix pop
 currentpoint translate 1 diamv diamh div scale /rad diamh 2 div def
 rad 0 rad -180 180 arc oldmat setmatrix}def
/Dc{gsave dup /diamv exch def /diamh exch def arcellipse dstroke 
    grestore diamh 0 rmoveto}def
/De{gsave /diamv exch def /diamh exch def arcellipse dstroke
    grestore diamh 0 rmoveto}def
/Da{currentpoint /by exch def /bx exch def /fy exch def /fx exch def
   /cy exch def /cx exch def /rad cx cx mul cy cy mul add sqrt def
   /ang1 cy neg cx neg atan def /ang2 fy fx atan def cx bx add cy by add
   2 copy rad ang1 ang2 arcn stroke exch fx add exch fy add moveto}def
/Barray 200 array def % 200 values in a wiggle
/D~{mark}def
/D~~{counttomark Barray exch 0 exch getinterval astore /Bcontrol exch def pop
 /Blen Bcontrol length def Blen 4 ge Blen 2 mod 0 eq and
 {Bcontrol 0 get Bcontrol 1 get abspoint /Ycont exch def /Xcont exch def
  Bcontrol 0 2 copy get 2 mul put Bcontrol 1 2 copy get 2 mul put
  Bcontrol Blen 2 sub 2 copy get 2 mul put
  Bcontrol Blen 1 sub 2 copy get 2 mul put
  /Ybi /Xbi currentpoint 3 1 roll def def 0 2 Blen 4 sub
  {/i exch def
   Bcontrol i get 3 div Bcontrol i 1 add get 3 div
   Bcontrol i get 3 mul Bcontrol i 2 add get add 6 div
   Bcontrol i 1 add get 3 mul Bcontrol i 3 add get add 6 div
   /Xbi Xcont Bcontrol i 2 add get 2 div add def
   /Ybi Ycont Bcontrol i 3 add get 2 div add def
   /Xcont Xcont Bcontrol i 2 add get add def
   /Ycont Ycont Bcontrol i 3 add get add def
   Xbi currentpoint pop sub Ybi currentpoint exch pop sub rcurveto
  }for dstroke}if}def
end
/ditstart{$DITroff begin
 /nfonts 60 def			% NFONTS makedev/ditroff dependent!
 /fonts[nfonts{0}repeat]def
 /fontnames[nfonts{()}repeat]def
/docsave save def
}def

% character outcalls
/oc {/pswid exch def /cc exch def /name exch def
   /ditwid pswid fontsize mul resolution mul 72000 div def
   /ditsiz fontsize resolution mul 72 div def
   ocprocs name known{ocprocs name get exec}{name cb}
   ifelse}def
/fractm [.65 0 0 .6 0 0] def
/fraction
 {/fden exch def /fnum exch def gsave /cf currentfont def
  cf fractm makefont setfont 0 .3 dm 2 copy neg rmoveto
  fnum show rmoveto currentfont cf setfont(\244)show setfont fden show 
  grestore ditwid 0 rmoveto} def
/oce {grestore ditwid 0 rmoveto}def
/dm {ditsiz mul}def
/ocprocs 50 dict def ocprocs begin
(14){(1)(4)fraction}def
(12){(1)(2)fraction}def
(34){(3)(4)fraction}def
(13){(1)(3)fraction}def
(23){(2)(3)fraction}def
(18){(1)(8)fraction}def
(38){(3)(8)fraction}def
(58){(5)(8)fraction}def
(78){(7)(8)fraction}def
(sr){gsave .05 dm .16 dm rmoveto(\326)show oce}def
(is){gsave 0 .15 dm rmoveto(\362)show oce}def
(->){gsave 0 .02 dm rmoveto(\256)show oce}def
(<-){gsave 0 .02 dm rmoveto(\254)show oce}def
(==){gsave 0 .05 dm rmoveto(\272)show oce}def
end
%%BeginResource: font DIThacks
% DIThacks fonts for some special chars
50 dict dup begin
/FontType 3 def
/FontName /DIThacks def
/FontMatrix [.001 0.0 0.0 .001 0.0 0.0] def
/FontBBox [-220 -280 900 900] def% a lie but ...
/Encoding 256 array def
0 1 255{Encoding exch /.notdef put}for
Encoding
 dup 8#040/space put %space
 dup 8#110/rc put %right ceil
 dup 8#111/lt put %left  top curl
 dup 8#112/bv put %bold vert
 dup 8#113/lk put %left  mid curl
 dup 8#114/lb put %left  bot curl
 dup 8#115/rt put %right top curl
 dup 8#116/rk put %right mid curl
 dup 8#117/rb put %right bot curl
 dup 8#120/rf put %right floor
 dup 8#121/lf put %left  floor
 dup 8#122/lc put %left  ceil
 dup 8#140/sq put %square
 dup 8#141/bx put %box
 dup 8#142/ci put %circle
 dup 8#143/br put %box rule
 dup 8#144/rn put %root extender
 dup 8#145/vr put %vertical rule
 dup 8#146/ob put %outline bullet
 dup 8#147/bu put %bullet
 dup 8#150/ru put %rule
 dup 8#151/ul put %underline
 pop
/DITfd 100 dict def
/BuildChar{0 begin
 /cc exch def /fd exch def
 /charname fd /Encoding get cc get def
 /charwid fd /Metrics get charname get def
 /charproc fd /CharProcs get charname get def
 charwid 0 fd /FontBBox get aload pop setcachedevice
 40 setlinewidth
 newpath 0 0 moveto gsave charproc grestore
 end}def
/BuildChar load 0 DITfd put
%/UniqueID 5 def
/CharProcs 50 dict def
CharProcs begin
/space{}def
/.notdef{}def
/ru{500 0 rls}def
/rn{0 750 moveto 500 0 rls}def
/vr{20 800 moveto 0 -770 rls}def
/bv{20 800 moveto 0 -1000 rls}def
/br{20 770 moveto 0 -1040 rls}def
/ul{0 -250 moveto 500 0 rls}def
/ob{200 250 rmoveto currentpoint newpath 200 0 360 arc closepath stroke}def
/bu{200 250 rmoveto currentpoint newpath 200 0 360 arc closepath fill}def
/sq{80 0 rmoveto currentpoint dround newpath moveto
    640 0 rlineto 0 640 rlineto -640 0 rlineto closepath stroke}def
/bx{80 0 rmoveto currentpoint dround newpath moveto
    640 0 rlineto 0 640 rlineto -640 0 rlineto closepath fill}def
/ci{355 333 rmoveto currentpoint newpath 333 0 360 arc
    50 setlinewidth stroke}def

/lt{20 -200 moveto 0 550 rlineto currx 800 2cx s4 add exch s4 a4p stroke}def
/lb{20 800 moveto 0 -550 rlineto currx -200 2cx s4 add exch s4 a4p stroke}def
/rt{20 -200 moveto 0 550 rlineto currx 800 2cx s4 sub exch s4 a4p stroke}def
/rb{20 800 moveto 0 -500 rlineto currx -200 2cx s4 sub exch s4 a4p stroke}def
/lk{20 800 moveto 20 300 -280 300 s4 arcto pop pop 1000 sub
    currentpoint stroke moveto
    20 300 4 2 roll s4 a4p 20 -200 lineto stroke}def
/rk{20 800 moveto 20 300 320 300 s4 arcto pop pop 1000 sub
    currentpoint stroke moveto
    20 300 4 2 roll s4 a4p 20 -200 lineto stroke}def
/lf{20 800 moveto 0 -1000 rlineto s4 0 rls}def
/rf{20 800 moveto 0 -1000 rlineto s4 neg 0 rls}def
/lc{20 -200 moveto 0 1000 rlineto s4 0 rls}def
/rc{20 -200 moveto 0 1000 rlineto s4 neg 0 rls}def
end

/Metrics 50 dict def Metrics begin
/.notdef 0 def
/space 500 def
/ru 500 def
/br 0 def
/lt 250 def
/lb 250 def
/rt 250 def
/rb 250 def
/lk 250 def
/rk 250 def
/rc 250 def
/lc 250 def
/rf 250 def
/lf 250 def
/bv 250 def
/ob 350 def
/bu 350 def
/ci 750 def
/bx 750 def
/sq 750 def
/rn 500 def
/ul 500 def
/vr 0 def
end

DITfd begin
/s2 500 def /s4 250 def /s3 333 def
/a4p{arcto pop pop pop pop}def
/2cx{2 copy exch}def
/rls{rlineto stroke}def
/currx{currentpoint pop}def
/dround{transform round exch round exch itransform} def
end
end
/DIThacks exch definefont pop
%%EndResource 
