%%
%% This is file `hebtech.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% hebtech.dtx  (with options: `hebtech')
%% 
%% This is a generated file.
%% 
%% Copyright 1997-1998 Boris Lavva and any individual authors
%% listed elsewhere in this file. All rights reserved.
%% 
%% 
%% This is a generated file.
%% 
%% Copyright 1989-2001 Johannes L. Braams and any individual authors
%% listed elsewhere in this file.  All rights reserved.
%% 
%% This file is part of the Babel system release 3.7.
%% --------------------------------------------------
%% 
%% It may be distributed under the terms of the LaTeX Project Public
%% License, as described in lppl.txt in the base LaTeX distribution.
%% Either version 1.2 or, at your option, any later version.
%% LaTeX2e hebtech class, derived by Boris Lavva from original hebtech
%% macros created for LaTeX 2.09.
%%
%% Copyright 1994 Irina Abramovici, Taub Computer Center, Technion ---
%% Israel Institute of Technology, e-mail ccaiire@tx.technion.ac.il.
%%
%% This package can be used for writing a thesis in the Hebrew/English
%% languages, at the Technion --- Israel Institute of Technology.
%%
%% Bugs reports and suggestions should be addressed to
%% Irina Abramovici, Taub Computer Center, Technion, tel: 04-8293690,
%% e-mail: ccaiire@tx.technion.ac.il
%% OR
%% to Boris Lavva, tel: 04-8294659, e-mail: lavva@tx.technion.ac.il.
%%
%% This package is an experimental meant to answer to the demands of
%% the Graduate School of the Technion --- Israel Institute of
%% Technology.
%%
\NeedsTeXFormat{LaTeX2e}[1996/06/01]
\ProvidesClass{hebtech}
        [1997/12/09 v.1.0a Technion thesis class]
\ExecuteOptions{a4paper,12pt,oneside,onecolumn,final,openany}
\DeclareOption{draft}{%
  \PassOptionsToPackage{all,light}{draftcopy}
  \PassOptionsToClass{draft}{report}
  \AtEndOfClass{\RequirePackage{draftcopy}}}
\DeclareOption*{%
  \PassOptionsToClass{\CurrentOption}{report}}
\ProcessOptions
\LoadClass{report}
\RequirePackage{setspace}
\AtBeginDocument{\doublespacing}
\RequirePackage[english,hebrew]{babel}
\RequirePackage{hebcal}
\renewcommand{\Hebrewdate}[3]{%
  \HebrewFromGregorian{#1}{#2}{#3}
     {\Hebrewday}{\Hebrewmonth}{\Hebrewyear}%
  \if@rl%           Hebrew (right-to-left) mode
     \FormatForHebrew{\Hebrewmonth}{\Hebrewyear}%
  \else%            English mode
     \FormatForEnglish{\Hebrewmonth}{\Hebrewyear}%
  \fi}
\renewcommand{\FormatForEnglish}[2]{%
  \HebrewMonthNameInEnglish{#1}{#2}, \space\number#2}
\renewcommand{\FormatForHebrew}[2]{%
  \HebrewMonthName{#1}{#2} \space\HebrewYearName{#2}}
\renewcommand{\HebrewMonthNameInEnglish}[2]{%
  \ifnum #1 = 7%
     \CheckLeapHebrewYear{#2}%
     \if@HebrewLeap ADAR II\else ADAR\fi%
  \else%
     \ifcase #1%
        % nothing for 0
        \or TISHREI%
        \or HESHVAN%
        \or KISLEV%
        \or TEBETH%
        \or SHEBAT%
        \or ADAR I%
        \or ADAR II%
        \or NISAN%
        \or IYAR%
        \or SIVAN%
        \or TAMMUZ%
        \or AV%
        \or ELUL%
     \fi
  \fi}
\newcommand\engmonth[1]{%
  \ifcase#1\or January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi}
\newcommand{\Gregoriandate}[3]{%
  \if@rl%
     \hebmonth{#2} \space{\beginL\number#3\endL}%
  \else%
     \engmonth{#2}, \space{\number#3}%
  \fi}
\newcount\dd \newcount\mm \newcount\yy
\dd=\number\day \mm=\number\month \yy=\number\year
\renewcommand{\date}[3]{\dd=#1 \mm=#2 \yy=#3}
\addto\captionsenglish{%
  \def\acknowledgmentname{Acknowledgment}%
  \def\bibname{References}%
  \def\continuedname{Cont'd}%
  \def\definitionname{Definition}%
  \def\theoremname{Theorem}%
  \def\lemmaname{Lemma}%
  \def\claimname{Claim}%
  \def\corollaryname{Corollary}%
  \def\axiomname{Axiom}%
  \def\assumptionname{Assumption}%
  \def\examplename{Example}%
  \def\propertyname{Property}%
  \def\remarkname{Remark}%
  \def\problemname{Problem}%
  \def\algorithmname{Algorithm}%
}
\addto\captionshebrew{%
  \def\acknowledgmentname{\he\kaf\resh\tav\ \tav\vav\dalet\he}%
  \def\bibname{\resh\shin\yod\mem\tav\ \mem\qof\vav\resh\vav\tav}%
  \def\continuedname{\he\mem\shin\finalkaf}%
  \def\definitionname{\he\gimel\dalet\resh\he}%
  \def\theoremname{\mem\shin\pe\tet}%
  \def\lemmaname{\lamed\mem\he}%
  \def\claimname{\tet\ayin\nun\he}%
  \def\corollaryname{\mem\samekh\qof\nun\he}%
  \def\axiomname{\alef\qof\samekh\yod\vav\mem\he}%
  \def\assumptionname{\he\nun\het\he}%
  \def\examplename{\dalet\vav\gimel\mem\he}%
  \def\propertyname{\tav\kaf\vav\nun\he}%
  \def\remarkname{\he\ayin\resh\he}%
  \def\problemname{\bet\ayin\yod\he}%
  \def\algorithmname{\alef\lamed\gimel\vav\resh\yod\tav\finalmem}%
}
\def\@makeschapterhead#1{%   % Heading for \chapter* command
  \vspace*{50\p@}%           % Space at top of text page.
  {\parindent \z@ \centering % Center the header
    \normalfont
    \interlinepenalty\@M
    \huge \bfseries #1\par   % Title.
    \nobreak                 % TeX penalty to prevent page break.
    \vskip 40\p@             % Space between title and text.
  }}
\renewcommand{\bchapternn}[2]{\chapter*{%
    \protect\if@rl #1\protect\else #2\protect\fi%
    \@mkboth{\MakeUppercase{\protect\if@rl #1\protect\else
                                           #2\protect\fi}}%
            {\MakeUppercase{\protect\if@rl #1\protect\else
                                           #2\protect\fi}}}
  \addcontentsline{toc}{chapter}{%
    \protect\hspace{-1.5em}\protect\numberline{}%
    \protect\if@rl #1\protect\else #2\protect\fi}
  \addcontentsline{cot}{chapter}{%
    \protect\hspace{-1.5em}\protect\numberline{}%
    \protect\if@rl #1\protect\else #2\protect\fi}}
\let\@@thebibliography\thebibliography
\def\thebibliography{%
  \addcontentsline{toc}{chapter}{\protect\hspace{-1.5em}\protect{%
      \bfseries\protect\numberline{}\protect\bibname}}
  \addcontentsline{cot}{chapter}{\protect\hspace{-1.5em}\protect{%
      \bfseries\protect\numberline{}\protect\bibname}}
  \@@thebibliography}
\newcommand{\bcaptionf}[2]{%
  \caption[\protect\if@rl \R{#1}\protect\else \L{#2}\protect\fi]{%
    \if@rl\R{#1} \protect\\ \L{Figure~\thefigure: #2}
    \else\L{#2} \protect\\ \R{\alef\yod\vav\resh~\thefigure: #1}
    \fi}}
\newcommand{\bcaptionff}[4]{%
  \caption[\protect\if@rl \R{#1}\protect\else \L{#2}\protect\fi]{%
    \if@rl\R{#3}\protect\\ \L{Figure~\thefigure: #4}
    \else\L{#4}\protect\\ \R{\alef\yod\vav\resh~\thefigure: #3}
    \fi}}
\newcommand{\bcaptiont}[2]{%
  \caption[\protect\if@rl \R{#1}\protect\else \L{#2}\protect\fi]{%
    \if@rl\R{#1} \protect\\ \L{Table~\thetable: #2}
    \else\L{#2} \protect\\ \R{\tet\bet\lamed\he~\thetable: #1}
    \fi}}
\newcommand{\bcaptiontt}[4]{%
  \caption[\protect\if@rl \R{#1}\protect\else \L{#2}\protect\fi]{%
    \if@rl\R{#3} \protect\\ \L{Table~\thetable: #4}
    \else\L{#4} \protect\\ \R{\tet\bet\lamed\he~\thetable: #3}
    \fi}}
\newcounter{thestype}
\newcounter{grade}
\def\title#1#2{\gdef\@@titleh{#1}\gdef\@@title{#2}}
\def\author#1#2{\gdef\@@authorh{#1}\gdef\@@author{#2}}
\def\dept#1#2{\gdef\@@depth{#1}\gdef\@@dept{#2}}
\def\dom#1#2{\gdef\@@domh{#1}\gdef\@@dom{#2}}
\def\advisori#1#2{\gdef\@@advisorih{#1}\gdef\@@advisori{#2}}
\def\advisorii#1#2{\gdef\@@advisoriih{#1}\gdef\@@advisorii{#2}}
\def\donori#1#2{\gdef\@@donorih{#1}\gdef\@@donori{#2}}
\def\donorii#1#2{\gdef\@@donoriih{#1}\gdef\@@donorii{#2}}
\def\donor#1#2{\gdef\@donorh{#1}\gdef\@donor{#2}}
\def\@@title{}
\def\@@author{}
\def\@@dom{}
\def\@@advisori{}
\def\@@advisorii{}
\def\@@donori{}
\def\@@donorii{}
\def\@donor{}
\def\@thestype{%
   \ifcase\value{thestype}%
   \or \MakeUppercase{Research Thesis}%
   \or \MakeUppercase{Project Thesis}%
   \or \MakeUppercase{Final Paper} \else {}\fi}
\def\@linei{%
  \MakeUppercase{Submitted in Partial Fulfillment
                 of the Requirements}}
\def\@lineii{%
   \ifcase\value{grade}%
        \or \MakeUppercase{For the Degree of Master of Science}
        \or \MakeUppercase{For the Degree of Master}
        \or \MakeUppercase{For the Degree of Master of Technology}
        \or \MakeUppercase{For the Degree of Doctor of Science}
        \else {}\fi}
\def\@lineiii{%
  \if\@@dom\else%
     \ifcase\value{grade}%
        \or \MakeUppercase{in} \MakeUppercase{\@@dom}%
        \or \MakeUppercase{in} \MakeUppercase{\@@dom}%
        \or \MakeUppercase{in} \MakeUppercase{\@@dom}%
        \else{}\fi%
  \fi}
\def\@lineiv{\mbox{\hspace{-.2in}\footnotesize\MakeUppercase{%
      Submitted to the Senate of the Technion --- %
      Israel Institute of Technology}}}
\def\@linev{\footnotesize{\Hebrewdate{\dd}{\mm}{\yy}%
            \hfill\MakeUppercase{Haifa}\hfill%
            \MakeUppercase{\Gregoriandate{\dd}{\mm}{\yy}}}}
\def\@linevi{\MakeUppercase{This}\
             \@thestype\
             \MakeUppercase{was supervised by}}
\def\@linevii{\MakeUppercase{under the auspices of the}}
\def\@donor{%
  \MakeUppercase{The generous financial help of}\ %
  \MakeUppercase{\@@donori}\ %
  \ifx\empty\@@donorii\else\MakeUppercase{and \@@donorii{} }\fi%
  \MakeUppercase{is gratefully acknowledged}}
\def\@@titleh{}
\def\@@authorh{}
\def\@@depth{}
\def\@@domh{}
\def\@@advisorih{}
\def\@@advisoriih{}
\def\@@donorih{}
\def\@@donoriih{}
\def\@donorh{}
\def\@thestypeh{%
   \ifcase\value{thestype}%
   \or \het\yod\bet\vav\resh\ \ayin\lamed\ \mem\het\qof\resh%
   \or \het\yod\bet\vav\resh\ \ayin\lamed\ \pe\resh\vav\yod\qof\tet%
   \or \ayin\bet\vav\dalet\tav\ \gimel\mem\resh \else {}\fi}
\def\@lineih{%
  \lamed\shin\finalmem\ \mem\yod\lamed\vav\yod\ \het\lamed\qof\yod\ %
  \shin\lamed\ \he\dalet\resh\yod\shin\vav\tav\ %
  \lamed\qof\bet\lamed\tav\ \tav\vav\alef\resh{}}
\def\@lineiih{%
   \ifcase\value{grade}%
        \or \mem\gimel\yod\samekh\tet\resh\ %
            \lamed\mem\dalet\ayin\yod\finalmem%
        \or \mem\gimel\yod\samekh\tet\resh%
        \or \mem\gimel\yod\samekh\tet\resh\ %
            \bet\tet\kaf\nun\vav\lamed\vav\gimel\yod\he%
        \or \dalet\qof\tet\vav\resh\ %
            \lamed\mem\dalet\ayin\yod\finalmem%
        \else {}\fi}
\def\@lineiiih{%
   \ifcase\value{grade}%
     \or \@@domh%
     \or \@@domh%
     \or \@@domh%
     \else{}\fi}
\def\@lineivh{\he\vav\gimel\shin\ \lamed\samekh\nun\tet\ %
             \he\tet\kaf\nun\yod\vav\finalnun\ \textemdash\ %
             \mem\kaf\vav\finalnun\ %
             \tet\kaf\nun\vav\lamed\vav\gimel\yod\ %
             \lamed\yod\shin\resh\alef\lamed}
\def\@linevh{\Hebrewdate{\dd}{\mm}{\yy}%
             \hfill\het\yod\pe\he\hfill%
             \Gregoriandate{\dd}{\mm}{\yy}}
\def\@linevih{%
   \ifcase\value{thestype}%
   \or \@thestypeh{} \nun\ayin\shin\he%
   \or \@thestypeh{} \nun\ayin\shin\he%
   \or \@thestypeh{} \nun\ayin\shin\tav\he\else{}\fi}
\def\@lineviih{\bet\he\dalet\resh\kaf\tav{}}
\def\@donorh{\alef\nun\yod\ \mem\vav\dalet\he{}
  \lamed\@@donorih{}
  \ifx\empty\@@donoriih\else\vav\lamed\@@donoriih{} \fi
  \ayin\lamed\ \he\tav\mem\yod\kaf\he\ \he\kaf\samekh\pe\yod\tav\ %
  \he\nun\dalet\yod\bet\he\ \bet\he\shin\tav\lamed\mem\vav\tav\yod}%
\newcommand\frontmatter{%
  \if@openright\cleardoublepage\else\clearpage\fi%
  \if@rl\pagenumbering{alph}\else\pagenumbering{roman}\fi}
\newcommand\mainmatter{%
  \if@openright\cleardoublepage\else\clearpage\fi%
  \pagenumbering{arabic}}
\let\backmatter\frontmatter
\newcommand{\makecover}{%
  \begin{titlepage}
    \thispagestyle{empty}
    \null\vskip.8truein
    \begin{doublespace}
      \begin{center}
        \bfseries\if@rl\Huge\@@titleh
        \else\LARGE\MakeUppercase{\@@title}\fi
        \vskip2.7truein
        \if@rl\huge\@@authorh
        \else\LARGE\MakeUppercase{\@@author}\fi
      \end{center}
    \end{doublespace}
    \vfill\null
  \end{titlepage}
  \thispagestyle{empty}}
\renewcommand{\maketitle}{%
  \if@openright\cleardoublepage\else\clearpage\fi
  \thispagestyle{empty}
  \begin{doublespace}
    \begin{center}
      \bfseries\if@rl\huge\@@titleh
      \else\Large\MakeUppercase{\@@title}\fi
    \end{center}
    \vfill
    \begin{center}
      \if@rl\Large\@thestypeh\else\large\@thestype\fi
    \end{center}
    \vfill
    \begin{center}
      \if@rl\large
         \@lineih\\
         \@lineiih\\
         \@lineiiih
      \else\large
         \@linei\\
         \@lineii\\
         \@lineiii\fi
    \end{center}
    \vfill%
    \begin{center}
      \bfseries\if@rl\LARGE\@@authorh
      \else\Large\MakeUppercase{\@@author}\fi
    \end{center}
    \vfill
    \begin{center}
      \if@rl\large
         \@lineivh\\
         \@linevh
      \else
         \@lineiv\\
         \@linev\fi
    \end{center}
  \end{doublespace}
  \newpage\thispagestyle{empty}}
\newenvironment{acknowledgment}{%
  \if@openright\cleardoublepage\else\clearpage\fi
  \thispagestyle{empty}%
  \begin{center}
    \if@rl\large
      \@linevih{}
      \@lineviih{}
      \@@advisorih{}
      \ifx\empty\@@advisoriih\else\vav\@@advisoriih{} \fi\\
      \@@depth
    \else
      \@linevi{}
      \MakeUppercase{\@@advisori}\ %
      \ifx\empty\@@advisorii\else\MakeUppercase{and \@@advisorii{} }\fi
      \@linevii{}
      \MakeUppercase{\@@dept}
    \fi
  \end{center}
  \vfill
  \begin{center}%
    \if@rl\LARGE\acknowledgmentname%
    \else\Large\MakeUppercase{\acknowledgmentname}\fi
  \end{center}
  \vspace{2\baselineskip}
  \begingroup\large}
{\endgroup\vfill
  \begin{center}
    \if@rl\large\@donorh\else\@donor\fi
  \end{center}
  \newpage\normalsize\thispagestyle{empty}}
\newenvironment{dedication}{%
  \thispagestyle{empty}%
  \vspace*{2\baselineskip}
  \begin{center}\large}
  {\end{center}
  \newpage\normalsize\thispagestyle{empty}}
\renewenvironment{abstract}{%
  \if@openright\cleardoublepage\else\clearpage\fi%
  \if@rl\bchapternn{\abstractname}{Hebrew Abstract}
  \else\bchapternn{\tav\qof\tsadi\yod\resh\ %
                   \bet\alef\nun\gimel\lamed\yod\tav}
                  {\abstractname}\fi%
  \begingroup}{%
  \newpage\endgroup}
\def\ps@thesis{%
  \def\headeven{\protect\rule[-4pt]{\textwidth}{.5pt}\kern-\textwidth%
                \protect\if@rl\beginR\thepage\hfil{%
                              \bfseries\rightmark}\endR%
                \protect\else\thepage\hfil{%
                             \bfseries\leftmark}\protect\fi}
  \def\headodd{\protect\rule[-4pt]{\textwidth}{.5pt}\kern-\textwidth%
               \protect\if@rl\beginR{\bfseries\leftmark}%
                             \hfil\thepage\endR%
               \protect\else{\bfseries\rightmark}%
                             \hfil\thepage\protect\fi}
  \ps@headings}
\setlength{\textheight}{8.25in}
\setlength{\textwidth}{6.0in}
\setlength{\topmargin}{0pt}
\setlength{\headsep}{.5in}
\addtolength{\headsep}{-\headheight}
\setlength{\oddsidemargin}{.25in}
\setlength{\evensidemargin}{.25in}
\setlength{\marginparwidth}{40pt}
\setlength{\marginparsep}{10pt}
\brokenpenalty=10000
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
\endinput
%%
%% End of file `hebtech.cls'.
