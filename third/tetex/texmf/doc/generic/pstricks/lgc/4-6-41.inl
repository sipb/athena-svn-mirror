\newcommand{\XX}[1]{\Tr{\psframebox{\rule{0pt}{9pt}#1}}}
\renewcommand{\psedge}{\ncangle}

\psset{angleB=90,angleA=-90,levelsep=36pt,armB=14pt}

\pstree{\XX{grandmother}}
       {\pstree{\XX{aunt}}{\XX{cousin}}
        \pstree{\XX{mother}}
               {\pstree{\XX{sister}}{\XX{niece}}
                \pstree{\XX{Me}}
                       {\pstree{\XX{daughter}}
                               {\XX{granddaughter}}
                       }
               }
       }
