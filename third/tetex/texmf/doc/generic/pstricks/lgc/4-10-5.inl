\newdimen\X
\newdimen\Y
\newdimen\Coor

% #1 is number of steps
\newcommand{\RandomWalk}[1]{%
  \setrandim\X{0pt}{1pt}
  \setrandim\Y{0pt}{1pt}
  \psdots[dotstyle=*,dotsize=6pt](\pointless\X,\pointless\Y)
  \pscustom{%
    \moveto(\pointless\X,\pointless\Y)
    \multido{\i=1+1}{#1}{%
      \MoveCoordinate{\X}
      \MoveCoordinate{\Y}
      \lineto(\pointless\X,\pointless\Y)}}}

% Computation of new horizontal or vertical position of point
\newcommand{\MoveCoordinate}[1]{%
  \setrandim\Coor{-0.03pt}{0.03pt}
  \advance #1 by \Coor
  \ifdim#1>1pt #1=1pt \fi
  \ifdim#1<0pt #1=0pt \fi}

\newcommand{\randomdemo}[2]{{%
  \psframe[linewidth=0.6mm](1,1)
  \rput(#1,#2){\RandomWalk{300}}
  \psset{linecolor=red}
  \rput(#1,#2){\RandomWalk{200}}
  \psset{linecolor=green}
  \rput(#1,#2){\RandomWalk{100}}}}

\psset{unit=5,dimen=middle}

\framebox{%
  \begin{pspicture}(1,3)
    \randomdemo{0}{0}
    \randomdemo{0}{1}
    \randomdemo{0}{2}
  \end{pspicture}}

