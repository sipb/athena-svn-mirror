%%
%% This is file `scrreprt.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% scrclass.dtx  (with options: `scrreprt')
%% 
%% Copyright 1994-1997 Markus Kohm and Frank Neukam.
%% All rights reserved. For further copyright information see the file
%% readme.txt or liesmich.txt, and any other copyright indicated in this
%% file.
%% 
%% This file is part of the LaTeX2e KOMA-Script bundle.
%% ----------------------------------------------------
%% 
%% The KOMA-Script bundle bases on the Script-family created by
%% Frank Neukam 1993 and the standard documents classes created
%% by the LaTeX3-Team 1994-1996.
%% -------------------------------------------------------------
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
%% and liesmich.txt.
%% 
%% If you receive only some of these files from someone, complain!
%% 
%% You are NOT ALLOWED to distribute this file alone.  You are NOT
%% ALLOWED to take money for the distribution or use of either this
%% file or a changed version, except for a nominal charge for copying
%% etc.
%% 
\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{scrreprt}
              [1998/07/17 v2.5e LaTeX2e KOMA
               document class]
\newcommand*\@ptsize{}
\newif\if@restonecol
\newif\if@titlepage
\@titlepagetrue
\newif\if@openright
\newif\if@hsl
\newif\if@fsl
\newif\if@abstrt
\def\@BCOR{\expandafter\@@BCOR\CurrentOption BCORBCOR\@@BCOR}
\def\@@BCOR #1BCOR#2BCOR#3\@@BCOR{
 \edef\@tempa{#1}\ifx\@tempa\@empty
  \edef\@tempa{#3}\def\@tempb{BCOR}\ifx\@tempa\@tempb
   \PassOptionsToPackage{BCOR#2}{typearea}
  \else\OptionNotUsed\fi
 \else\OptionNotUsed\fi}
\def\@DIV{\expandafter\@@DIV\CurrentOption DIVDIV\@@DIV}
\def\@@DIV #1DIV#2DIV#3\@@DIV{
 \edef\@tempa{#1}\ifx\@tempa\@empty
  \edef\@tempa{#3}\def\@tempb{DIV}\ifx\@tempa\@tempb
   \PassOptionsToPackage{DIV#2}{typearea}
  \else\OptionNotUsed\fi
 \else\@BCOR\fi}
\def\x@paper{\expandafter\x@@paper\CurrentOption paper\x@@paper}
\def\x@@paper #1#2paper#3\x@@paper{
 \edef\@tempa{paper}\edef\@tempb{#3}\ifx\@tempa\@tempb
  \ifx a#1\PassOptionsToPackage{a#2paper}{typearea}
  \else\ifx b#1\PassOptionsToPackage{b#2paper}{typearea}
  \else\ifx c#1\PassOptionsToPackage{c#2paper}{typearea}
  \else\ifx d#1\PassOptionsToPackage{d#2paper}{typearea}
  \else\OptionNotUsed\fi\fi\fi\fi
 \else\@DIV\fi}
%%
\DeclareOption{headsepline}{\PassOptionsToPackage{headinclude}{typearea}
                            \@hsltrue}
\DeclareOption{headnosepline}{\@hslfalse}
\DeclareOption{footsepline}{\PassOptionsToPackage{footinclude}{typearea}
                            \@fsltrue}
\DeclareOption{footnosepline}{\@fslfalse}
\DeclareOption{10pt}{\renewcommand*\@ptsize{0}}
\DeclareOption{11pt}{\renewcommand*\@ptsize{1}}
\DeclareOption{12pt}{\renewcommand*\@ptsize{2}}
\DeclareOption{oneside}{\@twosidefalse \@mparswitchfalse}
\DeclareOption{twoside}{\@twosidetrue  \@mparswitchtrue}
\DeclareOption{draft}{\setlength\overfullrule{5pt}}
\DeclareOption{final}{\setlength\overfullrule{0pt}}
\DeclareOption{titlepage}{\@titlepagetrue}
\DeclareOption{notitlepage}{\@titlepagefalse}
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
\DeclareOption{abstracton}{\@abstrttrue}
\DeclareOption{abstractoff}{\@abstrtfalse}
\DeclareOption{pointednumbers}
  {\AtEndOfClass{\@altsecnumformattrue
                 \global\let\@altsecnumformatfalse\@altsecnumformattrue}}
\DeclareOption{pointednumber}
  {\AtEndOfClass{\@altsecnumformattrue
                 \global\let\@altsecnumformatfalse\@altsecnumformattrue}
   \ClassError
    {scrreprt}
    {undefined option `pointednumber' instead of `pointednumbers'.}
    {You've used the class-option `pointednumber'.\MessageBreak
     This name was an implementation-error at versions from 2.3h till
     2.4f.\MessageBreak
     The correct name ist `pointednumber'.
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
    {undefined option `pointlessnumber' instead of `pointlessnumbers'.}
    {You've used the class-option `pointlessnumber'.\MessageBreak
     This name was an implementation-error at versions from 2.3h till
     2.4f.\MessageBreak
     The correct name ist `pointlessnumber'.
     Change this at your document.\MessageBreak
     If you'll continue, the option `pointlessnumbers' will be used.}}
\newcommand\@bigheadings
  {\AtEndOfClass{
    \def\chapterheadstartvskip{\vspace*{2.3\baselineskip}}
    \def\chapterheadendvskip{\vspace{1.7\baselineskip}}
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
    {You've used the old option name `bigheading'!\MessageBreak
     The correct name of the option ist `bigheadings'.\MessageBreak
     The old name was an implementation error from\MessageBreak
     version 2.3h till 2.4f.\MessageBreak
     You should change this at your document!\MessageBreak
     If you'll continue, option `bigheadings' will be used.}}
\newcommand\@normalheadings
  {\AtEndOfClass{
    \def\chapterheadstartvskip{\vspace*{2\baselineskip}}
    \def\chapterheadendvskip{\vspace{1.5\baselineskip}}
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
    {You've used the old option name `normalheading'!\MessageBreak
     The correct name of the option ist `normalheadings'.\MessageBreak
     The old name was an implementation error from\MessageBreak
     version 2.3h till 2.4f.\MessageBreak
     You should change this at your document!\MessageBreak
     If you'll continue, option `normalheadings' will be used.}}
\newcommand\@smallheadings
  {\AtEndOfClass{
    \def\chapterheadstartvskip{\vspace*{1.8\baselineskip}}
    \def\chapterheadendvskip{\vspace{1.3\baselineskip}}
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
    {You've used the old option name `smallheading'!\MessageBreak
     The correct name of the option ist `smallheadings'.\MessageBreak
     The old name was an implementation error from\MessageBreak
     version 2.3h till 2.4f.\MessageBreak
     You should change this at your document!\MessageBreak
     If you'll continue, option `smallheadings' will be used.}}
\DeclareOption{liststotoc}
  {\AtEndOfClass{%
    \renewcommand*\lot@heading{%
      \addchap{\listtablename}
      \@mkboth{\listtablename}{\listtablename}}
    \renewcommand*\lof@heading{%
      \addchap{\listfigurename}
      \@mkboth{\listfigurename}{\listfigurename}}
  }}
\DeclareOption{bibtotoc}
  {\AtEndOfClass{%
    \renewcommand*\bib@heading{%
  \addchap{\bibname}}
  }}
\DeclareOption{bibtotocnumbered}
  {\AtEndOfClass{%
    \renewcommand*\bib@heading{%
  \chapter{\bibname}}
  }}
\DeclareOption{idxtotoc}
 {\AtEndOfClass{%
  \renewcommand*\idx@heading{\twocolumn[%
   \@makeschapterhead{\indexname}]%
   \@mkboth{\indexname}{\indexname}%
   \addcontentsline{toc}{chapter}{\indexname}}%
  }}
\if@compatibility\else
  \DeclareOption*{\x@paper}
\fi
\ExecuteOptions{11pt,onecolumn,final,headnosepline,footnosepline,bigheadings,%
oneside,openany,abstractoff}
\ProcessOptions
\input{size1\@ptsize.clo}
\RequirePackage{typearea}[1995/06/25]
\setlength\lineskip{1\p@}
\setlength\normallineskip{1\p@}
\renewcommand*\baselinestretch{}
\setlength\parskip{0\p@ \@plus \p@}
\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
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
\if@twoside
 \def\ps@plain{\let\@mkboth\@gobbletwo
  \def\@evenhead{}%
  \def\@oddhead{}%
  \def\@evenfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\thepage\hfil}}}}%
  \def\@oddfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\hfil\thepage}}}}}
 \def\ps@headings{\let\@mkboth\markboth
  \def\@evenhead{\vbox{\hsize=\textwidth
   \hb@xt@ \textwidth{{\headfont\strut\leftmark\hfil}}%
   \if@hsl \vskip 1.5\p@ \hrule \fi}}
  \def\@oddhead{\vbox{\hsize=\textwidth
   \hb@xt@ \textwidth{{\headfont\hfil\strut\rightmark}}%
   \if@hsl \vskip 1.5\p@ \hrule \fi}}
  \def\@evenfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\thepage\hfil}}}}%
  \def\@oddfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\hfil\thepage}}}}%
  \def\chaptermark##1{%
   \markboth {\ifnum \c@secnumdepth >\m@ne
        \chaptermarkformat\fi
        ##1}{}}%
  \def\sectionmark##1{%
    \markright {\ifnum \c@secnumdepth >\z@
        \sectionmarkformat\fi
        ##1}}}
 \def\ps@myheadings{\let\@mkboth\@gobbletwo
  \def\@evenhead{\vbox{\hsize=\textwidth
   \hb@xt@ \textwidth{{\headfont\strut\leftmark\hfil}}
   \if@hsl \vskip 1.5\p@ \hrule \fi}}%
  \def\@oddhead{\vbox{\hsize=\textwidth
   \hb@xt@ \textwidth{{\headfont\hfil\strut\rightmark}}
   \if@hsl \vskip 1.5\p@ \hrule \fi}}%
  \def\@evenfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\thepage\hfil}}}}%
  \def\@oddfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\hfil\thepage}}}}%
  \def\chaptermark##1{}
  \def\sectionmark##1{}}
\else
 \def\ps@plain{\let\@mkboth\@gobbletwo
  \def\@evenhead{}%
  \def\@oddhead{}%
  \def\@evenfoot{}%
  \def\@oddfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\hfil\thepage\hfil}}}}}
 \def\ps@headings{\let\@mkboth\markboth
  \def\@evenhead{}%
  \def\@oddhead{\vbox{\hsize=\textwidth
   \hb@xt@ \textwidth{{\headfont\hfil\strut\rightmark\hfil}}
   \if@hsl \vskip 1.5\p@ \hrule \fi}}%
  \def\@evenfoot{}%
  \def\@oddfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\hfil\thepage\hfil}}}}%
  \def\sectionmark##1{}
  \def\chaptermark##1{\markright {\ifnum \c@secnumdepth >\m@ne
      \chaptermarkformat\fi
    ##1}}}
 \def\ps@myheadings{\let\@mkboth\@gobbletwo
  \def\@evenhead{}%
  \def\@oddhead{\vbox{\hsize=\textwidth
   \hb@xt@ \textwidth{{\headfont\hfil\strut\rightmark\hfil}}
   \if@hsl \vskip 1.5\p@ \hrule \fi}}%
  \def\@evenfoot{}%
  \def\@oddfoot{\vbox{\hsize=\textwidth
   \if@fsl \hrule \vskip 3\p@ \fi
   \hb@xt@ \textwidth{{\pnumfont\hfil\thepage\hfil}}}}%
  \def\chaptermark##1{}%
  \def\sectionmark##1{}}
\fi
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
    \def\@makefnmark{\rlap\@oldmakefnmark}
    \ifx\@extratitle\@empty \else
        \noindent\@extratitle\next@tpage\cleardoublepage\thispagestyle{empty}
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
    {\sectfont\huge \@title\par}
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
         \def\@makefnmark{\rlap\@oldmakefnmark}
         \if@twocolumn
           \ifnum \col@number=\@ne
             \@maketitle
           \else
             \twocolumn[\@maketitle]%
           \fi
         \else
           \newpage
           \global\@topnum\z@   % Prevents figures from going at top of page.
           \@maketitle
         \fi
         \thispagestyle{plain}\@thanks
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
\def\@maketitle{%
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
    {\sectfont\huge \@title \par}%
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
  \hb@xt@\@tempdima{#1\if@altsecnumformat.\fi\hfil}}
\newcommand*\chaptermark[1]{}
\setcounter{secnumdepth}{2}
\newcounter {part}
\newcounter {chapter}
\newcounter {section}[chapter]
\newcounter {subsection}[section]
\newcounter {subsubsection}[subsection]
\newcounter {paragraph}[subsubsection]
\newcounter {subparagraph}[paragraph]
\renewcommand*\thepart          {\@Roman\c@part}
\let\@thepart\thepart
\newcommand*\@maybeasf{%
  \ifx\@thepart\thepart\global\@altsecnumformattrue\fi}
\renewcommand*\thechapter       {\@arabic\c@chapter}
\renewcommand*\thesection       {\thechapter.\@arabic\c@section}
\renewcommand*\thesubsection    {\thesection.\@arabic\c@subsection}
\renewcommand*\thesubsubsection {\thesubsection.\@arabic\c@subsubsection}
\renewcommand*\theparagraph     {\thesubsubsection.\@arabic\c@paragraph}
\renewcommand*\thesubparagraph  {\theparagraph.\@arabic\c@subparagraph}
\newcommand*\@chapapp{\chaptername}
\newcommand\part{\if@openright\cleardoublepage\else\clearpage\fi
                 \thispagestyle{plain}%
                 \if@twocolumn
                     \onecolumn
                     \@tempswatrue
                   \else
                     \@tempswafalse
                 \fi
                 \null\vfil
                 \secdef\@part\@spart}
\def\@part[#1]#2{%
    \ifnum \c@secnumdepth >-2\relax
      \refstepcounter{part}\@maybeasf%
      \addcontentsline{toc}{part}{\protect\numberline{\thepart}#1}%
    \else
      \addcontentsline{toc}{part}{#1}%
    \fi
    \chaptermark{}
    {\centering
     \interlinepenalty \@M
     \normalfont
     \ifnum \c@secnumdepth >-2\relax
       \size@partnumber\sectfont\partformat
       \par
       \vskip 20\p@
     \fi
     \size@part\sectfont #2\par}%
    \@endpart}
\def\@spart#1{%
    {\centering
     \interlinepenalty \@M
     \normalfont
     \size@part\sectfont #1\chaptermark{}\par}%
    \@endpart}
\def\@endpart{\vfil\newpage
              \if@twoside
                \null%
                \thispagestyle{empty}%
                \newpage
              \fi
              \if@tempswa
                \twocolumn
              \fi}
\newcommand\chapter{\if@openright\cleardoublepage\else\clearpage\fi
                    \thispagestyle{plain}%
                    \global\@topnum\z@
                    \@afterindentfalse
                    \secdef\@chapter\@schapter}
\def\@chapter[#1]#2{\ifnum \c@secnumdepth >\m@ne
                         \refstepcounter{chapter}%
                         \typeout{\@chapapp\space\thechapter.}%
                         \addcontentsline{toc}{chapter}%
                                   {\protect\numberline{\thechapter}#1}%
                    \else
                      \addcontentsline{toc}{chapter}{#1}
                    \fi
                    \chaptermark{#1}%
                    \addtocontents{lof}{\protect\addvspace{10\p@}}%
                    \addtocontents{lot}{\protect\addvspace{10\p@}}%
                    \if@twocolumn
                      \@topnewpage[\@makechapterhead{#2}]%
                    \else
                      \@makechapterhead{#2}%
                      \@afterheading
                    \fi}
\def\@makechapterhead#1{\chapterheadstartvskip%
  {\size@chapter{\sectfont
    \@hangfrom{\ifnum \c@secnumdepth >\m@ne%
                  \chapterformat\fi}%
              {\raggedsection \interlinepenalty \@M #1\par}}
    \nobreak\chapterheadendvskip
  }}
\def\@schapter#1{\if@twocolumn
                   \@topnewpage[\@makeschapterhead{#1}]%
                 \else
                   \@makeschapterhead{#1}%
                   \@afterheading
                 \fi}
\def\@makeschapterhead#1{\chapterheadstartvskip%
  {\parindent \z@ \raggedsection
    \normalfont
    \size@chapter\sectfont #1\par
    \nobreak\chapterheadendvskip
  }}
\newcommand\addsec{\secdef\@addsec\@saddsec}
\def\@addsec[#1]#2{\section*{#2}\addcontentsline{toc}{section}{#1}
 \if@twoside\ifx\@mkboth\markboth\markright{#1}\fi\fi}
\def\@saddsec#1{\section*{#1}\@mkboth{}{}}
\newcommand\addchap{\if@openright\cleardoublepage\else\clearpage\fi
                    \thispagestyle{plain}%
                    \global\@topnum\z@
                    \@afterindentfalse
                    \secdef\@addchap\@saddchap}
\def\@addchap[#1]#2{\typeout{#2}
                    \addcontentsline{toc}{chapter}{#1}%
                    \if@twoside\@mkboth{#1}{}\else\@mkboth{}{#1}\fi
                    \addtocontents{lof}{\protect\addvspace{10\p@}}%
                    \addtocontents{lot}{\protect\addvspace{10\p@}}%
                    \if@twocolumn
                         \@topnewpage[\@makeschapterhead{#2}]%
                    \else
                         \@makeschapterhead{#2}%
                         \@afterheading
                    \fi}
\def\@saddchap#1{\@mkboth{}{}
                 \if@twocolumn
                      \@topnewpage[\@makeschapterhead{#1}]%
                 \else
                      \@makeschapterhead{#1}%
                      \@afterheading
                 \fi}
\newcommand\section{\@startsection{section}{1}{\z@}%
  {-3.5ex \@plus -1ex \@minus -.2ex}%
  {2.3ex \@plus.2ex}%
  {\raggedsection\normalfont\size@section\sectfont}}
\newcommand\subsection{\@startsection{subsection}{2}{\z@}%
  {-3.25ex\@plus -1ex \@minus -.2ex}%
  {1.5ex \@plus .2ex}%
  {\raggedsection\normalfont\size@subsection\sectfont}}
\newcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}%
  {-3.25ex\@plus -1ex \@minus -.2ex}%
  {1.5ex \@plus .2ex}%
  {\raggedsection\normalfont\size@subsubsection\sectfont}}
\newcommand\paragraph{\@startsection{paragraph}{4}{\z@}%
  {3.25ex \@plus1ex \@minus.2ex}%
  {-1em}%
  {\raggedsection\normalfont\size@paragraph\sectfont}}
\newcommand\subparagraph{\@startsection{subparagraph}{5}{\parindent}%
  {3.25ex \@plus1ex \@minus .2ex}%
  {-1em}%
  {\raggedsection\normalfont\size@subparagraph\sectfont}}
\newcommand\minisec[1]{\@afterindentfalse \vskip 1.5ex
  {\parindent \z@ \raggedsection\sectfont #1\par\nobreak}%
  \@afterheading}
\let\raggedsection\raggedright
\if@twocolumn
  \setlength\leftmargini  {2em}
\else
  \setlength\leftmargini  {2.5em}
\fi
\setlength\leftmarginii  {2.2em}
\setlength\leftmarginiii {1.87em}
\setlength\leftmarginiv  {1.7em}
\if@twocolumn
  \setlength\leftmarginv  {.5em}
  \setlength\leftmarginvi {.5em}
\else
  \setlength\leftmarginv  {1em}
  \setlength\leftmarginvi {1em}
\fi
\setlength\leftmargin    {\leftmargini}
\setlength  \labelsep  {.5em}
\setlength  \labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
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
\newcommand*\labelitemi{$\m@th\bullet$}
\newcommand*\labelitemii{\normalfont\bfseries --}
\newcommand*\labelitemiii{$\m@th\triangleright$}
\newcommand*\labelitemiv{$\m@th\cdot$}
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
\if@titlepage
 \newenvironment{abstract}{%
     \titlepage
     \null\vfil
     \@beginparpenalty\@lowpenalty
     \if@abstrt
     \begin{center}
       \sectfont \abstractname
       \@endparpenalty\@M
     \end{center}\fi}
    {\par\vfil\null\endtitlepage}
\else
 \newenvironment{abstract}{%
     \if@twocolumn\if@abstrt
       \section*{\abstractname}\fi%
     \else
       \if@abstrt
       \small
       \begin{center}%
         {\sectfont \abstractname\vspace{-.5em}\vspace{\z@}}%
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
\newcommand\marginline[1]{\marginpar[\raggedleft #1]{\raggedright #1}}
\newcommand*\appendix{\par\global\@altsecnumformattrue
  \setcounter{chapter}{0}%
  \setcounter{section}{0}%
  \renewcommand*\@chapapp{\appendixname}%
  \renewcommand*\thechapter{\@Alph\c@chapter}}
\setlength\arraycolsep{5\p@}
\setlength\tabcolsep{6\p@}
\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\skip\@mpfootins = \skip\footins
\setlength\fboxsep{3\p@}
\setlength\fboxrule{.4\p@}
\@addtoreset{equation}{chapter}
\renewcommand*\theequation{%
                          \thechapter.%
                          \@arabic\c@equation}
\newcounter{figure}[chapter]
\renewcommand*\thefigure{%
                        \thechapter.%
                        \@arabic\c@figure}
\def\fps@figure{tbp}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fnum@figure{\figureformat}
\newenvironment{figure}
               {\@float{figure}}
               {\end@float}
\newenvironment{figure*}
               {\@dblfloat{figure}}
               {\end@dblfloat}
\newcounter{table}[chapter]
\renewcommand*\thetable{%
                       \thechapter.%
                       \@arabic\c@table}
\def\fps@table{tbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\fnum@table{\tableformat}
\newenvironment{table}
               {\@float{table}}
               {\end@float}
\newenvironment{table*}
               {\@dblfloat{table}}
               {\end@dblfloat}
\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{10\p@}
\setlength\belowcaptionskip{0\p@}
\long\def\@makecaption#1#2{%
  \vskip\abovecaptionskip
  \sbox\@tempboxa{{\caplabelfont #1\captionformat}{\capfont #2}}%
  \ifdim \wd\@tempboxa >\hsize
    \@hangfrom{{\caplabelfont #1\captionformat}}{\capfont #2\par}
  \else
    \global \@minipagefalse
    \hb@xt@\hsize{\hfil\box\@tempboxa\hfil}%
  \fi
  \vskip\belowcaptionskip}
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareOldFontCommand{\sfb}{\normalfont\sffamily\bfseries}{\@nomath\sfb}
\DeclareRobustCommand*\cal{\@fontswitch\relax\mathcal}
\DeclareRobustCommand*\mit{\@fontswitch\relax\mathnormal}
\newcommand*\descfont{\sffamily\bfseries}
\newcommand*\sectfont{\sffamily\bfseries}
\newcommand*\pnumfont{\normalfont}
\newcommand*\headfont{\slshape}
\newcommand*\capfont{\normalfont}
\newcommand*\caplabelfont{\normalfont}
\newcommand*\@pnumwidth{1.55em}
\newcommand*\@tocrmarg {2.55em}
\newcommand*\@dotsep{4.5}
\setcounter{tocdepth}{2}
\newcommand*\toc@heading{\section*{\contentsname
  \@mkboth{\contentsname}{\contentsname}}}
\newcommand*\tableofcontents{%
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \chapter*{\contentsname
        \@mkboth{\contentsname}{\contentsname}}%
    \@starttoc{toc}%
    \if@restonecol\twocolumn\fi
    }
\newcommand*\l@part[2]{%
  \ifnum \c@tocdepth >-2\relax
    \addpenalty{-\@highpenalty}%
    \addvspace{2.25em \@plus\p@}%
    \setlength\@tempdima{2em}%
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
\newcommand*\lof@heading{
  \chapter*{\listfigurename
  \@mkboth{\listfigurename}{\listfigurename}}}
\newcommand*\listoffigures{%
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \lof@heading%
    \@starttoc{lof}%
    \if@restonecol\twocolumn\fi
    }
\newcommand*\l@figure{\@dottedtocline{1}{1.5em}{2.3em}}
\newcommand*\lot@heading{%
  \chapter*{\listtablename
  \@mkboth{\listtablename}{\listtablename}}}
\newcommand*\listoftables{%
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \lot@heading%
    \@starttoc{lot}%
    \if@restonecol\twocolumn\fi
    }
\let\l@table\l@figure
\newcommand*\bib@heading{%
  \chapter*{\bibname
    \@mkboth{\bibname}{\bibname}}}
\newdimen\bibindent
\bibindent=1.5em
\newenvironment{thebibliography}[1]{\bib@heading%
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
\newcommand*{\idx@heading}{\twocolumn[%
  \@makeschapterhead{\indexname}]%
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
                \idx@heading%
                \thispagestyle{plain}\parindent\z@
                \parskip\z@ \@plus .3\p@\relax
                \let\item\@idxitem}
               {\if@restonecol\onecolumn\else\clearpage\fi}
\newcommand*\@idxitem  {\par\hangindent 40\p@}
\newcommand*\subitem   {\@idxitem \hspace*{20\p@}}
\newcommand*\subsubitem{\@idxitem \hspace*{30\p@}}
\newcommand*\indexspace{\par \vskip 10\p@ \@plus5\p@ \@minus3\p@\relax}
\renewcommand*\footnoterule{%
  \kern-3\p@
  \hrule\@width.4\columnwidth
  \kern 2.6\p@}
\@addtoreset{footnote}{chapter}
\newcommand\deffootnote[4][]{%
  \long\def\@makefntext##1{%
    \edef\@tempa{#1}\ifx\@tempa\@empty
      \@setpar{\@@par
        \@tempdima = \hsize
        \addtolength{\@tempdima}{-#2}
        \parshape \@ne #2 \@tempdima}%
    \else
      \@setpar{\@@par
        \@tempdima = \hsize
        \addtolength{\@tempdima}{-#1}
        \@tempdimb = \hsize
        \addtolength{\@tempdimb}{-#2}
        \parshape \tw@ #1 \@tempdima #2 \@tempdimb}%
    \fi
    \par
    \parindent #3\noindent
    \hbox to \z@{\hss\@@makefnmark}##1}
  \def\@@makefnmark{\hbox{#4}}
}
\newcommand*\deffootnotemark[1]{\def\@makefnmark{\hbox{#1}}}
\def\thefootnotemark{\@thefnmark}
\let\textsuperscript\@textsuperscript
\deffootnote[1em]{1.5em}{1em}
  {\textsuperscript{\normalfont\thefootnotemark}}
\deffootnotemark{\textsuperscript{\normalfont\thefootnotemark}}
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
\newif\if@altsecnumformat\@altsecnumformatfalse
\AtEndDocument{\if@altsecnumformat\if@filesw
    \immediate\write\@auxout{\string\global\string\@altsecnumformattrue}
  \fi\fi}
\newcommand*\chaptermarkformat    {\thechapter\if@altsecnumformat.\fi\quad}
\newcommand*\sectionmarkformat    {\thesection\if@altsecnumformat.\fi\quad}
\newcommand*\partformat    {\partname~\thepart\if@altsecnumformat.\fi\ }
\newcommand*\chapterformat {\thechapter\if@altsecnumformat.\fi\ }
\newcommand*\figureformat  {\figurename~\thefigure%
  \if@altsecnumformat.\fi%
  }
\newcommand*\tableformat   {\tablename~\thetable%
  \if@altsecnumformat.\fi%
}
\newcommand*\captionformat {:\ }
\renewcommand*{\@seccntformat}[1]{%
  \csname the#1\endcsname\if@altsecnumformat.\fi\quad}
\setlength\columnsep{10\p@}
\setlength\columnseprule{0\p@}
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
\endinput
%%
%% End of file `scrreprt.cls'.
