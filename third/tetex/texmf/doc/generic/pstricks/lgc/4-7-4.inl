\newcounter{lastval}
\newcounter{thishalf}
\newcounter{thisval}

\newcommand\lang[4]{% name, value, percentage, color
  \setcounter{thisval}{\value{lastval}}
  \addtocounter{thisval}{#3}
  \pswedge[fillcolor=#4]{1}{\thelastval}{\thethisval}%
  \setcounter{thishalf}{((\value{thisval}-\value{lastval})/2)+\value{lastval}}
  %\typeout{#1: \thethishalf}%
  \ifnum#3>200\rput(1.3;\thethishalf){#1}\fi
  \setcounter{lastval}{\value{thisval}}
}

\bgroup
\psset{unit=3}
\centerline{%
  \begin{pspicture}(-1.3,-1.3)(1.3,1.3)
  \psset{fillstyle=solid}
  \degrees[10000]
  \SpecialCoor
  \setcounter{lastval}{0}
  \lang{Romanian}{1}{3}{green}
  \lang{Czech}{2}{6}{blue}
  \lang{Bulgarian}{4}{11}{green}
  \lang{Japanese}{4}{11}{palegreen}
  \lang{Dutch}{7}{20}{black}
  \lang{Norwegian}{20}{56}{cyan}
  \lang{Greek}{26}{73}{magenta}
  \lang{Swedish}{34}{95}{lightgray}
  \lang{Danish}{46}{129}{white}
  \lang{French}{83}{232}{pink}
  \lang{Latin}{146}{409}{wheat}
  \lang{Russian}{243}{680}{white}
  \lang{Italian}{391}{1093}{gray}
  \lang{German}{508}{1421}{lightblue}
  \lang{unknown}{599}{1676}{red}
  \lang{English}{1462}{4085}{yellow}
\end{pspicture}}
\egroup

