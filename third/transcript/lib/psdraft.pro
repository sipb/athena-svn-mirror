/DoDraft { % string outline gray -pagewidth rotation x y size name
  gsave
  findfont exch scalefont setfont
  moveto
  rotate 0 exch translate
  setgray
  exch /tmp exch def
  { tmp false charpath stroke }
  { tmp show } ifelse
  grestore
} def
