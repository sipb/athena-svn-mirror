\psset{angleB=-90,arrows=->,nrot=:U}

\newcommand{\molesworth}[1]{%
\pstree[#1]{\Tdia{ }}
           {\Tp[arrows=->,edge={\ncbar[angleA=180]}]
            \nbput{Gabbitas}
            {\psset{linestyle=dashed,arrows=-} \Tp }
            \Tp[arrows=->,edge={\ncbar}]
            \naput{Thring}
           }
}

\psset{showbbox=true}
\begin{tabular}{l}
  \molesworth{}\\[1cm]
  \molesworth{xbbl=1cm,xbbr=1cm}
\end{tabular}
