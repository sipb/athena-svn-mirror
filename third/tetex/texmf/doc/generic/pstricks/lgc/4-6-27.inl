\begin{pspicture}(0,0)(5,2.5)
  \begin{psmatrix}[rowsep=1.5cm]
                  & City  &\\
    {\tiny Shack} & House & {\Large Hotel}
    \psset{arrows=<<-}
    \ncline{1,2}{2,1}<{a}
   \ncline{1,2}{2,2}>{b}
   \ncline{1,2}{2,3}>{b}
   \psset{arrows=-,linestyle=dotted}
   \ncline{2,1}{2,2}
   \ncline{2,2}{2,3}
  \end{psmatrix}
\end{pspicture}
