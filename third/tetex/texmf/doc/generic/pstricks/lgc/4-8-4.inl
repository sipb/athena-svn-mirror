\def\CubeSide#1#2#3#4{%
  \ThreeDput[normal=#1](#2){%
    \psframe*[linecolor=#3](4,4)
    \rput(2,2){\footnotesize#4}}
}

\catcode`\<=12
\catcode`\>=12

\def\TheCube#1#2{%
  \begin{pspicture}(-9,-7)(9,9)
    \bfseries
    \ifcase #1 \relax
    \or %1
      \psset{viewpoint=-1 -1 #2}
      \rput[l](-8,-6){{\normalsize Viewpoint: -1 -1 #2}}
      \ifnum#2>0\ThreeDput[normal=0 0 1]{\psgrid(-3,-3)(6,6)}\fi
      \CubeSide{-1 0 0}{0,4,0}{yellow}{FRONT}
      \CubeSide{0 -1 0}{0,0,0}{blue}{FRONT}
    \or %2
      \psset{viewpoint=1 -1 #2}
      \rput[l](-8,-6){{\normalsize Viewpoint: 1 -1 #2}}
      \ifnum#2>0\ThreeDput[normal=0 0 1]{\psgrid(-3,-3)(6,6)}\fi
      \CubeSide{0 -1 0}{0,0,0}{blue}{FRONT}
      \CubeSide{1 0 0}{4,0,0}{white}{BACK}
    \or %3
      \psset{viewpoint=1 1 #2}
      \rput[l](-8,-6){{\normalsize Viewpoint: 1 1 #2}}
      \ifnum#2>0\ThreeDput[normal=0 0 1]{\psgrid(-3,-3)(6,6)}\fi
      \CubeSide{1 0 0}{4,0,0}{white}{BACK}
      \CubeSide{0 1 0}{4,4,0}{green}{BACK}
    \or %4
      \psset{viewpoint=-1 1 #2}
      \rput[l](-8,-6){{\normalsize Viewpoint: -1 1 #2}}
      \ifnum#2>0\ThreeDput[normal=0 0 1]{\psgrid(-3,-3)(6,6)}\fi
      \CubeSide{0 1 0}{4,4,0}{green}{BACK}
      \CubeSide{-1 0 0}{0,4,0}{yellow}{FRONT}
    \fi
    \ifnum#2<0
      \CubeSide{0 0 -1}{0,4,0}{magenta}{BOT}
    \else
      \CubeSide{0 0 1}{0,0,4}{red}{TOP}
    \fi
  \end{pspicture}
}

\psset{unit=.3cm,subgriddiv=0}
\begin{tabular}{cc}
  \TheCube{1}{1}&
  \TheCube{2}{1}\\[-10pt]
  \TheCube{3}{1}&
  \TheCube{4}{1}\\
\end{tabular}
