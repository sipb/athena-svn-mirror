\begin{pspicture}(-.2,-.5)(3,1)\showgrid
  \rput(.5,.5){\rnode{A}{\psframebox{Cat}}}
  \rput(2.5,.5){\rnode{B}{\psframebox{Dog}}}
  \ncarc{->}{A}{B}
  \ncarc{->}{B}{A}
\end{pspicture}
