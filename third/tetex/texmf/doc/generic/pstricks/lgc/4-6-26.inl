\newcounter{CtA}
\newcounter{CtB}

\newcommand{\Wheel}[3]{{%
  \color{black}
  \psset{unit=#2}
  \begin{pspicture}(-1,-1.2)(1,1.2)
    \SpecialCoor
    \degrees[#1]
    \multido{\ia=1+1}{#1}{%
      \setcounter{CtA}{\ia}%
      \stepcounter{CtA}%
      \setcounter{CtB}{#1}%
      \addtocounter{CtB}{-\ia}%
      \multido{\ib=\value{CtA}+1}{%
        \value{CtB}}{#3(1;\ia)(1;\ib)}}
    \multido{\i=1+1}{#1}{%
      \rput(1;\i){%
        \pscirclebox[fillstyle=solid,fillcolor=white]{\footnotesize\i}}}
  \end{pspicture}}}% end of newcommand

{%
 \color{white}
 \fbox{\Wheel{3}{1.2}{\psline}
 \Wheel{5}{1.2}{\psline}}
 \psset{arcangle=10}
 \fbox{\Wheel{12}{3}{\pcarc[linecolor=blue]}}
}

