\psset{arrows=->,labelsep=3pt,linecolor=gray,mnode=circle}
\begin{psmatrix}[rowsep=20pt,colsep=28pt]
  A & B \\  $\sqrt{\frac{x + y}{z}}$ & D
  \psset{linestyle=dotted}
  \ncline{1,1}{1,2}^{\emph{firstly}}
  \ncline{1,2}{2,2}>{\emph{next}}
  \ncline{2,2}{2,1}_{\emph{then}}
  \ncline{2,1}{1,1}<{\emph{lastly}}
\end{psmatrix}
