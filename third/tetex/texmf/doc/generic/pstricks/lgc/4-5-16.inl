\DeclareFixedFont{\bigsf}{T1}{phv}{b}{n}{1.75cm}
\DeclareFixedFont{\tinyrm}{T1}{ptm}{m}{n}{2mm}

\newcounter{myN}
\setcounter{myN}{300}

\begin{pspicture}(0,0)(11,1.4)
  \begin{pscharclip}[linecolor=red,fillstyle=solid,fillcolor=yellow]
                    {\rput[bl](0,0){\bigsf CHOCOLATE}}
  \rput[t]{90}(0,0){%
    \begin{minipage}{4cm}
      \offinterlineskip
      \raggedright\tinyrm\color{black}%
      \whiledo{\value{myN}>0}%
        {%
         \addtocounter{myN}{-1}
          nuts and raisins
        }%
    \end{minipage}}%
  \end{pscharclip}
\end{pspicture}
