%% 
%% This is file `dinbrief.cls',
%% generated with the docstrip utility.
%% 
%% The original source files were:
%% 
%% dinbrief.dtx  (with options: `class')
%% 
%% Copyright (C) 1993, 96 by Richard Gussmann, University of Karlsruhe. 
%% For additional copyright information see further down in this file. 
%% 
%% This file is part of the dinbrief-class. 
%% ---------------------------------------- 
%% 
%% This system is distributed in the hope that it will be useful, 
%% but WITHOUT ANY WARRANTY; without even the implied warranty of 
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
%% 
%% 
%% IMPORTANT NOTICE: 
%% 
%% For error reports in case of UNCHANGED versions see readme files. 
%% 
%% Please do not request updates from us directly. Distribution is 
%% done through Mail-Servers and TeX organizations. 
%% 
%% You are not allowed to change this file. 
%% 
%% You are allowed to distribute this file under the condition that 
%% it is distributed together with all files mentioned in 00readme.. 
%% 
%% If you receive only some of these files from someone, complain! 
%% 
%% You are NOT ALLOWED to distribute this file alone. You are NOT 
%% ALLOWED to take money for the distribution or use of either this 
%% file or a changed version, except for a nominal charge for copying 
%% etc. 
%%
%% \CharacterTable
%%  {Upper-case    \A\B\C\D\E\F\G\H\I\J\K\L\M\N\O\P\Q\R\S\T\U\V\W\X\Y\Z
%%   Lower-case    \a\b\c\d\e\f\g\h\i\j\k\l\m\n\o\p\q\r\s\t\u\v\w\x\y\z
%%   Digits        \0\1\2\3\4\5\6\7\8\9
%%   Exclamation   \!     Double quote  \"     Hash (number) \#
%%   Dollar        \$     Percent       \%     Ampersand     \&
%%   Acute accent  \'     Left paren    \(     Right paren   \)
%%   Asterisk      \*     Plus          \+     Comma         \,
%%   Minus         \-     Point         \.     Solidus       \/
%%   Colon         \:     Semicolon     \;     Less than     \<
%%   Equals        \=     Greater than  \>     Question mark \?
%%   Commercial at \@     Left bracket  \[     Backslash     \\
%%   Right bracket \]     Circumflex    \^     Underscore    \_
%%   Grave accent  \`     Left brace    \{     Vertical bar  \|
%%   Right brace   \}     Tilde         \~}
%%
\def\RCSdate{$Date: 2000-03-10 18:03:20 $}
\def\RCSrevision{$Revision: 1.1.1.1 $}
{%
  \def\stripone $#1${\def\partone{#1}}
  \def\striptwo Date: #1\stop{\gdef\filedate{#1}\gdef\docdate{#1}}
  \expandafter\stripone\RCSdate
  \expandafter\striptwo\partone\stop
}%
{%
  \def\stripone $#1${\def\partone{#1}}
  \def\striptwo Revision: #1\stop{\gdef\fileversion{#1}}
  \expandafter\stripone\RCSrevision
  \expandafter\striptwo\partone\stop
}%
\def\filename{dinbrief.dtx}
\newif\ifka@db@ltxtwoe
\ka@db@ltxtwoefalse
\newif\ifka@db@nfss
\ka@db@nfssfalse
\newif\ifka@db@nfsstwo
\ka@db@nfsstwofalse
\expandafter\ifx\csname documentclass\endcsname\relax\else
    \ka@db@ltxtwoetrue
  \fi
\expandafter\ifx\csname size\endcsname\relax\else
    \ka@db@nfsstrue
  \fi
\expandafter\ifx\csname fontsize\endcsname\relax\else
    \ka@db@nfsstwotrue
  \fi
\typeout{}
\typeout{Document Class/Style 'dinbrief' - %
         (Preliminary) Version \fileversion}
\typeout{University of Karlsruhe - \filedate}
\typeout{}
\ifka@db@ltxtwoe
    \typeout{*** dinbrief: Running in LaTeX 2e mode!}
  \else
    \typeout{*** dinbrief: Running in LaTeX 2.09 mode!}
  \fi
\ifka@db@nfss
    \typeout{*** dinbrief: NFSS!}
  \else
    \ifka@db@nfsstwo
      \typeout{*** dinbrief: NFSS 2!}
    \else
      \typeout{*** dinbrief: original LaTeX 2.09 font %
               selection mechanism found!!!}
    \fi
  \fi
\ifka@db@ltxtwoe
  \NeedsTeXFormat{LaTeX2e}[1994/12/01]
  \ProvidesClass{dinbrief}[1995/07/01 LaTeX2e class]
\fi
\newdimen\ka@db@tmpdima
\long\def\ka@db@assignvarglobal#1#2{%
  \protect\pka@db@assignvarglobal{#1}{#2}}%
\long\def\pka@db@assignvarglobal#1#2{%
  % \bgroup
  %   \let\\=\relax
  %   \message{assignvarglobal: #1 = #2}%
  % \egroup
  \global\@namedef{ka@db@g@#1}{#2}%
}
\long\def\ka@db@assignvarlocal#1#2{%
  \protect\pka@db@assignvarlocal{#1}{#2}}%
\long\def\pka@db@assignvarlocal#1#2{%
  % \bgroup
  %   \let\\=\relax
  %   \message{assignvarlocal: #1 = #2}%
  % \egroup
  \global\@namedef{ka@db@l@#1}{#2}%
}
\def\ka@db@declvar#1{%
  \ka@db@assignvarglobal{#1}{}%
  \ka@db@assignvarlocal{#1}{}%
}
\long\def\ka@db@assignvar#1#2{\protect\pka@db@assignvar{#1}{#2}}%
\long\def\pka@db@assignvar#1#2{%
  % \message{assignvar: #1 = #2}%
  \ifnum\c@lt@state>0\relax
    \pka@db@assignvarlocal{#1}{#2}%
  \else
    \pka@db@assignvarglobal{#1}{#2}%
  \fi
}
\def\ka@db@usevarglobal#1{\protect\pka@db@usevarglobal{#1}}%
\def\pka@db@usevarglobal#1{%
  \@nameuse{ka@db@g@#1}%
}
\def\ka@db@usevarlocal#1{\protect\pka@db@usevarlocal{#1}}%
\def\pka@db@usevarlocal#1{%
  \@nameuse{ka@db@l@#1}%
}
\def\ka@db@usevar#1{%
  \bgroup
    \def\protect{\noexpand\protect\noexpand}%
    \let\\=\relax
    \edef\tmpa{\pka@db@usevarlocal{#1}}%
    % \typeout{usevar: varlocal #1 = \meaning\tmpa}%
    \expandafter\egroup
    \ifx\tmpa\@empty
      % \message{Using global var, because local var is empty!}%
      \pka@db@usevarglobal{#1}%
    \else
      % \message{Using local var!}%
      \pka@db@usevarlocal{#1}%
    \fi
}
\long\def\ka@db@ifemptydynvar#1#2#3#4{%
  \bgroup
    \def\protect{\noexpand\protect\noexpand}%
    \let\\=a% because characters are not expanded
    \edef\tmpa{#1{#2}}%
    % \typeout{ifempty: \string#1\space var #2 = \meaning#1{#2}}%
    % \typeout{ifempty: \string#1\space var #2 = \meaning\tmpa}%
    \expandafter\egroup
    \ifx\tmpa\@empty
      % \message{var is empty!}%
      #3% <- otherwise a space is inserted
    \else
      % \message{var is not empty!}%
      #4% <- otherwise a space is inserted
    \fi
}
\long\def\ka@db@ifemptyvarlocal#1#2#3{%
  \ka@db@ifemptydynvar{\pka@db@usevarlocal}{#1}{#2}{#3}%
}
\long\def\ka@db@ifemptyvarglobal#1#2#3{%
  \ka@db@ifemptydynvar{\pka@db@usevarglobal}{#1}{#2}{#3}%
}
\long\def\ka@db@ifemptyvar#1#2#3{%
  \ka@db@ifemptyvarlocal{#1}{% then
    \ka@db@ifemptyvarglobal{#1}{#2}{#3}%
    }{#3}%
}
\def\ka@db@execatletter#1{%
  \bgroup
    \let\\=\relax
    \def\protect{\noexpand\protect\noexpand}%
    \xdef\ka@db@atletter{\ka@db@atletter #1}
  \egroup
}
\def\ka@db@atletter{}
\def\ka@db@execatendletter#1{%
  \bgroup
    \let\\=\relax
    \def\protect{\noexpand\protect\noexpand}%
    \xdef\ka@db@atendletter{\ka@db@atendletter #1}
  \egroup
}
\def\ka@db@atendletter{}
\ifka@db@ltxtwoe
  \newcommand\@ptsize{}
  \DeclareOption{10pt}{\renewcommand\@ptsize{0}}
  \DeclareOption{11pt}{\renewcommand\@ptsize{1}}
  \DeclareOption{12pt}{\renewcommand\@ptsize{2}}
  \DeclareOption{norm}{\renewcommand\@ptsize{3}}
\else
  \def\@ptsize{0}
  \@namedef{ds@10pt}{\def\@ptsize{0}}
  \@namedef{ds@11pt}{\def\@ptsize{1}}
  \@namedef{ds@12pt}{\def\@ptsize{2}}
  \@namedef{ds@norm}{\def\@ptsize{3}}
\fi
{%
  \catcode`\_=12
  \gdef\jobname@aux{\jobname.aux}% .aux or _aux or ...
  \gdef\jobname@lbl{\jobname.lbl}%
}%
\newwrite\@mainlbl
\newif\if@restonecol
\ifka@db@ltxtwoe
  \DeclareOption{a4paper}
     {\setlength\paperheight {297mm}%
      \setlength\paperwidth  {210mm}}
  \DeclareOption{a5paper}
     {\setlength\paperheight {210mm}%
      \setlength\paperwidth  {148mm}}
  \DeclareOption{b5paper}
     {\setlength\paperheight {250mm}%
      \setlength\paperwidth  {176mm}}
  \DeclareOption{letterpaper}
     {\setlength\paperheight {11in}%
      \setlength\paperwidth  {8.5in}}
  \DeclareOption{legalpaper}
     {\setlength\paperheight {14in}%
      \setlength\paperwidth  {8.5in}}
  \DeclareOption{executivepaper}
     {\setlength\paperheight {10.5in}%
      \setlength\paperwidth  {7.25in}}
\fi
\ifka@db@ltxtwoe
  \DeclareOption{twoside}{\@twosidetrue}
\fi
\@twosidefalse
\def\ds@draft{\overfullrule 5pt}
\def\ds@final{\overfullrule 0pt}
\ifka@db@ltxtwoe
  \DeclareOption{onecolumn}{\@twocolumnfalse}
  \DeclareOption{twocolumn}{\@twocolumntrue}
\fi
\ifka@db@nfss
    \let\ka@db@selectfont\selectfont
    \let\ka@db@fontseries\series
    \let\ka@db@fontshape\shape
    \let\ka@db@fontsize\size
  \else
    \ifka@db@nfsstwo
        \let\ka@db@selectfont\selectfont
        \let\ka@db@fontseries\fontseries
        \let\ka@db@fontshape\fontshape
        \let\ka@db@fontsize\fontsize
      \else
        \def\ka@db@selectfont{\relax}
        \def\ka@db@fontseries#1{\relax}
        \def\ka@db@fontshape#1{\ifx#1n\rm\else\relax\fi}
        \def\ka@db@fontsize#1#2{\expandafter
            \ifnum#1=12
                \@setsize\normalsize{15pt}\xiipt\@xiipt
                \abovedisplayskip 12pt plus3pt minus7pt
                \belowdisplayskip \abovedisplayskip
                \abovedisplayshortskip \z@ plus3pt
                \belowdisplayshortskip 6.5pt plus3.5pt minus3pt
              \else
                \ifnum#1=8
                    \@setsize\scriptsize{8pt}\viipt\@viipt
                  \else
                    \@setsize\scriptsize{9.5pt}\viiipt\@viiipt
                  \fi
              \fi}
      \fi
  \fi
\ifka@db@ltxtwoe
  \ExecuteOptions{a4paper,10pt,onecolumn,final}
\fi
\ifka@db@ltxtwoe
    \ProcessOptions
  \else
    \@options
  \fi
\def\ka@db@error#1{%
   \@latexerr{%
      Document style/class `dinbrief' error^^J%
      #1%
   }{%
      No help is available for this error message.^^J%
      Please check your input file!
   }%
}
\ifka@db@ltxtwoe
  \def\ka@db@warning#1{%
    \@@warning{The control sequence \string#1\space should %
               not be used in LaTeX 2e}%
  }
\fi
\ifka@db@ltxtwoe\else
  \def\ClassWarning#1#2{%
    \typeout{Style #1 Warning: #2\on@line.}%
  }
\fi
\ifka@db@ltxtwoe
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*{\cal}{\@fontswitch{\relax}{\mathcal}}
\DeclareRobustCommand*{\mit}{\@fontswitch{\relax}{\mathnormal}}
\fi
\newdimen\GZA
\GZA=1in \divide\GZA by 6
\ifcase \@ptsize\relax
 \def\@normalsize{\@setsize\normalsize{12pt}\xpt\@xpt
  \abovedisplayskip 10\p@ plus2\p@ minus5\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus3\p@
  \belowdisplayshortskip 6\p@ plus3\p@ minus3\p@
  \let\@listi\@listI
  }%
 \def\small{\@setsize\small{11pt}\ixpt\@ixpt
  \abovedisplayskip 8.5\p@ plus 3\p@ minus 4\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus2\p@
  \belowdisplayshortskip 4\p@ plus2\p@ minus 2\p@
  \def\@listi{\leftmargin\leftmargini
              \topsep 4\p@ \@plus2\p@ \@minus2\p@
              \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
  }%
 \def\footnotesize{\@setsize\footnotesize{9.5pt}\viiipt\@viiipt
  \abovedisplayskip 6\p@ plus 2\p@ minus 4\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus 1\p@
  \belowdisplayshortskip 3\p@ plus 1\p@ minus 2\p@
  \def\@listi{\leftmargin\leftmargini
              \topsep 3\p@ \@plus\p@ \@minus\p@
              \parsep 2\p@ \@plus\p@ \@minus\p@
              \itemsep \parsep}%
  }%
 \def\scriptsize{\@setsize\scriptsize{8pt}\viipt\@viipt}
 \def\tiny{\@setsize\tiny{6pt}\vpt\@vpt}
 \def\large{\@setsize\large{14pt}\xiipt\@xiipt}
 \def\Large{\@setsize\Large{18pt}\xivpt\@xivpt}
 \def\LARGE{\@setsize\LARGE{22pt}\xviipt\@xviipt}
 \def\huge{\@setsize\huge{25pt}\xxpt\@xxpt}
 \def\Huge{\@setsize\Huge{30pt}\xxvpt\@xxvpt}
\or % 11 pt option
 \def\@normalsize{\@setsize\normalsize{13.6pt}\xipt\@xipt
  \abovedisplayskip 11\p@ plus3\p@ minus6\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus3\p@
  \belowdisplayshortskip 6.5\p@ plus3.5\p@ minus3\p@
  \let\@listi\@listI
  }%
 \def\small{\@setsize\small{12pt}\xpt\@xpt
  \abovedisplayskip 10\p@ plus2\p@ minus5\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus3\p@
  \belowdisplayshortskip 6\p@ plus3\p@ minus3\p@
  \def\@listi{\leftmargin\leftmargini
              \topsep 6\p@ \@plus2\p@ \@minus2\p@
              \parsep 3\p@ \@plus2\p@ \@minus\p@
              \itemsep \parsep}%
  }%
 \def\footnotesize{\@setsize\footnotesize{11pt}\ixpt\@ixpt
  \abovedisplayskip 8\p@ plus 2\p@ minus 4\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus 1\p@
  \belowdisplayshortskip 4\p@ plus 2\p@ minus 2\p@
  \def\@listi{\leftmargin\leftmargini
              \topsep 4\p@ \@plus2\p@ \@minus2\p@
              \parsep 2\p@ \@plus\p@ \@minus\p@
              \itemsep \parsep}%
  }%
 \def\scriptsize{\@setsize\scriptsize{9.5pt}\viiipt\@viiipt}
 \def\tiny{\@setsize\tiny{7pt}\vipt\@vipt}
 \def\large{\@setsize\large{14pt}\xiipt\@xiipt}
 \def\Large{\@setsize\Large{18pt}\xivpt\@xivpt}
 \def\LARGE{\@setsize\LARGE{22pt}\xviipt\@xviipt}
 \def\huge{\@setsize\huge{25pt}\xxpt\@xxpt}
 \def\Huge{\@setsize\Huge{30pt}\xxvpt\@xxvpt}
\or % 12pt option
 \def\@normalsize{\@setsize\normalsize{15pt}\xiipt\@xiipt
  \abovedisplayskip 12\p@ plus3\p@ minus7\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus3\p@
  \belowdisplayshortskip 6.5\p@ plus3.5\p@ minus3\p@
  \let\listi\listI
  }%
 \def\small{\@setsize\small{13.6pt}\xipt\@xipt
  \abovedisplayskip 11\p@ plus3\p@ minus6\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus3\p@
  \belowdisplayshortskip 6.5\p@ plus3.5\p@ minus3\p@
  \def\@listi{\leftmargin\leftmargini
              \topsep 9\p@ \@plus3\p@ \@minus5\p@
              \parsep 4.5\p@ \@plus2\p@ \@minus\p@
              \itemsep \parsep}%
  }%
 \def\footnotesize{\@setsize\footnotesize{12pt}\xpt\@xpt
  \abovedisplayskip 10\p@ plus2\p@ minus5\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus3\p@
  \belowdisplayshortskip 6\p@ plus3\p@ minus3\p@
  \def\@listi{\leftmargin\leftmargini
              \topsep 6\p@ \@plus2\p@ \@minus2\p@
              \parsep 3\p@ \@plus2\p@ \@minus\p@
              \itemsep \parsep}%
  }%
 \def\scriptsize{\@setsize\scriptsize{9.5pt}\viiipt\@viiipt}
 \def\tiny{\@setsize\tiny{7pt}\vipt\@vipt}
 \def\large{\@setsize\large{18pt}\xivpt\@xivpt}
 \def\Large{\@setsize\Large{22pt}\xviipt\@xviipt}
 \def\LARGE{\@setsize\LARGE{25pt}\xxpt\@xxpt}
 \def\huge{\@setsize\huge{30pt}\xxvpt\@xxvpt}
 \let\Huge=\huge
\or  % Norm-Option (DIN 2107, 2142)
 \def\@normalsize{\@setsize\normalsize{\GZA}\xipt\@xipt
  \abovedisplayskip 11\p@ plus3\p@ minus6\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus3\p@
  \belowdisplayshortskip 6.5\p@ plus3.5\p@ minus3\p@
  \let\@listi\@listI
  }% Setting of \@listi added 22 Dec 87
 \def\small{\@setsize\small{11pt}\xpt\@xpt
  \abovedisplayskip 10\p@ plus2\p@ minus5\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip  \z@ plus3\p@
  \belowdisplayshortskip  6\p@ plus3\p@ minus3\p@
  \def\@listi{\leftmargin\leftmargini %% Def of \@listi added 22 Dec 87
              \topsep 6\p@ plus2\p@ minus2\p@
              \parsep 3\p@ plus2\p@ minus\p@
              \itemsep \parsep}%
  }%
 \def\footnotesize{\@setsize\footnotesize{11pt}\ixpt\@ixpt
  \abovedisplayskip 8\p@ plus2\p@ minus4\p@
  \belowdisplayskip \abovedisplayskip
  \abovedisplayshortskip \z@ plus\p@
  \belowdisplayshortskip 4\p@ plus2\p@ minus2\p@
  \def\@listi{\leftmargin\leftmargini %% Def of \@listi added 22 Dec 87
              \topsep 4\p@ plus2\p@ minus2\p@
              \parsep 2\p@ plus\p@ minus\p@
              \itemsep \parsep}%
  }%
 \def\scriptsize{\@setsize\scriptsize{9.5pt}\viiipt\@viiipt}
 \def\tiny{\@setsize\tiny{7pt}\vipt\@vipt}
 \def\large{\@setsize\large{14pt}\xiipt\@xiipt}
 \def\Large{\@setsize\Large{18pt}\xivpt\@xivpt}
 \def\LARGE{\@setsize\LARGE{22pt}\xviipt\@xviipt}
 \def\huge{\@setsize\huge{25pt}\xxpt\@xxpt}
 \def\Huge{\@setsize\Huge{30pt}\xxvpt\@xxvpt}
\fi
\ifka@db@ltxtwoe
\let\normalsize\@normalsize
\fi
\ifka@db@nfss
    \normalsize
  \else
    \ifka@db@nfsstwo
      \normalsize
    \fi
  \fi
\oddsidemargin 0in
\evensidemargin 0in
\marginparwidth .08in
\marginparsep .01in
\marginparpush 5pt
\topmargin0pt
\topmargin-5mm
\newdimen\ltf@headheight
\newdimen\lts@headheight
\newdimen\ltf@headsep
\newdimen\lts@headsep
\newdimen\ltf@textheight
\newdimen\lts@textheight
\ltf@headheight 4.2truemm %
\lts@headheight 0truemm %
\ltf@headsep 8.8truemm %
\lts@headsep 0truemm %
\ifka@db@ltxtwoe
    \relax
  \else
    \footheight 4.2truemm
  \fi
\footskip 8.8truemm
\textheight 260truemm
\textheight 254truemm
\textwidth 165truemm
\columnsep 10pt
\columnseprule 0pt
\raggedbottom
\footnotesep 4.2truemm
\skip\footins 10pt plus 2pt minus 4pt
\floatsep 12pt plus 2pt minus 2pt
\textfloatsep 20pt plus 2pt minus 4pt
\intextsep 12pt plus 2pt minus 2pt
\dblfloatsep 12pt plus 2pt minus 2pt
\dbltextfloatsep 20pt plus 2pt minus 4pt
\ifka@db@ltxtwoe
    \relax
  \else
    \@maxsep 20pt
    \@dblmaxsep 20pt
  \fi
\@fptop 0pt plus 1fil
\@fpsep 8pt plus 2fil
\@fpbot 0pt plus 1fil
\@dblfptop 0pt plus 1fil
\@dblfpsep 8pt plus 2fil
\@dblfpbot 0pt plus 1fil
\newif\ifka@db@windowrules
\def\windowrules{\global\ka@db@windowrulestrue}
\def\nowindowrules{\global\ka@db@windowrulesfalse}
\windowrules
\newif\ifka@db@windowtics
\def\windowtics{\global\ka@db@windowticstrue}
\def\nowindowtics{\global\ka@db@windowticsfalse}
\windowtics
\def\backaddress#1{\gdef\@backaddress{#1}}
\backaddress{}
\long\def\@splitoncr#1#2{%
  \begingroup
    % \tracingmacros=1
    \let\\=a% because characters are not expanded
    \long\def\@rgjob##1{#2{##1}}%
    % \expandafter\@splitoncrlist#1\\\stop \\
    \@splitoncrlist#1\\\stop \\
  \endgroup}
\long\def\@splitoncrlist#1\\{%
  \ifx\stop#1\relax
    \let\next=\relax
  \else
    \@rgjob{#1}%
    \let\next=\@splitoncrlist
  \fi
  \next
}
\long\def\@ignorebrackets[#1]#2\stop#3{%
  \gdef#3{#2}\relax
}
\long\def\@withmarkcr!#1\stop{%
  \xdef\@addresslist{%
    \@addresslist\ifx\@addresslist\empty\else\leavevmode\\\fi{#1}}%
  \@ifnextchar[%]
    {\@ignorebrackets}{\@ignorebrackets[]}#1\stop{\@addressname}
}
\long\def\@withoutmarkcr#1\stop{%
  \xdef\@addresslist{%
    \@addresslist\ifx\@addresslist\empty\else\leavevmode\\\fi{#1}}
  \ifx\@addressname\relax
    \@ifnextchar[%]
      {\@ignorebrackets}{\@ignorebrackets[]}#1\stop{\@addressname}
  \fi
}
\long\def\@withmark!#1\stop{%
  \xdef\@addresslist{\@addresslist {#1}}
  \@ifnextchar[%]
    {\@ignorebrackets}{\@ignorebrackets[]}#1\stop{\@addressname}%
}
\long\def\@withoutmark#1\stop{%
  \xdef\@addresslist{\@addresslist {#1}}
  \ifx\@addressname\relax
    \@ifnextchar[%]
      {\@ignorebrackets}{\@ignorebrackets[]}#1\stop{\@addressname}
  \fi
}
\long\def\@withlbracket[#1]#2\stop{%
  \xdef\@addresslist{\@addresslist\ifx\@addresslist\empty\else\leavevmode\\\fi[#1]}
  \@ifnextchar!{\@withmark}{\@withoutmark}#2\stop
}
\long\def\@withoutlbracket#1\stop{%
  \@ifnextchar!{\@withmarkcr}{\@withoutmarkcr}#1\stop
}
\long\def\@checkformark#1{%
  %\@ifnextchar!{\@withmark}{\@withoutmark}#1\stop
  \@ifnextchar[{\@withlbracket}{\@withoutlbracket}#1\stop
}
\long\def\@splitaddress#1{%
  \def\@addressbase{#1}
  \let\@addressname=\relax
  \def\@addresslist{}
  \expandafter\@splitoncr\expandafter{\@addressbase}{\@checkformark}%
  \gdef\toname{\@addressname}
  % \typeout{Addresslist: \@addresslist}%
  % \typeout{Addressname: \@addressname}%
  % \typeout{\string\toname: \toname}%
}
\newdimen\ka@db@indadr
\ka@db@indadr=0pt
\def\ka@db@tsfromaddress{%
  \ifx\@empty\@fromaddress
    \vbox to 0pt{\vss}\nointerlineskip
  \else
    \vbox to 0pt{\vskip -19.4truemm\raggedright\@fromaddress\vss}%
    \nointerlineskip
  \fi
}
\def\ka@db@tswindowtics{%
  \ifka@db@windowtics
    \vbox to 0pt{%
     \rule{0pt}{81.1truemm}% 62.5truemm > 80.0truemm
     \hbox to 0pt{\hspace*{-24truemm}\vbox{\hrule width 7truemm}\hss}%
     \vss}\nointerlineskip
    \vbox to 0pt{%
     \rule{0pt}{124.6truemm}% 106.0truemm > 123.5truemm
     \hbox to 0pt{\hspace*{-18truemm}\vbox{\hrule width 6truemm}\hss}%
     \vss}\nointerlineskip
    \vbox to 0pt{%
     \rule{0pt}{186.1truemm}% 167.5truemm > 185.0truemm
     \hbox to 0pt{\hspace*{-24truemm}\vbox{\hrule width 7truemm}\hss}%
     \vss}\nointerlineskip
  \else
    \vbox to 0pt{\rule{0pt}{81.1truemm}%
     \hbox to 0pt{\vbox{\hrule width 0pt}\hss}%
     \vss}\nointerlineskip
    \vbox to 0pt{\rule{0pt}{124.6truemm}%
     \hbox to 0pt{\vbox{\hrule width 0pt}\hss}%
     \vss}\nointerlineskip
    \vbox to 0pt{\rule{0pt}{186.1truemm}%
     \hbox to 0pt{\vbox{\hrule width 0pt}\hss}%
     \vss}\nointerlineskip
  \fi
}
\newdimen\dbottomtext
\dbottomtext=261.0truemm
\def\ka@db@tsbottomtext{%
  \ifdim\ht\@@bottomtext>0pt
    \vbox to 0pt{%
      \rule{0pt}{\dbottomtext}% 243.5truemm
      \unhcopy\@@bottomtext
      \vss
    }\nointerlineskip%
  \fi
}
\def\ka@db@bafntsel{%
  \ka@db@fontsize{8}{9pt}%
  \ka@db@selectfont
}
\def\ka@db@tstoaddress{%
  \vbox to 0pt{\vss%
    \ifka@db@windowrules
      \hbox to 0pt{\hspace*{-4truemm}\vbox{\hrule width 85truemm}\hss}%
    \else
      \hbox to 0pt{\hss}%
    \fi
    \vss
  }%
  \nointerlineskip
  \ifx\@empty\@backaddress
    \vbox to 7.5truemm{%
      \vfill
    }%
    \nointerlineskip
  \else
    \vbox to 7.5truemm{%
      \vskip 2.5truemm
      \vss
      \hbox to 0pt{\hspace*{-4truemm}%
        \vbox{%
          \hbox to 85truemm{\hfill
                            {\ka@db@bafntsel\@backaddress}\hfill}%
          \hrule width 85truemm}\hss}%
      % \vss % <- removed to get exact layout
    }%
    \nointerlineskip
  \fi
  \vbox to 40truemm{%
    \vss
    \hbox to 0pt{\hskip0pt minus 4truemm%
      \setbox\@tempboxa=\vbox{%
        \if@letterform
          \relax
        \else
          % \typeout{Start dumping \string\@addresslist}%
          \leavevmode
          \ka@db@ifemptyvarlocal{postremark}{}{%
            \ka@db@usevarlocal{postremark}\\[\medskipamount]
          }%
          \raggedright \@addresslist %\toname \\ \toaddress%
          % \typeout{End dumping \string\@addresslist}%
        \fi
      }%
      \ifdim\ht\@tempboxa>40mm
        %\@warning{Receivers address is larger than letter window}%
        \ClassWarning{dinbrief}{%
          Receivers address is larger than letter window}%
      \else
        \setbox\@tempboxa=\vbox{%
          \hbox{%
            \ifdim\ka@db@indadr>0pt
              \else
                \hbox to 0pt{\hskip4in\ka@db@usevarlocal{handling}\hss}%
              \fi
            %\unvbox\@tempboxa
            \box\@tempboxa
          }%
          \if@toadrcenter\relax
            \else\vskip 2.5truemm minus 2.5truemm
          \fi
        }%
      \fi
      \box\@tempboxa
      \hss
    }%
    \if@toadrcenter
      \vss
    \fi
  }\nointerlineskip
  \vskip 2.5truemm
  \vbox to 0pt{\vss%
    \ifka@db@windowrules
      \hbox to 0pt{\hspace*{-4truemm}\vbox{\hrule width 85truemm}\hss}%
    \else
      \hbox to 0pt{\hss}%
    \fi
    \vss
  }\nointerlineskip
}
\def\ka@db@tsrefline{%
  \if@norefline\else
      \vbox{%
        \if@refline
          \@answerto
        \else
          \if@letterform\relax\else
            \ifx\@empty\@place
              \raggedleft \@date
            \else
              \raggedleft \@place,\space\@date
            \fi
          \fi
        \fi
      }%
    \nointerlineskip
    \par
    \bigskip
  \fi %\if@norefline
}
\def\ka@db@tsconcern{%
  \ifx\@empty\@concern \else
    \if@letterform\relax\else
      \@concern \par
      \bigskip
    \fi
  \fi
  \vspace{0pt plus 10truemm}%
}
\def\ka@db@tsopening#1{%
  \if@letterform\relax\else
   #1\par
  \fi
}
\def\ka@db@placefromaddress{%
  \vbox to 0pt{\ka@db@tsfromaddress\vss}\nointerlineskip
}%
\def\ka@db@placewindowtics{%
  \vbox to 0pt{\ka@db@tswindowtics\vss}\nointerlineskip
}%
\def\ka@db@placebottomtext{%
  \vbox to 0pt{\ka@db@tsbottomtext\vss}\nointerlineskip
}%
\def\ka@db@placetoaddress{%
  \vskip 18.6truemm % old value 13.1truemm
  %   \vbox to 50truemm{\vss\ka@db@tstoaddress\vss}\nointerlineskip
  %   \vbox to 50truemm{\ka@db@tstoaddress}\nointerlineskip
  % \vbox to 50truemm{\ka@db@tstoaddress\vss}\nointerlineskip
  % ^- org. value
  \moveright\ka@db@indadr\vbox to 50truemm{\ka@db@tstoaddress\vss}%
  \nointerlineskip
  \vskip 6truemm
}%
\def\ka@db@placerefline{%
  \vbox{\ka@db@tsrefline}\nointerlineskip
}%
\def\ka@db@placeconcern{
  \vbox{\ka@db@tsconcern}\nointerlineskip
}%
\def\ka@db@placeopening#1{%
  \ka@db@tsopening{#1}%
  \nobreak
}%
\def\opening#1{%
  \setcounter{lt@state}{2}
  \thispagestyle{first@page}
  \ifka@db@ltxtwoe
     \enlargethispage{-\ht\@@bottomtext}
  \fi
  \ka@db@placefromaddress
  \ka@db@placewindowtics
  \ka@db@placebottomtext
  \ka@db@placetoaddress
  \ka@db@placerefline
  \ka@db@placeconcern
  \ka@db@placeopening{#1}%
}
\def\oldopening#1{\thispagestyle{first@page}
  \ifka@db@ltxtwoe
     \enlargethispage{-\ht\@@bottomtext}
  \fi
 \ifx\@empty\@fromaddress
    \vbox to 0pt{\vss}\nointerlineskip
  \else
     \vbox to 0pt{\vskip -19.4truemm\raggedright\@fromaddress\vss}%
  \fi
 \vskip 13.1truemm
 \ifx\@empty\@@windowtics
    \vbox to 0pt{\rule{0pt}{62.5truemm}%
     \hbox to 0pt{\vbox{\hrule width 0pt}\hss}%
     \vss}\nointerlineskip
    \vbox to 0pt{\rule{0pt}{106.0truemm}%
     \hbox to 0pt{\vbox{\hrule width 0pt}\hss}%
     \vss}\nointerlineskip
    \vbox to 0pt{\rule{0pt}{167.5truemm}%
     \hbox to 0pt{\vbox{\hrule width 0pt}\hss}%
     \vss}\nointerlineskip
  \else
    \vbox to 0pt{%
     \rule{0pt}{62.5truemm}%
     \hbox to 0pt{\hspace*{-24truemm}\vbox{\hrule width 7truemm}\hss}%
     \vss}\nointerlineskip
    \vbox to 0pt{%
     \rule{0pt}{106.0truemm}%
     \hbox to 0pt{\hspace*{-18truemm}\vbox{\hrule width 6truemm}\hss}%
     \vss}\nointerlineskip
    \vbox to 0pt{%
     \rule{0pt}{167.5truemm}%
     \hbox to 0pt{\hspace*{-24truemm}\vbox{\hrule width 7truemm}\hss}%
     \vss}\nointerlineskip
  \fi
 \ifdim\ht\@@bottomtext>0pt
    \vbox to 0pt{%
      \rule{0pt}{243.5truemm}%
      \unhcopy\@@bottomtext
      \vss}\nointerlineskip%
  \fi
 \vbox to 0pt{\vss%
  \ifx\@empty\@@windowrules
     \hbox to 0pt{\hss}%
   \else
     \hbox to 0pt{\hspace*{-4truemm}\vbox{\hrule width 85truemm}\hss}%
   \fi
  \vss}\nointerlineskip
 \ifx\@empty\@backaddress
    \vbox to 7.5truemm{
     \vfill
     }\nointerlineskip
  \else
    \vbox to 7.5truemm{
     \vskip 2.5truemm
     \vss
     \hbox to 0pt{\hspace*{-4truemm}\vbox{%
      \hbox to 85truemm{\hfill
                        {\ka@db@fontsize{8}{9pt}
                         \ka@db@selectfont \@backaddress}\hfill}%
      \hrule width 85truemm}\hss}%
     \vss}\nointerlineskip
  \fi
 \vbox to 40truemm{%
  \vss
  \hbox to 0pt{\hskip0pt minus 4truemm%
   \vbox{%
         \if@letterform
             \relax
           \else
             % \typeout{Start dumping \string\@addresslist}%
             \raggedright \@addresslist %\toname \\ \toaddress%
             % \typeout{End dumping \string\@addresslist}%
             \if@toadrcenter\relax
              \else\vskip 2.5truemm minus 2.5truemm
              \fi
           \fi
        }%
   \hss}
  \if@toadrcenter
    \vss
  \fi
  }\nointerlineskip
 \vskip 2.5truemm
 \vbox to 0pt{\vss%
  \ifx\@empty\@@windowrules
     \hbox to 0pt{\hss}%
   \else
     \hbox to 0pt{\hspace*{-4truemm}\vbox{\hrule width 85truemm}\hss}%
   \fi
  \vss}\nointerlineskip
  \if@norefline\else
    \vbox{\vskip 6truemm
      \vbox{%
        \if@refline
          \@answerto
        \else
          \if@letterform\relax\else
            \ifx\@empty\@place
              \raggedleft \@date
            \else
              \raggedleft \@place,\space\@date
            \fi
          \fi
        \fi
      }%
    }%
    \nointerlineskip
    \par
    \bigskip
  \fi %\if@norefline
 \ifx\@empty\@concern \else
   \if@letterform\relax\else
      \@concern \par
      \bigskip
    \fi
  \fi
 \ifx\@empty\@footer \else
  \fi
 \vspace{0pt plus 10truemm}
 \if@letterform\relax\else
   #1\par
  \fi
  \nobreak}
\long\def\closing{%
  \@ifnextchar[%]
    {\ka@db@closing}{\ka@db@closing[]}%
}
\long\def\ka@db@closing[#1]#2{%
  \setcounter{lt@state}{3}
  \par\nobreak
  \vspace{\parskip}
  \stopbreaks
  {%
    \leavevmode
    \ifka@db@enclright
      \bgroup
        \ka@db@tmpdima=4in
        \parbox[t]{\ka@db@tmpdima}{%\parskip .7em%
          \raggedright
          \ignorespaces #2\\[3\baselineskip]
          \vbox to 0pt{%
            \vskip -4\baselineskip\relax
            \hbox to 0pt{\strut#1\hss}%
          \vss}%
          \ka@db@ifemptyvar{signature}{}{%
            \ka@db@usevar{signature}%
          }%
          \strut
        }%
        \@tempdima=\textwidth
        \advance\@tempdima by -\ka@db@tmpdima
        \parbox[t]{\@tempdima}{%\parskip .7em%
          \raggedright
          \ka@db@tsenclcc
          \unskip % <- removing unneccessary skip from
                  %    \cmd\ka@db@tsenclcc
        }%
      \egroup
    \else
      \raggedright
      \ignorespaces #2\\[3\baselineskip]
      \leavevmode
      % here the signature <- not correct
      \vbox to 0pt{%
        \vskip -4\baselineskip\relax
        \hbox to 0pt{\strut#1\hss}%
      \vss}%
      \ka@db@ifemptyvar{signature}{}{%
        \ka@db@usevar{signature}%
      }%
      \strut
    \fi
  }%
  \par\medskip
  \ifka@db@enclright
  \else
    \raggedright
    \ka@db@tsenclcc
  \fi
  \ka@db@closinghook
}
\def\ka@db@closinghook{}
\def\ka@db@tsenclcc{%
  \ka@db@ifemptyvarlocal{encl}%
    {}{\ka@db@usevarlocal{encl}\\[\baselineskip]}%
  \ka@db@ifemptyvarlocal{cc}%
    {}{\ka@db@usevarlocal{cc}\\[\baselineskip]}%
}
\def\ccname{Verteiler}
\def\enclname{Anlage(n)}
\def\psname{PS}
\def\headtoname{An}
\def\pagename{Seite}
\def\handling#1{%
  \ka@db@assignvarlocal{handling}{#1}%
}
\ka@db@execatletter{\ka@db@assignvarlocal{handling}{}}%
\def\postremark#1{%
  \ka@db@assignvarlocal{postremark}{#1}%
}
\ka@db@execatletter{\ka@db@assignvarlocal{postremark}{}}%
\def\ka@db@genencl#1[#2]#3{%
  \ifnum\c@lt@state=3\relax
    \par
    \noindent{%
    \bgroup
      \def\tmpa{#2}%
      \expandafter\egroup
    \ifx\tmpa\@empty\else
      #2\hfil\break
    \fi
    \ignorespaces #3\strut}%
    \par
  \else
    \bgroup
      \def\tmpa{#2}%
      \expandafter\egroup
    \ifx\tmpa\@empty
      \ka@db@assignvarlocal{#1}{#3}%
    \else
      \ka@db@assignvarlocal{#1}{#2\hfil\break #3}%
    \fi
  \fi
}
\def\cc{%
  \@ifnextchar[%]
    {\ka@db@genencl{cc}}{\ka@db@genencl{cc}[\ccname]}%
}
\ka@db@execatletter{\ka@db@assignvarlocal{cc}{}}%
\def\encl{%
  \@ifnextchar[%]
    {\ka@db@genencl{encl}}{\ka@db@genencl{encl}[\enclname]}%
}
\ka@db@execatletter{\ka@db@assignvarlocal{encl}{}}%
\def\ps#1{\par\noindent{\psname \\
          \ignorespaces #1\strut}\par}
\def\stopletter{}
\newdimen\dlabelheight
\newdimen\dlabeltopmargin
\newdimen\dlabelwidth
\newcount\clabelnumber
\def\setlabelheight#1{\global\dlabelheight=#1\relax}
\def\setlabelwidth#1{\global\dlabelwidth=#1\relax}
\def\setlabeltopmargin#1{\global\dlabeltopmargin=#1\relax}
\def\setlabelnumber#1{\global\clabelnumber=#1\relax}
\def\Etihoehe#1{%
  %\@warning{Use \string\setlabelheight\space instead %
  %          of \string\Etihoehe}%
  \ClassWarning{dinbrief}{%
    Use \string\setlabelheight\space instead of \string\Etihoehe}%
  \setlabelheight{#1}%
  }
\def\Etirand#1{
  %\@warning{Use \string\setlabeltopmargin\space instead %
  %          of \string\Etirand}%
  \ClassWarning{dinbrief}{%
    Use \string\setlabeltopmargin\space instead of \string\Etirand}%
  \setlabeltopmargin{#1}%
  }
\def\Etizahl#1{
  %\@warning{Use \string\setlabelnumber\space instead %
  %          of \string\Etizahl}%
  \ClassWarning{dinbrief}{%
    Use \string\setlabelnumber\space instead of \string\Etizahl}%
  \setlabelnumber{#1}%
  }
\setlabelheight{41.15truemm}%
\setlabeltopmargin{8.85truemm}%
\setlabelwidth{80truemm}%
\setlabelnumber{7}%
\newcount\cspare
\cspare=0
\def\spare#1{\cspare=#1\relax}
\def\labelstyle#1{\def\@labelstyle{#1}}
\labelstyle{plain}
\newif\if@kadb@tracelabels
\@kadb@tracelabelsfalse
\long\def\label@plain#1#2{\setbox0\vbox{\hsize\dlabelwidth
 \strut\ignorespaces #2}%{}%
 %\vbox to 50.8truemm{\vss \box0 \vss}%
 \vbox to\dlabelheight{%
   \if@kadb@tracelabels
     \hrule
   \fi
   \vss
   \hbox to \dlabelwidth{%
     \hss
     \if@kadb@tracelabels
       \vrule
     \fi
     \vbox to\dlabelheight{\vss \box0 \vss}%
     \if@kadb@tracelabels
       \vrule
     \fi
     \hss
   }%
   \vss
   \if@kadb@tracelabels
     \hrule
   \fi
 }%
 \advance\labelcount by 1\nointerlineskip
 \ifnum\labelcount<\clabelnumber
 \else
   \vfill\newpage
   \labelcount=0
 \fi}
\long\def\label@deskjet#1#2{\setbox0\vbox{\parbox[b]{3.6in}{
    \vbox to 0pt{\vss%
      \ifx\@empty\@@windowrules
        \hbox to 0pt{\hss}%
      \else
        \hbox to 0pt{\hspace*{-4truemm}%
                     \vbox{\hrule width 85truemm}\hss}%
      \fi
      \vss}\nointerlineskip
    \vbox to 7.5truemm{
      \vskip 2.5truemm
      \vss
      \hbox to 0pt{\hspace*{-4truemm}\vbox{%
        \hbox to 85truemm{\hfill{\ka@db@fontsize{8}{9pt}
                                 \ka@db@selectfont #1}\hfill}
        \hrule width 85truemm}\hss}%
        \vss}\nointerlineskip
    \vbox to 40truemm{%
      \vss
      \hbox to 0pt{\hskip0pt minus 4truemm%
        \vbox{\raggedright #2%
          \if@toadrcenter
           \relax\else\vskip 2.5truemm minus 2.5truemm
           \fi}%
        \hss}
      \if@toadrcenter
        \vss
      \fi
    }\nointerlineskip
    \vskip 2.5truemm
    \vbox to 0pt{\vss%
      \ifx\@empty\@@windowrules
        \hbox to 0pt{\hss}%
      \else
        \hbox to 0pt{\hspace*{-4truemm}%
                     \vbox{\hrule width 85truemm}\hss}%
      \fi
      \vss}\nointerlineskip
    }}%
  \vbox to 0pt{\vss}%
  \vskip 18.0truemm
  \vbox to 50.8truemm{\vss \box0 \vss}%
  \ifnum\labelcount=4 \labelcount=0
    \else \advance\labelcount by 1\nointerlineskip
    \fi
  \vfill
  \newpage
}
\expandafter\let\csname label@C6\endcsname    =\label@deskjet
\expandafter\let\csname label@DL\endcsname    =\label@deskjet
\expandafter\let\csname label@C6/C5\endcsname =\label@deskjet
\long\def\mlabel#1#2{\@nameuse{label@\@labelstyle}{#1}{#2}}
\def\lstyle@plain{
  \topmargin-25.4truemm%
  \advance\topmargin\dlabeltopmargin
  \headsep\z@
  \headheight\z@
  \topskip\z@
  \oddsidemargin -12.9truemm
  \evensidemargin -12.9truemm
  \textheight=\clabelnumber\dlabelheight
  \advance\textheight by .0001truemm
  \@colht\textheight \@colroom\textheight \vsize\textheight
  \columnsep 25mm
  \textwidth2\dlabelwidth
  \advance\textwidth by\columnsep
  \ka@db@fontsize{12}{14.4pt}\ka@db@selectfont
  \boxmaxdepth\z@
  \parindent\z@
  \parskip\z@skip
  \twocolumn\relax
  \ifnum\cspare>\z@
    \ifnum\cspare<\clabelnumber\else
      \mbox{}%
      \newpage
      \advance\cspare -\clabelnumber
    \fi
    \advance\labelcount by\cspare
    \nointerlineskip
    \vbox to \cspare\dlabelheight{\vfill}%
    \nointerlineskip
    \nobreak
  \else
    \nointerlineskip
  \fi
}
\def\lstyle@deskjet{
  \textheight 25.4truecm
  \advance\textheight by .0001truemm
  \textwidth 193.3truemm
  \columnsep 25pt
 \relax
}
\def\startlabels{%
  \nointerlineskip
  \labelcount=0
  \pagestyle{empty}
  \let\@texttop=\relax
  \@nameuse{lstyle@\@labelstyle}
}
\newcounter{lt@state}%
\setcounter{lt@state}{0}%
\newcounter{letter}
\long\def\letter#1{%
  \ifnum\c@lt@state>0
    \ka@db@error{%
      Command \string\begin{letter} inside the
      \string\begin{letter}-environment found.
    }
  \fi
  \setcounter{lt@state}{1}%
  % \typeout{\string\ka@db@atletter\space\ka@db@atletter}%
  \ka@db@atletter
  \clearpage
  \thispagestyle{empty}
  \cleardoublepage
  \refstepcounter{letter}%
  \c@page\@ne
  \global\ka@db@lastpagefalse
  \interlinepenalty=200
  % \@processto{#1}%
  % \message{Starting \string\@splitaddress}%
  \bgroup
    \def\protect{\noexpand\protect\noexpand}%
    \let\\=\relax
    \let\vspace=\relax
    \@splitaddress{#1}
  \egroup
  % \message{Ending \string\@splitaddress}%
  }
\def\endletter{%
  \setcounter{lt@state}{0}%
  \ka@db@lastpagetrue
  \stopletter
  \@@par
  \pagebreak
  \@@par
  \gdef\@yourmail{}
  \gdef\@mymail{}
  \global\@reflinefalse
  \if@filesw
    \ifnum\c@labelgen>1
      \begingroup
        \def\protect{\string}
        \let\\=\relax
        \def\protect##1{\string##1\space}
        \let\vspace=\relax
        \let\rule=\relax
        %\immediate\write\@auxout{\string\mlabel{\@backaddress}{\toname
        %\\\toaddress}}
        %\immediate\write\@auxout{%
        %  \string\mlabel{\@backaddress}{\@addresslist}}%
        %\immediate\write\@auxout{\string\mlabel{\@backaddress}{%
        %  \ka@db@ifemptyvarlocal{postremark}{}{%
        %    \ka@db@usevarlocal{postremark}\\[\string\medskipamount]%
        %  }%
        %  \@addresslist
        %}}%
        \ka@db@ifemptyvarlocal{postremark}{% then
          \immediate\write\@mainlbl{\string\mlabel{\@backaddress}{%
          \@addresslist}}%
        }{% else
          \immediate\write\@mainlbl{\string\mlabel{\@backaddress}{%
          \pka@db@usevarlocal{postremark}\\[\string\medskipamount]%
          \@addresslist}}%
        }% endif
      \endgroup
    \fi
    %\ifnum\c@labelgen=1
    %  \onlabels
    %\fi
  \fi
  \ka@db@atendletter
  }
\long\def\@processto#1{%
  \@xproc #1\\@@@%
  \ifx\toaddress\@empty
  \else
    \@yproc #1@@@%
  \fi}
\long\def\@xproc #1\\#2@@@{\def\toname{#1}\def\toaddress{#2}}
\long\def\@yproc #1\\#2@@@{\def\toaddress{#2}}
\def\stopbreaks{\interlinepenalty \@M
 \def\par{\@@par\nobreak}\let\\=\@nobreakcr
 \let\vspace\@nobreakvspace}
\def\@nobreakvspace{\@ifstar{\@nobreakvspacex}{\@nobreakvspacex}}
\def\@nobreakvspacex#1{\ifvmode\nobreak\vskip #1\relax\else
 \@bsphack\vadjust{\nobreak\vskip #1}\@esphack\fi}
\def\@nobreakcr{\vadjust{\penalty\@M}\@ifstar{\@xnewline}{\@xnewline}}
\def\startbreaks{\let\\=\@normalcr
 \interlinepenalty 200\def\par{\@@par\penalty 200}}
\newcount\labelcount
\newif\if@refline
\@reflinefalse
\newif\if@norefline
\@noreflinefalse
\newif\if@toadrcenter
\@toadrcenterfalse
\newif\if@letterform
\@letterformfalse
\def\letterform{\@letterformtrue}
\def\noletterform{\@letterformfalse}
\def\centeraddress{\@toadrcentertrue}
\def\normaladdress{\@toadrcenterfalse}
\def\signature#1{%
  \ka@db@assignvar{signature}{#1}%
}
\ka@db@execatletter{\ka@db@assignvarlocal{signature}{}}%
\def\@fromsig{}
\long\def\address#1{\def\@fromaddress{#1}}
\def\@fromaddress{}
\long\def\footer#1{\def\@footer{#1}}
\def\@footer{}
\def\returnaddress{}
\def\place#1{\gdef\@place{#1}}
\def\@place{}
\def\phone#1#2{%
  \gdef\@prephone{#1}%
  \gdef\@phone{#2}%
  \gdef\@telephonenum{#1#2}}
\def\@prephone{}
\def\@phone{}
\def\@telephonenum{}
\def\phonemsg{Telefon}
\def\date#1{\gdef\@date{#1}\gdef\@ntoday{#1}}
\def\@date{\today}
\def\sign#1{\gdef\@mymail{#1}\@reflinetrue}
\def\@mymail{}
\def\signmsgold{Unsere Zeichen}
\def\signmsgnew{Unsere Zeichen, unsere Nachricht vom}
\let\signmsg\signmsgold
\def\datemsg{Datum}
\def\yourmail#1{\gdef\@yourmail{#1}\global\@reflinetrue}
\def\@yourmail{}
\def\yourmailmsg{Ihre Zeichen, Ihre Nachricht vom}
\def\writer#1{%
  \ka@db@assignvar{writer}{#1}%
  \@reflinetrue
  \enabledraftstandard
}
\ka@db@execatletter{\ka@db@assignvarlocal{writer}{}}%
\ka@db@assignvarglobal{writer}{}%
\def\writermsg{,\ Bearbeiter}
\def\@concern{}
\newbox\@@bottomtext
\setbox\@@bottomtext=\vbox to 0pt{}
\def\ka@db@bottomtextmsg{}
\long\def\ka@db@praebottomtext#1{%
  \gdef\ka@db@bottomtextmsg{#1}%
}
\long\def\ka@db@mainbottomtext#1{%
  \setbox\@@bottomtext=\hbox to 0pt{\hskip0pt minus 4truemm%
       \vbox{\raggedright #1}%
       \hss}%
}
\long\def\bottomtext#1{%
  \ifka@db@ltxtwoe \relax \else
      \@@warning{\string\bottomtext \space is reserving space for
                 typesetting its argument only in case of LaTeX 2e.
                 With other LaTeX formats you will have to place a
                 \string\clearpage-command at the appropriate position}%
  \fi
  \ka@db@dobottomtext{#1}%
  % \gdef\ka@db@bottomtextmsg{#1}%
  % \setbox\@@bottomtext=\hbox to 0pt{\hskip0pt minus 4truemm%
  %      \vbox{\raggedright #1}%
  %      \hss}%
  }
\long\def\ka@db@dobottomtext#1{\ka@db@praebottomtext{#1}}

\def\document{%
  \endgroup
  \ifka@db@ltxtwoe
    \ifx\@unusedoptionlist\@empty\else
      \@latex@warning@no@line{Unused global option(s):^^J%
              \@spaces[\@unusedoptionlist]}%
    \fi
  \fi
  \@colht\textheight
  \@colroom\textheight
  \vsize\textheight
  \columnwidth\textwidth
  \@clubpenalty\clubpenalty
  \if@twocolumn
    \advance\columnwidth -\columnsep
    \divide\columnwidth\tw@
    \hsize\columnwidth
    \@firstcolumntrue
  \fi
  \hsize\columnwidth
  \linewidth\hsize
  \begingroup
    \@floatplacement
    \@dblfloatplacement
    \makeatletter
    \ifka@db@ltxtwoe
      \let\@writefile\@gobbletwo
    \fi
    \@input{\jobname@aux}%
  \endgroup
  \if@filesw
    \ifnum\c@labelgen>0
      \immediate\openout\@mainlbl=\jobname@lbl
      \immediate\write\@mainlbl{\string\relax}%
      \immediate\write\@mainlbl{\string\startlabels\string\@startlabels}%
    \fi
    \immediate\openout\@mainaux=\jobname@aux
    \immediate\write\@mainaux{\string\relax}%
  \fi
  \ifka@db@ltxtwoe
    \process@table
    \let\glb@currsize\@empty  %% Force math initialisation.
  \fi
  \@normalsize
  \everypar{}%
  \ifka@db@ltxtwoe
    \@noskipsecfalse
  \fi
  \ifka@db@ltxtwoe
    \@begindocumenthook
    \ifdim\topskip<1sp
      \topskip 1sp
      \relax
    \fi
    \let\@begindocumenthook\@undefined
    \ifx\@listfiles\@undefined
      \let\@filelist\relax
      \let\@addtofilelist\@gobble
    \fi
  \fi
  \def\do##1{\let ##1\@notprerr}%
  \@preamblecmds
  \global\let\do\noexpand
  % \ka@db@assbottomtext{\ka@db@bottomtextmsg}%
  \long\def\ka@db@dobottomtext##1{\ka@db@mainbottomtext{##1}}
}
\def\enddocument{%
  \@checkend{document}
  \newpage
  \begingroup
    \if@filesw
      \ifnum\c@labelgen>0
        \immediate\closeout\@mainlbl
        \makeatletter
        \input \jobname@lbl\relax
        \clearpage
      \fi
      \immediate\closeout\@mainaux
    \fi
  \endgroup
  \deadcycles\z@
  \@@end}
\def\makelabels{%
  % \@fileswtrue
  \c@labelgen=2
}
\newcount\c@labelgen
\c@labelgen=0
\def\ka@db@asslabelgen#1#2#3{%
  \if@filesw
    #3\c@labelgen=#1\relax
  \else
    %\@warning{\string\makelabels\space not given. %
    %          \string#2\space has no effect}%
    \ClassWarning{dinbrief}{%
      \string\makelabels\space not given. %
      \string#2\space has no effect}%
  \fi
}
\def\nolabels{%
  \ka@db@asslabelgen{1}{\nolabels}{\relax}%
}
\def\offlabels{%
  \ka@db@asslabelgen{0}{\offlabels}{\global}%
}
\def\onlabels{%
  \ka@db@asslabelgen{2}{\onlabels}{\global}%
}
\def\@startlabels{}
\let\@texttop=\relax
\newif\ifka@db@lastpage
\ka@db@lastpagefalse
\def\ps@headings{
  \headheight=\ltf@headheight
  \headsep=\ltf@headsep
  \ifka@db@ltxtwoe
      \relax
    \else
      \footheight 0truemm
    \fi
  \footskip 0truemm
  \def\@oddhead{%
    \sl \headtoname\
    \ignorespaces\toname \hfil \@date
    \hfil \pagename{} \thepage}
  \def\@oddfoot{}
  \def\@evenhead{%
    \sl \headtoname\
    \ignorespaces\toname \hfil \@date
    \hfil \pagename{} \thepage}
  \def\@evenfoot{}
  }
\def\ps@empty{%
  \headheight=\lts@headheight
  \headsep=\lts@headsep
  \ifka@db@ltxtwoe
      \relax
    \else
      \footheight 0truemm
    \fi
  \footskip 0truemm
  \def\@oddhead{}
  \def\@oddfoot{}
  \def\@evenhead{}
  \def\@evenfoot{}
  }
\def\ps@first@page{%
  \headheight=\lts@headheight
  \headsep=\lts@headsep
  \ifka@db@ltxtwoe
      \relax
    \else
      \footheight 4.2truemm
    \fi
  \footskip 8.8truemm
  \def\@oddhead{}
  \def\@evenhead{}
  \expandafter\ifx\csname psfirstpagehook\endcsname\relax
    \else
      \psfirstpagehook
    \fi
}
\def\psfirstpagehook{%
  \def\@oddfoot{}
  \def\@evenfoot{}}
\def\ps@plain{%
  \headheight=\lts@headheight
  \headsep=\lts@headsep
  \ifka@db@ltxtwoe
      \relax
    \else
      \footheight 4.2truemm
    \fi
  \footskip 8.8truemm
  \def\@oddhead{}
  \def\@oddfoot{\hfil{\ka@db@fontshape{n}%
                      \ka@db@fontseries{m}\ka@db@selectfont\thepage}
                      \hfil}%
  \def\@evenhead{}
  \def\@evenfoot{}
  }
\def\ps@myheadings{%\let\@mkboth\@gobbletwo
  \headheight=\ltf@headheight
  \headsep=\ltf@headsep
  \ifka@db@ltxtwoe
      \relax
    \else
      \footheight 0truemm
    \fi
  \footskip 0truemm
  \def\@oddhead{{\sl \rightmark}}%
  \def\@oddfoot{}
  \def\@evenhead{{\sl \leftmark}}%
  \def\@evenfoot{}
  }
\def\ps@contheadings{%\let\@mkboth\@gobbletwo
  \headheight=\ltf@headheight
  \headsep=\ltf@headsep
  \ifka@db@ltxtwoe
      \relax
    \else
      \footheight 0truemm
    \fi
  \footskip 8.8truemm
  \def\@oddhead{{\sl \headtoname\ \ignorespaces\toname \hfil \@date
                \hfil\pagename{} \thepage}}%
  \def\@oddfoot{\ifka@db@lastpage
                  \relax
                \else
                  \addtocounter{page}{1}%
                  \hfil--- \thepage{} ---\hfil%
                  \addtocounter{page}{-1}%
                \fi}
  \def\@evenhead{{\sl \headtoname\ \ignorespaces\toname \hfil \@date
                \hfil\pagename{} \thepage}}%
  \def\@evenfoot{\ifka@db@lastpage
                   \relax
                 \else
                   \addtocounter{page}{1}%
                   ~\hfil--- \thepage{} ---~%
                   \addtocounter{page}{-1}%
                 \fi}
                }
\lineskip 1pt
\normallineskip 1pt
\def\baselinestretch{1}
\parskip .7em
\parindent 0pt
\topsep .4em
\partopsep 0pt
\itemsep .4em
\@lowpenalty 51
\@medpenalty 151
\@highpenalty 301
\@beginparpenalty -\@lowpenalty
\@endparpenalty -\@lowpenalty
\@itempenalty -\@lowpenalty
\leftmargini 2.5em
\leftmarginii 2.2em
\leftmarginiii 1.87em
\leftmarginiv 1.7em
\leftmarginv 1em
\leftmarginvi 1em
\leftmargin\leftmargini
\labelwidth\leftmargini
\advance\labelwidth-\labelsep
\labelsep 5pt
\parsep 0pt
\let\@listi\relax
\def\@listii{%
  \leftmargin\leftmarginii
  \labelwidth\leftmarginii
  \advance\labelwidth-\labelsep}
\def\@listiii{%
  \leftmargin\leftmarginiii
  \labelwidth\leftmarginiii
  \advance\labelwidth-\labelsep
  \topsep .2em
  \itemsep \topsep}
\def\@listiv{%
  \leftmargin\leftmarginiv
  \labelwidth\leftmarginiv
  \advance\labelwidth-\labelsep}
\def\@listv{%
  \leftmargin\leftmarginv
  \labelwidth\leftmarginv
  \advance\labelwidth-\labelsep}
\def\@listvi{%
  \leftmargin\leftmarginvi
  \labelwidth\leftmarginvi
  \advance\labelwidth-\labelsep}
\def\theenumi{\arabic{enumi}}
\def\theenumii{\alph{enumii}}
\def\theenumiii{\roman{enumiii}}
\def\theenumiv{\Alph{enumiv}}
\def\labelenumi{\arabic{enumi}.}
\def\labelenumii{(\alph{enumii})}
\def\labelenumiii{\roman{enumiii}.}
\def\labelenumiv{\Alph{enumiv}.}
\def\p@enumii{\theenumi}
\def\p@enumiii{\theenumi(\theenumii)}
\def\p@enumiv{\p@enumiii\theenumiii}
\def\labelitemi{$\bullet$}
\def\labelitemii{\bf --}
\def\labelitemiii{$\ast$}
\def\labelitemiv{$\cdot$}
\def\descriptionlabel#1{%
  \hspace\labelsep \bf #1}
\def\description{%
  \list{}{\labelwidth\z@ \itemindent-\leftmargin
          \let\makelabel\descriptionlabel}}
\let\enddescription\endlist
\def\verse{\let\\=\@centercr
  \list{}{\itemsep\z@
          \itemindent -15pt
          \listparindent \itemindent
          \rightmargin\leftmargin
          \advance\leftmargin 15pt}\item[]}
\let\endverse\endlist
\def\quotation{%
  \list{}{\listparindent 1.5em
          \itemindent\listparindent
          \rightmargin\leftmargin}%
  \item[]}
\let\endquotation=\endlist
\def\quote{%
  \list{}{\rightmargin\leftmargin}%
  \item[]}
\let\endquote=\endlist
\arraycolsep 5pt
\tabcolsep 6pt
\arrayrulewidth .4pt
\doublerulesep 2pt
\tabbingsep \labelsep
\skip\@mpfootins = \skip\footins
\fboxsep = 3pt
\fboxsep = 3pt
\fboxrule = .4pt
\def\theequation{\arabic{equation}}
\@addtoreset{equation}{letter}
\def\footnoterule{%
  \kern-1\p@
  \hrule width .4\columnwidth
  \kern .6\p@}
\long\def\@makefntext#1{%
  \noindent
  \hangindent 5pt%
  \hbox  to 5pt{\hss $^{\@thefnmark}$}#1}
\c@topnumber=2
\def\topfraction{.7}
\c@bottomnumber=1
\def\bottomfraction{.3}
\c@totalnumber=3
\def\textfraction{.2}
\def\floatpagefraction{.5}
\c@dbltopnumber= 2
\def\dbltopfraction{.7}
\def\dblfloatpagefraction{.5}
\def\today{\number\day.\space\ifcase\month\or
  Januar\or Februar\or M\"arz\or April\or Mai\or Juni\or
  Juli\or August\or September\or Oktober\or November\or Dezember\fi
  \space\number\year}
\newcount\yearcnt
\yearcnt=\year
\advance\yearcnt-\number1900
\def\@znumber#1{\ifnum\number#1<10 0\number#1\else\number#1\fi}
\def\ntoday{\@znumber{\number\day}.%
            \@znumber{\number\month}.%
            \@znumber{\the\yearcnt}}
\def\@ntoday{\ntoday}
\def\up#1{\leavevmode \raise.16ex\hbox{#1}}
\newbox\@betr
\long\def\concern#1{%
  \setbox\@betr=\hbox{}
  \def\@concern{\hangindent=\wd\@betr
                \hangafter=1
                \unhbox\@betr #1\par}}
\let\subject=\concern
\newlength{\@fordate}
\setlength{\@fordate}{\textwidth}
\addtolength{\@fordate}{-131truemm}
\def\@answertoold{%
  \setlength{\@fordate}{\textwidth}
  \addtolength{\@fordate}{-131truemm}
  \parbox[b]{50.8truemm}{{\ka@db@fontsize{9}{11pt}\ka@db@selectfont
                          \yourmailmsg{}}%
                         \hfill\\ \@yourmail\hbox{}\hss}%
  \parbox[b]{50.8truemm}{{\ka@db@fontsize{9}{11pt}\ka@db@selectfont
                          \signmsg{}}%
                         \hfill\\ \@mymail\hbox{}\hss}%
  \parbox[b]{25.4truemm}{{\ka@db@fontsize{9}{11pt}\ka@db@selectfont
                          \phonemsg{} \@prephone{}}%
                         \hfill\\ \@phone\hbox{}\hss}%
  \parbox[b]{\@fordate}{{\ka@db@fontsize{9}{11pt}\ka@db@selectfont
    \ifx\@empty\@place
       \vphantom{K}\rule{2pt}{0pt}
     \else
       \@place{}%
     \fi
    }\hfill\\ \@ntoday}
  }
\def\@answertonew{%
  \hbox to \textwidth{%
    \setlength{\@fordate}{\textwidth}%
    \addtolength{\@fordate}{-156.4truemm}%
    \addtolength{\@fordate}{3cm}% This is a heavens value.
    \parbox[b]{50.8truemm}{{\ka@db@fontsize{7}{9pt}\ka@db@selectfont
                            \yourmailmsg{}}%
                           \hfill\\ \@yourmail\hbox{}\hss}%
    \parbox[b]{50.8truemm}{{\ka@db@fontsize{7}{9pt}\ka@db@selectfont
                            \signmsg{}}%
                           \hfill\\ \@mymail\hbox{}\hss}%
    \parbox[b]{50.8truemm}{{\ka@db@fontsize{7}{9pt}\ka@db@selectfont
                            \phonemsg{}%
                            \writermsg{}%
                            }%
                           \hfill\\
                           \ifx\@empty\@prephone\relax
                           \else
                             \@prephone{}
                           \fi
                           \@phone
                           \ka@db@ifemptyvar{writer}%
                             {}% true
                             {% false
                               \ifx\@empty\@phone\relax
                               \else
                                 ,\ % insert comma and blank only
                               \fi%   if both are nonempty
                               \ka@db@usevar{writer}%
                             }%
                           \hbox{}\hss}%
    \parbox[b]{\@fordate}{{\ka@db@fontsize{7}{9pt}\ka@db@selectfont
      \datemsg
      }\hfill\\ \@ntoday}
    \hss}%
  }
\def\enabledraftstandard{%
  \let\signmsg=\signmsgnew
  \let\@answerto=\@answertonew
}
\def\disabledraftstandard{%
  \let\signmsg=\signmsgold
  \let\@answerto=\@answertoold
}
\disabledraftstandard
\def\Postvermerk#1{%
  %\@warning{Use \string\postremark\space instead %
  %          of \string\Postvermerk}%
  \ClassWarning{dinbrief}{%
    Use \string\postremark\space instead %
    of \string\Postvermerk}%
  \postremark{#1}%
}
\def\Behandlungsvermerk#1{%
  %\@warning{Use \string\handling\space instead of %
  %          \string\Behandlungsvermerk}%
  \ClassWarning{dinbrief}{%
    Use \string\handling\space instead of %
    \string\Behandlungsvermerk}%
  \handling{#1}%
}
\def\Datum#1{%
  \date{#1}%
}
\long\def\ka@db@splitondblpt#1{%
  \ka@db@splitdblptx #1::\@st@p@
  \ifx\ka@db@fromadrtwo\@empty
  \else
    \ka@db@splitdblpty #1\@st@p@
  \fi
}
\def\ka@db@fromadrone{}%
\def\ka@db@fromadrtwo{}%
\long\def\ka@db@splitdblptx #1::#2\@st@p@{%
  \def\ka@db@fromadrone{#1}%
  \def\ka@db@fromadrtwo{#2}%
}
\long\def\ka@db@splitdblpty #1::#2\@st@p@{%
  \def\ka@db@fromadrtwo{#2}%
}
\long\def\stdaddress#1{%
  \ka@db@splitondblpt{#1}
  \address{%
    \vspace*{7truemm}
    \leavevmode
    \hbox to 0pt{\hbox to \textwidth{\hfil\@date}\hss}%
    \parbox[t]{\textwidth}{\ka@db@fromadrone\ka@db@fromadrtwo}%
  }%
  \@noreflinetrue
}
\long\def\Absender#1{%
  %\@warning{Use \string\stdaddress\space instead of \string\Absender}%
  \stdaddress{#1}%
}
\def\Retourlabel{%
  %\@warning{\string\Retourlabel\space not yet implemented.}%
  \ClassWarning{dinbrief}{%
    \string\Retourlabel\space not yet implemented.}%
}
\def\Fenster{%
  \ClassWarning{dinbrief}{%
    \string\Fenster\space not yet implemented.}%
}
\def\Retouradresse#1{%
  \ClassWarning{dinbrief}{%
    Use \string\backaddress\space instead of \string\Retouradresse}%
  \backaddress{#1}%
}
\def\Betreff#1{%
  \ClassWarning{dinbrief}{%
    Use \string\subject\space instead of \string\Betreff}%
  \concern{#1}%
}
\def\dinquote{%
  \list{}{\leftmargin=1in}%
  \item[]}
\let\enddinquote=\endlist
\long\def\Einrueckung#1{%
  \ClassWarning{dinbrief}{%
    Use the environment dinquote in future}%
  \begin{dinquote}%
  #1
  \end{dinquote}%
}
\newif\ifka@db@enclright
\ka@db@enclrightfalse
\def\enclright{%
  \ka@db@enclrighttrue
}
\def\noenclright{%
  \ka@db@enclrightfalse
}
\def\anlagenrechts{%
  \ka@db@enclrighttrue
}
\def\Anlagen#1{%
  %\@warning{Use \string\encl\space instead of \string\Anlagen}%
  \ClassWarning{dinbrief}{%
    Use \string\encl\space instead of \string\Anlagen}%
  \encl[]{#1}%
}
\def\Verteiler#1{%
  %\@warning{Use \string\cc\space instead of \string\Verteiler}%
  \ClassWarning{dinbrief}{%
    Use \string\cc\space instead of \string\Verteiler}%
  \cc[]{#1}%
}
\ifka@db@nfss
  \else
    \ifka@db@nfsstwo
      \else
        \normalsize
      \fi
  \fi
\smallskipamount=.5\parskip
\medskipamount=\parskip
\bigskipamount=2\parskip
\pagestyle{plain}
\pagenumbering{arabic}
\onecolumn
\errorcontextlines=10
\endinput
%% 
%% End of file `dinbrief.cls'.
