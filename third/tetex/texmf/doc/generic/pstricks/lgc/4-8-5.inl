\newcommand{\DieFrame}[1][darkgray]{%
  \psframe[linecolor=black,fillcolor=#1,fillstyle=solid](4,4)}

\newcommand{\SpotColor}{white}
\newcommand{\DieOne}{\DieFrame[lightgray]
  \pscircle*[linecolor=\SpotColor](2,2){.3}
}
\newcommand{\DieTwo}{\DieFrame[lightgray]
  \pscircle*[linecolor=\SpotColor](1,2){.3}
  \pscircle*[linecolor=\SpotColor](3,2){.3}
}
\newcommand{\DieThree}{\DieFrame[lightgray]
  \pscircle*[linecolor=\SpotColor](1,1){.3}
  \pscircle*[linecolor=\SpotColor](2,2){.3}
  \pscircle*[linecolor=\SpotColor](3,3){.3}
}
\newcommand{\DieFour}{\DieFrame
  \pscircle*[linecolor=\SpotColor](1,1){.3}
  \pscircle*[linecolor=\SpotColor](3,3){.3}
  \pscircle*[linecolor=\SpotColor](1,3){.3}
  \pscircle*[linecolor=\SpotColor](3,1){.3}
}
\newcommand{\DieFive}{\DieFrame
  \pscircle*[linecolor=\SpotColor](1,1){.3}
  \pscircle*[linecolor=\SpotColor](3,3){.3}
  \pscircle*[linecolor=\SpotColor](1,3){.3}
  \pscircle*[linecolor=\SpotColor](3,1){.3}
  \pscircle*[linecolor=\SpotColor](2,2){.3}
}
\newcommand{\DieSix}{\DieFrame
  \pscircle*[linecolor=\SpotColor](1,1){.3}
  \pscircle*[linecolor=\SpotColor](1,2){.3}
  \pscircle*[linecolor=\SpotColor](1,3){.3}
  \pscircle*[linecolor=\SpotColor](3,1){.3}
  \pscircle*[linecolor=\SpotColor](3,2){.3}
  \pscircle*[linecolor=\SpotColor](3,3){.3}
}
\newcommand\DieSide[3]{\ThreeDput[normal=#1](#2){#3}}

\def\TheDie#1#2(#3)(#4){%
  \begin{pspicture}(#3)(#4)
  \ifcase #1 \relax
  \or %1
    \psset{viewpoint=-1 -1 #2}
    \DieSide{-1 0 0}{0,4,0}{\DieThree}
    \DieSide{0 -1 0}{0,0,0}{\DieFive}
  \or %2
    \psset{viewpoint=1 -1 #2}
    \DieSide{0 -1 0}{0,0,0}{\DieFive}
    \DieSide{1 0 0}{4,0,0}{\DieFour}
  \or %3
    \psset{viewpoint=1 1 #2}
    \DieSide{1 0 0}{4,0,0}{\DieFour}
    \DieSide{0 1 0}{4,4,0}{\DieTwo}
  \or %4
    \psset{viewpoint=-1 1 #2}
    \DieSide{0 1 0}{4,4,0}{\DieTwo}
    \DieSide{-1 0 0}{0,4,0}{\DieThree}
  \fi
  \ifnum#2<0
    \DieSide{0 0 -1}{0,4,0}{\DieSix}
  \else
    \DieSide{0 0 1}{0,0,4}{\DieOne}
  \fi
  \end{pspicture}}

\psset{unit=.5cm}
\begin{tabular}{ccc}
  \TheDie{1}{1}(-4,-0.5)(5,7.5)&
  \TheDie{2}{1}(-1.5,-1.5)(7.5,6.5)&
  \TheDie{3}{1}(-4,-3)(5,5)\\
  \TheDie{4}{1}(-6.5,-2)(2.5,6)&
  \TheDie{1}{-1}(-4,-3)(5,5)&
  \TheDie{2}{-1}(-1.5,-1.5)(7.5,6.5)\\
  \TheDie{3}{-1}(-4,-0.5)(5,7.5)&
  \TheDie{4}{-1}(-6.5,-2)(2.5,6)\\
\end{tabular}
