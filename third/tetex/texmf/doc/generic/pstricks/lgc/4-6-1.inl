\begin{pspicture}(0,0)(5,5)\showgrid
  \rput(1,1){\rnode{A}{Dog}}
  \rput(2,4){\rnode{B}{Cat}}
  \rput(4,2){Mouse}
  \ncline{A}{B}
  \nccurve[linestyle=dotted]{A}{B}
  \ncarc[linestyle=dashed]{A}{B}
\end{pspicture}
