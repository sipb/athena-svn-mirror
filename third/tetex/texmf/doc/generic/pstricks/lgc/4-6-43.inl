\psset{framearc=.2,levelsep=4cm,armB=1cm,angleB=-180}

\renewcommand{\psedge}{\ncangle}
\newcommand{\TreeBox}[1]{\Tr{\psframebox{#1}}}

\pstree[treemode=R]{\TreeBox{Monitor}}
                   {\psset{offsetA=4pt}
                    \TreeBox{Attitude Generator}
                    \naput[npos=2.5]{{\small init}}
                    \nbput[npos=2.5]{{\small stop}}
                    \psset{offsetA=-4pt}
                    \TreeBox{Normal Generator}}
