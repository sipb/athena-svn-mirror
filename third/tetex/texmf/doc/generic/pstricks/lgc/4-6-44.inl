\psset{arrows=->,framearc=.2}

\newcommand{\Treebox}[1]{\Tr{\psframebox{#1}}}

\pstree[treemode=R]{\Treebox{A$\rightarrow$B}}
                   {\pstree{\Treebox{B$\rightarrow$C}}
                           {\Treebox{A$\rightarrow$D}
                            \pstree[treemode=L]{\Treebox{B$\rightarrow$E}}
                                               {\Tn\TC[arrows=<-]}
                           }
                   }
