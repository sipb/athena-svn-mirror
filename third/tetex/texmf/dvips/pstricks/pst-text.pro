%!
% PostScript header file pst-text.pro
% Version 97, 94/04/20
% For distribution, see pstricks.tex.

/tx@TextPathDict 40 dict def
tx@TextPathDict begin

% Syntax:  <dist> PathPosition -
% Function: Searches for position of currentpath distance <dist> from
%           beginning. Sets (X,Y)=position, and Angle=tangent.
/PathPosition
{ /targetdist exch def
  /pathdist 0 def
  /continue true def
  /X { newx } def /Y { newy } def /Angle 0 def
  gsave
    flattenpath
    { movetoproc }  { linetoproc } { } { firstx firsty linetoproc }
    /pathforall load stopped { pop pop pop pop /X 0 def /Y 0 def } if
  grestore
} def

/movetoproc { continue { @movetoproc } { pop pop } ifelse } def

/@movetoproc
{ /newy exch def /newx exch def
  /firstx newx def /firsty newy def
} def

/linetoproc { continue { @linetoproc } { pop pop } ifelse } def

/@linetoproc
{
  /oldx newx def /oldy newy def
  /newy exch def /newx exch def
  /dx newx oldx sub def
  /dy newy oldy sub def
  /dist dx dup mul dy dup mul add sqrt def
  /pathdist pathdist dist add def
  pathdist targetdist ge
  { pathdist targetdist sub dist div dup
    dy mul neg newy add /Y exch def
    dx mul neg newx add /X exch def
    /Angle dy dx atan def
    /continue false def
  } if
} def

/TextPathShow
{ /String exch def
  /CharCount 0 def
  String length
  { String CharCount 1 getinterval ShowChar
    /CharCount CharCount 1 add def
  } repeat
} def

% Syntax: <pathlength> <position> InitTextPath -
/InitTextPath
{ gsave
    currentpoint /Y exch def /X exch def
    exch X Hoffset sub sub mul
    Voffset Hoffset sub add
    neg X add /Hoffset exch def
    /Voffset Y def
  grestore
} def

/Transform
{ PathPosition
  dup
  Angle cos mul Y add exch
  Angle sin mul neg X add exch
  translate
  Angle rotate
} def

/ShowChar
{ /Char exch def
  gsave
    Char end stringwidth
    tx@TextPathDict begin
    2 div /Sy exch def 2 div /Sx exch def
    currentpoint
    Voffset sub Sy add exch
    Hoffset sub Sx add
    Transform
    Sx neg Sy neg moveto
    Char end tx@TextPathSavedShow
    tx@TextPathDict begin
  grestore
  Sx 2 mul Sy 2 mul rmoveto
} def

end
% END pst-text.pro
