% \iffalse meta-comment
%
% Copyright 1993 1994 1995 1996 1997
% The LaTeX3 Project and any individual authors listed elsewhere
% in this file. 
% 
% For further copyright information, and conditions for modification
% and distribution, see the file legal.txt, and any other copyright
% notices in this file.
% 
% This file is part of the LaTeX2e system.
% ----------------------------------------
%   This system is distributed in the hope that it will be useful,
%   but WITHOUT ANY WARRANTY; without even the implied warranty of
%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
% 
%   For error reports concerning UNCHANGED versions of this file no
%   more than one year old, see bugs.txt.
% 
%   Please do not request updates from us directly.  Primary
%   distribution is through the CTAN archives.
% 
% 
% IMPORTANT COPYRIGHT NOTICE:
% 
% You are NOT ALLOWED to distribute this file alone.
% 
% You are allowed to distribute this file under the condition that it
% is distributed together with all the files listed in manifest.txt.
% 
% If you receive only some of these files from someone, complain!
% 
% 
% Permission is granted to copy this file to another file with a
% clearly different name and to customize the declarations in that
% copy to serve the needs of your installation, provided that you
% comply with the conditions in the file legal.txt.
% 
% However, NO PERMISSION is granted to produce or to distribute a
% modified version of this file under its original name.
%  
% You are NOT ALLOWED to change this file.
% 
% 
% 
% \fi
%%
%% Minimal LaTeX class file.
%%

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{minimal}[1996/05/20 Standard LaTeX minimal class]

\setlength{\textwidth}{6.5in}
\setlength{\textheight}{8in}
\renewcommand\normalsize{\fontsize{10pt}{12pt}\selectfont}


% This documentclass is intended primarily for testing and reference
% purposes; loading it with \LoadClass{minimal} to use it as a base
% class for some other document class is probably a mistake. If you wish
% to start a new document class based on the minimal class, it is better
% to start by copying the *contents* of minimal.cls directly into your
% new class and making suitable modifications. You may, at that point
% also want to start documenting the code using the conventions of the
% doc package, rather than using simple ascii comments as used here.
