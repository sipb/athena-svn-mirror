%%
%% This is file `scrreprt.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% scrclass.dtx  (with options: `class,scrreprt')
%% scrlogo.dtx 
%% 
%% Copyright (C) 1994-2002 Markus Kohm and Frank Neukam.
%% 
%% All rights reserved. For further copyright information see the file
%% LEGAL.TXT or LEGALDE.TXT.
%% 
%% This file is part of the LaTeX2e KOMA-Script bundle.
%% ----------------------------------------------------
%% 
%% The KOMA-Script bundle bases on the Script-family created by
%% Frank Neukam 1993 and the standard documents classes created
%% by the LaTeX3-Team 1994-1996.
%% ------------------------------------------------------------------------
%% 
%%  This system is distributed in the hope that it will be useful,
%%  but WITHOUT ANY WARRANTY; without even the implied warranty of
%%  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
%% 
%% 
%% IMPORTANT NOTICE:
%% 
%% For error reports in case of UNCHANGED versions see documentation.
%% 
%% Please do not request updates from us directly. Distribution is
%% done through Mail-Servers, Mailboxes and TeX organizations.
%% 
%% You are not allowed to change this file.
%% 
%% You are allowed to distribute this file under the condition that
%% it is distributed together with all files mentioned in readme.txt
%% and liesmich.txt. See LEGAL.TXT or LEGALDE.TXT for further
%% information.
%% 
%% If you receive only some of these files from someone, complain!
%% 
%% You are NOT ALLOWED to distribute this file alone.
%% 
\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{%
  scrreprt%
  }
  [2003/01/31 v2.9n LaTeX2e KOMA
    document class]
\newcommand\ifnumber[3]{
 \begingroup\@tempswafalse\let\scr@next\test@number
 \expandafter\scr@next#1\scr@next
 \if@tempswa\endgroup#2\else\endgroup#3\fi
}
\newcommand*{\test@number}[1]{
 \ifx \scr@next#1
  \let\scr@next\relax
 \else
  \@tempcnta=\expandafter\expandafter\expandafter\number
  \expandafter`#1\relax
  \ifnum \@tempcnta>47\relax
   \ifnum \@tempcnta<58\relax
    \@tempswatrue
   \else\@tempswafalse\fi
  \else\@tempswafalse\fi
  \if@tempswa\else\let\scr@next\gobble@till@next\fi\fi
 \scr@next}
\newcommand*{\gobble@till@next}{}
\def\gobble@till@next#1\scr@next{}
\newcommand*{\@BCOR}{\expandafter\@@BCOR\CurrentOption BCORBCOR\@@BCOR}
\newcommand*{\@@BCOR}{}
\def\@@BCOR #1BCOR#2BCOR#3\@@BCOR{
 \edef\@tempa{#1}\ifx\@tempa\@empty
  \edef\@tempa{#3}\def\@tempb{BCOR}\ifx\@tempa\@tempb
   \PassOptionsToPackage{BCOR#2}{typearea}
  \else\OptionNotUsed\fi
 \else\OptionNotUsed\fi}
\newcommand*{\@DIV}{\expandafter\@@DIV\CurrentOption DIVDIV\@@DIV}
\newcommand*{\@@DIV}{}
\def\@@DIV #1DIV#2DIV#3\@@DIV{
 \edef\@tempa{#1}\ifx\@tempa\@empty
  \edef\@tempa{#3}\def\@tempb{DIV}\ifx\@tempa\@tempb
   \PassOptionsToPackage{DIV#2}{typearea}
  \else\OptionNotUsed\fi
 \else\@BCOR\fi}
\newcommand{\@headlines}{%
  \expandafter\@@headlines\CurrentOption headlines\@@headlines}
\newcommand*{\@@headlines}{}
\def\@@headlines #1headlines#2\@@headlines{
  \edef\@tempa{#2}\edef\@tempb{headlines}\ifx\@tempa\@tempb
    \PassOptionsToPackage{#1headlines}{typearea}
  \else\@DIV\fi}
\newcommand*{\@pt@scan}{\expandafter\@pt@@scan\CurrentOption pt\@pt@@scan}
\newcommand*{\@pt@@scan}{}
\def\@pt@@scan #1pt#2\@pt@@scan{
 \edef\@tempa{pt}\edef\@tempb{#2}\ifx\@tempa\@tempb
  \ifnumber{#1}
   {\@tempcnta=#1\relax
    \advance \@tempcnta by-10\relax
    \edef\@ptsize{\the\@tempcnta}}
   {\@headlines}
  \else\@headlines\fi}
\newcommand*{\x@paper}{\expandafter\x@@paper\CurrentOption paper\x@@paper}
\newcommand*{\x@@paper}{}
\def\x@@paper #1#2paper#3\x@@paper{
 \edef\@tempa{paper}\edef\@tempb{#3}\ifx\@tempa\@tempb
  \ifx a#1\PassOptionsToPackage{a#2paper}{typearea}
   \else\ifx b#1\PassOptionsToPackage{b#2paper}{typearea}
    \else\ifx c#1\PassOptionsToPackage{c#2paper}{typearea}
     \else\ifx d#1\PassOptionsToPackage{d#2paper}{typearea}
      \else\OptionNotUsed\fi\fi\fi\fi
 \else\@pt@scan\fi}
\newif\ifonelinecaptions\onelinecaptionstrue
\DeclareOption{onelinecaption}{\onelinecaptionstrue%
  \PassOptionsToPackage{oneline}{caption2}}
\DeclareOption{noonelinecaption}{\onelinecaptionsfalse%
  \PassOptionsToPackage{nooneline}{caption2}}
\newif\if@tablecaptionabove\@tablecaptionabovefalse
\DeclareOption{tablecaptionabove}{\@tablecaptionabovetrue}
\DeclareOption{tablecaptionbelow}{\@tablecaptionabovefalse}
\newif\if@komalongtable\@komalongtabletrue
\DeclareOption{origlongtable}{\@komalongtablefalse}
\newif\if@hsl
\DeclareOption{headsepline}{%
  \PassOptionsToPackage{headinclude}{typearea}\@hsltrue}
\DeclareOption{headnosepline}{%
  \@hslfalse%
  }
\newif\if@fsl
\DeclareOption{footsepline}{\PassOptionsToPackage{footinclude}{typearea}
                            \@fsltrue}
\DeclareOption{footnosepline}{%
  \@fslfalse%
  }
\newif\if@mincl
\DeclareOption{mpinclude}{%
  \@mincltrue
  }
\DeclareOption{mpexclude}{
  \@minclfalse
  }
\newcommand*\@ptsize{%
  }
\DeclareOption{10pt}{%
  \renewcommand*\@ptsize{0}
  }
\DeclareOption{11pt}{%
  \renewcommand*\@ptsize{1}
  }
\DeclareOption{12pt}{%
  \renewcommand*\@ptsize{2}
  }
\DeclareOption{oneside}{%
  \@twosidefalse \@mparswitchfalse
  }
\DeclareOption{twoside}{\@twosidetrue  \@mparswitchtrue}
\DeclareOption{draft}{\setlength{\overfullrule}{5pt}}
\DeclareOption{final}{%
  \setlength{\overfullrule}{\z@}
  }
\newif\if@titlepage
\@titlepagetrue
\DeclareOption{titlepage}{\@titlepagetrue}
\DeclareOption{notitlepage}{\@titlepagefalse}
\newif\if@openright
\DeclareOption{openright}{\@openrighttrue}
\DeclareOption{openany}{\@openrightfalse}
\DeclareOption{onecolumn}{\@twocolumnfalse}
\DeclareOption{twocolumn}{\@twocolumntrue}
\DeclareOption{leqno}{\input{leqno.clo}}
\DeclareOption{fleqn}{\input{fleqn.clo}}
\DeclareOption{openbib}{%
  \AtEndOfPackage{%
   \renewcommand*\@openbib@code{%
      \advance\leftmargin\bibindent
      \itemindent -\bibindent
      \listparindent \itemindent
      \parsep \z@
      }%
   \renewcommand*\newblock{\par}}%
}
\newif\if@abstrt
\DeclareOption{abstracton}{\@abstrttrue}
\DeclareOption{abstractoff}{\@abstrtfalse}
\newif\if@parskip
\newcommand*{\scr@parskip}{\setlength{\parskip}{\z@}}
\newcommand*{\scr@parfillskip}{\setlength{\parfillskip}{\z@}}
\DeclareOption{parskip}{\@parskiptrue%
  \renewcommand*{\scr@parskip}{%
    \setlength{\parskip}{\baselineskip}%
    \addtolength{\parskip}{\z@ \@plus .1\baselineskip}}
  \renewcommand*{\scr@parfillskip}{%
    \setlength{\parfillskip}{1em \@plus 1fil}}}
\DeclareOption{parskip-}{\@parskiptrue%
  \renewcommand*{\scr@parskip}{%
    \setlength{\parskip}{\baselineskip}%
    \addtolength{\parskip}{\z@ \@plus .1\baselineskip}}
  \renewcommand*{\scr@parfillskip}{%
    \setlength{\parfillskip}{\z@ \@plus 1fil}}}
\DeclareOption{parskip+}{\@parskiptrue%
  \renewcommand*{\scr@parskip}{%
    \setlength{\parskip}{\baselineskip}%
    \addtolength{\parskip}{\z@ \@plus .1\baselineskip}}
  \renewcommand*{\scr@parfillskip}{%
    \setlength{\parfillskip}{.3333\linewidth}
    \addtolength{\parfillskip}{\z@ \@plus 1fil}}}
\DeclareOption{parskip*}{\@parskiptrue%
  \renewcommand*{\scr@parskip}{%
    \setlength{\parskip}{\baselineskip}%
    \addtolength{\parskip}{\z@ \@plus .1\baselineskip}}
  \renewcommand*{\scr@parfillskip}{%
    \setlength{\parfillskip}{.25\linewidth}
    \addtolength{\parfillskip}{\z@ \@plus 1fil}}}
\DeclareOption{halfparskip}{\@parskiptrue%
  \renewcommand*{\scr@parskip}{%
    \setlength{\parskip}{.5\baselineskip}%
    \addtolength{\parskip}{\z@ \@plus .5\baselineskip}}
  \renewcommand*{\scr@parfillskip}{%
    \setlength{\parfillskip}{1em \@plus 1fil}}}
\DeclareOption{halfparskip-}{\@parskiptrue%
  \renewcommand*{\scr@parskip}{%
    \setlength{\parskip}{.5\baselineskip}%
    \addtolength{\parskip}{\z@ \@plus .5\baselineskip}}
  \renewcommand*{\scr@parfillskip}{%
    \setlength{\parfillskip}{\z@ \@plus 1fil}}}
\DeclareOption{halfparskip+}{\@parskiptrue%
  \renewcommand*{\scr@parskip}{%
    \setlength{\parskip}{.5\baselineskip}%
    \addtolength{\parskip}{\z@ \@plus .5\baselineskip}}
  \renewcommand*{\scr@parfillskip}{%
    \setlength{\parfillskip}{.3333\linewidth}
    \addtolength{\parfillskip}{\z@ \@plus 1fil}}}
\DeclareOption{halfparskip*}{\@parskiptrue%
  \renewcommand*{\scr@parskip}{%
    \setlength{\parskip}{.5\baselineskip}%
    \addtolength{\parskip}{\z@ \@plus .5\baselineskip}}
  \renewcommand*{\scr@parfillskip}{%
    \setlength{\parfillskip}{.25\linewidth}
    \addtolength{\parfillskip}{\z@ \@plus 1fil}}}
\DeclareOption{parindent}{\@parskipfalse}
\DeclareOption{pointednumbers}
  {\AtEndOfClass{\@altsecnumformattrue
                 \global\let\@altsecnumformatfalse\@altsecnumformattrue}}
\DeclareOption{pointednumber}
  {\AtEndOfClass{\@altsecnumformattrue
                 \global\let\@altsecnumformatfalse\@altsecnumformattrue}
   \ClassError
    {scrreprt}
    {undefined option `pointednumber' instead of `pointednumbers'}
    {You've used the class-option `pointednumber'.\MessageBreak
     This name was an implementation-error at versions from 2.3h till
     2.4f.\MessageBreak
     The correct name is `pointednumbers'.
     Change this at your document.\MessageBreak
     If you'll continue, the option `pointednumbers' will be used.}}
\DeclareOption{pointlessnumbers}
  {\AtEndOfClass{\@altsecnumformatfalse
                 \global\let\@altsecnumformattrue\@altsecnumformatfalse}}
\DeclareOption{pointlessnumber}
  {\AtEndOfClass{\@altsecnumformatfalse
                 \global\let\@altsecnumformattrue\@altsecnumformatfalse}
   \ClassError
    {scrreprt}
    {undefined option `pointlessnumber' instead of `pointlessnumbers'}
    {You've used the class-option `pointlessnumber'.\MessageBreak
     This name was an implementation-error at versions from 2.3h till
     2.4f.\MessageBreak
     The correct name is `pointlessnumbers'.
     Change this at your document.\MessageBreak
     If you'll continue, the option `pointlessnumbers' will be used.}}
\newcommand\@bigheadings
  {\AtEndOfClass{
    \renewcommand*{\chapterheadstartvskip}{\vspace*{2.3\baselineskip}}
    \renewcommand*{\chapterheadendvskip}{\vspace{1.725\baselineskip
        \@plus .115\baselineskip \@minus .192\baselineskip}}
    \let\size@part\Huge
    \let\size@partnumber\huge
    \let\size@chapter\huge
    \let\size@section\Large
    \let\size@subsection\large
    \let\size@subsubsection\normalsize
    \let\size@paragraph\normalsize
    \let\size@subparagraph\normalsize}}
\DeclareOption{bigheadings}{\@bigheadings}
\DeclareOption{bigheading}{\@bigheadings
  \ClassError
    {scrreprt}
    {undefined option `bigheading' instead of `bigheadings'}
    {You've used the old option name `bigheading'!\MessageBreak
     The correct name of the option is `bigheadings'.\MessageBreak
     The old name was an implementation error from\MessageBreak
     version 2.3h till 2.4f.\MessageBreak
     You should change this at your document!\MessageBreak
     If you'll continue, option `bigheadings' will be used.}}
\newcommand\@normalheadings
  {\AtEndOfClass{
    \renewcommand*{\chapterheadstartvskip}{\vspace*{2\baselineskip}}
    \renewcommand*{\chapterheadendvskip}{\vspace{1.5\baselineskip
        \@plus .1\baselineskip \@minus .167\baselineskip}}
    \let\size@part\huge
    \let\size@partnumber\huge
    \let\size@chapter\LARGE
    \let\size@section\Large
    \let\size@subsection\large
    \let\size@subsubsection\normalsize
    \let\size@paragraph\normalsize
    \let\size@subparagraph\normalsize}}
\DeclareOption{normalheadings}{\@normalheadings}
\DeclareOption{normalheading}{\@normalheadings
  \ClassError
    {scrreprt}
    {undefined option `normalheading' instead of `normalheadings'}
    {You've used the old option name `normalheading'!\MessageBreak
     The correct name of the option is `normalheadings'.\MessageBreak
     The old name was an implementation error from\MessageBreak
     version 2.3h till 2.4f.\MessageBreak
     You should change this at your document!\MessageBreak
     If you'll continue, option `normalheadings' will be used.}}
\newcommand\@smallheadings
  {\AtEndOfClass{
    \renewcommand*{\chapterheadstartvskip}{\vspace*{1.8\baselineskip}}
    \renewcommand*{\chapterheadendvskip}{\vspace{1.35\baselineskip
        \@plus 0.09\baselineskip \@minus .15\baselineskip}}
    \let\size@part\LARGE
    \let\size@partnumber\LARGE
    \let\size@chapter\Large
    \let\size@section\large
    \let\size@subsection\normalsize
    \let\size@subsubsection\normalsize
    \let\size@paragraph\normalsize
    \let\size@subparagraph\normalsize}}
\DeclareOption{smallheadings}{\@smallheadings}
\DeclareOption{smallheading}{\@smallheadings
  \ClassError
    {scrreprt}
    {undefined option `smallheading' instead of `smallheadings'}
    {You've used the old option name `smallheading'!\MessageBreak
     The correct name of the option is `smallheadings'.\MessageBreak
     The old name was an implementation error from\MessageBreak
     version 2.3h till 2.4f.\MessageBreak
     You should change this at your document!\MessageBreak
     If you'll continue, option `smallheadings' will be used.}}
\newif\if@chapterprefix
\DeclareOption{chapterprefix}{\@chapterprefixtrue}
\DeclareOption{nochapterprefix}{\@chapterprefixfalse}
\DeclareOption{appendixprefix}{\def\appendixmore{\@chapterprefixtrue}}
\DeclareOption{noappendixprefix}{\def\appendixmore{\@chapterprefixfalse}}
\newcommand*{\cleardoublestandardpage}{}
\let\cleardoublestandardpage\cleardoublepage
\newcommand*{\cleardoubleemptypage}{\clearpage%
  {\thispagestyle{empty}\cleardoublestandardpage}}
\newcommand*{\cleardoubleplainpage}{\clearpage%
  {\thispagestyle{plain}\cleardoublestandardpage}}
\DeclareOption{cleardoubleempty}{%
  \let\cleardoublepage\cleardoubleemptypage}
\DeclareOption{cleardoubleplain}{%
  \let\cleardoublepage\cleardoubleplainpage}
\DeclareOption{cleardoublestandard}{%
  \let\cleardoublepage\cleardoublestandardpage}
\DeclareOption{liststotoc}{%
  \AtEndOfClass{%
    \renewcommand*{\float@listhead}[1]{%
      \addchap{#1}
      \@mkboth{#1}{#1}%
      \if@dynlist%
        \newcommand*{\scr@starttoc}{}%
        \let\scr@starttoc=\@starttoc%
        \renewcommand*{\@starttoc}[1]{%
          \before@starttoc{##1}\scr@starttoc{##1}\after@starttoc{##1}}%
      \fi%
      }}
  }
\DeclareOption{liststotocnumbered}{%
  \AtEndOfClass{%
    \renewcommand*{\float@listhead}[1]{%
      \chapter{#1}
      \@mkboth{#1}{#1}%
      \if@dynlist%
        \newcommand*{\scr@starttoc}{}%
        \let\scr@starttoc=\@starttoc%
        \renewcommand*{\@starttoc}[1]{%
          \before@starttoc{##1}\scr@starttoc{##1}\after@starttoc{##1}}%
      \fi%
      }}
  }
\DeclareOption{bibtotoc}
  {\AtEndOfClass{%
    \renewcommand*\bib@heading{%
      \addchap{\bibname}
      \@mkboth{\bibname}{\bibname}
  }}}
\DeclareOption{bibtotocnumbered}
  {\AtEndOfClass{%
    \renewcommand*\bib@heading{%
      \chapter{\bibname}
      \@mkboth{\bibname}{\bibname}
  }}}
\DeclareOption{idxtotoc}
 {\AtEndOfClass{%
   \renewcommand*\idx@heading{%
     \if@openright\cleardoublepage\else\clearpage\fi%
     \twocolumn[\addchap{\indexname}]
     \@mkboth{\indexname}{\indexname}%
 }}}
\newif\if@tocleft
\DeclareOption{tocleft}{\@toclefttrue}
\DeclareOption{tocindent}{\@tocleftfalse}
\newif\if@dynlist
\DeclareOption{listsleft}{\@dynlisttrue}
\DeclareOption{listsindent}{\@dynlistfalse}
\if@compatibility\else
  \DeclareOption*{%
    \x@paper
    }
\fi
\ExecuteOptions{%
  final,11pt,onecolumn%
  ,headnosepline,footnosepline,bigheadings%
  ,nochapterprefix,oneside,openany,abstractoff%
}
\ProcessOptions\relax
\RequirePackage{scrlfile}
\newif\if@restonecol
\@tempcnta=10
\advance\@tempcnta by\@ptsize\relax
\xdef\@tempa{%
size%
\the\@tempcnta.clo}
\InputIfFileExists{\@tempa}{}{%
    \ClassError
    {scrreprt}
      {File `\@tempa' not found!\MessageBreak
       This error is fatal!}
      {To every fontsize you want to select by class option,\MessageBreak
       you need a corresponding
       size-class-option-file declaring all\MessageBreak
       the macros based on fontsize.\MessageBreak
       Ask a wizzard for more information or use another
       fontsize.\MessageBreak
       You cannot continue!}
       \batchmode \csname @@end\endcsname \end
}
\let\test@number      =\undefined
\let\gobble@till@next =\undefined
\let\@BCOR            =\undefined
\let\@@BCOR           =\undefined
\let\@DIV             =\undefined
\let\@@DIV            =\undefined
\let\@headlines       =\undefined
\let\@@headlines      =\undefined
\let\x@paper          =\undefined
\let\x@@paper         =\undefined
\newcommand*{\scr@file@date@version}{}
\def\scr@file@date@version#1/#2/#3 #4 #5\@nil{#1/#2/#3 #4}
\RequirePackage{typearea}[\expandafter\expandafter\expandafter%
  \scr@file@date@version\csname ver@%
  scrreprt%
  .cls\endcsname\@nil]
\setlength{\lineskip}{\p@}
\setlength{\normallineskip}{\p@}
\setlength{\columnsep}{10\p@}
\setlength{\columnseprule}{\z@}
\g@addto@macro{\@listi}{\@list@extra}
\let\@listI=\@listi
\g@addto@macro{\@listii}{\@list@extra}
\g@addto@macro{\@listiii}{\@list@extra}
\newcommand*{\add@extra@listi}[1]{%
  \expandafter\let\csname #1@listi\endcsname=\@listi
  \def\@listi{\csname #1@listi\endcsname\@list@extra}%
}
\g@addto@macro{\footnotesize}{\protect\add@extra@listi{ftns}}
\g@addto@macro{\small}{\protect\add@extra@listi{sml}}
\newcommand*{\@list@extra}{}
\newcommand*{%
  \@parskip@indent}{%
  \if@parskip
    \setlength{\parindent}{\z@}%
    \scr@parskip
    \scr@parfillskip
    \renewcommand*{\@list@extra}{%
      \topsep\z@ \parsep\parskip \itemsep\z@}%
  \else
    \setlength{\parindent}{1em}%
    \setlength{\parskip}{\z@ \@plus \p@}%
    \setlength{\parfillskip}{\z@ \@plus 1fil}%
    \renewcommand*{\@list@extra}{}%
  \fi
}%
\@parskip@indent
\AtBeginDocument{%
  \if@parskip\ifdim\parfillskip=\z@ \scr@parfillskip\fi\fi
}
\@lowpenalty  = 51
\@medpenalty  =151
\@highpenalty =301
\setcounter{topnumber}{2}
\renewcommand*\topfraction{.7}
\setcounter{bottomnumber}{1}
\renewcommand*\bottomfraction{.3}
\setcounter{totalnumber}{3}
\renewcommand*\textfraction{.2}
\renewcommand*\floatpagefraction{.5}
\setcounter{dbltopnumber}{2}
\renewcommand*\dbltopfraction{.7}
\renewcommand*\dblfloatpagefraction{.5}
\if@compatibility
  \newcommand*\headincludeon{\@hincltrue}
  \newcommand*\headincludeoff{\@hinclfalse}
  \newcommand*\headseplineon{\@hsltrue \@hincltrue}
  \newcommand*\headseplineoff{\@hslfalse \@hinclfalse}
  \newcommand*\footincludeon{\@fincltrue}
  \newcommand*\footincludeoff{\@finclfalse}
  \newcommand*\footseplineon{\@fsltrue \@fincltrue}
  \newcommand*\footseplineoff{\@fslfalse \@finclfalse}
\fi
\newcommand*{\set@tempdima@hw}{%
  \setlength{\@tempdima}{\textwidth}%
  \if@mincl
    \addtolength{\@tempdima}{\marginparsep}%
    \addtolength{\@tempdima}{\marginparwidth}%
  \fi
  }
\if@twoside
 \renewcommand*{\ps@plain}{\let\@mkboth\@gobbletwo
  \renewcommand*{\@evenhead}{}%
  \renewcommand*{\@oddhead}{}%
  \renewcommand*{\@evenfoot}{%
   \set@tempdima@hw\hss\hb@xt@ \@tempdima{\vbox{%
    \if@fsl \hrule \vskip 3\p@ \fi
    \hb@xt@ \@tempdima{{\pnumfont\thepage\hfil}}}}}%
  \renewcommand*{\@oddfoot}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \if@fsl \hrule \vskip 3\p@ \fi
    \hb@xt@ \@tempdima{{\pnumfont\hfil\thepage}}}}\hss}}%
 \newcommand*{\ps@headings}{\let\@mkboth\markboth
  \renewcommand*{\@evenhead}{%
   \set@tempdima@hw\hss\hb@xt@ \@tempdima{\vbox{%
    \hb@xt@ \@tempdima{{\headfont\strut\leftmark\hfil}}%
    \if@hsl \vskip 1.5\p@ \hrule \fi}}}
  \renewcommand*{\@oddhead}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \hb@xt@ \@tempdima{{\headfont\hfil\strut\rightmark}}%
    \if@hsl \vskip 1.5\p@ \hrule \fi}}\hss}
  \renewcommand*{\@evenfoot}{%
   \set@tempdima@hw\hss\hb@xt@ \@tempdima{\vbox{%
    \if@fsl \hrule \vskip 3\p@ \fi
    \hb@xt@ \@tempdima{{\pnumfont\thepage\hfil}}}}}%
  \renewcommand*{\@oddfoot}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \if@fsl \hrule \vskip 3\p@ \fi
    \hb@xt@ \@tempdima{{\pnumfont\hfil\thepage}}}}\hss}%
  \renewcommand*{\chaptermark}[1]{%
   \markboth {\ifnum \c@secnumdepth >\m@ne
        \chaptermarkformat\fi
        ##1}{}}%
  \renewcommand*{\sectionmark}[1]{%
    \markright {\ifnum \c@secnumdepth >\z@
        \sectionmarkformat\fi
        ##1}}}
 \newcommand*{\ps@myheadings}{\let\@mkboth\@gobbletwo
  \renewcommand*{\@evenhead}{%
   \set@tempdima@hw\hss\hb@xt@ \@tempdima{\vbox{%
    \hb@xt@ \@tempdima{{\headfont\strut\leftmark\hfil}}%
    \if@hsl \vskip 1.5\p@ \hrule \fi}}}
  \renewcommand*{\@oddhead}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \hb@xt@ \@tempdima{{\headfont\hfil\strut\rightmark}}%
    \if@hsl \vskip 1.5\p@ \hrule \fi}}\hss}
  \renewcommand*{\@evenfoot}{%
   \set@tempdima@hw\hss\hb@xt@ \@tempdima{\vbox{%
    \if@fsl \hrule \vskip 3\p@ \fi
    \hb@xt@ \@tempdima{{\pnumfont\thepage\hfil}}}}}%
  \renewcommand*{\@oddfoot}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \if@fsl \hrule \vskip 3\p@ \fi
    \hb@xt@ \@tempdima{{\pnumfont\hfil\thepage}}}}\hss}%
  \renewcommand*{\chaptermark}[1]{}
  \renewcommand*{\sectionmark}[1]{}}
\else
 \renewcommand*{\ps@plain}{\let\@mkboth\@gobbletwo
  \renewcommand*{\@evenhead}{}%
  \renewcommand*{\@oddhead}{}%
  \renewcommand*{\@evenfoot}{}%
  \renewcommand*{\@oddfoot}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \if@fsl \hrule \vskip 3\p@ \fi
    \hb@xt@ \@tempdima{{\pnumfont\hfil\thepage\hfil}}}}\hss}}
 \newcommand*{\ps@headings}{\let\@mkboth\markboth
  \renewcommand*{\@evenhead}{}%
  \renewcommand*{\@oddhead}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \hb@xt@ \@tempdima{{\headfont\hfil\strut\rightmark\hfil}}
    \if@hsl \vskip 1.5\p@ \hrule \fi}}\hss}%
  \renewcommand*{\@evenfoot}{}%
  \renewcommand*{\@oddfoot}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \if@fsl \hrule \vskip 3\p@ \fi
    \hb@xt@ \@tempdima{{\pnumfont\hfil\thepage\hfil}}}\hss}}%
  \renewcommand*{\sectionmark}[1]{}
  \renewcommand*{\chaptermark}[1]{\markright {\ifnum \c@secnumdepth >\m@ne
      \chaptermarkformat\fi
    ##1}}}
 \newcommand*{\ps@myheadings}{\let\@mkboth\@gobbletwo
  \renewcommand*{\@evenhead}{}%
  \renewcommand*{\@oddhead}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
    \hb@xt@ \@tempdima{{\headfont\hfil\strut\rightmark\hfil}}
    \if@hsl \vskip 1.5\p@ \hrule \fi}}\hss}%
  \renewcommand*{\@evenfoot}{}%
  \renewcommand*{\@oddfoot}{%
   \set@tempdima@hw\hb@xt@ \@tempdima{\vbox{%
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \@tempdima{{\pnumfont\hfil\thepage\hfil}}}}\hss}%
  \renewcommand*{\chaptermark}[1]{}%
  \renewcommand*{\sectionmark}[1]{}}
\fi
\newcommand*{\titlepagestyle}{plain}
\newcommand*{\partpagestyle}{plain}
\newcommand*{\chapterpagestyle}{plain}
\newcommand*{\indexpagestyle}{plain}
\newcommand*{\@extratitle}{}
\newcommand{\extratitle}[1]{\gdef\@extratitle{#1}}
\newcommand*{\@titlehead}{}
\newcommand{\titlehead}[1]{\gdef\@titlehead{#1}}
\newcommand*{\@subject}{}
\newcommand*{\subject}[1]{\gdef\@subject{#1}}
\newcommand*{\@publishers}{}
\newcommand{\publishers}[1]{\gdef\@publishers{#1}}
\newcommand*{\@uppertitleback}{}
\newcommand{\uppertitleback}[1]{\gdef\@uppertitleback{#1}}
\newcommand*{\@lowertitleback}{}
\newcommand{\lowertitleback}[1]{\gdef\@lowertitleback{#1}}
\newcommand*{\@dedication}{}
\newcommand{\dedication}[1]{\gdef\@dedication{#1}}
\newcommand*{\next@tpage}{\clearpage\thispagestyle{empty}}
\if@titlepage
    \newcommand*\maketitle[1][1]{\begin{titlepage}%
    \setcounter{page}{#1}
    \let\footnotesize\small
    \let\footnoterule\relax
    \let\footnote\thanks
    \renewcommand*\thefootnote{\@fnsymbol\c@footnote}%
    \let\@oldmakefnmark\@makefnmark
    \renewcommand*{\@makefnmark}{\rlap\@oldmakefnmark}
    \ifx\@extratitle\@empty \else
        \noindent\@extratitle\next@tpage\cleardoublepage
        \thispagestyle{empty}
    \fi
    \ifx\@titlehead\@empty \else
        \noindent\begin{minipage}[t]{\textwidth}
        \@titlehead
        \end{minipage}\par
    \fi
    \null\vfill
    \begin{center}
    \ifx\@subject\@empty \else
        {\Large \@subject \par}
        \vskip 3em
    \fi
    {\titlefont\huge \@title\par}
    \vskip 3em
    {\Large \lineskip 0.75em
    \begin{tabular}[t]{c}
        \@author
    \end{tabular}\par}
    \vskip 1.5em
    {\Large \@date \par}
    \vskip \z@ \@plus3fill
    {\Large \@publishers \par}
    \vskip 3em
    \end{center}\par
    \@thanks
    \vfill\null
    \if@twoside\next@tpage
        \noindent\begin{minipage}[t]{\textwidth}
        \@uppertitleback
        \end{minipage}\par
        \vfill
        \noindent\begin{minipage}[b]{\textwidth}
        \@lowertitleback
        \end{minipage}
    \fi
    \ifx\@dedication\@empty \else
        \next@tpage\null\vfill
        {\centering \Large \@dedication \par}
        \vskip \z@ \@plus3fill
        \if@twoside \next@tpage\cleardoublepage \fi
    \fi
    \end{titlepage}
    \setcounter{footnote}{0}%
    \global\let\thanks\relax
    \global\let\maketitle\relax
    \global\let\@thanks\@empty
    \global\let\@author\@empty
    \global\let\@date\@empty
    \global\let\@title\@empty
    \global\let\@extratitle\@empty
    \global\let\@titlehead\@empty
    \global\let\@subject\@empty
    \global\let\@publishers\@empty
    \global\let\@uppertitleback\@empty
    \global\let\@lowertitleback\@empty
    \global\let\@dedication\@empty
    \global\let\author\relax
    \global\let\title\relax
    \global\let\extratitle\relax
    \global\let\titlehead\relax
    \global\let\subject\relax
    \global\let\publishers\relax
    \global\let\uppertitleback\relax
    \global\let\lowertitleback\relax
    \global\let\dedication\relax
    \global\let\date\relax
    \global\let\and\relax}
\else
     \newcommand*\maketitle[1][1]{\par
       \@tempcnta=#1\relax\ifnum\@tempcnta=1\else
         \ClassWarning{scrreprt}
                      {Optional argument of \string\maketitle\space ignored
                       at\MessageBreak
                       notitlepage-mode}\fi
       \begingroup
         \renewcommand*\thefootnote{\@fnsymbol\c@footnote}%
         \let\@oldmakefnmark\@makefnmark
         \renewcommand*{\@makefnmark}{\rlap\@oldmakefnmark}
         \if@twocolumn
           \ifnum \col@number=\@ne
             \@maketitle
           \else
             \twocolumn[\@maketitle]%
           \fi
         \else
           \newpage
           \global\@topnum\z@
           \@maketitle
         \fi
         \thispagestyle{\titlepagestyle}\@thanks
       \endgroup
       \setcounter{footnote}{0}%
       \let\thanks\relax
       \let\maketitle\relax
       \let\@maketitle\relax
       \global\let\@thanks\@empty
       \global\let\@author\@empty
       \global\let\@date\@empty
       \global\let\@title\@empty
       \global\let\@extratitle\@empty
       \global\let\@titlehead\@empty
       \global\let\@subject\@empty
       \global\let\@publishers\@empty
       \global\let\@uppertitleback\@empty
       \global\let\@lowertitleback\@empty
       \global\let\@dedication\@empty
       \global\let\author\relax
       \global\let\title\relax
       \global\let\extratitle\relax
       \global\let\titlehead\relax
       \global\let\subject\relax
       \global\let\publishers\relax
       \global\let\uppertitleback\relax
       \global\let\lowertitleback\relax
       \global\let\dedication\relax
       \global\let\date\relax
       \global\let\and\relax}
\fi
\newcommand*{\@maketitle}{%
    \clearpage
    \let\footnote\thanks
    \ifx\@extratitle\@empty \else
        \noindent\@extratitle \next@tpage \if@twoside \null\next@tpage \fi
    \fi
    \ifx\@titlehead\@empty \else
        \noindent\begin{minipage}[t]{\textwidth}
        \@titlehead
        \end{minipage}\par
    \fi
    \null
    \vskip 2em%
    \begin{center}%
    \ifx\@subject\@empty \else
        {\Large \@subject \par}
        \vskip 1.5em
    \fi
    {\titlefont\huge \@title \par}%
    \vskip 1.5em%
    {\Large
      \lineskip .5em%
      \begin{tabular}[t]{c}%
        \@author
      \end{tabular}\par}%
    \vskip 1em%
    {\Large \@date \par}%
    \vskip \z@ \@plus 1em
    {\Large \@publishers \par}
    \ifx\@dedication\@empty \else
        \vskip 2em
        {\Large \@dedication \par}
    \fi
  \end{center}%
  \par
  \vskip 2em}
\renewcommand*{\numberline}[1]{%
  \hb@xt@\@tempdima{#1\autodot\hfil}}
\newcommand*\chaptermark[1]{}
\setcounter{secnumdepth}{2}
\newcounter{part}
\newcounter{chapter}
\newcounter{section}[chapter]
\newcounter{subsection}[section]
\newcounter{subsubsection}[subsection]
\newcounter{paragraph}[subsubsection]
\newcounter{subparagraph}[paragraph]
\renewcommand*\thepart{\@Roman\c@part}
\renewcommand*\thechapter{\@arabic\c@chapter}
\renewcommand*\thesection{\thechapter.\@arabic\c@section}
\renewcommand*\thesubsection{\thesection.\@arabic\c@subsection}
\renewcommand*\thesubsubsection{\thesubsection.\@arabic\c@subsubsection}
\renewcommand*\theparagraph{\thesubsubsection.\@arabic\c@paragraph}
\renewcommand*\thesubparagraph{\theparagraph.\@arabic\c@subparagraph}
\newcommand*\@chapapp{\chaptername}
\newcommand\part{\if@openright\cleardoublepage\else\clearpage\fi
                 \thispagestyle{\partpagestyle}%
                 \if@twocolumn
                     \onecolumn
                     \@tempswatrue
                   \else
                     \@tempswafalse
                 \fi
                 \null\vfil%
                 \vbox to\z@{\vss\use@preamble{part@o}\strut\par}%
                 \vskip-\baselineskip\nobreak%
                 \secdef\@part\@spart}
\newcommand*{\@part}{}
\def\@part[#1]#2{%
    \ifnum \c@secnumdepth >-2\relax
      \refstepcounter{part}%
      \@maybeautodot\thepart%
      \addcontentsline{toc}{part}{\protect\numberline{\thepart}#1}%
    \else
      \addcontentsline{toc}{part}{#1}%
    \fi
    \@mkboth{}{}%
    {\@parskipfalse\@parskip@indent\centering
     \interlinepenalty \@M
     \normalfont\sectfont
     \ifnum \c@secnumdepth >-2\relax
       \size@partnumber\partformat
       \par
       \vskip 20\p@
     \fi
     \size@part #2\strut\par}%
    \@endpart}
\newcommand*{\@spart}[1]{%
    {\@parskipfalse\@parskip@indent\centering
     \interlinepenalty \@M
     \normalfont
     \sectfont\size@part #1\strut\@mkboth{}{}\par}%
    \@endpart}
\newcommand*{\@endpart}{\vbox to\z@{\use@preamble{part@u}\vss}%
  \vfil\newpage
  \if@twoside\if@openright
      \null%
      \thispagestyle{empty}%
      \newpage
  \fi\fi
  \if@tempswa
    \twocolumn
  \fi}
\newcommand\addpart{\if@openright\cleardoublepage\else\clearpage\fi
  \thispagestyle{\partpagestyle}%
  \if@twocolumn
    \onecolumn
    \@tempswatrue
  \else
    \@tempswafalse
  \fi
  \null\vfil
  \secdef\@addpart\@saddpart}
\newcommand*{\@addpart}{}
\def\@addpart[#1]#2{\addcontentsline{toc}{part}{#1}\@spart{#2}}
\newcommand*{\@saddpart}{}
\def\@saddpart#1{\@spart{#1}}
\newif\if@at@twocolumn
\newcommand*\scr@topnewpage{}
\let\scr@topnewpage\@topnewpage
\long\def\@topnewpage[#1]{%
  \@at@twocolumntrue\scr@topnewpage[{#1}]\@at@twocolumnfalse}
\newcommand\chapter{\if@openright\cleardoublepage\else\clearpage\fi
                    \thispagestyle{\chapterpagestyle}%
                    \global\@topnum\z@
                    \@afterindentfalse
                    \secdef\@chapter\@schapter}
\newcommand*{\@chapter}{}
\def\@chapter[#1]#2{\ifnum \c@secnumdepth >\m@ne
                         \refstepcounter{chapter}%
                         \@maybeautodot\thechapter%
                         \typeout{\@chapapp\space\thechapter.}%
                         \addcontentsline{toc}{chapter}%
                                   {\protect\numberline{\thechapter}#1}%
                    \else
                      \addcontentsline{toc}{chapter}{#1}
                    \fi
                    \chaptermark{#1}%
                    \addtocontents{lof}{\protect\addvspace{10\p@}}%
                    \addtocontents{lot}{\protect\addvspace{10\p@}}%
                    \@ifundefined{float@addtolists}{}{%
                      \float@addtolists{\protect\addvspace{10\p@}}}
                    \if@twocolumn
                      \if@at@twocolumn
                        \@makechapterhead{#2}%
                      \else
                        \@topnewpage[\@makechapterhead{#2}]%
                      \fi
                    \else
                      \@makechapterhead{#2}%
                      \@afterheading
                    \fi}
\newcommand*{\chapterheadstartvskip}{}
\newcommand*{\chapterheadendvskip}{}
\newcommand*{\@@makechapterhead}[1]{\chapterheadstartvskip%
  {\normalfont\sectfont\size@chapter\parindent \z@%
      \if@chapterprefix\let\@tempa\raggedsection\else\let\@tempa\@hangfrom\fi%
      \@tempa{\ifnum \c@secnumdepth >\m@ne%
        \chapterformat\fi}%
      \if@chapterprefix\par\nobreak\vskip.5\baselineskip\fi%
      {\raggedsection \interlinepenalty \@M #1\par}}%
    \nobreak\chapterheadendvskip%
    }
\newcommand*{\use@chapter@o@preamble}{%
  {\settoheight{\@tempdima}{\vbox{\chapterheadstartvskip}}%
    \settodepth{\@tempdimb}{\vbox{\chapterheadstartvskip}}%
    \addtolength{\@tempdima}{\@tempdimb}%
    \setbox0\vbox{\use@preamble{chapter@o}%
      \vskip-\parskip}%
    \setlength{\@tempdimb}{\ht0}%
    \addtolength{\@tempdimb}{\dp0}%
    \vbox to \z@{%
      \vbox to \@tempdima{\vfill\box0}%
      \vskip-\@tempdima%
      }%
    \ifdim \@tempdimb>\@tempdima%
      \addtolength{\@tempdimb}{-\@tempdima}%
      \ClassWarning{%
        scrreprt%
        }{%
        preamble before chapter is \the\@tempdimb\space too
        high.\MessageBreak
        To avoid the overfull \string\vbox\space you may
        redefine\MessageBreak
        \string\chapterheadstartvskip\space at the preamble of
        your\MessageBreak
        document. You may also change
        \string\setchapterpreamble\MessageBreak
        before the command, which generates the message
        about\MessageBreak
        an overfull \string\vbox
        }%
    \fi%
    }
}
\newcommand*{\@makechapterhead}[1]{%
  \use@chapter@o@preamble%
  \@@makechapterhead{#1}%
  \use@preamble{chapter@u}\nobreak%
  }
\newcommand*{\@schapter}[1]{\if@twocolumn
                    \if@at@twocolumn
                      \@makeschapterhead{#1}%
                    \else
                      \@topnewpage[\@makeschapterhead{#1}]%
                    \fi
                 \else
                   \@makeschapterhead{#1}%
                   \@afterheading
                 \fi}
\newcommand*{\@@makeschapterhead}[1]{\chapterheadstartvskip%
  {\parindent \z@ \raggedsection
    \normalfont
    \sectfont\size@chapter #1\par
    \nobreak\chapterheadendvskip
  }}
\newcommand*{\@makeschapterhead}[1]{%
  \use@chapter@o@preamble%
  \@@makeschapterhead{#1}%
  \use@preamble{chapter@u}\nobreak%
  }
\newcommand*{\use@preamble}[1]{%
  \@ifundefined{#1@preamble}{}{%
    \@nameuse{#1@preamble}%
    \global\expandafter\let\csname#1@preamble\endcsname=\relax}%
}
\newcommand*{\set@preamble}[1]{%
  \@ifnextchar [
  {\set@@preamble{#1}}{\set@@@preamble{#1}[][\hsize]}}
\newcommand*{\set@@preamble}{}
\def\set@@preamble#1[#2]{%
  \@ifnextchar [
  {\set@@@preamble{#1}[{#2}]}{\set@@@preamble{#1}[{#2}][\hsize]}}
\newcommand{\set@@@preamble}{}
\long\def\set@@@preamble#1[#2][#3]#4{%
  {%
    \def\prmbl@pos{#2}\let\prmbl@hpos\relax\let\prmbl@vpos\relax%
    \expandafter \@tfor \expandafter \@tempa%
      \expandafter :\expandafter =\prmbl@pos%
    \do{%
      \if \@tempa l%
        \set@preamble@hpos{0}{#2}%
      \fi%
      \if \@tempa c%
        \set@preamble@hpos{1}{#2}%
      \fi%
      \if \@tempa r%
        \set@preamble@hpos{2}{#2}%
      \fi%
      \if \@tempa u%
        \set@preamble@vpos{0}{#2}%
      \fi%
      \if \@tempa o%
        \set@preamble@vpos{1}{#2}%
      \fi%
      }
    \@ifundefined{prmbl@hpos}{\def\prmbl@hpos{0}}{}%
    \@ifundefined{prmbl@vpos}{\def\prmbl@vpos{0}}{}%
    \ifcase\prmbl@hpos
      \ifcase\prmbl@vpos
        \set@@@@preamble{#1@u}{t}{#3}{}{\hfil}{#4}
      \else
        \set@@@@preamble{#1@o}{b}{#3}{}{\hfil}{#4}
      \fi
    \or
      \ifcase\prmbl@vpos
        \set@@@@preamble{#1@u}{t}{#3}{\hfil}{\hfil}{#4}
      \else
        \set@@@@preamble{#1@o}{b}{#3}{\hfil}{\hfil}{#4}
      \fi
    \else
      \ifcase\prmbl@vpos
        \set@@@@preamble{#1@u}{t}{#3}{\hfil}{}{#4}
      \else
        \set@@@@preamble{#1@o}{b}{#3}{\hfil}{}{#4}
      \fi
    \fi
   }
}
\newcommand{\set@@@@preamble}[6]{%
  \expandafter\gdef\csname #1@preamble\endcsname{%
    \hbox to\hsize{#4\parbox[{#2}]{#3}{#6\par}#5\par}}}
\newcommand*{\set@preamble@hpos}[2]{%
  \@ifundefined{prbml@hpos}{%
    \def\prmbl@hpos{#1}%
    }{%
    \ifnum \prmbl@hpos=0\relax\else%
      \ClassError{%
        scrreprt%
        }{%
        inconsistent hpos options%
        }{%
        You said `#2' as position option. But you have
        to\MessageBreak
        use only one from `l', `r' or `c' not two of
        these.\MessageBreak
        If you'll continue only first hpos option will be
        used}%
    \fi}}
\newcommand*{\set@preamble@vpos}[2]{%
  \@ifundefined{prbml@vpos}{%
    \def\prmbl@vpos{#1}%
    }{%
    \ifnum \prmbl@vpos=0\relax\else%
      \ClassError{%
        scrreprt%
        }{%
        inconsistent vpos options%
        }{%
        You said `#2' as position option. But you have
        to\MessageBreak
        use only one from `o' or `u' not two of
        these.\MessageBreak
        If you'll continue only first vpos option will be
        used}%
    \fi}}
\newcommand{\dictum}[2][]{\par%
  \raggeddictum\parbox{\dictumwidth}{%
    {\@dictumfont{\raggeddictumtext #2\strut\par}%
      \def\@tempa{#1}\ifx\@tempa\@empty\else%
      {\raggeddictumtext\vskip-1ex\hrulefill\par}%
      \raggeddictumauthor\@dictumauthorfont\dictumauthorformat{#1}%
      \strut\par%
      \fi%
      }%
    }\par%
  }
\newcommand*{\dictumwidth}{.3333\textwidth}
\newcommand*{\raggeddictum}{\raggedleft}
\newcommand*{\dictumauthorformat}[1]{(#1)}
\newcommand*{\raggeddictumtext}{\raggedright}
\newcommand*{\raggeddictumauthor}{\raggedleft}
\newcommand*{\@dictumfont}{\normalfont\normalcolor\sffamily\small}
\newcommand*{\@dictumauthorfont}{\itshape}
\newcommand*{\scr@fnt@dictumtext}{\@dictumfont}
\newcommand*{\scr@fnt@dictumauthor}{\@dictumauthorfont}
\newcommand*{\scr@fnt@instead@dictum}{dictumtext}
\newcommand*{\setchapterpreamble}{\set@preamble{chapter}}
\newcommand*{\chapter@u@preamble}{}
\let\chapter@u@preamble\relax
\newcommand*{\chapter@o@preamble}{}
\let\chapter@o@preamble\relax
\newcommand*{\setpartpreamble}{\set@preamble{part}}
\newcommand*{\part@preamble@u}{}
\let\part@preamble@u\relax
\newcommand*{\part@preamble@o}{}
\let\part@preamble@o\relax
\newcommand*\addsec{\secdef\@addsec\@saddsec}
\newcommand*{\@addsec}{}
\def\@addsec[#1]#2{\section*{#2}\addcontentsline{toc}{section}{#1}
 \if@twoside\ifx\@mkboth\markboth\markright{#1}\fi\fi}
\newcommand*{\@saddsec}[1]{\section*{#1}\@mkboth{}{}}
\newcommand\addchap{\if@openright\cleardoublepage\else\clearpage\fi
  \thispagestyle{\chapterpagestyle}%
  \global\@topnum\z@
  \@afterindentfalse
  \secdef\@addchap\@saddchap%
}
\newcommand*{\@addchap}{}
\def\@addchap[#1]#2{\@schapter{#2}%
  \addcontentsline{toc}{chapter}{#1}%
  \if@twoside\@mkboth{#1}{}\else\@mkboth{}{#1}\fi}
\newcommand*{\@saddchap}[1]{\@schapter{#1}\@mkboth{}{}}
\newcommand\section{\@startsection{section}{1}{\z@}%
  {-3.5ex \@plus -1ex \@minus -.2ex}%
  {2.3ex \@plus.2ex}%
  {\raggedsection\normalfont\sectfont\nobreak\size@section\nobreak}}
\newcommand\subsection{\@startsection{subsection}{2}{\z@}%
  {-3.25ex\@plus -1ex \@minus -.2ex}%
  {1.5ex \@plus .2ex}%
  {\raggedsection\normalfont\sectfont\nobreak\size@subsection\nobreak}}
\newcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}%
  {-3.25ex\@plus -1ex \@minus -.2ex}%
  {1.5ex \@plus .2ex}%
  {\raggedsection\normalfont\sectfont\nobreak\size@subsubsection\nobreak}}
\newcommand\paragraph{\@startsection{paragraph}{4}{\z@}%
  {3.25ex \@plus1ex \@minus.2ex}%
  {-1em}%
  {\raggedsection\normalfont\sectfont\nobreak\size@paragraph\nobreak}}
\newcommand\subparagraph{\@startsection{subparagraph}{5}{\parindent}%
  {3.25ex \@plus1ex \@minus .2ex}%
  {-1em}%
  {\raggedsection\normalfont\sectfont\nobreak\size@subparagraph\nobreak}}
\newcommand\minisec[1]{\@afterindentfalse \vskip 1.5ex
  {\parindent \z@
    \raggedsection\normalfont\sectfont\nobreak#1\par\nobreak}%
  \@afterheading}
\let\raggedsection\raggedright
\setlength\arraycolsep{5\p@}
\setlength\tabcolsep{6\p@}
\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\setlength{\skip\@mpfootins}{\skip\footins}
\setlength\fboxsep{3\p@}
\setlength\fboxrule{.4\p@}
\@addtoreset{equation}{chapter}
\renewcommand*\theequation{%
  \thechapter.%
  \@arabic\c@equation}
\if@twocolumn
  \setlength{\leftmargini}{2em}
\else
  \setlength{\leftmargini}{2.5em}
\fi
\setlength{\leftmarginii}{2.2em}
\setlength{\leftmarginiii}{1.87em}
\setlength{\leftmarginiv}{1.7em}
\if@twocolumn
  \setlength{\leftmarginv}{.5em}
  \setlength{\leftmarginvi}{.5em}
\else
  \setlength{\leftmarginv}{1em}
  \setlength{\leftmarginvi}{1em}
\fi
\setlength{\leftmargin}{\leftmargini}
\setlength{\labelsep}{.5em}
\setlength{\labelwidth}{\leftmargini}
\addtolength{\labelwidth}{-\labelsep}
\@beginparpenalty=-\@lowpenalty
\@endparpenalty  =-\@lowpenalty
\@itempenalty    =-\@lowpenalty
\renewcommand*\theenumi{\@arabic\c@enumi}
\renewcommand*\theenumii{\@alph\c@enumii}
\renewcommand*\theenumiii{\@roman\c@enumiii}
\renewcommand*\theenumiv{\@Alph\c@enumiv}
\newcommand*\labelenumi{\theenumi.}
\newcommand*\labelenumii{\theenumii)}
\newcommand*\labelenumiii{\theenumiii.}
\newcommand*\labelenumiv{\theenumiv.}
\renewcommand*\p@enumii{\theenumi}
\renewcommand*\p@enumiii{\p@enumii(\theenumii)}
\renewcommand*\p@enumiv{\p@enumiii\theenumiii}
\newcommand*\labelitemi{\textbullet}
\newcommand*\labelitemii{\normalfont\bfseries\textendash}
\newcommand*\labelitemiii{\textasteriskcentered}
\newcommand*\labelitemiv{\textperiodcentered}
\newenvironment{description}
               {\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}}
               {\endlist}
\newcommand*{\descriptionlabel}[1]{\hspace\labelsep
                                \descfont #1}
\newenvironment{labeling}[2][]
  {\def\sc@septext{#1}
   \list{}{\settowidth{\labelwidth}{#2#1}
     \leftmargin\labelwidth \advance\leftmargin by \labelsep
     \let\makelabel\labelinglabel}}
  {\endlist}
\newcommand\labelinglabel[1]{#1\hfil\sc@septext}
\newenvironment{verse}
               {\let\\=\@centercr
                \list{}{\itemsep      \z@
                        \itemindent   -1.5em%
                        \listparindent\itemindent
                        \rightmargin  \leftmargin
                        \advance\leftmargin 1.5em}%
                \item\relax}
               {\endlist}
\newenvironment{quotation}
               {\list{}{\listparindent 1em%
                        \itemindent    \listparindent
                        \rightmargin   \leftmargin
                        \parsep        \z@ \@plus\p@}%
                \item\relax}
               {\endlist}
\newenvironment{quote}
               {\list{}{\rightmargin\leftmargin}%
                \item\relax}
               {\endlist}
\newenvironment{addmargin}{%
  \@tempswafalse\@addmargin
  }{\endlist}
\newenvironment{addmargin*}{%
  \@tempswafalse
  \if@twoside\ifthispageodd{}{\@tempswatrue}\fi
  \@addmargin
  }{\endlist}
\newcommand*{\@addmargin}[2][\@tempa]{%
  \@tempcnta=\@listdepth
  \list{}{%
    \if@tempswa
      \def\@tempa{\leftmargin}%
      \setlength{\leftmargin}{#2}%
      \setlength{\rightmargin}{#1}%
    \else
      \def\@tempa{\rightmargin}%
      \setlength{\rightmargin}{#2}%
      \setlength{\leftmargin}{#1}%
    \fi
    \setlength{\listparindent}{\parindent}%
    \setlength{\itemsep}{\parskip}%
    \setlength{\itemindent}{\z@}%
    \setlength{\topsep}{\z@}%
    \setlength{\parsep}{\parskip}%
    \setlength{\partopsep}{\parskip}%
    \let\makelabel\@gobble
    \setlength{\labelsep}{\z@}
    \setlength{\labelwidth}{\z@}
    \@listdepth=\@tempcnta%
    }%
  \item[]%
  }
\if@titlepage
 \newenvironment{abstract}{%
     \titlepage
     \null\vfil
     \@beginparpenalty\@lowpenalty
     \if@abstrt
     \begin{center}
       \normalfont\sectfont\nobreak\abstractname
       \@endparpenalty\@M
     \end{center}\fi}
    {\par\vfil\null\endtitlepage}
\else
 \newenvironment{abstract}{%
     \if@twocolumn\if@abstrt
       \addsec*{\abstractname}\fi%
     \else
       \if@abstrt
       \small
       \begin{center}%
         {\normalfont\sectfont\nobreak\abstractname
           \vspace{-.5em}\vspace{\z@}}%
       \end{center}\fi%
       \quotation
     \fi}
     {\if@twocolumn\else\endquotation\fi}
\fi
\newenvironment{titlepage}
    {%
      \cleardoublepage
      \if@twocolumn
        \@restonecoltrue\onecolumn
      \else
        \@restonecolfalse\newpage
      \fi
      \thispagestyle{empty}%
      \if@compatibility
        \setcounter{page}{0}
      \fi}%
    {\if@restonecol\twocolumn \else \newpage \fi
    }
\newcommand\marginline[1]{\marginpar[\raggedleft{#1}]{\raggedright #1}}
\newcommand*\appendix{\par%
  \setcounter{chapter}{0}%
  \setcounter{section}{0}%
  \renewcommand*\@chapapp{\appendixname}%
  \renewcommand*\thechapter{\@Alph\c@chapter}%
  \@ifundefined{appendixmore}{}{\appendixmore}}
\newcounter{figure}[chapter]
\renewcommand*\thefigure{%
                        \thechapter.%
                        \@arabic\c@figure}
\newcommand*{\fps@figure}{tbp}
\newcommand*{\ftype@figure}{1}
\newcommand*{\ext@figure}{lof}
\newcommand*{\fnum@figure}{\figureformat}
\newenvironment{figure}
               {\let\caption\captionbelow\@float{figure}}
               {\end@float}
\newenvironment{figure*}
               {\let\caption\captionbelow\@dblfloat{figure}}
               {\end@dblfloat}
\newcounter{table}[chapter]
\renewcommand*\thetable{%
                       \thechapter.%
                       \@arabic\c@table}
\newcommand*{\fps@table}{tbp}
\newcommand*{\ftype@table}{2}
\newcommand*{\ext@table}{lot}
\newcommand*{\fnum@table}{\tableformat}
\newenvironment{table}
               {\if@tablecaptionabove\let\caption\captionabove%
                 \else\let\caption\captionbelow\fi\@float{table}}
               {\end@float}
\newenvironment{table*}
               {\if@tablecaptionabove\let\caption\captionabove%
                 \else\let\caption\captionbelow\fi\@dblfloat{table}}
               {\end@dblfloat}
\newcommand*{\new@tpo@label}[2]{%
  \expandafter\gdef\csname scr@tpo@#1\endcsname{#2}%
  }
\newcommand*{\ref@tpo@label}[1]{%
  \expandafter\ifx\csname scr@tpo@#1\endcsname\relax\value{page}%
  \else\csname scr@tpo@#1\endcsname\fi}
\newcommand*{\scr@tpo}{0}
\newcommand*{\is@thispageodd}{%
  \begingroup%
    \@tempcnta=\scr@tpo%
    \advance\@tempcnta by\@ne%
    \xdef\scr@tpo{\the\@tempcnta}%
    \@bsphack
    \protected@write\@auxout{}{%
      \string\new@tpo@label{\scr@tpo}{\noexpand\the\value{page}}}%
    \@esphack
    \count@\ref@tpo@label{\scr@tpo}\relax
    \ifodd\count@
      \aftergroup\thispagewasoddtrue
    \else
      \aftergroup\thispagewasoddfalse
    \fi
  \endgroup
}
\AtEndDocument{%
  \if@filesw\immediate\write\@auxout{%
    \string\gdef\string\scr@tpo@lst{\scr@tpo}}\fi%
  \ifx\scr@tpo@lst\scr@tpo\else\ifnum \scr@tpo>0
    \ClassWarningNoLine{%
      scrreprt%
      }{Rerun to get odd- an even-page-stuff right}%
  \fi\fi
  }
\newif\ifthispagewasodd
\newcommand{\ifthispageodd}{%
  \is@thispageodd
  \ifthispagewasodd
    \expandafter\@firstoftwo
  \else
    \expandafter\@secondoftwo
  \fi
}
\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{10\p@}
\setlength\belowcaptionskip{0\p@}
\newif\if@captionabove\@captionabovefalse
\newcommand{\scr@caption}{}
\AtBeginDocument{\let\scr@caption\caption}
\newcommand{\captionabove}{\@captionabovetrue\scr@caption}
\newcommand{\captionbelow}{\@captionabovefalse\scr@caption}
\newcommand*{\scap@pos}{r}
\newcommand*{\scap@valign}{b}
\newcommand*{\scap@caption}{}
\newcommand*{\scap@width}{\linewidth}
\newcommand*{\scap@move}{\z@}
\newcommand*{\scap@currentlabel}{}
\newif\ifscap@alt
\newcommand*{\@captionbeside}{}
\long\def\@captionbeside[#1]#2{%
  \@ifnextchar [%]
    {\@@captionbeside[{#1}]{#2}}
    {\c@pti@nb@sid@[{#1}]{#2}}%
  }
\newcommand*{\@@captionbeside}{}
\long\def\@@captionbeside[#1]#2[#3]{%
  \def\scap@pos{#3}%
  \@ifnextchar [%]
    {\@@@captionbeside[{#1}]{#2}}
    {\c@pti@nb@sid@[{#1}]{#2}}%
  }
\newcommand*{\@@@captionbeside}{}
\long\def\@@@captionbeside[#1]#2[#3]{%
  \def\scap@width{#3}%
  \@ifnextchar [%]
    {\@@@@captionbeside[{#1}]{#2}}
    {\setlength{\@tempdima}{#3}%
      \addtolength{\@tempdima}{-\linewidth}%
      \setlength{\@tempdima}{-.5\@tempdima}%
      \edef\scap@move{\the\@tempdima}%
      \c@pti@nb@sid@[{#1}]{#2}}%
  }
\newcommand*{\@@@@captionbeside}{}
\long\def\@@@@captionbeside[#1]#2[#3]{%
  \def\scap@move{#3}%
  \@ifstar
    {\scap@alttrue\c@pti@nb@sid@[{#1}]{#2}}
    {\scap@altfalse\c@pti@nb@sid@[{#1}]{#2}}%
  }
\newbox\c@pti@nb@sid@b@x
\newcommand{\c@pti@nb@sid@}{}
\long\def\c@pti@nb@sid@[#1]#2{%
  \def\scap@caption{\captionbelow[{#1}]{#2}}%
  \begin{lrbox}{\c@pti@nb@sid@b@x}%
  }
\newenvironment*{captionbeside}{%
  \@dblarg\@captionbeside%
  }{%
  \end{lrbox}%
  \settowidth{\@tempdima}{\usebox\c@pti@nb@sid@b@x}%
  \setlength{\@tempdimb}{\scap@width}%
  \addtolength{\@tempdimb}{-\@tempdima}%
  \addtolength{\@tempdimb}{-\columnsep}%
  \ifdim \@tempdimb<2em
    \ClassWarning{%
      scrreprt%
      }{%
      caption beside will be very, very small.\MessageBreak
      In most cases this will result in overfull and
      underfull\MessageBreak
      \string\hbox\space messages%
      }%
  \fi%
  \onelinecaptionsfalse
  \setcapwidth[r]{\@tempdimb}%
  \let\@tempa\scap@pos
  \setlength{\@tempdimc}{\scap@move}%
  \if i\scap@pos\def\@tempa{l}\fi
  \if o\scap@pos\def\@tempa{r}\fi
  \if@twoside
    \ifthispageodd{}{%
      \if o\scap@pos\def\@tempa{l}\fi
      \if i\scap@pos\def\@tempa{r}\fi
      \ifscap@alt
        \addtolength{\@tempdimc}{\scap@width}%
        \addtolength{\@tempdimc}{-\linewidth}%
        \setlength{\@tempdimc}{-\@tempdimc}%
      \fi
    }%
  \fi
  \hspace*{\@tempdimc}%
  \if l\@tempa%
    \rlap{%
      \parbox[\scap@valign]{\@tempdimb}{\scap@caption
        \global\let\scap@currentlabel\@currentlabel}%
      \nobreak\hskip\columnsep%
      \parbox[\scap@valign]{\@tempdima}{\usebox\c@pti@nb@sid@b@x}%
      }%
  \else
    \rlap{%
      \parbox[\scap@valign]{\@tempdima}{\usebox\c@pti@nb@sid@b@x}%
      \nobreak\hskip\columnsep%
      \parbox[\scap@valign]{\@tempdimb}{\scap@caption
        \global\let\scap@currentlabel\@currentlabel}%
      }%
  \fi
  \par
  \aftergroup\let\aftergroup\@currentlabel\aftergroup\scap@currentlabel
  }
\newif\ifdofullc@p
\dofullc@ptrue
\newif\if@capbreak\@capbreakfalse
\newcommand*{\setcapindent}{\@ifstar{\@@setcapindent}{\@setcapindent}}
\newcommand*{\@setcapindent}[1]{
  \setlength{\@tempdima}{#1}%
  \ifdim\@tempdima>\hsize\addtolength{\@tempdima}{-\hsize}\@capbreaktrue
  \else\@capbreakfalse\fi
  \edef\cap@indent{\the\@tempdima}
  \dofullc@pfalse
}
\newcommand*{\@@setcapindent}[1]{
  \setlength{\@tempdima}{#1}%
  \edef\cap@indent{\the\@tempdima}
  \dofullc@pfalse\@capbreaktrue
}
\AfterPackage{caption2}{%
  \renewcommand*{\setcapindent}{%
    \ClassWarning{%
      scrreprt%
      }{%
      You're using package caption2.\MessageBreak
      Because of this you should also use \string\captionstyle\space
      and\MessageBreak
      set length \string\captionindent\space instead if
      using\MessageBreak
      \string\setcapindent
      }
    \captionstyle{indent}%
    \@ifstar{\setlength{\captionindent}}{\setlength{\captionindent}}}}
\newcommand*{\setcaphanging}{\dofullc@ptrue\@capbreakfalse}
\AfterPackage{caption2}{%
  \renewcommand*{\setcaphanging}{%
    \ClassWarning{%
      scrreprt%
      }{%
      You're using package caption2.\MessageBreak
      Because of this you should also use \string\captionstyle\MessageBreak
      instead of \string\setcaphang
      }
    \captionstyle{hang}}}
\newcommand{\@makecaption}[2]{%
  \if@captionabove\vskip\belowcaptionskip\else\vskip\abovecaptionskip\fi
  \@@makecaption\@firstofone{#1}{#2}
  \if@captionabove\vskip\abovecaptionskip\else\vskip\belowcaptionskip\fi}
\newcommand*{\cap@width}{\hsize}
\newcommand*{\cap@left}{\hfill}
\newcommand*{\cap@right}{\hfill}
\newcommand*{\setcapwidth}[2][]{%
  \begingroup%
    \setlength{\@tempdima}{#2}%
    \edef\@tempa{\endgroup\def\noexpand\cap@width{\the\@tempdima}}%
  \@tempa%
  \begingroup
    \expandafter\def\expandafter\@tempa\expandafter{#1}%
    \def\@tempb{c}\ifx\@tempa\@tempb
      \endgroup%
      \def\cap@margin{\let\cap@left=\hfill\let\cap@right=\hfill}%
    \else
      \def\@tempb{l}\ifx\@tempa\@tempb
        \endgroup%
        \def\cap@margin{\let\cap@left=\relax\let\cap@right\hfill}%
      \else
        \def\@tempb{r}\ifx\@tempa\@tempb
          \endgroup%
          \def\cap@margin{\let\cap@left=\hfill\let\cap@right=\relax}%
        \else
          \def\@tempb{i}\ifx\@tempa\@tempb
            \endgroup
            \def\cap@margin{%
              \let\cap@left=\relax
              \let\cap@right=\hfill
              \if@twoside
                \ifthispageodd{}{%
                  \let\cap@left=\hfill
                  \let\cap@right=\relax
                }%
              \fi
            }%
          \else
            \def\@tempb{o}\ifx\@tempa\@tempb
              \endgroup
              \def\cap@margin{%
                \let\cap@left=\hfill
                \let\cap@right=\relax
                \if@twoside
                  \ifthispageodd{}{%
                    \let\cap@left=\relax
                    \let\cap@right=\hfill
                  }%
                \fi
              }%
            \else
              \ifx \@tempa\@empty\else
                \ClassError{%
                  scrreprt%
                  }{%
                  unknown optional position argument%
                  }{%
                  You've used `#1' as position argument at
                  \string\setcaptionwidth,\MessageBreak
                  but only ONE of `c', `l', `r', `i`, `o` ist
                  allowed.\MessageBreak
                  Argument will be ignored, if you'll continue.%
                  }%
              \fi%
              \endgroup%
  \fi\fi\fi\fi\fi%
  }
\AfterPackage{caption2}{%
  \renewcommand*{\setcapwidth}[2][]{%
    \ClassWarning{%
      scrbook%
      }{%
      Ignorring optional argument of \string\setcapwidth!\MessageBreak
      You're using package caption2.\MessageBreak
      Because of this you should also use
      \string\setcaptionwidth\MessageBreak
      instead of \string\setcapwidth\MessageBreak
      }
    \setcaptionwidth{#2}}}%
\newcommand*{\cap@margin}{}
\let\cap@margin=\relax%
\newcommand*{\setcapmargin}{\@ifstar{\@setcapmargin}{\@@setcapmargin}}
\newcommand*{\@setcapmargin}[2][\@tempdimb]{%
  \def\cap@margin{%
    \onelinecaptionsfalse%
    \begingroup%
      \setlength{\@tempdima}{\hsize}%
      \setlength{\@tempdimb}{#2}%
      \addtolength{\@tempdima}{-\@tempdimb}%
      \setlength{\@tempdimc}{#1}%
      \addtolength{\@tempdima}{-\@tempdimc}%
      \edef\@tempa{\endgroup%
        \def\noexpand\cap@width{\the\@tempdima}%
        \noexpand\if@twoside%
          \noexpand\ifthispageodd{%
            \def\noexpand\cap@left{\noexpand\hspace*{\the\@tempdimc}}%
            \def\noexpand\cap@right{\noexpand\hspace*{\the\@tempdimb}}%
          }{%
            \def\noexpand\cap@right{\noexpand\hspace*{\the\@tempdimc}}%
            \def\noexpand\cap@left{\noexpand\hspace*{\the\@tempdimb}}%
          }%
        \noexpand\else%
          \def\noexpand\cap@left{\noexpand\hspace*{\the\@tempdimc}}%
          \def\noexpand\cap@right{\noexpand\hspace*{\the\@tempdimb}}%
        \noexpand\fi}
    \@tempa}%
  }
\newcommand*{\@@setcapmargin}[2][\@tempdimb]{%
  \def\cap@margin{%
    \onelinecaptionsfalse%
    \begingroup%
      \setlength{\@tempdima}{\hsize}%
      \setlength{\@tempdimb}{#2}%
      \addtolength{\@tempdima}{-\@tempdimb}%
      \setlength{\@tempdimc}{#1}%
      \addtolength{\@tempdima}{-\@tempdimc}%
      \edef\@tempa{\endgroup%
        \def\noexpand\cap@width{\the\@tempdima}%
        \def\noexpand\cap@left{\noexpand\hspace*{\the\@tempdimc}}%
        \def\noexpand\cap@right{\noexpand\hspace*{\the\@tempdimb}}}%
    \@tempa}%
  }
\AfterPackage{caption2}{%
  \renewcommand*{\@setcapmargin}[2][\@tempdimb]{%
    \setlength{\@tempdimb}{#2}%
    \setlength{\@tempdima}{#1}%
    \ifdim\@tempdima\@tempdimb\else
      \ClassWarning{%
        scrbook%
        }{%
        Can set only centered caption!\MessageBreak
        You're using package caption2.\MessageBreak
        Because of this you should also use
        \string\setcaptionmargin\MessageBreak
        instead of \string\setcapmargin
        }
    \fi
    \addtolength{\@tempdimb}{\@tempdimb}%
    \setcaptionmargin{.5\@tempdimb}}
  \let\@@setcapmargin=\@setcapmargin
  }
\newcommand{\@@makecaption}[3]{%
  \cap@margin%
  \ifonelinecaptions%
    \@tempcnta=\value{mpfootnote}\@tempcntb=\value{footnote}%
    \let\scr@tmp@footnotetext=\@footnotetext
    \let\@footnotetext=\@gobble
    \sbox\@tempboxa{%
      {\capfont#1{{\caplabelfont#2\captionformat}}%
        #3}%
      }%
    \let\@footnotetext=\scr@tmp@footnotetext
    \let\scr@tmp@footnotetext=\undefined
    \setcounter{footnote}{\@tempcntb}\setcounter{mpfootnote}{\@tempcnta}%
  \else
    \sbox\@tempboxa{\hspace*{2\cap@width}}%
  \fi
  \begingroup%
    \setlength{\@tempdima}{\wd\@tempboxa}%
    \setlength{\@tempdima}{\cap@width}%
    \ifdim \wd\@tempboxa >\@tempdima%
      \cap@left%
      \vbox{\hsize=\cap@width%
        \ifdofullc@p
          {\capfont\@hangfrom{#1{{\caplabelfont#2\captionformat}}}{#3\par}}
        \else%
          \ifdim\cap@indent<\z@%
            {\capfont#1{{\caplabelfont #2\captionformat\par}}\noindent%
              \hspace*{-\cap@indent}#3\par}
          \else%
            \if@capbreak
              {\capfont#1{{\caplabelfont #2\captionformat\par}}\noindent%
                \@hangfrom{\hspace*{\cap@indent}}{#3\par}}
            \else
              {\capfont\@hangfrom{\hspace*{\cap@indent}}%
                {\hspace{-\cap@indent}#1%
                  {{\caplabelfont#2\captionformat}}#3\par}}
            \fi
          \fi
        \fi}%
      \cap@right%
    \else
      \global \@minipagefalse
      \hb@xt@\hsize{\cap@left\box\@tempboxa\cap@right}%
    \fi%
  \endgroup}
\AfterPackage{longtable}{%
  \@ifpackageloaded{caption2}{\@komalongtablefalse%
    \ClassWarning{%
     scrreprt}{%
      you've loaded package caption2 before package\MessageBreak
      longtable!\MessageBreak
      Because of this package caption2 isn't able to
      control\MessageBreak
      \string\caption\space at longtables. After loading package
      caption2\MessageBreak
      KOMA-Script also doesn't control \string\caption.\MessageBreak
      I think, you should load package caption2 after
      package\MessageBreak
      longtable%
      }}{}
  \if@komalongtable
    \renewcommand{\LT@makecaption}[3]{%
      \LT@mcol\LT@cols c{%
        \hbox to\z@{\hss\parbox[t]\linewidth{%
            \if@captionabove%
              \vskip\belowcaptionskip%
            \else%
              \vskip\abovecaptionskip%
            \fi%
            \@@makecaption{#1}{#2}{#3}
            \endgraf%
            \if@captionabove%
              \vskip\abovecaptionskip%
            \else%
              \vskip\belowcaptionskip%
            \fi%
            }%
          \hss}%
        }%
      }%
    \newcommand*{\scr@LT@array}{}%
    \let\scr@LT@array=\LT@array%
    \renewcommand*{\LT@array}{%
      \let\scr@caption=\LT@caption
      \def\captionabove{\noalign{\global\@captionabovetrue}%
        \scr@caption}%
      \def\captionbelow{\noalign{\global\@captionabovefalse}%
        \scr@caption}%
      \if@tablecaptionabove
        \let\LT@caption=\captionabove
      \else
        \let\LT@caption=\captionbelow
      \fi
      \scr@LT@array}
    \ClassInfo{%
      scrreprt}{%
      longtable captions redefined}
  \fi
  }
\newcommand*{\fs@komaabove}{%
  \let\@fs@capt\floatc@komascript
  \let\@fs@pre\@empty
  \def\@fs@mid{\vskip\abovecaptionskip}
  \let\@fs@post\@empty
  \let\@fs@iftopcapt\iftrue}
\newcommand*{\fs@komabelow}{%
  \let\@fs@capt\floatc@komascript
  \let\@fs@pre\@empty
  \def\@fs@mid{\vskip\abovecaptionskip}
  \let\@fs@post\@empty
  \let\@fs@iftopcapt\iffalse}
\newcommand{\floatc@komascript}{\@@makecaption\@firstofone}
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareOldFontCommand{\sfb}{\normalfont\sffamily\bfseries}{\@nomath\sfb}
\DeclareRobustCommand*{\cal}{\@fontswitch\relax\mathcal}
\DeclareRobustCommand*{\mit}{\@fontswitch\relax\mathnormal}
\newcommand*\descfont{\sffamily\bfseries}
\newcommand*\sectfont{\normalcolor\sffamily\bfseries}
\newcommand*\pnumfont{\normalfont\normalcolor}
\newcommand*\headfont{\normalfont\normalcolor\slshape}
\newcommand*\capfont{\normalfont\normalcolor}
\newcommand*\caplabelfont{\normalfont\normalcolor}
\newcommand*\titlefont{\sectfont}
\newcommand*{\setkomafont}[2]{%
  \@ifundefined{scr@fnt@#1}{%
    \@ifundefined{scr@fnt@instead@#1}{%
      \ClassError{%
        scrreprt%
      }{font of element `#1' can't be set}{%
        You've told me to redefine the font selection of the element,%
        \MessageBreak%
        but either no such element is known by KOMA-Script\MessageBreak
        or the element does not use a special font selection}
    }{%
      \ClassError{%
        scrreprt%
      }{font of element `#1' can't be set}{%
        You've told me to redefine the font selection of the element,%
        \MessageBreak%
        but the element does not use a special font
        selection. The\MessageBeak
        The font of `#1' may be changed with element `\csname
        scr@fnt@instead@#1\endcsname'}
    }%
  }{%
    \expandafter\expandafter\expandafter\renewcommand%
    \csname scr@fnt@#1\endcsname{#2}%
    \@ifundefined{scr@fnt@wrn@#1}{}{%
      \ClassWarning{%
      scrreprt%
      }{\csname scr@fnt@wrn@#1\endcsname{#1}}
    }
  }
}
\newcommand{\l@addto@macro}[2]{%
  \begingroup%
    \toks@\expandafter{#1#2}%
    \edef\@tempa{\endgroup\def\noexpand#1{\the\toks@}}%
  \@tempa}
\newcommand*{\addtokomafont}[2]{%
  \@ifundefined{scr@fnt@#1}{%
    \@ifundefined{scr@fnt@instead@#1}{%
      \ClassError{%
        scrreprt%
      }{font of element `#1' can't be extended}{%
        You've told me to extend the font selection of the element,%
        \MessageBreak%
        but either no such element is known by KOMA-Script\MessageBreak
        or the element does not use a special font selection}
    }{%
      \ClassError{%
        scrreprt%
      }{font of element `#1' can't be extended}{%
        You've told me to extend the font selection of the element,%
        \MessageBreak%
        but the element does not use a special font
        selection. The\MessageBeak
        The font of `#1' may be changed with element `\csname
        scr@fnt@instead@#1\endcsname'}
    }%
  }{%
    \expandafter\expandafter\expandafter\l@addto@macro%
    \csname scr@fnt@#1\endcsname{#2}%
    \@ifundefined{scr@fnt@wrn@#1}{}{%
      \ClassWarning{%
      scrreprt%
      }{\csname scr@fnt@wrn@#1\endcsname{#1}}
    }
  }
}
\newcommand*{\usekomafont}[1]{%
  \@ifundefined{scr@fnt@#1}{%
    \@ifundefined{scr@fnt@instead@#1}{%
      \ClassError{%
        scrreprt%
        }{font of element `#1' can't be used}{%
        You've told me to use the font selection of the element,%
        \MessageBreak%
        but either no such element is known by KOMA-Script\MessageBreak
        or the element does not use a special font selection}
      }{%
      \ClassWarning{%
        scrreprt%
      }{%
        You've told me to use the font selection of the element,%
        \MessageBreak%
        but the element does not use a special font
        selection. The\MessageBeak
        The font of `#1' may be selected with element `\csname
        scr@fnt@instead@#1\endcsname'.\MessageBreak
        I fix this using font of `\csname
        scr@fnt@instead@#1\endcsname'}%
      }%
    \@nameuse{scr@fnt@instead@#1}%
    }{%
    \@nameuse{scr@fnt@#1}%
    }%
  }
\newcommand*{\scr@fnt@descriptionlabel}{\descfont}
\newcommand*{\scr@fnt@title}{\titlefont}
\newcommand*{\scr@fnt@sectioning}{\sectfont}
\newcommand*{\scr@fnt@part}{\size@part}
\newcommand*{\scr@fnt@partnumber}{\size@partnumber}
\newcommand*{\scr@fnt@chapter}{\size@chapter}
\newcommand*{\scr@fnt@section}{\size@section}
\newcommand*{\scr@fnt@subsection}{\size@subsection}
\newcommand*{\scr@fnt@subsubsection}{\size@subsubsection}
\newcommand*{\scr@fnt@paragraph}{\size@paragraph}
\newcommand*{\scr@fnt@subparagraph}{\size@subparagraph}
\newcommand*{\scr@fnt@instead@minisec}{sectioning}
\newcommand*{\scr@fnt@pagenumber}{\pnumfont}
\let\scr@fnt@pagination=\scr@fnt@pagenumber
\newcommand*{\scr@fnt@pagehead}{\headfont}
\newcommand*{\scr@fnt@wrn@pagehead}[1]{%
  font selection of elements `pagehead' and `pagefoot'\MessageBreak
  changed, because you wanted to change font selection\MessageBreak
  of element `#1'%
}
\let\scr@fnt@pagefoot=\scr@fnt@pagehead
\let\scr@fnt@wrn@pagefoot=\scr@fnt@wrn@pagehead
\newcommand*{\scr@fnt@caption}{\capfont}
\newcommand*{\scr@fnt@captionlabel}{\caplabelfont}
\newcommand*\@pnumwidth{1.55em}
\newcommand*\@tocrmarg {2.55em}
\newcommand*\@dotsep{4.5}
\setcounter{tocdepth}{2}
\newcommand*\toc@heading{\section*{\contentsname
  \@mkboth{\contentsname}{\contentsname}}}
\newcommand*{\scr@dottedtocline}{}%
\newcommand*{\scr@numberline}{}%
\newcommand*{\last@l@number}{}%
\newcommand*{\@l@number}{}
\newcommand*{\set@l@number}[1]{%
  \settowidth{\@tempdima}{#1\enskip}%
  \ifdim\@tempdima >\@l@number\relax%
    \xdef\@l@number{\the\@tempdima}%
  \fi%
  \ifdim\@tempdima >\last@l@number \else%
    \setlength{\@tempdima}{\last@l@number}%
  \fi%
  }
\newcommand*{\before@starttoc}[1]{
  \let\scr@dottedtocline=\@dottedtocline%
  \renewcommand*{\@dottedtocline}[3]{%
    \scr@dottedtocline{##1}{\z@}{\last@l@number}}%
  \let\scr@numberline=\numberline%
  \renewcommand*{\numberline}[1]{%
    \set@l@number{##1}\scr@numberline{##1}}%
  \gdef\@l@number{\z@}%
  \@ifundefined{#1@l@number}{%
    \def\last@l@number{2em}%
    }{%
    \expandafter\let\expandafter\last@l@number
    \expandafter=\csname#1@l@number\endcsname%
    }%
  }
\newcommand*{\after@starttoc}[1]{%
  \protected@write\@auxout{}{%
    \string\gdef\expandafter\string\csname#1@l@number\endcsname{%
      \@l@number}}%
  }
\newcommand*{\toc@l@number}{}
\newcommand*{\tableofcontents}{%
  \begingroup
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \chapter*{\contentsname
        \@mkboth{\contentsname}{\contentsname}}%
    \@parskipfalse\@parskip@indent%
    \if@tocleft\before@starttoc{toc}\fi%
    \@starttoc{toc}%
    \if@tocleft\after@starttoc{toc}\fi%
    \if@restonecol\twocolumn\fi
  \endgroup
}
\newcommand*\l@part[2]{%
  \ifnum \c@tocdepth >-2\relax
    \addpenalty{-\@highpenalty}%
    \addvspace{2.25em \@plus\p@}%
    \setlength\@tempdima{2em}%
    \if@tocleft
      \ifx\toc@l@number\@empty\else
        \setlength\@tempdima{0\toc@l@number}%
      \fi
    \fi
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode\sectfont\large
      \advance\leftskip\@tempdima
      \hskip -\leftskip
      #1\nobreak\hfil \nobreak\hb@xt@\@pnumwidth{\hss #2}\par
      \penalty\@highpenalty
    \endgroup
  \fi}
\newcommand*\l@chapter[2]{%
  \ifnum \c@tocdepth >\m@ne
    \addpenalty{-\@highpenalty}%
    \vskip 1.0em \@plus\p@
    \setlength\@tempdima{1.5em}%
    \if@tocleft
      \ifx\toc@l@number\@empty\else
        \setlength\@tempdima{0\toc@l@number}%
      \fi
    \fi
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode \sectfont
      \advance\leftskip\@tempdima
      \hskip -\leftskip
      #1\nobreak\hfil \nobreak\hb@xt@\@pnumwidth{\hss #2}\par
      \penalty\@highpenalty
    \endgroup
  \fi}
\newcommand*\l@section{\@dottedtocline{1}{1.5em}{2.3em}}
\newcommand*\l@subsection{\@dottedtocline{2}{3.8em}{3.2em}}
\newcommand*\l@subsubsection{\@dottedtocline{3}{7.0em}{4.1em}}
\newcommand*\l@paragraph{\@dottedtocline{4}{10em}{5em}}
\newcommand*\l@subparagraph{\@dottedtocline{5}{12em}{6em}}
\newcommand*{\float@listhead}[1]{%
  \chapter*{#1}%
  \@mkboth{#1}{#1}%
  \if@dynlist%
    \newcommand*{\scr@starttoc}{}%
    \let\scr@starttoc=\@starttoc%
    \renewcommand*{\@starttoc}[1]{%
      \before@starttoc{##1}\scr@starttoc{##1}\after@starttoc{##1}}%
  \fi%
}
\newcommand*\lof@heading{\float@listhead{\listfigurename}}
\newcommand*\listoffigures{%
  \begingroup%
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \lof@heading%
    \@parskipfalse\@parskip@indent%
    \@starttoc{lof}%
    \if@restonecol\twocolumn\fi
  \endgroup
}
\newcommand*\l@figure{\@dottedtocline{1}{1.5em}{2.3em}}
\newcommand*\lot@heading{\float@listhead{\listtablename}}
\newcommand*\listoftables{%
  \begingroup
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \lot@heading%
    \@parskipfalse\@parskip@indent%
    \@starttoc{lot}%
    \if@restonecol\twocolumn\fi
  \endgroup
}
\let\l@table\l@figure
\newcommand*{\bibpreamble}{\relax}
\let\bibpreamble\relax
\newcommand{\setbibpreamble}[1]{\def\bibpreamble{#1}}
\newcommand*\bib@heading{%
  \chapter*{\bibname}
  \@mkboth{\bibname}{\bibname}
}
\newdimen\bibindent
\bibindent=1.5em
\newenvironment{thebibliography}[1]{%
  \ifx\bibpreamble\relax\else\ifx\bibpreamle\@empty\else
    \setchapterpreamble{\bibpreamble}%
  \fi\fi
  \bib@heading%
  \list{\@biblabel{\@arabic\c@enumiv}}%
  {\settowidth\labelwidth{\@biblabel{#1}}%
    \leftmargin\labelwidth
    \advance\leftmargin\labelsep
    \@openbib@code
    \usecounter{enumiv}%
    \let\p@enumiv\@empty
    \renewcommand*\theenumiv{\@arabic\c@enumiv}}%
  \sloppy\clubpenalty4000\widowpenalty4000%
  \sfcode`\.=\@m}
{\def\@noitemerr
  {\@latex@warning{Empty `thebibliography' environment}}%
  \endlist}
\newcommand*\newblock{\hskip .11em\@plus.33em\@minus.07em}
\let\@openbib@code\@empty
\newcommand*{\index@preamble}{\relax}
\newcommand{\setindexpreamble}[1]{\renewcommand*{\index@preamble}{#1}}
\newcommand*{\idx@heading}{%
  \if@openright\cleardoublepage\else\clearpage\fi%
  \twocolumn[\chapter*{\indexname}]
  \@mkboth{\indexname}{\indexname}%
}
\newenvironment{theindex}
  {\if@twocolumn
    \@restonecolfalse
    \else
    \@restonecoltrue
    \fi
    \columnseprule \z@
    \columnsep 35\p@
    \setchapterpreamble{\index@preamble}
    \idx@heading%
    \thispagestyle{\indexpagestyle}\parindent\z@
    \setlength{\parskip}{\z@ \@plus .3\p@}
    \setlength{\parfillskip}{\z@ \@plus 1fil}
    \let\item\@idxitem}
  {\if@restonecol\onecolumn\else\clearpage\fi}
\newcommand*\@idxitem  {\par\hangindent 40\p@}
\newcommand*\subitem   {\@idxitem \hspace*{20\p@}}
\newcommand*\subsubitem{\@idxitem \hspace*{30\p@}}
\newcommand*\indexspace{\par \vskip 10\p@ \@plus5\p@ \@minus3\p@\relax}
\renewcommand*\footnoterule{%
  \kern-3\p@
  \ifx\@textbottom\relax\else\vskip \z@ \@plus.0005fil\fi
  \hrule\@width.4\columnwidth
  \kern 2.6\p@}
\@addtoreset{footnote}{chapter}
\newcommand\deffootnote[4][]{%
  \long\def\@makefntext##1{%
    \setlength{\@tempdimc}{#3}%
    \def\@tempa{#1}\ifx\@tempa\@empty
      \@setpar{\@@par
        \@tempdima = \hsize
        \addtolength{\@tempdima}{-#2}
        \parshape \@ne #2 \@tempdima}%
    \else
      \addtolength{\@tempdimc}{#2}%
      \addtolength{\@tempdimc}{-#1}%
      \@setpar{\@@par
        \@tempdima = \hsize
        \addtolength{\@tempdima}{-#1}
        \@tempdimb = \hsize
        \addtolength{\@tempdimb}{-#2}
        \parshape \tw@ #1 \@tempdima #2 \@tempdimb}%
    \fi
    \par
    \parindent\@tempdimc\noindent
    \ftn@font{\hbox to \z@{\hss\@@makefnmark}##1}}
  \def\@@makefnmark{\hbox{\ftnm@font{#4}}}
}
\newcommand*{\ftn@font}{\normalfont}
\newcommand*{\scr@fnt@footnote}{\ftn@font}
\newcommand*{\ftnm@font}{}
\newcommand*{\scr@fnt@footnotenumber}{\ftnm@font}
\newcommand*{\scr@fnt@footnotelabel}{\ftnm@font}
\newcommand*{\scr@fnt@instead@footnotetext}{footnote}
\newcommand*\deffootnotemark[1]{\def\@makefnmark{\hbox{\ftntm@font{#1}}}}
\newcommand*{\ftntm@font}{}
\newcommand*{\scr@fnt@footnotereference}{\ftntm@font}
\newcommand*{\scr@fnt@footnoteref}{\ftntm@font}
\newcommand*{\thefootnotemark}{\@thefnmark}
\deffootnote[1em]{1.5em}{1em}
  {\textsuperscript{\thefootnotemark}}
\deffootnotemark{\textsuperscript{\thefootnotemark}}
\DeclareRobustCommand*\textsubscript[1]{%
  \@textsubscript{\selectfont#1}}
\newcommand{\@textsubscript}[1]{%
  {\m@th\ensuremath{_{\mbox{\fontsize\sf@size\z@#1}}}}}
\newcommand*\contentsname{Contents}
\newcommand*\listfigurename{List of Figures}
\newcommand*\listtablename{List of Tables}
\newcommand*\bibname{Bibliography}
\newcommand*\indexname{Index}
\newcommand*\figurename{Figure}
\newcommand*\tablename{Table}
\newcommand*\partname{Part}
\newcommand*\chaptername{Chapter}
\newcommand*\appendixname{Appendix}
\newcommand*\abstractname{Abstract}
\newcommand*\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space\number\day, \number\year}
\newcommand*{\providecaptionname}[3]{%
  \@ifundefined{captions#1}{%
    \@ifundefined{cap@info@#1@done}{%
      \ClassInfo{%
        scrlttr2%
        }{%
        No captions found for `#1'\MessageBreak
        --> skipped%
        }%
      \expandafter\let\csname cap@info@#1@done\endcsname\@empty}{}%
    }{%
    \begingroup%
      \let\@tempa\endgroup%
      \csname captions#1\endcsname%
      \ifx #2\undefined%
        \def\@tempa{\endgroup%
          \expandafter\g@addto@macro\csname captions#1\endcsname{%
            \def#2{#3}}}%
      \else
        \ClassInfo{%
          scrlttr2%
          }{%
          `\string#2' at language `#1' already
          defined\MessageBreak
          --> skipped%
          }
      \fi
    \@tempa
    }
  }
\newcommand*{\newcaptionname}[3]{%
  \@ifundefined{captions#1}{%
    \ClassInfo{%
      scrlttr2%
      }{%
      No captions found for `#1' --> defining%
      }%
    \@namedef{captions#1}{}%
    }{}%
  \begingroup%
    \csname captions#1\endcsname%
    \ifx #2\undefined\else%
      \ClassError{%
        scrlttr2%
        }{%
        `\string#2' at language `#1' already defined%
        }{%
        You've told me to define `\string#2' at language
        `#1',\MessageBreak
        but it is already defined.\MessageBreak
        It will be redefined, if you'll continue.%
        }
    \fi
  \endgroup%
  \expandafter\g@addto@macro\csname captions#1\endcsname{%
    \def#2{#3}}%
  }
\newcommand*{\renewcaptionname}[3]{%
  \@ifundefined{captions#1}{%
    \ClassError{%
      scrlttr2%
      }{%
      No captions found for `#1'%
      }{%
      You've told me to redefine the caption `\string#2' at language
      `#1',\MessageBreak
      but `\expandafter\string\csname captions#1\endcsname' is not
      defined.\MessageBreak
      If you'll continue it will be defined.%
      }
    \@namedef{captions#1}{}%
    }{}%
  \begingroup%
    \csname captions#1\endcsname%
    \ifx #2\undefined%
      \ClassError{%
        scrlttr2%
        }{%
        `\string#2' not defined at language `#1'%
        }{%
        You've told me to redefine `\string#2' at language
        `#1',\MessageBreak
        but it is not defined.\MessageBreak
        If you'll continue it will be defined.%
        }
    \fi
  \endgroup
  \expandafter\g@addto@macro\csname captions#1\endcsname{%
    \def#2{#3}}%
  }
\newif\if@altsecnumformat\@altsecnumformatfalse
\newif\if@autodot\@autodotfalse
\newcommand{\@maybeautodot}[1]{\if@autodot\else\begingroup%
  \expandafter\@@maybeautodot #1\@stop\endgroup\fi}
\newcommand*{\@@maybeautodot}[1]{%
  \ifx #1\@stop\let\@@maybeautodot\relax%
  \else%
    \ifx #1\Alph \aftergroup\@autodottrue\fi%
    \ifx #1\alph \aftergroup\@autodottrue\fi%
    \ifx #1\Roman \aftergroup\@autodottrue\fi%
    \ifx #1\roman \aftergroup\@autodottrue\fi%
    \ifx #1\@Alph \aftergroup\@autodottrue\fi%
    \ifx #1\@alph \aftergroup\@autodottrue\fi%
    \ifx #1\@Roman \aftergroup\@autodottrue\fi%
    \ifx #1\@roman \aftergroup\@autodottrue\fi%
    \ifx #1\romannumeral \aftergroup\@autodottrue\fi%
  \fi%
  \@@maybeautodot}
\let\scr@sect\@sect
\renewcommand*{\@sect}[1]{\stepcounter{#1}%
  \expandafter\@maybeautodot\csname the#1\endcsname%
  \addtocounter{#1}{-1}\scr@sect{#1}}
\AtEndDocument{%
  \if@autodot\if@filesw\immediate\write\@mainaux{%
    \string\global\string\@altsecnumformattrue}
  \fi\fi}
\newcommand*\autodot{\if@altsecnumformat.\fi}
\newcommand*\chaptermarkformat    {\chapappifchapterprefix{\ }%
                                   \thechapter\autodot\enskip}
\newcommand*\sectionmarkformat    {\thesection\autodot\enskip}
\newcommand*{\partformat}{\partname~\thepart\autodot}
\newcommand*{\chapterformat}{%
  \chapappifchapterprefix{\ }\thechapter\autodot\enskip}
\newcommand*{\figureformat}{\figurename~\thefigure%
  \autodot%
  }
\newcommand*{\tableformat}{\tablename~\thetable%
  \autodot%
}
\newcommand*{\captionformat}{:\ }
\newcommand*{\chapappifprefix}[1][]{%
  \ClassWarning{%
    scrreprt%
  }{%
    Please don't use obsolete command
    \string\chapappifprefix.\MessageBreak
    The new command \string\chapappifchapterprefix\space has
    an\MessageBreak
    obligatory instead of an optional argument. Use that\MessageBreak
    new command%
  }%
  \if@chapterprefix\chapapp#1\fi}
\newcommand*{\chapappifchapterprefix}[1]{\if@chapterprefix\chapapp#1\fi}
\newcommand*{\chapapp}{\@chapapp}
\newcommand*{\othersectionlevelsformat}[1]{%
  \csname the#1\endcsname\autodot\enskip}
\renewcommand*{\@seccntformat}{\othersectionlevelsformat}
\pagestyle{plain}
\pagenumbering{arabic}      % Arabic page numbers
\if@twoside
\else
  \raggedbottom
\fi
\if@twocolumn
  \twocolumn
  \sloppy
  \flushbottom
\else
  \onecolumn
\fi
\@ifundefined{KOMAScript}{%
  \DeclareRobustCommand{\KOMAScript}{\textsf{K\kern.05em O\kern.05em%
      M\kern.05em A\kern.1em-\kern.1em Script}}}{}
\endinput
%%
%% End of file `scrreprt.cls'.
