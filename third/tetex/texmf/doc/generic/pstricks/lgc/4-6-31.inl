\psset{framearc=.4}
\begin{psmatrix}[colsep=3]
  \pnode{A}&\psframebox{Waiting} & \psframebox{Calculating}
  \ncline[arrows=->]{1,1}{1,2}
  \psset{offset=-.1}
  \ncline[arrows=->]{1,2}{1,3}\naput{{\scriptsize TRIGGER/activated}}
  \psset{offset=.1}
  \ncline[arrows=<-]{1,2}{1,3}\nbput{{\scriptsize [4,11]/finished}}
\end{psmatrix}
