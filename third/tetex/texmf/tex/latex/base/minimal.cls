% \iffalse meta-comment
%
% Copyright 1993 1994 1995 1996 1997 1998 1999
% The LaTeX3 Project and any individual authors listed elsewhere
% in this file. 
% 
% This file is part of the LaTeX2e system.
% ----------------------------------------
% 
% It may be distributed under the terms of the LaTeX Project Public
% License, as described in lppl.txt in the base LaTeX distribution.
% Either version 1.0 or, at your option, any later version.
% 
% \fi
%%
%% Minimal LaTeX class file.
%%

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{minimal}[1996/05/20 Standard LaTeX minimal class]

\renewcommand\normalsize{\fontsize{10pt}{12pt}\selectfont}

\setlength{\textwidth}{6.5in}
\setlength{\textheight}{8in}


% This documentclass is intended primarily for testing and reference
% purposes; loading it with \LoadClass{minimal} to use it as a base
% class for some other document class is probably a mistake. If you wish
% to start a new document class based on the minimal class, it is better
% to start by copying the *contents* of minimal.cls directly into your
% new class and making suitable modifications. You may, at that point
% also want to start documenting the code using the conventions of the
% doc package, rather than using simple ascii comments as used here.
