\makeatletter
\newdimen\CurX
\newdimen\CurY
\newdimen\Cir@temp
\newcount\Cir@direction % 0 right, 1 left, 2 up, 3 down

\Cir@direction0

\def\SetX#1{\CurX=#1pt}
\def\SetY#1{\CurY=#1pt}

\def\ShowDirection{%
 \typeout{Direction:
 \ifcase\Cir@direction right\or left\or up\or down\fi,
    now at (\strip@pt\CurX,\strip@pt\CurY)}%
}

\def\ShowXY{\message{(\strip@pt\CurX,\strip@pt\CurY)}}

\def\Down#1{\advance\CurY  by -#1\p@\Cir@direction3%\ShowDirection
}
\def\Up#1{\advance\CurY  by #1\p@\Cir@direction2%\ShowDirection
}
\def\Left#1{\advance\CurX by -#1\p@\Cir@direction1%\ShowDirection
}
\def\Right#1{\advance\CurX by #1\p@\Cir@direction0%\ShowDirection
}

\def\MyBox#1#2{% width,height
  \pssetlength{\Cir@temp}{#1}%
  \rule{\Cir@temp}{\z@}%
  \pssetlength{\Cir@temp}{#2}%
  \rule{\z@}{\Cir@temp}%
}

\def\Point#1{%
  \rput(\strip@pt\CurX,\strip@pt\CurY){\pnode{#1}}%
}

\def\Switch#1{%
  \message{[Switch] #1}\ShowXY
  \relax
  \ifcase\Cir@direction % right
    \rput[b](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode[b]{#1}{%
        \psline(.5,.4)%
        \psarc{<-}{.4}{-10}{70}%
        \MyBox{.5}{.4}%
        }}%
    \advance\CurX by .5\p@
  \or % left
    \advance\CurX by -.5\p@
    \rput[b](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode[b]{#1}{%
        \psline(0,0)(.5,.4)%
        \psarc{<-}{.4}{-10}{70}%
        \MyBox{.5}{.4}%
        }}%
  \fi
}

\def\Battery#1{%
  \message{[Battery] #1}\ShowXY
  \relax
  \ifcase\Cir@direction % right
    \rput[r](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode{#1}{%
        \psline(0,-.4)(0,.4)%
        \psline(.2,-.2)(.2,.2)%
        \MyBox{.2}{.8}%
        }}%
    \advance\CurX by .2\p@
  \or % left
    \advance\CurX by -.2\p@
    \rput[r](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode{#1}{%
        \psline(0,-.4)(0,.4)%
        \psline(.2,-.2)(.2,.2)%
        \MyBox{.2}{.8}%
        }}%
  \or % up
    \advance\CurY by .2\p@
    \rput[b](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode{#1}{%
        \psline(.1,.2)(.9,.2)%
        \psline(0.3,0)(0.7,0)%
        \MyBox{1}{.2}%
      }}%
  \or % down
    \rput[b](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode{#1}{%
        \psline(-.4,0)(.4,0)%
        \psline(-.2,-.2)(.2,-.2)%
        \MyBox{1}{.2}%
        }}%
    \advance\CurY by -.2\p@
  \fi
}

\def\Resistor{\message{[Resistor]}\ShowXY
  \@ifnextchar[{\@ResInd}{\@ResInd[1]{pszigzag}}}
\def\Inductor{\message{[Inductor]}\ShowXY
  \@ifnextchar[{\@ResInd}{\@ResInd[1]{pscoil}}}

\def\@ResInd[#1]#2#3{%
  \ifcase\Cir@direction % right
    \rput[l](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode{#3}{%
        \csname #2\endcsname[coilarm=.01,coilwidth=.3](0,.15)(#1,.15)%
        \MyBox{#1}{.3}%
        }}%
    \advance\CurX by #1\p@
  \or % left
    \advance\CurX by -#1\p@
    \rput[l](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode{#3}{%
        \csname #2\endcsname[coilarm=.01,coilwidth=.3](0,.15)(#1,.15)%
        \MyBox{#1}{.3}%
        }}%
  \or % up
    \rput[b](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode{#3}{%
        \csname #2\endcsname[coilarm=.01,coilwidth=.3](.15,0)(.15,#1)%
        \MyBox{.3}{#1}%
        }}%
    \advance\CurY by #1\p@
  \or % down
    \advance\CurY by -#1\p@
    \rput[b](\strip@pt\CurX,\strip@pt\CurY){%
      \rnode{#3}{%
        \csname #2\endcsname[coilarm=.01,coilwidth=.3](.15,0)(.15,#1)%
        \MyBox{.3}{#1}%
        }}%
  \fi
}
\makeatother

\begin{pspicture}(-0.3,0)(8.4,2.8)
  \SetX{1}
  \Up{1}
  \Battery{A}
  \Up{1}
  \Right{2}
  \Resistor{B}
  \Right{1}
  \Down{.3}
  \Inductor{C}
  \Down{.5}
  \Left{1}
  \Switch{D}
  \ncangle[angleA=90,angleB=180,armB=0]{A}{B}
  \ncangle[angleA=0,angleB=90,armB=0]{B}{C}
  \ncput[ref=l]{~~~$i=i_{\mbox{max}}(1-w^{-t/3})$}
  \ncangle[angleA=-90,armB=0,angleB=0]{C}{D}
  \ncangle[angleA=180,armB=0,angleB=-90]{D}{A}
  \nput{180}{A}{10V}
  \nput{90}{B}{R}
  \nput{0}{C}{3mH}
  \nput{270}{D}{S}
\end{pspicture}
