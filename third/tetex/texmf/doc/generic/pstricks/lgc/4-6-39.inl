\renewcommand\psedge{\nccurve}
\newcommand{\Female}[2][]{{\psset{linecolor=pink}\TR[#1]{\emph{#2}}}}
\newcommand{\Male}[2][]{{\psset{linecolor=blue}\TR[#1]{#2}}}

\psset{nodesep=2pt,angleA=90,angleB=-90}

\pstree[treemode=U]{\Female{{\bfseries Matilde}}}{%
  \pstree{\Male{Sebastian}}{%
    \pstree{\Male[name=P]{Philip}}{\Male{Frederick}\Female{Ethel}}
    \pstree{\Female[name=W]{Mary}}{\Male{Lionel}\Female{Agnes}}}
  \pstree{\Female{Leonor}}{
  \pstree{\Male[name=R]{Ra\'ul}}{\Male{Joaquim}\Female{J\'ulia}}
  \pstree{\Female[name=A]{Am\'elia}}{\Male{\'Alvaro}\Female{Augusta}}}
}

\psset{linecolor=green,doubleline=true,linestyle=dotted}
\ncline{P}{W}\nbput{1940}
\ncline{R}{A}\nbput{1954}
