\psset{arrows=->,fillcolor=white,fillstyle=solid}

\newcommand{\Show}[1]{\psshadowbox{#1}}

\begin{psmatrix}[mnode=r,ref=t]
  \psframebox[linestyle=none,framesep=.75]{%
    \begin{psmatrix}[name=A,ref=c]
      \Show{Stakeholder}
    \end{psmatrix}} &
  \psframebox[fillstyle=solid,fillcolor=pink,framesep=.75]{%
    \rule{1cm}{0pt}
    \begin{psmatrix}[ref=c]
      [name=B]\Show{Goal} & \Show{Criteria}\\
              \Show{Sub-goal} & \Show{Justification}
      \ncline{1,1}{1,2}
      \ncline{1,1}{2,2}
      \ncline{1,1}{2,1}\tlput{Strategy}
      \ncline{2,1}{2,2}
    \end{psmatrix}}
\end{psmatrix}
\ncline[angleB=180]{A}{B}\naput[npos=.7]{Model}
