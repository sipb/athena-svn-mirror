\psset{arrows=->,labelsep=3pt,linecolor=gray,mnode=circle}
\begin{psmatrix}[rowsep=20pt,colsep=28pt]
  A & B \\
  $\sqrt{\frac{x + y}{z}}$ & D
  \psset{linestyle=dotted}
  \ncline{1,1}{1,2}\naput{\emph{firstly}}
  \ncline{1,2}{2,2}\naput{\emph{next}}
  \ncline{2,2}{2,1}\naput{\emph{then}}
  \ncline{2,1}{1,1}\naput{\emph{lastly}}
  \nccurve[ncurv=2,linestyle=solid,angleA=90]{1,1}{2,2}
\end{psmatrix}
