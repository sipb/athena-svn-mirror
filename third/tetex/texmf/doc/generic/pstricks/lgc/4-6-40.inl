\newcommand{\Item}[1]{\Tr[ref=l]{\psframebox[linestyle=none]{#1}}}
\renewcommand{\psedge}{\ncangle}

\psset{xbbd=1.5cm,treemode=R,angleB=-180,angleA=0,levelsep=72pt}

\pstree{\Item{langs}}{%
  \Item{german}
  \pstree{\Item{greek}}{%
    \Item{h  kelly}
    \pstree{\Item{levy}}{%
      \Item{doc}
      \Item{src}
       }
    }
  \Item{italian}
  \pstree{\Item{russian}}{
    \Item{hyphen}
   }
  \pstree{\Item{turkish}}{%
    \Item{hyphen}
    \Item{inputs}
    \Item{mf}
   }
}
