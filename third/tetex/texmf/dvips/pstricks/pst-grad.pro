%!
% PostScript prologue for pst-grad.tex.
% Version 97, 93/05/12
% For distribution, see pstricks.tex.
%
% For the PSTricks gradient fillstyle.
%
% Based on some EPS files by leeweyr!bill@nuchat.sccsi.com (W. R. Lee).
%
% Syntax:
%   R0 G0 B0 R1 G1 B1 NumLines MidPoint Angle GradientFill

/tx@GradientDict 40 dict def
tx@GradientDict begin
/GradientFill {
  rotate
  /MidPoint ED
  /NumLines ED
  /LastBlue ED
  /LastGreen ED
  /LastRed ED
  /FirstBlue ED
  /FirstGreen ED
  /FirstRed ED
  % This avoids gaps due to rounding errors:
  clip
  pathbbox           %leave llx,lly,urx,ury on stack
  /y ED /x ED
  2 copy translate
  y sub neg /y ED
  x sub neg /x ED
  % This avoids gaps due to rounding errors:
  LastRed FirstRed add 2 div
  LastGreen FirstGreen add 2 div
  LastBlue FirstBlue add 2 div
  setrgbcolor
  fill
  /YSizePerLine y NumLines div def
  /CurrentY 0 def
  /MidLine NumLines 1 MidPoint sub mul abs cvi def
  MidLine NumLines 2 sub gt
  { /MidLine NumLines def }
  { MidLine 2 lt { /MidLine 0 def } if }
  ifelse
  MidLine 0 gt
  {
    /Red FirstRed def
    /Green FirstGreen def
    /Blue FirstBlue def
    /RedIncrement LastRed FirstRed sub MidLine 1 sub div def
    /GreenIncrement LastGreen FirstGreen sub MidLine 1 sub div def
    /BlueIncrement LastBlue FirstBlue sub MidLine 1 sub div def
    MidLine { GradientLoop } repeat
  } if
  MidLine NumLines lt
  {
    /Red LastRed def
    /Green LastGreen def
    /Blue LastBlue def
    /RedIncrement FirstRed LastRed sub NumLines MidLine sub 1 sub div def
    /GreenIncrement FirstGreen LastGreen sub NumLines MidLine sub 1 sub div def
    /BlueIncrement FirstBlue LastBlue sub NumLines MidLine sub 1 sub div def
    NumLines MidLine sub { GradientLoop } repeat
  } if
} def
/GradientLoop {
  0 CurrentY moveto
  x 0 rlineto
  0 YSizePerLine rlineto
  x neg 0 rlineto
  closepath
  Red Green Blue setrgbcolor fill
  /CurrentY CurrentY YSizePerLine add def
  /Blue Blue BlueIncrement add def
  /Green Green GreenIncrement add def
  /Red Red RedIncrement add def
} def

end
% END pst-grad.pro
