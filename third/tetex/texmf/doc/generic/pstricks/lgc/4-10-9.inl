\DeclareFixedFont{\curly}{T1}{pzc}{m}{it}{30}
%
% The cat is designed to appear on a 10 x 10 grid
%
% cat head
\newcommand{\Cathead}{%
  \pscircle[fillcolor=black](5,4.2){2.5}%
  % ears
  {%
   \psset{linecolor=black,fillcolor=pink,linewidth=.05,linestyle=solid}
   \rput{45}(5,4.2){\pspolygon(2.5,.5)(2.5,-.5)(3.5,0)}
   \rput{135}(5,4.2){\pspolygon(2.5,.5)(2.5,-.5)(3.5,0)}
  }%
}

% eyes, nose and whiskers
\newcommand{\Catface}{%
  \pscircle[fillcolor=yellow](4,5.2){.5}
  \psdiamond[fillcolor=gray](4,5.2)(.2,.5)
  \pscircle[fillcolor=yellow](6,5.2){.5}
  \psdiamond[fillcolor=gray](6,5.2)(.2,.5)
  % nose
  \rput{180}(5,4.6){\pstriangle[fillcolor=pink](.5,.5)}
  % whiskers
  {%
   \psset{linecolor=white,linestyle=solid,linewidth=.1}
   \rput{5}(5,4.2){\psline(.8,0)(1.8,0)}
   \rput{15}(5,4.2){\psline(.8,0)(1.8,0)}
   \rput{165}(5,4.2){\psline(.8,0)(1.8,0)}
   \rput{175}(5,4.2){\psline(.8,0)(1.8,0)}
  }%
}

% paws with claws
\newcommand{\Catpaws}{%
  \pscircle[fillcolor=black](1,4.2){.8}
  \pscircle[fillcolor=black](9,4.2){.8}
  {%
   \psset{fillcolor=yellow}
   \newcommand{\clawsize}{.4,.4}
   \rput{180}(1,4.4){\pstriangle(\clawsize)}
   \rput{180}(1,4.4){\pstriangle(-.45,0)(\clawsize)}
   \rput{180}(1,4.4){\pstriangle(.45,0)(\clawsize)}
   \rput{180}(9,4.4){\pstriangle(\clawsize)}
   \rput{180}(9,4.4){\pstriangle(-.45,0)(\clawsize)}
   \rput{180}(9,4.4){\pstriangle(.45,0)(\clawsize)}
  }%
}

% wall
\newcommand{\Wall}{%
  \psframe[fillcolor=brown](0,0)(10,4)
}

% The whole cat on its wall
\newcommand{\Cat}[1]{%
  {\psset{unit=#1}
   \Cathead\Catface\Catpaws\Wall}%
}

% bricks
\newcommand{\Bricks}{%
  \bfseries\large
  \psset{fillcolor=wheat}
  \psframe(1,.4)(2.5,1.9)
  \rput[bl](1.1,1){\LaTeX}
  \psframe(5,.4)(6.3,1.9)
  \psframe(7,.4)(8.5,1.9)
  \psframe(2,2.2)(3.2,3.7)
  \rput[bl]{90}(2.6,2.4){\normalsize$e=mc^2$}
  \psframe(5.3,2.2)(8,3.7)
  \rput[bl](5.4,2.8){\textsc{PostScript}}
}

\begin{pspicture}(10,8)
  \psset{fillstyle=solid,linestyle=none,linewidth=0}
  \psframe[fillcolor=lightblue](10,8)
  \Cat{1}
  \rput[bl]{5}(1,1){\curly\color{white}Don Knuth Rules OK}
  \Bricks
  \rput(7,1){\Cat{.1}}
  \rput(.1,2){\Cat{.15}}
\end{pspicture}
