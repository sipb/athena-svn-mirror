%%
%% This is file `proc.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% proc.dtx  (with options: `class')
%% 
%% This is a generated file.
%% 
%% Copyright 1993 1994 1995 1996 1997
%% The LaTeX3 Project and any individual authors listed elsewhere
%% in this file.
%% 
%% For further copyright information, and conditions for modification
%% and distribution, see the file legal.txt, and any other copyright
%% notices in this file.
%% 
%% This file is part of the LaTeX2e system.
%% ----------------------------------------
%%   This system is distributed in the hope that it will be useful,
%%   but WITHOUT ANY WARRANTY; without even the implied warranty of
%%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
%% 
%%   For error reports concerning UNCHANGED versions of this file no
%%   more than one year old, see bugs.txt.
%% 
%%   Please do not request updates from us directly.  Primary
%%   distribution is through the CTAN archives.
%% 
%% 
%% IMPORTANT COPYRIGHT NOTICE:
%% 
%% You are NOT ALLOWED to distribute this file alone.
%% 
%% You are allowed to distribute this file under the condition that it
%% is distributed together with all the files listed in manifest.txt.
%% 
%% If you receive only some of these files from someone, complain!
%% 
%% 
%% Permission is granted to copy this file to another file with a
%% clearly different name and to customize the declarations in that
%% copy to serve the needs of your installation, provided that you
%% comply with the conditions in the file legal.txt.
%% 
%% However, NO PERMISSION is granted to generate or to distribute a
%% modified version of this file under its original name.
%% 
%% You are NOT ALLOWED to change this file.
%% 
%% 
%% MODIFICATION ADVICE:
%% 
%% If you want to customize this file, it is best to make a copy of
%% the source file(s) from which it was produced.  Use a different
%% name for your copy(ies) and modify the copy(ies); this will ensure
%% that your modifications do not get overwritten when you install a
%% new release of the standard system.  You should also ensure that
%% your modified source file does not generate any modified file with
%% the same name as a standard file.
%% 
%% You can then easily distribute your modifications by distributing
%% the modified and renamed copy of the source file, taking care to
%% observe the conditions in legal.txt; this will ensure that other
%% users can safely use your modifications.
%% 
%% You will also need to produce your own, suitably named, .ins file to
%% control the generation of files from your source file; this file
%% should contain your own preambles for the files it generates, not
%% those in the standard .ins files.
%% 
%% The names of the source files used are shown above.
%% 
%% 
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
\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{proc}
              [1995/11/30 v1.3l
               Standard LaTeX document class]
\DeclareOption{a5paper}
    {\ClassError{proc}{Option `a5paper' not supported}{}}
\DeclareOption{b5paper}
    {\ClassError{proc}{Option `b5paper' not supported}{}}
\DeclareOption{onecolumn}%
    {\ClassError{proc}{Option `onecolumn' not supported}{}}
\DeclareOption{titlepage}%
    {\ClassError{proc}{Option `titlepage' not supported}{}}
\PassOptionsToClass{twocolumn}{article}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ProcessOptions
\LoadClass{article}
\setlength\footskip{75\p@}
\if@compatibility
  \setlength\textwidth{6.75in}
\else
  \setlength\textwidth{\paperwidth}
  \addtolength\textwidth{-126\p@}
  \@settopoint\textwidth
\fi
\if@compatibility
\else
  \@settopoint\textwidth
\fi
\if@compatibility
  \setlength\textheight{9in}
\else
  \ifcase\@ptsize
    \setlength\textheight{53\baselineskip}
  \or
    \setlength\textheight{47\baselineskip}
  \or
    \setlength\textheight{44\baselineskip}
  \fi
\fi
\advance\textheight by \topskip
\setlength\oddsidemargin{-10\p@}
\setlength\evensidemargin{-10\p@}
\setlength\marginparwidth {\z@}
\setlength\topmargin{-47\p@}
\setlength\columnsep{.375in}
\let\@oddhead\@empty
\let\@evenhead\@empty
\def\@oddfoot{\normalfont\rightmark \hfil
              \pagename{} \thepage}
\def\@evenfoot{\@oddfoot}
\def\maketitle{\par
 \begingroup
   \renewcommand\thefootnote{\fnsymbol{footnote}}%
    \def\@makefnmark{\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%
     \long\def\@makefntext##1{\parindent 1em\noindent
             \hb@xt@1.8em{%
                 \hss\@textsuperscript{\normalfont\@thefnmark}}##1}%
   \twocolumn[\@maketitle]%
   \@thanks
 \endgroup
 \setcounter{footnote}{0}%
 \let\maketitle\relax
 \let\@maketitle\relax
 \gdef\@thanks{}\gdef\@author{}\gdef\@title{}\let\thanks\relax}
\def\@maketitle{%
  \vbox to 2.25in{%
    \hsize\textwidth
    \linewidth\hsize
    \vfil
    \centering
    {\LARGE \@title \par}
    \vskip 2em
    {\large \begin{tabular}[t]{c}\@author \end{tabular}\par}
    \vfil}}
\def\copyrightspace{%
  \footnotetext[0]{\mbox{}\vrule \@height 97\p@ \@width \z@}}
\renewenvironment{abstract}%
  {\section*{\abstractname}}
  {\par}
\newcommand\pagename{Page}
\endinput
%%
%% End of file `proc.cls'.
