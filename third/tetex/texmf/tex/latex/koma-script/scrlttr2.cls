%%
%% This is file `scrlttr2.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% scrclass.dtx  (with options: `class,scrlettr')
%% scrlogo.dtx 
%% 
%% Copyright (C) 1994-2002 Markus Kohm.
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
  scrlttr2%
  }
  [2003/01/31 v2.9n LaTeX2e KOMA
    document class]
\RequirePackage{keyval}
\newcommand*{\KOMA@key}{\define@key{KOMA}}
\newcommand*{\KOMAoptions}[1]{%
  \@ifundefined{scr@save@KV@errx}{%
    \let\scr@save@KV@errx=\KV@errx
    \renewcommand*{\KV@errx}[1]{%
      \ClassError{%
        scrlttr2%
        }{unknown KOMA option `#1'}{%
        You've tried to use the KOMA option `#1'.\MessageBreak
        But there is no KOMA option with that name.\MessageBreak
        See the manual of KOMA-Script for information about known koma
        options.}}%
    \setkeys{KOMA}{#1}%
    \let\KV@errx=\scr@save@KV@errx
    \let\scr@save@KV@errx=\relax
  }{%
    \setkeys{KOMA}{#1}%
  }\ignorespaces%
}
\newcommand*{\AfterProcessOptions@hook}{}
\newcommand*{\AfterProcessOptions}[1]{%
  \g@addto@macro{\AfterProcessOptions@hook}{#1}%
}
\newcommand*{\@setif}[3][]{%
  \begingroup%
    \edef\@tempa{#1}
    \ifx\@tempa\@empty\def\@tempc{@#2}\else\def\@tempc{#1}\fi
    \edef\@tempa{#3}\def\@tempb{on}\ifx\@tempa\@tempb
      \def\@tempa{true}%
    \else\def\@tempb{off}\ifx\@tempa\@tempb
      \def\@tempa{false}%
    \fi\fi%
    \@ifundefined{\@tempc\@tempa}{%
      \ClassError{%
        scrlttr2%
        }{You've used an unknown switch or value}{%
        You've tried to set switch `#2' to value `#3', but there's
        either no\MessageBreak%
        switch `#2' defined or `#3' is not a switch
        value.\MessageBreak%
        Allowed switch values are `true', `false', `on',
        `off'\@setif@alsoerr.}
      \let\@tempa\endgroup
      }{%
      \edef\@tempa{\endgroup\noexpand\@nameuse{\@tempc\@tempa}}
      }
  \@tempa
  \let\@setif@alsoerr=\@empty
  }
\newcommand*{\@setif@alsoerr}{}
\newcommand*{\@setxif}[5][]{%
  \begingroup%
    \edef\@tempa{#1}
    \ifx\@tempa\@empty\def\@tempc{@#2}\else\def\@tempc{#1}\fi
    \@ifundefined{\@tempc true}{%
      \ClassError{%
        scrlttr2%
        }{You've used an unknown switch}{%
        You've tried to set switch `#2' to value `#3', but
        there's no\MessageBreak
        switch `#2' defined.}
        \let\@tempa\endgroup
      }{%
      \edef\@tempa{#3}\edef\@tempb{#4}\ifx\@tempa\@tempb
        \edef\@tempa{\endgroup\noexpand\@nameuse{\@tempc true}}
      \else\def\@tempb{#5}\ifx\@tempa\@tempb
        \edef\@tempa{\endgroup\noexpand\@nameuse{\@tempc false}}
      \else
        \ClassError{%
          scrlttr2%
          }{You've used an unknown switch value}{%
          You've tried to set switch `#2' to value `#3', but allowed
          switch\MessageBreak
          values are `#4', `#5'\@setif@alsoerr\space only.}
        \let\@tempa\endgroup
      \fi\fi%
      }
  \@tempa
  \let\@setif@alsoerr=\@empty
  }
\newcommand*{\@setnumstate}[4]{%
  \begingroup
    \edef\@searched{#4}%
    \let\@tempa=\@empty
    \let\@tempb=\@empty
    \@tempswafalse
    \@for\@valuelist:=#3\do{%
      \if@tempswa\else
        \edef\@tempc{\expandafter\@firstoftwo\@valuelist}%
        \edef\@tempa{\@tempa\@tempb`\@tempc'}%
        \edef\@tempb{, }%
        \ifx\@tempc\@searched
          \@tempswatrue
          \edef\@tempa{\expandafter\@secondoftwo\@valuelist}%
        \fi
      \fi
      }
    \if@tempswa
      \edef\@tempa{\endgroup
        \noexpand\renewcommand*{\noexpand#1}{\@tempa}}
    \else
      \ClassError{%
        scrlttr2%
        }{You've used an unknown option value}{%
        You've tried to set option `#2' to value `#4',
        but allowed values\MessageBreak
        are only:\MessageBreak
        \@tempa.}%
        \let\@tempa=\endgroup
    \fi
  \@tempa
  }
\newcommand*{\@ObsoleteOption}[1]{%
  \ClassWarningNoLine{%
    scrlttr2%
    }{%
    You've used the obsolete option `\CurrentOption'.\MessageBreak
    I'm using option `#1' instead.\MessageBreak
    You should do this change also}
  \KOMAoptions{#1}
}
\newcommand*{\@ObsoleteCommand}[2]{%
  \ClassWarning{%
    scrlttr2%
    }{%
    You`ve used the obsolete command `\string#1'.\MessageBreak
    I`m using `\string\KOMAoptions{#2}' instead.\MessageBreak
    You should do this change also}
  \KOMAoptions{#2}
}
\KOMA@key{BCOR}{\PassOptionsToPackage{BCOR#1}{typearea}}
\KOMA@key{DIV}{\PassOptionsToPackage{DIV#1}{typearea}}
\KOMA@key{headlines}{\PassOptionsToPackage{#1headlines}{typearea}}
\KOMA@key{paper}{\PassOptionsToPackage{#1paper}{typearea}}
\DeclareOption{a4paper}{\@ObsoleteOption{paper=a4}}
\DeclareOption{a5paper}{\@ObsoleteOption{paper=a5}}
\DeclareOption{b5paper}{\@ObsoleteOption{paper=b5}}
\DeclareOption{letterpaper}{\@ObsoleteOption{paper=letter}}
\DeclareOption{legalpaper}{\@ObsoleteOption{paper=legal}}
\DeclareOption{executivepaper}{\@ObsoleteOption{paper=executive}}
\newif\if@hsl
\KOMA@key{headsepline}[true]{\@setif{hsl}{#1}}%
\AfterProcessOptions{%
  \if@hsl\PassOptionsToPackage{headinclude}{typearea}\fi}
\DeclareOption{headnosepline}{%
  \@ObsoleteOption{headsepline=false}%
  }
\newif\if@fsl
\KOMA@key{footsepline}[true]{\@setif{fsl}{#1}}%
\AfterProcessOptions{%
  \if@fsl\PassOptionsToPackage{footinclude}{typearea}\fi}
\DeclareOption{footnosepline}{%
  \@ObsoleteOption{footsepline=false}%
  }
\newif\if@mincl
\DeclareOption{mpinclude}{%
  \@mincltrue
  \ClassWarning{scrlttr2}{%
    You should not use option `mpinclude' with this\MessageBreak
    class}
  }
\DeclareOption{mpexclude}{
  \@minclfalse
  \ClassWarning{scrlttr2}{%
    You should not use option `mpexclude' with this\MessageBreak
    class}
  }
\newcommand*{\@alignfrom}{0}
\KOMA@key{fromalign}{%
  \@setnumstate{\@alignfrom}{fromalign}{%
    {left}{0},%
    {center}{1},{centered}{1},{middle}{1},%
    {right}{2},%
    {no}{10000},{false}{10000},{off}{10000}}{#1}%
  }
\newcommand*{\@rulefrom}{0}
\KOMA@key{fromrule}[below]{%
  \@setnumstate{\@rulefrom}{fromrule}{%
    {false}{0},{no}{0},{off}{0},{aftername}{1},%
    {below}{2},{afteraddress}{2},{true}{2},{yes}{2},{on}{2}%
    }{#1}%
  }
\newif\if@phone
\KOMA@key{fromphone}[true]{\@setif{phone}{#1}}%
\newif\if@fax
\KOMA@key{fromfax}[true]{\@setif{fax}{#1}}%
\newif\if@email
\KOMA@key{fromemail}[true]{\@setif{email}{#1}}%
\newif\if@www
\KOMA@key{fromurl}[true]{\@setif{www}{#1}}%
\newif\if@logo
\KOMA@key{fromlogo}[true]{\@setif{logo}{#1}}%
\newcommand*{\@pageat}{4}
\KOMA@key{pagenumber}{%
  \begingroup
    \@setnumstate{\@tempa}{pagenumber}{%
      {no}{-1},{off}{-1},{false}{-1},%
      {topleft}{0},{headleft}{0},%
      {topmiddle}{1},{topcenter}{1},{topcentered}{1},%
      {headmiddle}{1},{headcenter}{1},{headcentered}{1},%
      {topright}{2},{headright}{2},%
      {botleft}{3},{footleft}{3},%
      {botmiddle}{4},{botcenter}{4},{botcentered}{4},%
      {footmiddle}{4},{footcenter}{4},{footcentered}{4},%
      {botright}{5},{footright}{5},%
      {top}{10},{head}{10},{bot}{11},{foot}{11},%
      {left}{20},{middle}{21},{center}{21},{centered}{21},{right}{22}%
      }{#1}%
    \ifnum\@tempa<10
      \edef\@tempa{\endgroup\edef\noexpand\@pageat{\@tempa}}%
    \else
      \ifnum\@tempa<20
        \@tempcnta\@pageat\relax
        \ifnum\@tempcnta>2
          \advance\@tempcnta by -3
        \fi
        \ifnum\@tempa=11
          \advance\@tempcnta by 3
        \fi
        \edef\@tempa{\endgroup\edef\noexpand\@pageat{\the\@tempcnta}}%
      \else
        \ifnum\@pageat>2
          \edef\@pageat{3}%
        \else
          \edef\@pageat{0}
        \fi
        \@tempcnta\@tempa
        \advance\@tempcnta by -20
        \advance\@tempcnta by\@pageat
        \edef\@tempa{\endgroup\edef\noexpand\@pageat{\the\@tempcnta}}%
      \fi
    \fi
  \@tempa
  }
\newcommand*\@ptsize{%
  2%
  }
\KOMA@key{fontsize}{%
  \begingroup
    \setlength{\@tempdima}{#1}
    \addtolength{\@tempdima}{-10\p@}
    \edef\@tempa{\endgroup\noexpand\renewcommand*\noexpand\@ptsize{%
        \strip@pt\@tempdima}}
  \@tempa
  }
\DeclareOption{10pt}{%
  \@ObsoleteOption{fontsize=10pt}
  }
\DeclareOption{11pt}{%
  \@ObsoleteOption{fontsize=11pt}
  }
\DeclareOption{12pt}{%
  \@ObsoleteOption{fontsize=12pt}
  }
\KOMA@key{twoside}[true]{\@setif{twoside}{#1}%
  \if@twoside\@mparswitchtrue\else\@mpseswitchfalse\fi}
\DeclareOption{oneside}{%
  \@ObsoleteOption{twoside=false}
  }
\KOMA@key{draft}[true]{%
  \begingroup
    \let\@drafttrue=\@tempswatrue
    \let\@draftfalse=\@tempswafalse
    \@setif{draft}{#1}
    \if@tempswa
      \global\setlength{\overfullrule}{5\p@}
    \else
      \global\setlength{\overfullrule}{\z@}
    \fi
  \endgroup
  }
\DeclareOption{final}{%
  \@ObsoleteOption{draft=false}
  }
\newif\if@enlargefp
\KOMA@key{enlargefirstpage}[true]{%
  \@setif[@enlargefp]{enlargefirstpage}{#1}}
\newif\if@parskip
\newcommand*{\scr@parskip}{\setlength{\parskip}{\z@}}
\newcommand*{\scr@parfillskip}{\setlength{\parfillskip}{\z@}}
\newcommand*{\@parskip@indent}{}
\KOMA@key{parskip}[true]{%
  \begingroup%
    \@tempcnta=-1\edef\@tempa{#1}%
    \def\@tempb{false}\ifx\@tempa\@tempb\@tempcnta=0\fi
    \def\@tempb{off}\ifx\@tempa\@tempb\@tempcnta=0\fi
    \def\@tempb{full-}\ifx\@tempa\@tempb\@tempcnta=1\fi
    \def\@tempb{half-}\ifx\@tempa\@tempb\@tempcnta=2\fi
    \def\@tempb{full}\ifx\@tempa\@tempb\@tempcnta=3\fi
    \def\@tempb{true}\ifx\@tempa\@tempb\@tempcnta=3\fi
    \def\@tempb{on}\ifx\@tempa\@tempb\@tempcnta=3\fi
    \def\@tempb{half}\ifx\@tempa\@tempb\@tempcnta=4\fi
    \def\@tempb{full+}\ifx\@tempa\@tempb\@tempcnta=5\fi
    \def\@tempb{half+}\ifx\@tempa\@tempb\@tempcnta=6\fi
    \def\@tempb{full*}\ifx\@tempa\@tempb\@tempcnta=7\fi
    \def\@tempb{half*}\ifx\@tempa\@tempb\@tempcnta=8\fi
    \let\@tempa\endgroup
    \ifnum \@tempcnta <\z@\relax
      \ClassError{%
        scrlttr2%
        }{You've used an unknown option value}{%
        You've tried to set option `parskip' to value `#1', but
        allowed values\MessageBreak
        are `full-', `full', `full+', `full*', `half-',
        `half', `half+',\MessageBreak
        `half*', `false' and `off' only.\MessageBreak
        You may also use `on' instead of `full'.}%
    \else\ifnum \@tempcnta =\z@
        \def\@tempa{\endgroup\@parskipfalse\@parskip@indent}%
      \else
        \let\@tempb=\relax\let\@tempc=\relax
        \ifodd \@tempcnta
          \def\@tempb{\def\noexpand\scr@parskip{%
              \noexpand\setlength{\parskip}{\baselineskip}%
              \noexpand\addtolength{\parskip}{%
                \z@ \@plus .1\baselineskip}}}%
        \else
          \def\@tempb{\def\noexpand\scr@parskip{%
            \noexpand\setlength{\parskip}{.5\baselineskip}%
            \noexpand\addtolength{\parskip}{%
              \z@ \@plus .5\baselineskip}}}%
        \fi
        \advance\@tempcnta by \m@ne
        \divide\@tempcnta by \tw@
        \ifcase\@tempcnta
          \def\@tempc{\def\noexpand\scr@parfillskip{%
            \noexpand\setlength{\parfillskip}{\z@ \@plus 1fil}}}%
        \or
          \def\@tempc{\def\noexpand\scr@parfillskip{%
            \noexpand\setlength{\parfillskip}{1em \@plus 1fil}}}%
        \or
          \def\@tempc{\def\noexpand\scr@parfillskip{%
            \noexpand\setlength{\parfillskip}{.3333\linewidth}%
            \noexpand\addtolength{\parfillskip}{\z@ \@plus 1fil}}}%
        \or
          \def\@tempc{\def\noexpand\scr@parfillskip{%
            \noexpand\setlength{\parfillskip}{.25\linewidth}%
            \noexpand\addtolength{\parfillskip}{\z@ \@plus 1fil}}}%
        \fi
        \edef\@tempa{%
          \endgroup\noexpand\@parskiptrue\@tempb\@tempc%
          \noexpand\@parskip@indent}%
      \fi
    \fi
  \@tempa
  }
\newcommand*{\cleardoublestandardpage}{}
\let\cleardoublestandardpage\cleardoublepage
\newcommand*{\cleardoubleemptypage}{\clearpage%
  {\thispagestyle{empty}\cleardoublestandardpage}}
\newcommand*{\cleardoubleplainpage}{\clearpage%
  {\thispagestyle{plain}\cleardoublestandardpage}}
\DeclareOption{cleardoubleempty}{%
  \@ObsoleteOption{cleardoublepage=empty}}
\DeclareOption{cleardoubleplain}{%
  \@ObsoleteOption{cleardoublepage=plain}}
\DeclareOption{cleardoublestandard}{%
  \@ObsoleteOption{cleardoublepage=standard}}
\KOMA@key{cleardoublepage}{%
  \begingroup%
    \def\@tempc{%
      \endgroup%
      \ClassError{%
        scrlttr2%
        }{You've used an unknown switch value}{%
        You've tried to set switch `cleardoublepage' to value
        `#1', but\MessageBreak
        allowed switch values are `emtpy', `plain' and
        `standard' only.}}
    \edef\@tempa{#1}%
    \def\@tempb{empty}\ifx\@tempa\@tempb%
      \def\@tempc{\endgroup%
        \let\cleardoublepage\cleardoubleemptypage}\fi
    \def\@tempb{plain}\ifx\@tempa\@tempb%
      \def\@tempc{\endgroup%
        \let\cleardoublepage\cleardoubleplainpage}\fi
    \def\@tempb{standard}\ifx\@tempa\@tempb%
      \def\@tempc{\endgroup%
        \let\cleardoublepage\cleardoublestandardpage}\fi
  \@tempc}
\newif\if@afield\@afieldtrue
\KOMA@key{addrfield}[true]{\@setif[@afield]{addrfield}{#1}}
\newif\if@baddr\@baddrtrue
\KOMA@key{backaddress}[true]{\@setif[@baddr]{backaddress}{#1}}
\newif\if@subj
\newif\if@subjafter
\KOMA@key{subject}[true]{%
  \begingroup
    \edef\@tempa{#1}%
    \def\@tempb{afteropening}%
    \ifx\@tempa\@tempb\endgroup\@subjaftertrue
    \else
      \def\@tempb{beforeopening}%
      \ifx\@tempa\@tempb\endgroup\@subjafterfalse
      \else
        \endgroup\def\@setif@alsoerr{, `afteropening' and
          `beforeopening'\MessageBreak}%
        \@setxif[@subj]{subject}{#1}{titled}{untitled}%
      \fi
    \fi
  }
\newif\if@bigloc
\KOMA@key{locfield}[wide]{\@setxif[@bigloc]{locfield}{#1}{wide}{narrow}}
\DeclareOption{wlocfield}{\@ObsoleteOption{locfield=wide}}
\DeclareOption{slocfield}{\@ObsoleteOption{locfield=narrow}}
\newif\if@fold\@foldtrue
\KOMA@key{foldmarks}[true]{\@setif[@fold]{foldmarks}{#1}}
\newif\if@orgdate\@orgdatetrue
\KOMA@key{numericaldate}[true]{%
  \if@orgdate\@orgdatefalse\else\@orgdatetrue\fi
  \@setif[@orgdate]{numericaldate}{#1}
  \if@orgdate\@orgdatefalse\else\@orgdatetrue\fi
}
\DeclareOption{scrdate}{\@ObsoleteOption{numericaldate=true}}
\DeclareOption{orgdate}{\@ObsoleteOption{numericaldate=false}}
\newif\if@refwide
\KOMA@key{refline}[wide]{\@setxif[@refwide]{refline}{#1}{wide}{narrow}}
  \DeclareOption*{%
    \let\scr@save@KV@errx=\KV@errx%
    \IfFileExists{\CurrentOption.lco}{%
      \begingroup\edef\@tempa{\endgroup%
        \noexpand\AtEndOfClass{%
          \noexpand\LoadLetterOption{\CurrentOption}}%
        }%
      \@tempa\let\KV@errx\@gobble
      }{%
      \renewcommand*{\KV@errx}[1]{\OptionNotUsed}
      }
    \expandafter\KOMAoptions\expandafter{\CurrentOption}
    \let\KV@errx=\scr@save@KV@errx%
    \let\scr@save@KV@errx=\relax%
    }
\ExecuteOptions{%
}
\ProcessOptions\relax
\AfterProcessOptions@hook
\let\AfterProcessOptions@hook=\undefined
\RequirePackage{scrlfile}
\newif\if@restonecol
\@tempcnta=10
\advance\@tempcnta by\@ptsize\relax
\xdef\@tempa{%
size%
\the\@tempcnta.clo}
\InputIfFileExists{\@tempa}{}{%
    \ClassError
    {scrlttr2}
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
\KOMA@key{fontsize}{%
  \begingroup
    \setlength{\@tempdima}{#1}
    \addtolength{\@tempdima}{-10\p@}
    \ifnum \@tempdima=\@ptsize
    \else
      \ClassError{%
        scrlttr%
      }{option `fontsize' not allowed\MessageBreak
      after loading sizeX.clo file}{%
      Maybe you've used option `fontsize' at
      \string\KOMAoptions.\MessageBreak
      You should set this option at \string\documentclass.}
    \fi
  \endgroup
  }
\newcommand*{\scr@file@date@version}{}
\def\scr@file@date@version#1/#2/#3 #4 #5\@nil{#1/#2/#3 #4}
\RequirePackage{typearea}[\expandafter\expandafter\expandafter%
  \scr@file@date@version\csname ver@%
  scrlttr2%
  .cls\endcsname\@nil]
\if@twoside
  \addtolength{\oddsidemargin}{\evensidemargin}
  \addtolength{\oddsidemargin}{-\ta@bcor}
  \setlength{\evensidemargin}{.5\oddsidemargin}
  \setlength{\oddsidemargin}{\evensidemargin}
  \addtolength{\oddsidemargin}{\ta@bcor}
\fi
\KOMA@key{BCOR}{\typearea[{#1}]{last}}
\KOMA@key{DIV}{\typearea[current]{#1}}
\KOMA@key{headlines}{%
  \def\ta@hh{#1\baselineskip}%
  \typearea[current]{last}}
\KOMA@key{paper}{%
  \begingroup
    \edef\@tempc{\noexpand\@tempswatrue%
      \noexpand\ifdim\paperwidth=\the\paperwidth\noexpand\else%
      \noexpand\@tempswafalse\noexpand\fi%
      \noexpand\ifdim\paperheight=\the\paperheight\noexpand\else%
      \noexpand\@tempswafalse\noexpand\fi%
      }
    \edef\@tempa{#1}%
    \def\@tempb{letter}\ifx\@tempa\@tempb%
      \setlength{\paperheight}{11in}\setlength{\paperwidth}{8.5in}%
    \else\def\@tempb{legal}\ifx\@tempa\@tempb%
      \setlength{\paperheight}{14in}\setlength{\paperwidth}{8.5in}%
    \else\def\@tempb{executive}\ifx\@tempa\@tempb%
      \setlength{\paperheight}{10.5in}\setlength{\paperwidth}{7.25in}%
    \else%
      \def\ta@opt@err{%
        \ClassError{%
          scrlttr2%
          }{papersize `#1' undefined}{%
          You've told me to set KOMA option `paper' to value
          `#1',\MessageBreak
          but the value `#1' is not allowed to this
          option.\MessageBreak
          See the KOMA-Script manual for informations about known
          papersizes.}%
        }
      \let\@headlines\ta@opt@err%
      \x@@paper #1paperpaper\x@@paper%
    \fi\fi\fi
    \@tempc%
    \if@tempswa%
      \ClassInfo{%
        scrlttr2%
        }{%
        change to already set paper size ignored%
        }
      \let\@tempc\endgroup%
    \else%
      \ClassWarning{%
        scrlttr2%
        }{%
        you're changing paper size after loading package\MessageBreak
        `typearea'. This may be dangerous.\MessageBreak
        You should set paper size only at \string\documentclass\space
        not\MessageBreak
        with \string\KOMAoptions%
        }
      \edef\@tempc{\endgroup%
        \setlength{\paperwidth}{\the\paperwidth}%
        \setlength{\paperheight}{\the\paperheight}%
        \noexpand\typearea[current]{last}}
    \fi
  \@tempc%
  }
\setlength{\lineskip}{\p@}
\setlength{\normallineskip}{\p@}
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
\renewcommand*{%
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
\newcommand*{\@firsthead}{\noindent%
  \parbox[b]{\useplength{firstheadwidth}}
  {\centering\usekomafont{fromaddress}{%
      \ifkomavarempty{fromname}{}{%
        {\usekomafont{fromname}%
          {\ignorespaces\usekomavar{fromname}}}\\[-8pt]
        \rule{\useplength{firstheadwidth}}{.4pt}\\
        }%
      \ifkomavarempty{fromaddress}{}{%
        \ignorespaces\usekomavar{fromaddress}%
        }%
      }%
    }%
  }
\newcommand*{\@gen@firsthead}{%
  \ifcase\@alignfrom
    \renewcommand*{\@firsthead}{\noindent
      \parbox[b]{\textwidth}{\raggedright
        {\usekomafont{fromname}\strut\ignorespaces
          \usekomavar{fromname}%
          {%
            \ifnum\@rulefrom=1\\[-.5\baselineskip]%
              \setlength{\@tempdima}{\useplength{fromrulewidth}}%
              \ifdim\@tempdima=\z@
                \setlength{\@tempdima}{\textwidth}%
                \if@logo
                  \settowidth{\@tempdimb}{\usekomavar{fromlogo}}%
                  \addtolength{\@tempdima}{-\@tempdimb}%
                \fi
              \fi
              \expandafter\rule\expandafter{\the\@tempdima}{.4pt}%
            \fi
            }%
          \\}%
        {\usekomafont{fromaddress}\strut\ignorespaces
          \usekomavar{fromaddress}%
          \if@phone\\\usekomavar*{fromphone}\usekomavar{fromphone}\fi%
          \if@fax\\\usekomavar*{fromfax}\usekomavar{fromfax}\fi%
          \if@email\\\usekomavar*{fromemail}\usekomavar{fromemail}\fi%
          \if@www\\\usekomavar*{fromurl}\usekomavar{fromurl}\fi%
          \ifnum\@rulefrom=2\\[-.5\baselineskip]%
            \setlength{\@tempdima}{\useplength{fromrulewidth}}%
            \ifdim\@tempdima=\z@
              \setlength{\@tempdima}{\textwidth}%
            \fi
            \expandafter\rule\expandafter{\the\@tempdima}{.4pt}%
          \fi
          \\
          }%
        }%
      \if@logo
        \llap{\usekomavar{fromlogo}}%
      \fi
      }%
  \or
    \renewcommand*{\@firsthead}{\noindent
      \parbox[b]{\textwidth}{\centering
        {\usekomafont{fromname}\strut\ignorespaces
          \usekomavar{fromname}%
          {%
            \ifnum\@rulefrom=1\\[-.5\baselineskip]%
              \setlength{\@tempdima}{\useplength{fromrulewidth}}%
              \ifdim\@tempdima=\z@
                \setlength{\@tempdima}{\textwidth}%
              \fi
              \expandafter\rule\expandafter{\the\@tempdima}{.4pt}%
            \fi
            }%
          \\
          }%
        {\usekomafont{fromaddress}\strut\ignorespaces
          \usekomavar{fromaddress}%
          \if@phone\\\usekomavar*{fromphone}\usekomavar{fromphone}\fi%
          \if@fax\\\usekomavar*{fromfax}\usekomavar{fromfax}\fi%
          \if@email\\\usekomavar*{fromemail}\usekomavar{fromemail}\fi%
          \if@www\\\usekomavar*{fromurl}\usekomavar{fromurl}\fi%
          \ifnum\@rulefrom=2\\[-.5\baselineskip]%
            \setlength{\@tempdima}{\useplength{fromrulewidth}}%
            \ifdim\@tempdima=\z@
              \setlength{\@tempdima}{\textwidth}%
            \fi
            \expandafter\rule\expandafter{\the\@tempdima}{.4pt}%
          \fi
          \\
          }%
        }%
      }%
  \or
    \renewcommand*{\@firsthead}{\noindent
      \if@logo
        \rlap{\usekomavar{fromlogo}}%
      \fi
      \parbox[b]{\textwidth}{\raggedleft
        {\usekomafont{fromname}\strut\ignorespaces
          \usekomavar{fromname}%
          {%
            \ifnum\@rulefrom=1\\[-.5\baselineskip]%
              \setlength{\@tempdima}{\useplength{fromrulewidth}}%
              \ifdim\@tempdima=\z@
                \setlength{\@tempdima}{\textwidth}%
                \if@logo
                  \settowidth{\@tempdimb}{\usekomavar{fromlogo}}%
                  \addtolength{\@tempdima}{-\@tempdimb}%
                \fi
              \fi
              \expandafter\rule\expandafter{\the\@tempdima}{.4pt}%
            \fi
            }%
          \\
          }%
        {\usekomafont{fromaddress}\strut\ignorespaces
          \usekomavar{fromaddress}%
          \if@phone\\\usekomavar*{fromphone}\usekomavar{fromphone}\fi%
          \if@fax\\\usekomavar*{fromfax}\usekomavar{fromfax}\fi%
          \if@email\\\usekomavar*{fromemail}\usekomavar{fromemail}\fi%
          \if@www\\\usekomavar*{fromurl}\usekomavar{fromurl}\fi%
          \ifnum\@rulefrom=2\\[-.5\baselineskip]%
            \setlength{\@tempdima}{\useplength{fromrulewidth}}%
            \ifdim\@tempdima=\z@
              \setlength{\@tempdima}{\textwidth}%
            \fi
            \expandafter\rule\expandafter{\the\@tempdima}{.4pt}%
          \fi
          \\
          }%
        }%
      }%
  \fi
  }
\newcommand*{\@firstfoot}{}
\newcommand*{\@nexthead}{%
  \rlap{\parbox[c]{\textwidth}{\raggedright\strut
      \ifnum\@pageat=0\pagemark\else\leftmark\fi\strut}}%
  \parbox[c]{\textwidth}{%
    \centering\strut
    \ifcase\@pageat
      \rightmark
    \or
      \pagemark
    \or
      \rightmark
    \fi\strut}%
  \llap{\parbox[c]{\textwidth}{\raggedleft\strut
      \ifcase\@pageat
        \leftmark
      \or
        \rightmark
      \or
        \pagemark
      \else
        \rightmark
      \fi\strut}}%
  }%
\newcommand*{\@nextfoot}{%
  \ifnum\@pageat>2
    \parbox[c]{\textwidth}{%
      \ifcase\@pageat\or\or\or\raggedright\or\centering\or\raggedleft\fi
      \strut\pagemark%
      }%
  \fi
}
\newcommand*{\pagemark}{{\usekomafont{pagenumber}\pagename\ \thepage}}
\renewcommand*{\ps@plain}{%
  \renewcommand*{\@evenhead}{%
    \vbox{\vbox{\hsize=\textwidth\hbox to\textwidth{%
          \parbox[b]{\textwidth}{\strut
            \ifnum\@pageat>-1
              \ifnum\@pageat<3
                \ifcase\@pageat\raggedright\or\centering\or\raggedleft\fi
                \pagemark
              \else
                \hfill
              \fi
            \else
              \hfill
            \fi}%
          }%
        \if@hsl\kern1pt\rule{\textwidth}{.4pt}\fi
        }%
      }%
    }%
  \let\@oddhead\@evenhead%
  \renewcommand*{\@evenfoot}{%
    \parbox[t]{\textwidth}{%
      \if@fsl
        {%
          \raggedright%
          \vskip-\baselineskip\vskip.4pt
          \hrulefill\\
          }%
      \fi
      \ifnum\@pageat>2
        \ifcase\@pageat\or\or\or\raggedright\or\centering\or\raggedleft\fi
        \strut\pagemark
      \else
        \hfill
      \fi
      }%
    }%
  \let\@oddfoot\@evenfoot
  }
\newcommand*{\ps@headings}{%
  \let\@mkboth=\markboth
  \renewcommand*{\@evenhead}{%
    \vbox{%
      \vbox{\hsize=\textwidth\hbox to\textwidth{\headfont\@nexthead}}%
      \if@hsl\kern1pt\rule{\textwidth}{.4pt}\fi%
      }%
    }%
  \let\@oddhead\@evenhead%
  \renewcommand*{\@evenfoot}{%
    \parbox[t]{\textwidth}{%
      \if@fsl
        {%
          \raggedright%
          \vskip-\baselineskip\vskip.4pt
          \hrulefill\\
          }%
      \fi
      \vbox{\hsize=\textwidth\hbox to\textwidth{\headfont\@nextfoot}}%
      }%
    }%
  \let\@oddfoot\@evenfoot
  }
\newcommand*{\ps@myheadings}{%
  \ps@headings\let\@mkboth=\@gobbletwo
  }
\let\@mkboth=\@gobbletwo
\setlength\arraycolsep{5\p@}
\setlength\tabcolsep{6\p@}
\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\setlength{\skip\@mpfootins}{\skip\footins}
\setlength\fboxsep{3\p@}
\setlength\fboxrule{.4\p@}
\renewcommand*\theequation{%
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
\newcommand\marginline[1]{\marginpar[\raggedleft{#1}]{\raggedright #1}}
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
\newcommand*\titlefont{\sectfont}
\newcommand*{\subject@font}{\normalfont\normalcolor\bfseries}
\newcommand*{\baddr@font}{\sffamily}
\newcommand*{\setkomafont}[2]{%
  \@ifundefined{scr@fnt@#1}{%
    \@ifundefined{scr@fnt@instead@#1}{%
      \ClassError{%
        scrlttr2%
      }{font of element `#1' can't be set}{%
        You've told me to redefine the font selection of the element,%
        \MessageBreak%
        but either no such element is known by KOMA-Script\MessageBreak
        or the element does not use a special font selection}
    }{%
      \ClassError{%
        scrlttr2%
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
      scrlttr2%
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
        scrlttr2%
      }{font of element `#1' can't be extended}{%
        You've told me to extend the font selection of the element,%
        \MessageBreak%
        but either no such element is known by KOMA-Script\MessageBreak
        or the element does not use a special font selection}
    }{%
      \ClassError{%
        scrlttr2%
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
      scrlttr2%
      }{\csname scr@fnt@wrn@#1\endcsname{#1}}
    }
  }
}
\newcommand*{\usekomafont}[1]{%
  \@ifundefined{scr@fnt@#1}{%
    \@ifundefined{scr@fnt@instead@#1}{%
      \ClassError{%
        scrlttr2%
        }{font of element `#1' can't be used}{%
        You've told me to use the font selection of the element,%
        \MessageBreak%
        but either no such element is known by KOMA-Script\MessageBreak
        or the element does not use a special font selection}
      }{%
      \ClassWarning{%
        scrlttr2%
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
\newcommand*{\scr@fnt@subject}{\subject@font}
\newcommand*{\scr@fnt@backaddress}{\baddr@font}
\newcommand*{\scr@fnt@fromaddress}{\faddr@font}
\newcommand*{\scr@fnt@fromname}{\fname@font}
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
\newcommand*{\g@addnumerical@date}[2]{%
  \@ifundefined{date#1}{%
    \ClassInfo{%
      scrlttr2%
      }{%
      no date found for language `#1'\MessageBreak
      --> skipped%
      }
    }{%
    \expandafter\g@addto@macro\csname date#1\endcsname{%
      \let\sym@date=\today%
      \def\num@date{#2}%
      \def\today{\if@orgdate\sym@date\else\num@date\fi}%
      }
    }
  }
\AtBeginDocument{
  \@ifundefined{captionsenglish}{\let\captionsenglish\@empty}{}%
  \providecaptionname{english}\yourrefname{Your ref.}%
  \providecaptionname{english}\yourmailname{Your letter of}%
  \providecaptionname{english}\myrefname{Our ref.}%
  \providecaptionname{english}\customername{Customer no.}%
  \providecaptionname{english}\invoicename{Invoice no.}%
  \providecaptionname{english}\subjectname{Subject}%
  \providecaptionname{english}\ccname{cc}%
  \providecaptionname{english}\enclname{encl}%
  \providecaptionname{english}\headtoname{To}%
  \providecaptionname{english}\headfromname{From}%
  \providecaptionname{english}\datename{Date}%
  \providecaptionname{english}\pagename{Page}%
  \providecaptionname{english}\phonename{Phone}%
  \providecaptionname{english}\faxname{Fax}%
  \providecaptionname{english}\emailname{Email}%
  \providecaptionname{english}\wwwname{Url}%
  \providecaptionname{english}\bankname{Bank account}%
  \providecaptionname{USenglish}\yourrefname{Your ref.}%
  \providecaptionname{USenglish}\yourmailname{Your letter of}%
  \providecaptionname{USenglish}\myrefname{Our ref.}%
  \providecaptionname{USenglish}\customername{Customer no.}%
  \providecaptionname{USenglish}\invoicename{Invoice no.}%
  \providecaptionname{USenglish}\subjectname{Subject}%
  \providecaptionname{USenglish}\ccname{cc}%
  \providecaptionname{USenglish}\enclname{encl}%
  \providecaptionname{USenglish}\headtoname{To}%
  \providecaptionname{USenglish}\headfromname{From}%
  \providecaptionname{USenglish}\datename{Date}%
  \providecaptionname{USenglish}\pagename{Page}%
  \providecaptionname{USenglish}\phonename{Phone}%
  \providecaptionname{USenglish}\faxname{Fax}%
  \providecaptionname{USenglish}\emailname{Email}%
  \providecaptionname{USenglish}\wwwname{Url}%
  \providecaptionname{USenglish}\bankname{Bank account}%
  \providecaptionname{american}\yourrefname{Your ref.}%
  \providecaptionname{american}\yourmailname{Your letter of}%
  \providecaptionname{american}\myrefname{Our ref.}%
  \providecaptionname{american}\customername{Customer no.}%
  \providecaptionname{american}\invoicename{Invoice no.}%
  \providecaptionname{american}\subjectname{Subject}%
  \providecaptionname{american}\ccname{cc}%
  \providecaptionname{american}\enclname{encl}%
  \providecaptionname{american}\headtoname{To}%
  \providecaptionname{american}\headfromname{From}%
  \providecaptionname{american}\datename{Date}%
  \providecaptionname{american}\pagename{Page}%
  \providecaptionname{american}\phonename{Phone}%
  \providecaptionname{american}\faxname{Fax}%
  \providecaptionname{american}\emailname{Email}%
  \providecaptionname{american}\wwwname{Url}%
  \providecaptionname{american}\bankname{Bank account}%
  \providecaptionname{british}\yourrefname{Your ref.}%
  \providecaptionname{british}\yourmailname{Your letter of}%
  \providecaptionname{british}\myrefname{Our ref.}%
  \providecaptionname{british}\customername{Customer no.}%
  \providecaptionname{british}\invoicename{Invoice no.}%
  \providecaptionname{british}\subjectname{Subject}%
  \providecaptionname{british}\ccname{cc}%
  \providecaptionname{british}\enclname{encl}%
  \providecaptionname{british}\headtoname{To}%
  \providecaptionname{british}\headfromname{From}%
  \providecaptionname{british}\datename{Date}%
  \providecaptionname{british}\pagename{Page}%
  \providecaptionname{british}\phonename{Phone}%
  \providecaptionname{british}\faxname{Fax}%
  \providecaptionname{british}\emailname{Email}%
  \providecaptionname{british}\wwwname{Url}%
  \providecaptionname{british}\bankname{Bank account}%
  \providecaptionname{UKenglish}\yourrefname{Your ref.}%
  \providecaptionname{UKenglish}\yourmailname{Your letter of}%
  \providecaptionname{UKenglish}\myrefname{Our ref.}%
  \providecaptionname{UKenglish}\customername{Customer no.}%
  \providecaptionname{UKenglish}\invoicename{Invoice no.}%
  \providecaptionname{UKenglish}\subjectname{Subject}%
  \providecaptionname{UKenglish}\ccname{cc}%
  \providecaptionname{UKenglish}\enclname{encl}%
  \providecaptionname{UKenglish}\headtoname{To}%
  \providecaptionname{UKenglish}\headfromname{From}%
  \providecaptionname{UKenglish}\datename{Date}%
  \providecaptionname{UKenglish}\pagename{Page}%
  \providecaptionname{UKenglish}\phonename{Phone}%
  \providecaptionname{UKenglish}\faxname{Fax}%
  \providecaptionname{UKenglish}\emailname{Email}%
  \providecaptionname{UKenglish}\wwwname{Url}%
  \providecaptionname{UKenglish}\bankname{Bank account}%
  \providecaptionname{german}\yourrefname{Ihr Zeichen}%
  \providecaptionname{german}\yourmailname{Ihr Schreiben vom}%
  \providecaptionname{german}\myrefname{Unser Zeichen}%
  \providecaptionname{german}\customername{Kundennummer}%
  \providecaptionname{german}\invoicename{Rechnungsnummer}%
  \providecaptionname{german}\subjectname{Betrifft}%
  \providecaptionname{german}\ccname{Kopien an}%
  \providecaptionname{german}\enclname{Anlagen}%
  \providecaptionname{german}\headtoname{An}%
  \providecaptionname{german}\headfromname{Von}%
  \providecaptionname{german}\datename{Datum}%
  \providecaptionname{german}\pagename{Seite}%
  \providecaptionname{german}\phonename{Telephon}%
  \providecaptionname{german}\faxname{Fax}%
  \providecaptionname{german}\emailname{E-Mail}%
  \providecaptionname{german}\wwwname{URL}%
  \providecaptionname{german}\bankname{Bankverbindung}%
  \providecaptionname{ngerman}\yourrefname{Ihr Zeichen}%
  \providecaptionname{ngerman}\yourmailname{Ihr Schreiben vom}%
  \providecaptionname{ngerman}\myrefname{Unser Zeichen}%
  \providecaptionname{ngerman}\customername{Kundennummer}%
  \providecaptionname{ngerman}\invoicename{Rechnungsnummer}%
  \providecaptionname{ngerman}\subjectname{Betrifft}%
  \providecaptionname{ngerman}\ccname{Kopien an}%
  \providecaptionname{ngerman}\enclname{Anlagen}%
  \providecaptionname{ngerman}\headtoname{An}%
  \providecaptionname{ngerman}\headfromname{Von}%
  \providecaptionname{ngerman}\datename{Datum}%
  \providecaptionname{ngerman}\pagename{Seite}%
  \providecaptionname{ngerman}\phonename{Telefon}%
  \providecaptionname{ngerman}\faxname{Fax}%
  \providecaptionname{ngerman}\emailname{E-Mail}%
  \providecaptionname{ngerman}\wwwname{URL}%
  \providecaptionname{ngerman}\bankname{Bankverbindung}%
  \providecaptionname{austrian}\yourrefname{Ihr Zeichen}%
  \providecaptionname{austrian}\yourmailname{Ihr Schreiben vom}%
  \providecaptionname{austrian}\myrefname{Unser Zeichen}%
  \providecaptionname{austrian}\customername{Kundennummer}%
  \providecaptionname{austrian}\invoicename{Rechnungsnummer}%
  \providecaptionname{austrian}\subjectname{Betrifft}%
  \providecaptionname{austrian}\ccname{Kopien an}%
  \providecaptionname{austrian}\enclname{Anlagen}%
  \providecaptionname{austrian}\headtoname{An}%
  \providecaptionname{austrian}\headfromname{Von}%
  \providecaptionname{austrian}\datename{Datum}%
  \providecaptionname{austrian}\pagename{Seite}%
  \providecaptionname{austrian}\phonename{Telefon}%
  \providecaptionname{austrian}\faxname{Fax}%
  \providecaptionname{austrian}\emailname{E-Mail}%
  \providecaptionname{austrian}\wwwname{URL}%
  \providecaptionname{austrian}\bankname{Bankverbindung}%
  \providecaptionname{french}\yourrefname{Vos r\'ef\'erences}%
  \providecaptionname{french}\yourmailname{Votre lettre du}%
  \providecaptionname{french}\myrefname{Nos r\'ef\'erences}%
  \providecaptionname{french}\customername{Num\'ero de client}%
  \providecaptionname{french}\invoicename{Num\'ero de facture}%
  \providecaptionname{french}\subjectname{Concernant}%
  \providecaptionname{french}\ccname{Copie \`a}%
  \providecaptionname{french}\enclname{Annexes}%
  \providecaptionname{french}\headtoname{A}%
  \providecaptionname{french}\headfromname{De}%
  \providecaptionname{french}\datename{Date}%
  \providecaptionname{french}\pagename{Page}%
  \providecaptionname{french}\phonename{T\'el\'ephone}%
  \providecaptionname{french}\faxname{T\'el\'efax}%
  \providecaptionname{french}\emailname{E-mail}%
  \providecaptionname{french}\wwwname{URL}%
  \providecaptionname{french}\bankname{Compte en banque}%
  \providecaptionname{italian}\yourrefname{Vs./Rif.}%
  \providecaptionname{italian}\yourmailname{Vs.~lettera del}%
  \providecaptionname{italian}\myrefname{Ns./Rif.}%
  \providecaptionname{italian}\customername{Nr.~cliente}%
  \providecaptionname{italian}\invoicename{Nr.~fattura}%
  \providecaptionname{italian}\subjectname{Oggetto}%
  \providecaptionname{italian}\ccname{Per conoscenza}%
  \providecaptionname{italian}\enclname{Allegato}%
  \providecaptionname{italian}\headtoname{A}%
  \providecaptionname{italian}\headfromname{}% TODO: translate it
  \providecaptionname{italian}\datename{Data}%
  \providecaptionname{italian}\pagename{Pagina}%
  \providecaptionname{italian}\phonename{}% TODO: translate it
  \providecaptionname{italian}\faxname{}% TODO: translate it
  \providecaptionname{italian}\emailname{}% TODO: translate it
  \providecaptionname{italian}\wwwname{}% TODO: translate it
  \providecaptionname{italian}\bankname{}% TODO: translate it
  \providecaptionname{spanish}\yourrefname{Su ref.}%
  \providecaptionname{spanish}\yourmailname{Su carta de}%
  \providecaptionname{spanish}\myrefname{Nuestra ref.}%
  \providecaptionname{spanish}\customername{No. de cliente}%
  \providecaptionname{spanish}\invoicename{No. de factura}%
  \providecaptionname{spanish}\subjectname{Asunto}%
  \providecaptionname{spanish}\ccname{Copias}%
  \providecaptionname{spanish}\enclname{Adjunto}%
  \providecaptionname{spanish}\headtoname{A}%
  \providecaptionname{spanish}\headfromname{De}%
  \providecaptionname{spanish}\datename{Fecha}%
  \providecaptionname{spanish}\pagename{P\'agina}%
  \providecaptionname{spanish}\phonename{Tel\'efono}%
  \providecaptionname{spanish}\faxname{Fax}%
  \providecaptionname{spanish}\emailname{Email}% or Correo electr\'onico
  \providecaptionname{spanish}\wwwname{URL}% or P\`agina web
  \providecaptionname{spanish}\bankname{Cuenta bancaria}%
  \providecaptionname{croatian}\yourrefname{Va\v{s} znak}%
  \providecaptionname{croatian}\yourmailname{Va\v{s}e pismo od}%
  \providecaptionname{croatian}\myrefname{Na\v{s} znak}%
  \providecaptionname{croatian}\customername{Broj kupca}%
  \providecaptionname{croatian}\invoicename{Broj fakture}%
  \providecaptionname{croatian}\subjectname{Predmet}%
  \providecaptionname{croatian}\ccname{Kopija}%
  \providecaptionname{croatian}\enclname{Privitak}%
  \providecaptionname{croatian}\headtoname{Prima}%
  \providecaptionname{croatian}\headfromname{\v{S}alje}%
  \providecaptionname{croatian}\datename{Nadnevak}%
  \providecaptionname{croatian}\pagename{Stranica}%
  \providecaptionname{croatian}\phonename{Telefon}%
  \providecaptionname{croatian}\faxname{Fax}%
  \providecaptionname{croatian}\emailname{E-Mail}%
  \providecaptionname{croatian}\wwwname{URL}%
  \providecaptionname{croatian}\bankname{Bankovna veza}%
  \providecaptionname{dutch}\yourrefname{Uw kenmerk}%
  \providecaptionname{dutch}\yourmailname{Uw brief van}%
  \providecaptionname{dutch}\myrefname{Ons kenmerk}%
  \providecaptionname{dutch}\customername{Klant No.}%
  \providecaptionname{dutch}\invoicename{Rekening No.}%
  \providecaptionname{dutch}\subjectname{Onderwerp}%
  \providecaptionname{dutch}\ccname{Kopie aan}%
  \providecaptionname{dutch}\enclname{Bijlage(n)}%
  \providecaptionname{dutch}\headtoname{Aan}%
  \providecaptionname{dutch}\headfromname{Van}%
  \providecaptionname{dutch}\datename{Datum}%
  \providecaptionname{dutch}\pagename{Pagina}%
  \providecaptionname{dutch}\phonename{Telefoon}%
  \providecaptionname{dutch}\faxname{Fax}%
  \providecaptionname{dutch}\emailname{E--mail}%
  \providecaptionname{dutch}\urlname{URL}%
  \providecaptionname{dutch}\bankname{Bankrekening}%
  \g@addnumerical@date{english}{%
    \number\day/\number\month/\number\year}
  \g@addnumerical@date{british}{%
    \number\day/\number\month/\number\year}
  \g@addnumerical@date{UKenglish}{%
    \number\day/\number\month/\number\year}
  \g@addnumerical@date{USenglish}{%
    \number\month/\number\day/\number\year}
  \g@addnumerical@date{american}{%
    \number\month/\number\day/\number\year}
  \g@addnumerical@date{german}{%
    \number\day.\,\number\month.\,\number\year}
  \g@addnumerical@date{ngerman}{%
    \number\day.\,\number\month.\,\number\year}
  \g@addnumerical@date{austrian}{%
    \number\day.\,\number\month.\,\number\year}
  \g@addnumerical@date{french}{%
    \number\day.\,\number\month.\,\number\year}
  \g@addnumerical@date{italian}{%
    \number\day.\,\number\month.\,\number\year}
  \g@addnumerical@date{spanish}{%
    \number\day.\,\number\month.\,\number\year}
  \g@addnumerical@date{croatian}{%
    \number\day.\,\number\month.\,\number\year.}
  \g@addnumerical@date{dutch}{%
    \number\day.\,\number\month.\,\number\year}
  \captionsenglish
  \dateenglish
  \begingroup\expandafter\expandafter\expandafter\endgroup
  \expandafter\ifx\csname date\languagename\endcsname\relax
    \ClassWarningNoLine{%
      scrlttr2%
    }{%
      \string\language\space is \the\language, \string\languagename\space is
      `\languagename'\MessageBreak
      but \expandafter\string\csname
      date\languagename\endcsname\space not defined!\MessageBreak
      This seems to be a bug at you're `hyphen.cfg'.\MessageBreak
      I'l undefine \string\languagename\space to avoid errors%
    }%
    \let\languagename=\undefined
  \fi
  \ifx\languagename\undefined
    \ClassWarningNoLine{scrlttr2}{\string\languagename\space not
      defined, using \string\language.\MessageBreak
      This may result in use of wrong language!\MessageBreak
      You should use a compatible language
      package\MessageBreak
      (e.g. `Babel', `german', `ngerman', ...)}
    \ifx\l@american\undefined\else\ifnum\language=\l@american
        \captionsamerican
        \dateamerican
    \fi\fi
    \ifx\l@british\undefined\else\ifnum\language=\l@british
        \captionsbritish
        \datebritish
    \fi\fi
    \ifx\l@UKenglish\undefined\else\ifnum\language=\l@UKenglish
        \captionsUKenglish
        \dateUKenglish
    \fi\fi
    \ifx\l@USenglish\undefined\else\ifnum\language=\l@USenglish
        \captionsUSenglish
        \dateUSenglish
    \fi\fi
    \ifx\l@austrian\undefined\else\ifnum\language=\l@austrian
        \captionsaustrian
        \dateaustrian
    \fi\fi
    \ifx\l@german\undefined\else\ifnum\language=\l@german
        \captionsgerman
        \dategerman
    \fi\fi
    \ifx\l@ngerman\undefined\else\ifnum\language=\l@ngerman
        \captionsngerman
        \datengerman
    \fi\fi
    \ifx\l@french\undefined\else\ifnum\language=\l@french
        \captionsfrench
        \datefrench
    \fi\fi
    \ifx\l@italian\undefined\else\ifnum\language=\l@italian
        \captionsitalian
        \dateitalian
    \fi\fi
    \ifx\l@spanish\undefined\else\ifnum\language=\l@spanish
        \captionsspanish
        \datespanish
    \fi\fi
    \ifx\l@croatian\undefined\else\ifnum\language=\l@croatian
        \captionscroatian
        \datecroatian
    \fi\fi
    \ifx\l@dutch\undefined\else\ifnum\language=\l@dutch
        \captionsdutch
        \datedutch
    \fi\fi
  \else
    \edef\@tempa{nohyphenation}
    \ifx\languagename\@tempa
      \ClassWarningNoLine{%
        scrlttr2%
        }{%
        You've selected language `\languagename'.\MessageBreak
        Maybe your LaTeX format contains Babel extension\MessageBreak
        but you have not selected a language using\MessageBreak
        Babel package.\MessageBreak
        Please select another language!\MessageBreak
        Only as a workaround english captions and date\MessageBreak
        will be used%
        }%
    \else
      \expandafter\selectlanguage\expandafter{\languagename}
      \ClassInfo{%
        scrlttr2%
        }{%
        used language is `\languagename'.\MessageBreak
        Supported languages are `english', `UKenglish',\MessageBreak
        `british', `american', `USenglish',\MessageBreak
        `german', `ngerman', `austrian',\MessageBreak
        `dutch', `italian', `spanish', `croatian'%
      }
    \fi
  \fi
  }
\newenvironment{letter}[2][]{%
  \newpage
  \if@twoside\ifodd\c@page\else
      \thispagestyle{empty}\null\newpage\fi\fi
  \setcounter{page}{\@ne}%
  \setcounter{footnote}{0}%
  \interlinepenalty=200\@processto{#2}%
  \KOMAoptions{#1}%
  \@beginletterhook
  }{%
  \stopletter\@@par\pagebreak\@@par%
  }
\newcommand*{\stopletter}{}
\newcommand{\@processto}[1]{%
  \expandafter\@xproc#1\\\@nil\ifkomavarempty{toaddress}{}{%
    \expandafter\@xproc#1\@nil}}
\newcommand*{\@xproc}{}
\long\def\@xproc#1\\#2\@nil{%
  \setkomavar{toname}{#1}\setkomavar{toaddress}{#2}}
\newcommand*{\fname@font}{}
\newcommand*{\faddr@font}{}
\newcommand{\AtBeginLetter}{\g@addto@macro\@beginletterhook}
\newcommand{\@beginletterhook}{}
\newcommand*{\newkomavar}{%
  \@ifstar {\@tempswatrue\@newkomavar}{\@tempswafalse\@newkomavar}}
\newcommand*{\@newkomavar}[2][\relax]{%
  \@ifundefined{scr@#2@name}{%
    \@ifundefined{scr@#2@var}{%
      \begingroup
        \def\@tempa{#1}\def\@tempb{\relax}%
        \ifx\@tempa\@tempb\endgroup\else
        \endgroup\@namedef{scr@#2@name}{#1}%
      \fi%
      \expandafter\let\csname scr@#2@var\endcsname=\@empty
      \if@tempswa\addtoreffields{#2}\fi
      }{%
      \ClassError{%
        scrlttr2%
        }{%
        This should never happen%
        }{%
        The contents of the KOMA-Script variable `#2'\MessageBreak
        is undefined, but the name of the variable is
        defined.\MessageBreak
        This should never happen. So someone crashs me!%
        }
      }
    }{%
    \ClassError{%
      scrlttr2%
      }{%
      Variable `#2' already defined%
      }{%
      I'll ignore this command, if you'll continue.%
      }
    }
  }
\newcommand*{\setkomavar}{%
  \@ifstar {\@setkomaname}{\@setkomavar}}
\newcommand*{\@setkomavar}[1]{%
  \@ifnextchar [%]
  {\@@setkomavar{#1}}{\@@setkomavar{#1}[\relax]}}
\newcommand*{\@setkomaname}[2]{%
  \@ifundefined{scr@#1@var}{%
    \@komavar@err{set}{#1}%
    }{%
    \@namedef{scr@#1@name}{#2}%
    }%
  }
\newcommand*{\@@setkomavar}{}
\long\def\@@setkomavar#1[#2]#3{%
  \@ifundefined{scr@#1@var}{%
    \@komavar@err{set}{#1}%
    }{%
    \@namedef{scr@#1@var}{#3}%
    \begingroup
      \def\@tempa{#2}\def\@tempb{\relax}%
      \ifx\@tempa\@tempb\endgroup\else
      \endgroup\@namedef{scr@#1@name}{#2}%
    \fi
    }
  }
\newcommand*{\@komavar@err}[2]{%
  \ClassError{%
    scrlttr2%
    }{%
    KOMA-Script variable not defined%
    }{%
    You've tried to #1 the not defined KOMA-Script variable
    `#2'.\MessageBreak
    You have to define the variable using \string\newkomavar\space
    before\MessageBreak
    you do this.%
    }%
  }
\DeclareRobustCommand*{\usekomavar}{%
  \@ifstar {\@usekomaname}{\@usekomavar}}
\newcommand*{\@usekomavar}[2][\@firstofone]{%
  \@ifundefined{scr@#2@var}{%
    \@komavar@err{use}{#2}%
    }{%
    #1{\@nameuse{scr@#2@var}}%
    }%
  }
\newcommand*{\@usekomaname}[2][\@firstofone]{%
  \@ifundefined{scr@#2@var}{%
    \@komavar@err{use}{#2}%
    }{%
    #1{\@nameuse{scr@#2@name}}%
    }%
  }
\DeclareRobustCommand*{\ifkomavarempty}{%
  \begingroup
  \@ifstar {\@tempswatrue\@ifkomavarempty}%
           {\@tempswafalse\@ifkomavarempty}%
  }
\newcommand{\@ifkomavarempty}[3]{%
  \@ifundefined{scr@#1@var}{%
    \endgroup\@komavar@err{use}{#1}%
    }{%
    \if@tempswa\endgroup
      \@ifundefined{scr@#1@name}{#2}{%
        \expandafter\ifx\csname scr@#1@name\endcsname\@empty
          #2%
        \else
          #3%
        \fi
        }%
    \else\endgroup
      \expandafter\ifx\csname scr@#1@var\endcsname\@empty
        #2%
      \else
        #3%
      \fi
    \fi
    }%
  }
\newkomavar{signature}
\setkomavar{signature}{\usekomavar{fromname}}
\newkomavar{place}
\newkomavar{location}
\newkomavar{backaddress}
\setkomavar{backaddress}{%
  \ifkomavarempty{fromname}{}{%
    \strut\ignorespaces\usekomavar{fromname}%
    \ifkomavarempty{fromaddress}{}{\\}}%
  \ifkomavarempty{fromaddress}{}{%
    \strut\ignorespaces\usekomavar{fromaddress}}%
  }
\newkomavar{specialmail}
\newkomavar{title}
\newkomavar{fromlogo}
\newkomavar[\usekomavar{place}]{placeseparator}
\setkomavar{placeseparator}{,~}
\newkomavar[\subjectname]{subjectseparator}
\setkomavar{subjectseparator}{: }
\newkomavar[\ccname]{ccseparator}
\setkomavar{ccseparator}{: }
\newkomavar[\enclname]{enclseparator}
\setkomavar{enclseparator}{: }
\newkomavar{backaddressseparator}
\setkomavar{backaddressseparator}{,~}
\newkomavar[\phonename]{phoneseparator}
\setkomavar{phoneseparator}{:~}
\newkomavar[\faxname]{faxseparator}
\setkomavar{faxseparator}{:~}
\newkomavar[\emailname]{emailseparator}
\setkomavar{emailseparator}{:~}
\newkomavar[\wwwname]{urlseparator}
\setkomavar{urlseparator}{:~}
\newkomavar[\headfromname]{fromname}
\newkomavar[\usekomavar{fromname}]{fromaddress}
\newkomavar[\headtoname]{toname}
\newkomavar[\usekomavar{toname}]{toaddress}
\newkomavar[\usekomavar*{phoneseparator}%
  \usekomavar{phoneseparator}]{fromphone}
\newkomavar[\usekomavar*{faxseparator}%
  \usekomavar{faxseparator}]{fromfax}
\newkomavar[\usekomavar*{emailseparator}%
  \usekomavar{emailseparator}]{fromemail}
\newkomavar[\usekomavar*{urlseparator}%
  \usekomavar{urlseparator}]{fromurl}
\newkomavar[\bankname]{frombank}
\newkomavar[\yourrefname]{yourref}
\newkomavar[\yourmailname]{yourmail}
\newkomavar[\myrefname]{myref}
\newkomavar[\customername]{customer}
\newkomavar[\invoicename]{invoice}
\newkomavar[\usekomavar*{subjectseparator}%
  \usekomavar{subjectseparator}]{subject}
\newkomavar[\datename]{date}
\setkomavar{date}{\@date}
\newcommand{\firsthead}[1]{%
  \renewcommand*{\@alignfrom}{10000}%
  \renewcommand*{\@firsthead}{\noindent#1}%
  }
\newcommand{\firstfoot}[1]{%
  \renewcommand*{\@firstfoot}{\noindent#1}%
  }
\newcommand{\nexthead}[1]{%
  \renewcommand*{\@nexthead}{\noindent#1}%
  }
\newcommand{\nextfoot}[1]{%
  \renewcommand*{\@nextfoot}{\noindent#1}%
  }
\newcommand*{\foldmarkson}{%
  \@ObsoleteCommand{\foldmarkson}{foldmarks=on}}
\newcommand*{\foldmarksoff}{%
  \@ObsoleteCommand{\foldmarksoff}{foldmarks=off}}
\newcommand*{\addrfieldon}{%
  \@ObsoleteCommand{\addrfieldon}{addrfield=on}}
\newcommand*{\addrfieldoff}{%
  \@ObsoleteCommand{\addrfieldoff}{addrfield=off}}
\newcommand*{\subjecton}{%
  \@ObsoleteCommand{\subjecton}{subject=titled}}
\newcommand*{\subjectoff}{%
  \@ObsoleteCommand{\subjectoff}{subject=untitled}}
\newcommand*{\subjectafteron}{%
  \@ObsoleteCommand{\subjectafteron}{subject=afteropening}}
\newcommand*{\subjectafteroff}{%
  \@ObsoleteCommand{\subjectafteroff}{subject=beforeopening}}
\newcommand*{\@newplength}[1]{%
  \@ifundefined{ltr@len@#1}{%
    \expandafter\let\csname ltr@len@#1\endcsname=\z@%
    }{%
    \ClassError{scrlttr2}{%
      pseudo-length \expandafter\string\csname ltr@len@#1\endcsname
      already defined%
      }{%
      You tried to define a new pseudo-length using
      \string\@newplength\MessageBreak
      which is already defined. Try another name or
      better\MessageBreak
      Don't use this KOMA-Script internal macro}
    }
  }
\newcommand*{\useplength}[1]{%
  \expandafter\number\expandafter\csname ltr@len@#1\endcsname}
\newcommand*{\setlengthtoplength}[3][]{%
  \setlength{#2}{\useplength{#3}}%
  \setlength{#2}{#1#2}}
\newcommand*{\@setplength}[3][]{%
  \begingroup%
    \setlength{\@tempskipa}{#3}%
    \setlength{\@tempskipa}{#1\@tempskipa}%
    \edef\@tempa{\noexpand\endgroup%
      \noexpand\expandafter\noexpand\renewcommand\noexpand\expandafter*%
      \noexpand\csname ltr@len@#2\noexpand\endcsname{\the\@tempskipa}%
      }%
    \@tempa%
  }
\newcommand*{\addtolengthplength}[3][]{%
  \begingroup%
    \setlengthtoplength[{#1}]{\@tempskipa}{#3}%
    \edef\@tempa{\endgroup%
      \noexpand\addtolength{#2}{\the\@tempskipa}}%
    \@tempa%
  }
\newcommand*{\@addtoplength}[3][]{%
  \begingroup%
    \setlength{\@tempskipa}{#3}%
    \setlength{\@tempskipa}{#1\@tempskipa}%
    \addtolengthplength{\@tempskipa}{#2}%
    \edef\@tempa{\noexpand\endgroup%
      \noexpand\expandafter\noexpand\renewcommand\noexpand\expandafter*%
      \noexpand\csname ltr@len@#2\noexpand\endcsname{\the\@tempskipa}%
      }%
    \@tempa%
  }
\@newplength{foldmarkhpos}
\@newplength{tfoldmarkvpos}
\@newplength{bfoldmarkvpos}
\@newplength{toaddrvpos}
\@newplength{backaddrheight}
\@newplength{toaddrhpos}
\@newplength{specialmailindent}
\@newplength{specialmailrightindent}
\@newplength{toaddrindent}
\@newplength{toaddrwidth}
\@newplength{toaddrheight}
\@newplength{locwidth}
\@newplength{refvpos}
\@newplength{refwidth}
\@newplength{refhpos}
\@newplength{refaftervskip}
\@newplength{sigbeforevskip}
\@newplength{sigindent}
\@newplength{firstfootvpos}
\@newplength{firstfootwidth}
\@newplength{firstheadvpos}
\@newplength{firstheadwidth}
\@newplength{fromrulewidth}
\@setplength{fromrulewidth}{0pt}
\newcommand*{\LoadLetterOption}[1]{%
  \@ifundefined{@restore@catcode@level}
    {\let\@restore@catcode@level=\z@}{}
  \ifnum\@restore@catcode@level =\z@
    \ifnum\catcode`\@=11
      \let\@restore@catcode\relax
    \else
      \@tempcnta=\catcode`\@
      \edef\@restore@catcode{%
        \noexpand\catcode`\noexpand\@=\the\@tempcnta}%
      \makeatletter
    \fi
  \fi
  \@tempcnta=\@restore@catcode@level\relax
  \advance\@tempcnta by \@ne\relax
  \edef\@restore@catcode@level{\the\@tempcnta}
  \InputIfFileExists{#1.lco}{%
    \ClassInfo{%
      scrlttr2%
      }{Letter-Class-Option `#1' loaded}
    }{%
    \ClassError{%
      scrlttr2%
      }{%
      Letter-Class-Option file `#1.lco' not found%
      }{%
      You've told me to load the Letter-Class-Option `#1'. So I have
      to load\MessageBreak
      the file `#1.lco'. But the file isn't available.}
    }
  \@tempcnta=\@restore@catcode@level\relax
  \advance\@tempcnta by \m@ne\relax
  \edef\@restore@catcode@level{\the\@tempcnta}
  \ifnum\@tempcnta =\z@
    \@restore@catcode
  \fi
  }
\newcommand*{\LOPNP@option}{}
\newcommand*{\LOPNP@size}{}
\newcommand*{\@PapersizeWarning}{
  \begingroup%
    \edef\@tempc{\noexpand\@tempswatrue
      \noexpand\ifdim\paperwidth=\the\paperwidth\noexpand\else
      \noexpand\@tempswafalse\noexpand\fi
      \noexpand\ifdim\paperheight=\the\paperheight\noexpand\else
      \noexpand\@tempswafalse\noexpand\fi
      \noexpand\@@PapersizeWarning
      }
    \def\@tempb{letter}\ifx\LOPNP@size\@tempb%
      \setlength{\paperheight}{11in}\setlength{\paperwidth}{8.5in}%
    \else\def\@tempb{legal}\ifx\LOPNP@size\@tempb%
      \setlength{\paperheight}{14in}\setlength{\paperwidth}{8.5in}%
    \else\def\@tempb{executive}\ifx\LOPNP@size\@tempb%
      \setlength{\paperheight}{10.5in}\setlength{\paperwidth}{7.25in}%
    \else%
      \def\ta@opt@err{%
        \ClassError{%
          scrlttr2%
          }{papersize `\LOPNP@size' undefined}{%
          You've told me to check for paper size
          `\LOPNP@size'\MessageBreak
          at letter option file `\LOPNP@option.lco',\MessageBreak
          but this paper size is not supported.\MessageBreak
          See the KOMA-Script manual for informations about known
          paper sizes.}%
        }
      \let\@headlines\ta@opt@err%
      \expandafter\x@@paper\LOPNP@size paperpaper\x@@paper%
    \fi\fi\fi
    \@tempc
  \endgroup%
}
\newcommand*{\@@PapersizeWarning}{%
  \if@tempswa\else%
  \ClassWarningNoLine{%
    scrlttr2%
    }{%
    Letter option file `\LOPNP@option.lco'\MessageBreak
    needs paper size `\LOPNP@size'.\MessageBreak
    Current paper size is not `\LOPNP@size'!\MessageBreak
    You should add `paper=\LOPNP@size' at the\MessageBreak
    option list of \string\documentclass!\MessageBreak
    Maybe you know what you are doing,\MessageBreak
    so I do not change this myself}
  \fi
  }
\newcommand*{\LetterOptionNeedsPapersize}[2]{%
  \edef\LOPNP@size{#2}\edef\LOPNP@option{#1}%
  }
\LoadLetterOption{DIN}
\newcommand*{\adrentry}[7]{%
  \addrentry{#1}{#2}{#3}{#4}{#5}{#6}{#7}{}}
\newcommand*{\adrchar}{\addrchar}
\newcommand*{\addrentry}[9]{\def\@tempa{#1}\ifx \@tempa\@empty \else
 \def\@tempa{#2}\ifx \@tempa\@empty
  \expandafter\def\csname #9\endcsname{#1\\#3}
 \else
  \expandafter\def\csname #9\endcsname{#2 #1\\#3}
 \fi \fi}
\newcommand*{\addrchar}[1]{}
\newcommand*{\stopbreaks}{\interlinepenalty \@M
 \def\par{\@@par\nobreak}\let\\=\@nobreakcr
 \let\vspace\@nobreakvspace}
\newcommand*{\@nobreakvspace}{%
  \@ifstar{\@nobreakvspacex}{\@nobreakvspacex}}
\newcommand*{\@nobreakvspacex}[1]{\ifvmode\nobreak\vskip #1\relax\else
 \@bsphack\vadjust{\nobreak\vskip #1}\@esphack\fi}
\newcommand*{\@nobreakcr}{%
  \vadjust{\penalty\@M}\@ifstar{\@xnewline}{\@xnewline}}
\newcommand*{\startbreaks}{\let\\=\@normalcr
 \interlinepenalty 200\def\par{\@@par\penalty 200}}
\newcommand*{\move@topt}{%
  \null\vskip -1in\vskip -\topmargin%
  \vskip -\headheight\vskip -\headsep%
  \vskip -\topskip%
  \ignorespaces%
  }
\newcommand*{\move@frompt}{%
  \vskip 1in\vskip \topmargin%
  \vskip \headheight\vskip \headsep%
  \vskip \topskip%
  \vskip -\baselineskip%
  \ignorespaces%
  }
\newcommand*{\move@topl}{%
  \null\hskip -1in%
  \ifodd\value{page}\hskip -\oddsidemargin\else\hskip -\evensidemargin\fi
  \ignorespaces%
  }
\newcommand*{\move@frompl}{%
  \hskip 1in%
  \ifodd\value{page}\hskip \oddsidemargin\else\hskip \evensidemargin\fi%
  \ignorespaces%
  }
\newcommand*{\vb@t@z}[1]{{%
    \setbox0\vbox to\z@{#1\vss}%
    \setlength{\parskip}{\z@}\setlength{\baselineskip}{\z@}%
    \setlength{\parfillskip}{\z@ \@plus 1fil}%
    \setlength{\@tempdima}{\ht0}\addtolength{\@tempdima}{\dp0}%
    \box0\vskip-\@tempdima%
    }}
\newcommand*{\@foldmark}[2]{%
  \vskip #1%
  \vb@t@z{\rlap{\move@topl\hskip\useplength{foldmarkhpos}%
    \rule{#2}{.2pt}}}%
  \vskip -#1\ignorespaces%
}
\newcommand*{\@foldmarks}{%
  \if@fold%
    \typeout{Foldmarks: yes}%
    \move@topt
    \@foldmark{\useplength{tfoldmarkvpos}}{2mm}
    \@foldmark{.5\paperheight}{4mm}
    \@foldmark{\useplength{bfoldmarkvpos}}{2mm}
    \move@frompt
  \else
    \typeout{Foldmarks: no}%
  \fi%
}
\newif\if@savevbox@warning\@savevbox@warningtrue
\newcommand{\@savevbox}[6][]{%
  \begingroup%
    \vbox to #2{#1#3}%
    \if@savevbox@warning\@savevbox@warningfalse%
    \setbox0\vbox{#3}%
    \setlength{\@tempdima}{#2}%
    \setlength{\@tempdimb}{\ht0}\addtolength{\@tempdimb}{\dp0}%
    \ifdim \@tempdimb>\@tempdima%
      \addtolength{\@tempdimb}{-\@tempdima}%
      \ClassWarning{%
        scrlttr2%
      }{%
        #4 is \the\@tempdimb\space too high.\MessageBreak
        You have to change `\string#5'\MessageBreak
        or you have to define a smaller\MessageBreak
        #4 using \string#6.\MessageBreak
        Because of this too high #4\MessageBreak
        you've got an overfull \string\vbox\space message%
      }%
    \fi%
    \fi%
  \endgroup%
}
\newcommand*{\@firstheadfootfield}{%
  \typeout{Head of first page}%
  \move@topt\vskip\useplength{firstheadvpos}%
  \vb@t@z{%
    \rlap{\move@topl
      \setlength\@tempskipa{.5\paperwidth}%
      \addtolengthplength[-.5]{\@tempskipa}{firstheadwidth}%
      \hskip\@tempskipa
      \vbox{\hsize\useplength{firstheadwidth}%
        \setlength{\@tempskipa}{\useplength{toaddrvpos}}%
        \addtolengthplength[-1]{\@tempskipa}{firstheadvpos}%
        \addtolength{\@tempskipa}{-\baselineskip}%
        \setlength{\textwidth}{\useplength{firstheadwidth}}%
        \setlength{\parfillskip}{\z@ \@plus 1fil}%
        \@savevbox{\@tempskipa}{\@firsthead\vskip \z@ \@plus .00001fil}%
        {head of first page}{firstheadvpos}{\firsthead}%
      }%
    }%
  }%
  \vskip-\useplength{firstheadvpos}\move@frompt
  \typeout{Foot of first page}%
  \move@topt
  \vb@t@z{%
    \rlap{\move@topl
      \setlength\@tempskipa{.5\paperwidth}%
      \addtolengthplength[-.5]{\@tempskipa}{firstfootwidth}%
      \hskip\@tempskipa%
      \vbox{\hsize\useplength{firstfootwidth}%
        \vskip\useplength{firstfootvpos}%
        \setlength{\textwidth}{\useplength{firstfootwidth}}%
        \setlength{\parfillskip}{\z@ \@plus 1fil}%
        \vbox{\hsize\useplength{firstfootwidth}\@firstfoot}%
      }%
    }%
  }%
  \move@frompt%
  {%
    \setlength{\@tempdima}{1in}\addtolength{\@tempdima}{\topmargin}%
    \addtolength{\@tempdima}{\headheight}%
    \addtolength{\@tempdima}{\headsep}%
    \addtolength{\@tempdima}{\textheight}%
    \addtolength{\@tempdima}{\footskip}%
    \addtolengthplength[-1]{\@tempdima}{firstfootvpos}%
    \ifdim\@tempdima>\z@\enlargethispage{-\@tempdima}%
    \else\if@enlargefp\enlargethispage{-\@tempdima}\fi\fi
  }%
}
\newcommand*{\@addrfield}{%
  \if@afield
    \begingroup%
      \if@baddr
        \ifkomavarempty{backaddress}{%
          \let\@tempa\@empty\let\@tempb\@empty
        }{%
          \def\@tempa{backaddress}\def\@tempb{,\space}%
        }%
      \else
        \let\@tempa\@empty\let\@tempb\@empty
      \fi
      \ifkomavarempty{specialmail}{}{%
        \edef\@tempa{\@tempa\@tempb specialmail}\def\@tempb{,\space}%
      }%
      \typeout{Address (\@tempa\@tempb addressee)}%
    \endgroup%
    \move@topt\vskip\useplength{toaddrvpos}%
    \vb@t@z{{\setlength{\parfillskip}{\z@ \@plus 1fil}%
        \rlap{\move@topl
          \setlengthtoplength{\@tempskipa}{toaddrhpos}%
          \ifdim \@tempskipa<\z@
            \addtolength{\@tempskipa}{\paperwidth}%
            \addtolengthplength[-]{\@tempskipa}{toaddrwidth}%
          \fi
          \hskip\@tempskipa
          \@addtoplength{toaddrheight}{\useplength{backaddrheight}}%
          \@savevbox{\useplength{toaddrheight}}{\hsize\useplength{toaddrwidth}%
            \@savevbox[\vss]{\useplength{backaddrheight}}{%
              \hsize\useplength{toaddrwidth}%
              \if@baddr
                \ifkomavarempty{backaddress}{}{{%
                    \noindent%
                    \def\\{\usekomavar{backaddressseparator}\@ogobble}%
                    \underline{\scriptsize\usekomafont{backaddress}%
                      \usekomavar{backaddress}}}}%
              \fi
            }{backaddress}{backaddrheight}{\setkomavar}%
            \ifkomavarempty{specialmail}{}{%
              \vbox{\hsize\useplength{toaddrwidth}%
                \hspace*{\useplength{specialmailindent}}%
                \underline{\usekomavar{specialmail}}%
                \hspace*{\useplength{specialmailrightindent}}}%
            }%
            \vfil%
            \noindent\hskip\useplength{toaddrindent}%
            \setlength{\@tempdima}{\useplength{toaddrwidth}}%
            \addtolengthplength[-2]{\@tempdima}{toaddrindent}%
            \vbox{\hsize\@tempdima%
              \noindent\usekomavar{toname}\\\usekomavar{toaddress}}%
            \vfil\null}%
          {address field}{toaddrheight}
          {\begin{letter} or you\MessageBreak
            have to define a smaller special purpose using\MessageBreak
            \string\specialmail}
        }}}%
    \vskip-\useplength{toaddrvpos}\move@frompt
  \else
    \typeout{Address: No}%
  \fi
  }
\newcommand*{\@ogobble}[1][]{}
\newcommand*{\@locfield}{%
  \ifdim \useplength{locwidth}=\z@%
    \@setplength{locwidth}{\paperwidth}%
    \ifdim \useplength{toaddrhpos}>\z@
      \@addtoplength[-2]{locwidth}{\useplength{toaddrhpos}}%
    \else
      \@addtoplength[2]{locwidth}{\useplength{toaddrhpos}}%
    \fi
    \@addtoplength[-1]{locwidth}{\useplength{toaddrwidth}}%
    \if@bigloc%
      \@setplength[.66667]{locwidth}{\useplength{locwidth}}%
    \else%
      \@setplength[.5]{locwidth}{\useplength{locwidth}}%
    \fi%
  \fi%
  \ifnum 1=\@alignfrom
    \if@logo
      \ifkomavarempty{location}{%
        \setkomavar{location}{\null}}{}%
    \fi
  \fi
  \ifkomavarempty{location}{%
    \typeout{Location field: empty}%
    }{%
    \typeout{Location field: yes}%
    \move@topt\vskip\useplength{toaddrvpos}%
    \vb@t@z{\rlap{%
        \move@topl
        \setlengthtoplength[-]{\@tempskipa}{toaddrhpos}%
        \ifdim \@tempskipa<\z@
          \addtolength{\@tempskipa}{\paperwidth}%
        \else
          \addtolengthplength{\@tempskipa}{locwidth}%
        \fi
        \hskip\@tempskipa
        \llap{%
          \@savevbox{\useplength{toaddrheight}}{%
            \noindent\hsize\useplength{locwidth}%
            \ifnum 1=\@alignfrom
              \if@logo
                \ifkomavarempty{fromlogo}{}{%
                  {\raggedleft\noindent\usekomavar{fromlogo}\\
                    \vskip\parskip}\noindent}%
              \fi
            \fi
            \usekomavar{location}\vskip \z@ \@plus .00001fil}
          {location field}{toaddrheight}{\setkomavar}%
          }}%
      }%
    \vskip-\useplength{toaddrvpos}\move@frompt
    }%
  }
\newcommand*{\@set@reffield}[1]{%
  \ifkomavarempty{#1}{}{%
    \settowidth{\@tempdima}{\strut\usekomavar{#1}}%
    \setbox0\hbox{\strut\scriptsize\sffamily\usekomavar*{#1}}%
    \ifdim \@tempdima<\wd0\setlength{\@tempdima}{\wd0}\fi
    \if@tempswa\enskip\hfill\fi
    \parbox[t]{\@tempdima}{\noindent\raggedright
        \strut\box0\\\strut\usekomavar{#1}}%
    \@tempswatrue
  }%
}
\newcommand*{\@set@reffields}{}%
\newcommand*{\addtoreffields}[1]{%
  \l@addto@macro{\@set@reffields}{\@set@reffield{#1}}%
  }
\newcommand*{\removereffields}{%
  \renewcommand*{\@set@reffields}{}%
  }
\newcommand*{\defaultreffields}{%
  \removereffields
  \addtoreffields{yourref}%
  \addtoreffields{yourmail}%
  \addtoreffields{myref}%
  \addtoreffields{customer}%
  \addtoreffields{invoice}%
}
\defaultreffields
\newcommand*{\@datefield}{{\@parskipfalse\@parskip@indent
    \ifdim\useplength{refwidth}=\z@
      \if@refwide
        \@setplength{refwidth}{\paperwidth}%
        \ifdim \useplength{toaddrhpos}>\z@
          \@setplength{refhpos}{\useplength{toaddrhpos}}%
        \else
          \@setplength[-]{refhpos}{\useplength{toaddrhpos}}%
        \fi
        \@addtoplength[-2]{refwidth}{\useplength{refhpos}}%
      \else
        \@setplength{refwidth}{\textwidth}%
        \@setplength{refhpos}{\oddsidemargin}%
        \@addtoplength{refhpos}{1in}%
      \fi
    \else\ifdim\useplength{refhpos}=\z@
        \begingroup
          \setlength\@tempdima{\textwidth}%
          \addtolengthplength[-]{\@tempdima}{refwidth}%
          \setlength\@tempdimb{\oddsidemargin}%
          \addtolength\@tempdimb{1in}%
          \setlength{\@tempdimc}{\paperwidth}%
          \addtolength{\@tempdimc}{-\textwidth}%
          \divide\@tempdimb by 32768\relax
          \divide\@tempdimc by 32768\relax
          \ifdim\@tempdimc=\z@\else
            \multiply\@tempdima by\@tempdimb
            \divide\@tempdima by\@tempdimc
          \fi
          \edef\@tempa{\noexpand\endgroup
            \noexpand\@setplength{refhpos}{\the\@tempdima}}%
        \@tempa
    \fi\fi
    \@tempswafalse
    \move@topt\vskip\useplength{refvpos}%
    \rlap{\noindent\move@topl\hskip\useplength{refhpos}%
      \vbox{\hsize\useplength{refwidth}%
        \noindent\@set@reffields
        \ifkomavarempty{date}{%
          \if@tempswa
            \typeout{Reference line(s): with fields but without date}%
          \else
            \typeout{Reference line(s): no}%
            \aftergroup\aftergroup\aftergroup\@tempswatrue
          \fi
          }{%
          \if@tempswa
            \typeout{Reference line(s): with fields and date}%
            \settowidth{\@tempdima}{\strut\usekomavar{date}}%
            \setbox0\hbox{\scriptsize\sffamily\usekomavar*{date}}%
            \ifdim \@tempdima<\wd0\setlength{\@tempdima}{\wd0}\fi
            \enskip\hfill
            \parbox[t]{\@tempdima}{\noindent\raggedright%
              \strut\box0\\\strut\usekomavar{date}}
          \else
            \typeout{Reference line: only place and date}%
            \enskip\hfill
            \ifkomavarempty{place}{}{%
              \usekomavar{place}\usekomavar{placeseparator}}%
            \usekomavar{date}%
            \@tempswatrue
          \fi
          }%
        }%
      }%
    \if@tempswa\vskip-\baselineskip\fi
    \vskip\useplength{refaftervskip}%
  }%
}
\newcommand*{\@maketitle}{%
  \ifkomavarempty{title}{%
    \typeout{Title: no}%
    }{%
    \typeout{Title: yes}%
    {\@parskipfalse\@parskip@indent%
      \centering{\LARGE\titlefont\usekomavar{title}}\par}%
    \vskip\baselineskip%
    }%
  }
\newcommand*{\@subjfield}{{\@parskipfalse\@parskip@indent%
    \ifkomavarempty{subject}{%
      \typeout{Subject: no}%
      }{%
      \if@subjafter%
        \typeout{Subject: after opening}%
        \centering%
      \else%
        \typeout{Subject: before opening}%
        \noindent%
      \fi%
      {\usekomafont{subject}\if@subj\@hangfrom{\usekomavar*{subject}}\fi%
        {\usekomavar{subject}}\par}\vskip2\baselineskip%
      }%
    }%
  }
\newcommand{\opening}[1]{%
  \@PapersizeWarning
  \@gen@firsthead
  \@mkboth{\scr@fromname@var}{\scr@subject@var}%
  \thispagestyle{empty}\noindent
  \@foldmarks\@firstheadfootfield\@addrfield\@locfield\@datefield
  \@maketitle
  \if@subjafter\else\@subjfield\fi
  {\@parskipfalse\@parskip@indent\noindent#1\par}%
  {\setlength{\@tempdima}{\baselineskip}%
    \addtolength{\@tempdima}{-\parskip}
    \ifdim \@tempdima>\z@\vskip\@tempdima\fi}
  \if@subjafter\@subjfield\fi%
  \@afterindentfalse\@afterheading
  }
\newcommand*{\setpresigskip}{\@setplength{sigbeforevskip}}
\newcommand*{\@xtrimtempa}{%
  \@ifnextchar [{\@@xtrimtempa}{\@@xtrimtempa[]}}
\newcommand*{\@@xtrimtempa}{}
\def\@@xtrimtempa[#1]#2\@nil{\def\@tempa{#2}}
\newcommand*{\@tempdimaismaxwidthof}{}
\def\@tempdimaismaxwidthof#1\\#2\@nil{%
  \settowidth{\@tempdimb}{\strut\ignorespaces#1}%
  \ifdim\@tempdima<\@tempdimb\setlength{\@tempdima}{\@tempdimb}\fi
  \def\@tempa{#2}\ifx\@tempa\@empty
    \let\@tempb\relax
  \else
    \expandafter\@xtrimtempa\@tempa\@nil
    \def\@tempb{\expandafter\@tempdimaismaxwidthof\@tempa\@nil}%
  \fi
  \@tempb
  }
\newcommand*{\closing}[1]{{\@parskipfalse\@parskip@indent\nobreak\par%
  \nobreak\vskip\baselineskip%
  \stopbreaks\noindent%
  \setlength{\@tempdima}{0pt}%
  \expandafter\@tempdimaismaxwidthof\scr@signature@var\\\@nil
  \expandafter\@tempdimaismaxwidthof#1\\\@nil
  \ifdim\@tempdima<\@tempdimb\setlength{\@tempdima}{\@tempdimb}\fi%
  \hspace{\useplength{sigindent}}%
  \parbox{\@tempdima}{\raggedsignature\strut\ignorespaces%
    #1\strut\\[\useplength{sigbeforevskip}]
    \strut\usekomavar{signature}\strut}}\par
  \nobreak\vskip\baselineskip%
}
\newcommand\ps{\par%
  \ifdim\parskip=\z@%
    \vskip\baselineskip%
  \fi\startbreaks\noindent\ignorespaces%
}
\newcommand*{\cc}[1]{\par%
  \ifdim\parskip=\z@%
    \vskip\baselineskip%
  \fi\noindent%
  \parbox[t]{\textwidth}{%
    \ifkomavarempty{ccseparator}{}{%
      \@hangfrom{%
        \strut\usekomavar*{ccseparator}\usekomavar{ccseparator}}%
      }%
    \ignorespaces #1\strut}\par%
}
\newcommand*{\encl}[1]{\par%
  \ifdim\parskip=\z@%
    \vskip\baselineskip%
  \fi\noindent%
  \parbox[t]{\textwidth}{%
    \ifkomavarempty*{enclseparator}{}{%
      \@hangfrom{%
        \strut\usekomavar*{enclseparator}\usekomavar{enclseparator}}%
      }%
    \ignorespaces #1\strut}\par%
  }
\pagestyle{plain}
\pagenumbering{arabic}      % Arabic page numbers
  \raggedbottom
\@ifundefined{KOMAScript}{%
  \DeclareRobustCommand{\KOMAScript}{\textsf{K\kern.05em O\kern.05em%
      M\kern.05em A\kern.1em-\kern.1em Script}}}{}
\endinput
%%
%% End of file `scrlttr2.cls'.
