\begin{pspicture}(-1,-2.5)(9,2.5)
  \psset{xunit=.25mm,yunit=2cm}
  \psset{plotpoints=50}
  \psplot[linestyle=dashed,linecolor=blue]{0}{360}{x sin}
  \psplot[plotstyle=dots,dotstyle=triangle]{0}{360}{x cos}
  \psset{plotpoints=200}
  \psplot[linecolor=red]{0}{360}{x dup sin exch cos mul}
\end{pspicture}
