%%% File: grafbase.mp
%%% A part of mfpic 0.6b 2003/01/02
%%%
%  Several corrections due to Jaromir Kuben, Mar 2000
string fileversion, filedate;
fileversion:="0.6 beta"; filedate:="2003/01/02";

message "MPpic version " & fileversion & ", " & filedate;
message "";

% Don't complain when variables get too large.
interim warningcheck := 0;

% ensure that Plain MetaPost is loaded.
if unknown base_name:
 input plain;
elseif not string base_name:
 input plain;
elseif base_name <> "plain":
 input plain;
fi

boolean METAPOST,METAFONT;

if unknown blue:
  message "This shouldn't happen, but..";
  message "You are inputting to Meta_FONT_ a file designed for Meta_POST_.";
  message "Did you mistakenly rename grafbase.mp to grafbase.mf?";
  endinput;
fi
METAPOST:=true;
METAFONT:=false;


%% Global Variables.

% To avoid error messages from processing mfpic files
% (and to avoid having to change how mfpic initializes those files):
pt#:=1pt;
aspect_ratio:=1;

if not boolean debug:  boolean debug;   fi
if not known debug:    debug := false;  fi

% when the boolean variable `grafbase' is known,
% then grafbase.mf has been input.
% (idea taken from DEK's cmbase.mf)

if known grafbase:
  errhelp ("Please make sure that grafbase is loaded only once.");
  errmessage ("You have loaded grafbase more than once");
else:
  boolean grafbase;
  grafbase := true;
fi

% METAPOST title string t, and message t.

vardef mftitle expr t =
  t;
  message t;
enddef;

% Picture size variables, and their default values.

numeric unitlen, xscale, yscale;
numeric xneg, xpos, yneg, ypos;

unitlen := 1 pt;
xscale := 7.227;  % (xscale * unitlen) = 1/10 inch
yscale := 7.227;  % (yscale * unitlen) = 1/10 inch
xneg := 0;
xpos := 10;
yneg := 0;
ypos := 10;

% degrees for angles.  In METAPOST, one degree is the unit of angle.

deg := 1;

% Support for radians: user can say
%             \sector{(0,0),1,0,pi/2*radian}
%           or \sector{(0,0),1,0,90 deg}
% (Real men don't use degrees.)
% First, a useful constant:
pi := 3.1415926;
numeric radian;
pi*radian = 180*deg;

% Drawing Pen.

% pen width for drawing, in _device_ coordinates.
newinternal penwd;
interim penwd:=0.5pt;

% pen for drawing.
pen drawpen;
% macro to resize it:
def resizedrawpen (expr s) =
  interim penwd := s;
  drawpen := pencircle scaled penwd yscaled aspect_ratio;
  pickup drawpen;
enddef;

% Hatching Pen.

% pen width for hatching, in _device_ coordinates.
numeric hatchwd;
hatchwd := 0.5pt;

% pen for hatching.
pen hatchpen;
hatchpen := pencircle scaled hatchwd;

% Colors: black, white, red, green and blue are part of plain.mp

color cyan, magenta, yellow,
% maybe maintain a current color (not actually used anywhere yet)
% Set default colors for certain graphic items:
fillcolor, drawcolor, hatchcolor, headcolor, tlabelcolor, currentcolor;
cyan := green + blue;
magenta := red + blue;
yellow := red + green;
currentcolor=fillcolor=drawcolor=hatchcolor=headcolor=tlabelcolor=black;

def wc_ = withcolor currentcolor enddef; % not yet in use
def wd_ = withcolor drawcolor enddef;
def wf_ = withcolor fillcolor enddef;

%% This is almost the algorithm used in color.pro (as distributed
%% with dvips) to convert cmyk color to rgb values.

vardef snapto (expr t) =
  if not (numeric t):
    0
  else :
    min(max(t,0),1)
  fi
enddef;

vardef cmyk (expr c, m, y ,k) =
 white - ( snapto(c+k), snapto(m+k), snapto(y+k))
enddef;

% Just divide RGB by 255 to get rgb:
vardef RGB (expr R, G, B) =
  (snapto(R/255), snapto(G/255), snapto(B/255))
enddef;

% gray is fraction of white:
vardef gray (expr g) =
  (snapto(g))*white
enddef;

% Almost no-ops:
vardef rgb(expr r,g,b)= (snapto(r),snapto(g),snapto(b)) enddef;
vardef named (expr c) = if color c : c else : black fi enddef;

% Storing a Path.

def store (suffix fs) expr f =
 hide (
   if (not path f) and (not pair f) :
     errhelp ("");
     errmessage ("Second argument to `store' must be a path or pair expression");
   fi
   if not path fs:
     path fs;
   fi
   fs := f;
 )
enddef;

% Clipping.

% clipping state.
boolean ClipOn;
ClipOn := false;

% For clipping whole picture, default is no clip:
boolean clipall; clipall:=false;

% For keeping actual bounding box of the whole picture.
% Default is to force the bbox to the given dimensions:
boolean truebbox; truebbox:=false;

% clipping paths.
path ClipPath[];
numeric ClipPath;
ClipPath = 0;  % initially no clipping paths, and no candidates known.

%% Frank Michielsen's macro, slightly modified.
vardef list (suffix v) (text lst) =
    v := 0;
    for item_= lst:
      v[incr v]:= item_;
    endfor;
enddef;


%% Utility Macros.

% (Reference for map macro: Abelson & Sussman,
% _Structure & Interpretation of Computer Programs_,
% page 250.)

% Generate new text list by applying proc to each item in given
% text list.  The new list must be separated by commas, since I
% wish to use it as the list in a `for' loop; this requirement
% is difficult to reconcile with other needs.

% NOTE WELL:  Text arguments are particularly pernicious,
% because they can inadvertently include a local name
% used in the body of the macro.  Also, METAPOST then gives
% rather uninformative error messages.

def map (text proc) (text lst) =
  hide(_map_:=0;)
  for a_=lst:
    if _map_= 0:
      hide(_map_:=1;) proc (a_)
    else :
      , proc(a_)
    fi
  endfor
enddef;

% apply procedure proc to each member of array p[] with p members.

def maparr (text proc) (suffix p) =
  for i_=1 upto p:
    proc (p[i_]);
  endfor
enddef;

% convert text pairs list t to array p.
% best used inside a vardef.
% note:  t must be a comma-separated list,
% because the `for' needs it.

%%%%HERE: testing "numeric" instead of "save". Why?
% 1. Could be problems if p had a suffix: not allowed to save those.
% 2. User might forget that textpairs did a save and be surprised
%    when his suffix disappeared. (No problems with our uses yet.)
% Calling routine must perform the save, if necessary.
def textpairs (suffix p) (text t) =
 numeric p; pair p[];
 list (p) (t)
enddef;

% chpair : `change pair'.
% apply macro proc (which maps numeric -> numeric)
% to each part of pair p, return resultant pair.

vardef chpair (text proc) (expr p) =
  (proc (xpart p), proc (ypart p))
enddef;

vardef floorpair (expr p) =
  chpair (floor) (p)
enddef;

vardef ceilingpair (expr p) =
  chpair (ceiling) (p)
enddef;

% Return the pair comprising the minimum x and minimum y coordinates
% of all pairs in the array p[], where p itself is a numeric count of
% the members in p[].

vardef minpair (suffix p) =
 save x_, y_; numeric x_, y_;
 x_ := xpart p1;
 y_ := ypart p1;
 for i_=2 upto p:
   x_ := min (x_, xpart p[i_]);
   y_ := min (y_, ypart p[i_]);
 endfor
 (x_,y_)
enddef;

% Similarly, but for the maximum coordinates.

vardef maxpair (suffix p) =
 save x_, y_; numeric x_, y_;
 x_ := xpart p1;
 y_ := ypart p1;
 for i_=2 upto p:
    x_ := max (x_, xpart p[i_]);
    y_ := max (y_, ypart p[i_]);
 endfor
 (x_, y_)
enddef;


%% Coordinate Conversion.
% (affine transformation)
% graph -> device
% xneg, xpos, yneg, ypos are in _graph_ coordinates.
% _graph_ and _device_ coordinates coincide iff ztr is the identity
% transform.

transform ztr, vtr;

% (Also, I may try coordinate transformations of currenttransform.
%  Consider bcoords ... ecoords.)

def setztr =
 if debug:
  message " DEBUG ";
  message "w = " & decimal w_ & " pt";
  message "h = " & decimal h_ & " pt";
  message "xneg = " & decimal xneg;
  message "xpos = " & decimal xpos;
  message "yneg = " & decimal yneg;
  message "ypos = " & decimal ypos;
  message "xscale = " & decimal xscale;
  message "yscale = " & decimal yscale;
  message "unitlen = " & decimal unitlen & "pt";
  message " GUBED ";
 fi
% help keep coordinate transforms local:
 save ztr, vtr;
 transform ztr, vtr;
 vtr := identity
   xscaled (xscale*unitlen)
   yscaled (yscale*unitlen);
 ztr := identity
   shifted (-(xneg,yneg)) transformed vtr;
 if debug:
   message " DEBUG ";
   message "ztr: ";
   show ztr;
   message "vtr: ";
   show vtr;
   message " GUBED ";
 fi
enddef;

% zconv converts a variety of expressions from graph -> device coords.
% The expressions include pairs, paths, and transforms.
% This is an affine transform.

vardef zconv (expr a) =
  a transformed ztr
enddef;

% invzconv converts a variety of expressions from device -> graph coords.
% The expressions include pairs, paths, and transforms.
% This is an affine transform.

vardef invzconv (expr a) =
 a transformed (inverse ztr)
enddef;

% vconv converts a vector v from graph -> device coords.
% This is a linear (ie, vector) transform.

vardef vconv (expr v) =
 v transformed vtr
enddef;

% invvconv converts a vector from device -> graph coords.
% This is a linear (ie, vector) transform.

vardef invvconv (expr v) =
 v transformed (inverse vtr)
enddef;

%% Initial Setup.

% active_plane is the active drawing plane.
% currentpicture is unknown at this stage
% (because it's set in beginfig),
% so use a def, not a picture assignment.

def active_plane = currentpicture enddef;

vardef image(text t) =
  save active_plane;
  picture active_plane;
  active_plane := nullpicture;
  t;
  active_plane
enddef;

def initpic =
 setztr;

 % Set drawpen to standard shape and size.
 drawpen := pencircle scaled penwd;
 % Sets currentpen to drawpen.
 pickup drawpen;

% If ClipOn is true, ClipPath must be correctly set.
 if ClipOn:
   ClipPath := 1;
   % Specified figure boundary
   ClipPath[1] := rect (origin, (w_,h_));
 fi
 if debug:
   message " DEBUG ";
   message "Drawing Nominal Bounding Box Around MP Picture";
   safedraw rect (origin, (w_,h_));
   message " GUBED ";
 fi
 % initialize text label bounds.
 labelbb.lft:= 0;
 labelbb.bot:= 0;
 labelbb.rt:= 0;
 labelbb.top:= 0;
enddef;

%% Compatibility with older graphbase.mf (for fig2dev's genmf.c)

def bounds (expr a,b,c,d) =
 xneg := a;
 xpos := b;
 yneg := c;
 ypos := d;
enddef;

%% Figure Wrapper.

def beginmfpic (expr ch) =
  beginfig (ch);
  gcode:=ch;
  save w_,h_,d_; numeric w_,h_,d_;
    w_:=(xpos-xneg)*xscale*unitlen;
    h_:=(ypos-yneg)*yscale*unitlen;
    d_:=0;
    w:=w_;
    h:=h_;
    d:=0;

  initpic;
enddef;

def endmfpic =
 if debug:
  message " DEBUG ";
  message "width = " & decimal w_ & "pt";
  message "height = " & decimal h_ & "pt";
  message " GUBED ";
 fi
 if ClipOn and (ClipPath > 0):
   clipsto(active_plane, ClipPath);
 fi
 if clipall:
  % Specified figure boundary.
  clipto (active_plane) rect(origin, (w_,h_));
 fi
 if showbbox:
  safedraw rect(origin, (w_,h_));
 fi
 save llx_, lly_, urx_, ury_;
 if truebbox:
  % If a BoundingBox has a side of length 0, GS barfs on dvips output.
  % So use natural bounds, only if neither dimension is 0.
  llx_ := xpart (llcorner currentpicture);
  lly_ := ypart (llcorner currentpicture);
  urx_ := xpart (urcorner currentpicture);
  ury_ := ypart (urcorner currentpicture);
  if urx_ = llx_ :
   urx_ := 1 + floor llx_;
  fi
  if ury_ = lly_ :
   ury_ := 1 + floor lly_;
  fi
  % Use calculated bbox instead of natural one.
  setbounds currentpicture to rect ((llx_,lly_),(urx_,ury_));
 else: % force bounds, but allow for labels.
  if clipall: % labels have been clipped off
   setbounds currentpicture to rect(origin,(h_,w_))
  else: % expand to enclose labels, if necessary.
   llx_ := min(0, labelbb.lft);
   lly_ := min(0, labelbb.bot);
   urx_ := max(w_,labelbb.rt);
   ury_ := max(h_,labelbb.top);
   setbounds currentpicture to rect((llx_,lly_),(urx_,ury_))
  fi
 fi
 endfig;
enddef;

%%%%%%
% When we want MP to do the labels
%
% The following adjust position of labels (device units)
pair label_adjust; % constant vector displacement of all labels
label_adjust:=(0,0);
numeric label_sep; % displacement _away_ from reference point _provided_
label_sep:=0;      % that point is on the boundary of the text box.

% a,b determine horizontal position (1,0)=l, (.5,.5)=c, (0,1) = r.
% c,d determine vertical position (1,0)=b, (0,0)=B, (.5,.5)=c, (0,1)=t.
% r is degrees of rotation about specified reference point.
% s is string or picture expression, pts are in graph coordinates
%

vardef thegblabel (expr z,r,p) =
  ((p shifted z) rotated r) shifted label_adjust
enddef;

vardef gblabel (expr a,b,c,d,r) (expr s) (text pts) =
  save pic_,_pic_,zz_;
  picture pic_;
  pic_ := s if not picture s : % i.e., s is a string
    infont defaultfont scaled defaultscale
  fi;
  pair zz_;
  zz_ := -(a*(xpart llcorner pic_) + b*(xpart urcorner pic_),
         c*(ypart llcorner pic_)+d*(ypart urcorner pic_))
         + (a-b,c-d)*label_sep;
  pic_:= thegblabel(zz_,r,pic_);
  save b_; pair b_;
  for a_ = pts :
    b_ := zconv(a_);
    addto active_plane also pic_ shifted b_ withcolor tlabelcolor;

    labelbb.lft := min(labelbb.lft, xpart(b_ + llcorner pic_));
    labelbb.bot := min(labelbb.bot, ypart(b_ + llcorner pic_));
    labelbb.rt := max(labelbb.rt, xpart(b_ + urcorner pic_));
    labelbb.top := max(labelbb.top, ypart(b_ + urcorner pic_));
  endfor
enddef;

% Define a rectangle to surround some text:
%  -- lbl is either a label (string or picture)
%     or a pair giving the height and width of the text, (passed by TeX).
%  -- rad is the radius of the rounded corners
%  -- point is the center of the text bbox _in_graph_coordinates
%
% The size of path is determined as follows: If rad = 0 and
% label_sep = 0, then it is exactly the bbox. The amount of
% label_sep is then added on all 4 sides. If rad > 0, the rounded corners
% are arranged so that the path passes through the same points where
% the corners would be without the rounding. The whole path is shifted
% by label_adjust.
%
% The path is returned in graph coordinates.
% labeldims returns the dimensions of lbl as a pair.

vardef textrect (expr lbl, rad, point) =
  save dims; pair dims;
  dims := labeldims(lbl);
  save zz; pair zz;
  zz:=.5dims+label_sep*(1,1);
  if rad=0 :
    invvconv(rect(-zz,zz) shifted label_adjust) shifted point
  else:
    save p,d,f,q;
    pair d,p[]; path f,q;
    d := (rad,rad)/(sqrt 2);
    p1 := zz - d;             % center of upper right arc
    p2 := p1 xscaled -1;      % upper left
    p3 := -p1;                % lower left
    p4 := p1 yscaled -1;      % lower right
    q  := quartercircle scaled 2rad;
    f  := (q shifted p1) -- (q rotated 90 shifted p2)
            -- (q rotated 180 shifted p3) -- (q rotated -90 shifted p4)
            -- cycle;
    invvconv(f shifted label_adjust) shifted point
  fi
enddef;

% textoval is similar, except it is an ellipse.
%  -- dims and point as in textrect
%  -- mult adjusts the width to height ratio of the ellipse.
% If mult=1, the ratio is the same as the ratio of width to height of
% the text bbox, taking label_sep into consideration. If mult = 0,
% textrect is called with rad = 0. For 0 < mult < 1, the width
% of the ellipse is decreased and the height increased, and for r > 1,
% the reverse. This is arranged so that the new wd/ht is mult times the
% old. The ellipse passes through the same corner points as mentioned
% above.

vardef textoval (expr lbl, mult, point) =
  if mult = 0:
    textrect (lbl,0,point)
  else:
    save dims; pair dims;
    dims := labeldims(lbl);
    save ww, hh;
    ww := .5*xpart(dims)+label_sep;
    hh := .5*ypart(dims)+label_sep;
    if (ww = 0) or (hh = 0): % make a line:
      invvconv((-(ww,hh)--(ww,hh)) shifted label_adjust) shifted point
    else:
      save a,b;
      a:= 1 ++ mult;
      b := a*hh/mult;
      a := ww*a;
      invvconv(ellipse(origin,a,b,0) shifted label_adjust) shifted point
    fi
  fi
enddef;

% textellipse is similar, to textoval, except an absolute ratio of
% width to height is specified (so a circle results if rat = 1).
%  -- dims and point as in textrect
%  -- rat is the width to height ratio of the ellipse.
% The ellipse passes through the same corner points as mentioned above.

vardef textellipse (expr lbl, rat, point) =
  if rat = 0:
    textrect (lbl,0,point)
  else:
    save dims; pair dims;
    dims := labeldims(lbl);
    save ww, hh, a, b;
    ww := .5*xpart(dims)+label_sep;
    hh := .5*ypart(dims)+label_sep;
    if (ww = 0) or (hh = 0): % make a line:
      invvconv((-(ww,hh)--(ww,hh)) shifted label_adjust) shifted point
    else:
      a := ww ++ (hh*rat);    % a = x-radius of ellipse
      b := a/rat;
      invvconv(ellipse(origin,a,b,0) shifted label_adjust) shifted point
    fi
  fi
enddef;

vardef labeldims (expr lbl) =
  if pair lbl:
    lbl
  elseif string lbl:
    save lbl_; picture lbl_;
    lbl_ := lbl infont defaultfont scaled defaultscale;
    urcorner lbl_ - llcorner lbl_
  elseif picture lbl:
    urcorner lbl - llcorner lbl
  elseif path lbl:
    urcorner (picpath lbl) - llcorner (picpath lbl)
  else:
    (0,0)
  fi
enddef;

%%%
% Extra Trigonometric Functions:
%  tand, cotd, secd, cscd, and inverses acos, asin, atan.
% Also versions which take angles in radians:
%  sin, cos, tan, cot, sec, csc, and inverses invsin, invcos, and invtan.
% Exponentials, logarithms and hyperbolic functions:
%  exp, ln, log(=ln), cosh, sinh, tanh, acosh, asinh, and atanh.
% Complex variable functions: zexp and Log (pair to pair),
%  Arg (pair to numeric), cis (numeric to pair).
%  Arg returns (and cis accepts) angle in radians.
%%%
% NOTE: eps/2 + epsilon is the smallest value with reciprocal less than
%       infinity. I set nottoosmall a speck bigger to ensure that
%       the same is true of 2*(nottoosmall/2).
newinternal nottoosmall; nottoosmall := eps/2 + 2epsilon;
newinternal reallysmall; reallysmall := 3epsilon; % 1/(2epsilon)=2^15 :
                                                  % arithmetic overflow
vardef secd primary X =
  save temp; temp:=cosd(X);
  if abs(temp) < reallysmall :
    errmessage "Secant too large; truncating.";
    temp := if temp<0 : - fi reallysmall;
  fi
  1/temp
enddef;

vardef tand primary X =
  sind(X)*secd(X)
enddef;

vardef cscd primary X =
  save temp; temp:=sind(X);
  if abs(temp) < reallysmall :
    errmessage "Cosecant too large; truncating.";
    temp := if temp<0 : - fi reallysmall;
  fi
  1/temp
enddef;

vardef cotd primary X =
  cosd(X)*cscd(X)
enddef;

vardef acos primary x = angle (x, 1+-+x) enddef;
vardef asin primary y = angle (1+-+y, y) enddef;
vardef atan primary z = angle (1,z) enddef;

% Trig functions with arg in radians:
vardef sin primary X = sind (X*radian) enddef;
vardef cos primary X = cosd (X*radian) enddef;
vardef tan primary X = tand (X*radian) enddef;
vardef cot primary X = cotd (X*radian) enddef;
vardef sec primary X = secd (X*radian) enddef;
vardef csc primary X = cscd (X*radian) enddef;

vardef invcos primary x = (acos (x))/radian enddef;
vardef invsin primary y = (asin (y))/radian enddef;
vardef invtan primary z = (atan (z))/radian enddef;

vardef exp primary X = (mexp (256 * X)) enddef;

% Complex exponential (Z is a pair (X,Y)):
%   returns e^X(cos Y, sin Y)
vardef cis primary T = dir(radian*T) enddef;
vardef zexp primary Z = (exp (xpart Z))*(cis(ypart Z)) enddef;

vardef ln  primary X = (mlog X) / 256 enddef;
def log = ln enddef;

% Complex logarithm (Z is a pair):
%   returns the (ln |Z|, arg Z)
vardef Arg primary Z = (angle Z)/radian enddef;
vardef Log primary Z = (ln(abs(Z)), Arg (Z)) enddef;

vardef cosh primary X =
  save temp; temp:= exp(-abs(X));
  if temp < reallysmall :
    message "Cosh too large; truncating.";
    temp :=  reallysmall;
  fi
  temp/2 + 1/(2temp)
enddef;

vardef sinh primary X =
  save temp; temp:=exp(-abs(X));
  if temp < reallysmall :
    message "Sinh to large; truncating.";
    temp :=  reallysmall;
  fi
  if X < 0 : - fi
  (1/(2temp) - temp/2)
enddef;

vardef tanh primary X =
  save temp; temp := exp(-2abs (X));
  if X < 0 : - fi (1 - temp)/(1 + temp)
enddef;

vardef acosh primary y =
  if y < 1 :
    errmessage "Undefined function acosh "&decimal y;
    errmessage "Using 0, expect more errors later";
    0
  else:
    ln (y + (y+-+1))
  fi
enddef;

vardef asinh primary y =
  ln (y + (y++1))
enddef;

vardef atanh primary y =
  if abs (y) < 1:
  (ln(1+y) - ln(1-y))/2
  else:
    errmessage "Undefined function atanh "& decimal y ;
    errmessage "Using +/-infinity, expect more errors later";
    if y < 0 : - fi infinity
  fi
enddef;

%% Coordinate Systems and Transformations.

% Coordinate Nesting.
% Make this local, by using a (global) transform stack;
% and transparent to METAPOST syntax, by using hide.
% (For a primary expression, would use gobble instead of hide.)

transform T_stack[];
T_stack := 0;

def T_push (expr T) =
  T_stack[incr T_stack] := T;
enddef;

def T_pop (suffix $) =
  if T_stack > 0:
    $ := T_stack[T_stack];
    T_stack := T_stack - 1;
  fi
enddef;

def bcoords =
  hide (T_push (ztr))
  hide (T_push (vtr))
enddef;

def ecoords =
  hide (T_pop (vtr))
  hide (T_pop (ztr))
enddef;

% Coordinate Changes.
% Example:  `apply_t (rotated theta);'
%   where Transformer is `rotated theta'.

def apply_t (text Transformer) =
  ztr := identity Transformer transformed ztr;
  vtr := ztr shifted -zconv((0,0));
enddef;

let xslant = slanted;  % (x+sy, y).

def yslant primary s =  % (x, y+sx).
  transformed
   begingroup
     save T_;
     transform T_;
     origin transformed T_ = origin;
     (1,0)  transformed T_ = (1,s);
     (0,1)  transformed T_ = (0,1);
     T_
   endgroup
enddef;

def zslant primary p =  % (xu+yv, xv+yu), where p = (u,v).
  transformed
    begingroup
      save T_;
      transform T_;
      xpart T_ = ypart T_ = 0;
      xxpart T_ = yypart T_ = xpart p;
      xypart T_ = yxpart T_ = ypart p;
      T_
    endgroup
enddef;

def xyswap =  % swap X and Y axes.
  zslant (0,1)
enddef;

def boost primary X =  % boosts for special relativity.
  zslant (cosh X, sinh X)
enddef;

% Path transformation.
% f and data are in _graph_ coordinates, returned path is also.

% Rotate path f about point p by angle th in degrees.
% We have to do it this way in case xscale and yscale differ.
vardef rotatedpath (expr p,th) expr f =
 f transformed vtr rotatedaround (p transformed vtr, th) transformed
 (inverse vtr)
enddef;

% Scale path f by factor s with fixed point p.
vardef scaledpath (expr p, s) expr f =
  f shifted -p scaled s shifted p
enddef;

% slant a path f by amount s keeping line y=b fixed.
vardef xslantedpath (expr b, s) expr f =
 f shifted (0,-b) slanted s shifted (0,b)
enddef;
def slantedpath = xslantedpath enddef;

% yslant a path f by amount s keeping line x=a fixed.
vardef yslantedpath (expr a, s) expr f =
 f shifted (-a,0) yslant s shifted (0,a)
enddef;

% xscale a path f by factor s keeping line x=a fixed.
vardef xscaledpath (expr a, s) expr f =
  f shifted (-a,0) xscaled s shifted (a,0)
enddef;

% yscale a path f by factor s keeping line y=b fixed.
vardef yscaledpath (expr b, s) expr f =
  f shifted (0,-b) yscaled s shifted (0,b)
enddef;

% shift a path f by vector v.
vardef shiftedpath (expr v) expr f =
  f shifted v
enddef;

% reflect a path f through line p--q.
% transformed vtr in case xscale and yscale differ.
vardef reflectedpath (expr p,q) expr f =
 f transformed vtr reflectedabout (p transformed vtr, q transformed vtr)
    transformed (inverse vtr)
enddef;
% no vtr so horizontal will become vertical, and vice versa.
vardef xyswappedpath expr f =
  f reflectedabout ((0,0),(1,1))
enddef;

% Clipping and Filling.

% (safely filled) interior of contour c,
% where c is in _device_ coordinates;
% adapted from _The METAFONTbook_'s "safefill".

vardef interior expr c =
  save vs; picture vs; vs=nullpicture;
  addto vs contour c withpen nullpen wf_;
  vs
enddef;

% (safely filled) interiors of contours cc[],
% where cc[] are in _device_ coordinates;

vardef interiors suffix cc =
 save vs_; picture vs_; vs_=nullpicture;
 for i_=1 upto cc:
  addto vs_ contour (cc[i_]) withpen nullpen wf_;
 endfor
 vs_
enddef;

% clip picture vt to interior of cycle c,
% where c is in _device_ coordinates.
% (Hardly needed, except to minimize changes.)
def clipto (suffix vt) expr c =
 clip vt to c;
enddef;

% clip picture vt to interiors of cycles in path array cc,
% where c is in _device_ coordinates.

def clipsto (suffix vt, cc) =
begingroup
 save vc_, vcc_; picture vc_, vcc_;
 vcc_:=nullpicture;
 for i_=1 upto cc:
  vc_:=vt;
  clipto (vc_) cc[i_];
  addto vcc_ also vc_;
 endfor
 vt:=vcc_;
endgroup
enddef;

% clip copy of picture vt to interior of cycle c,
% where c is in _device_ coordinates,
% return result (which is a `subpicture' of vt).
% (Note: this was called clip in grafbase.mf, but clip is a
%  Metapost primitive.)

vardef clipped (suffix vt) expr c =
  save vc_; picture vc_; vc_:=vt;
  clipto (vc_) c;
  vc_
enddef;

% (return) reverse video of vt inside c,
% where c is in _device_ coordinates.

vardef picneg (suffix vt) expr c =
  save v_; picture v_; v_ = interior c;
  save vx_; picture vx_; vx_ := vt;
  clip vx_ to c;
  addto v_ also vx_ withcolor background;
  v_
enddef;

% Rendering Paths --- Drawing and Filling.

% draw path f in picture v using pen q,
% where f is in _device_ coordinates.

def shpath (suffix v) (expr q, f) =
    colorshpath (v) (drawcolor, q, f)
enddef;

def colorshpath (suffix v) (expr clr, q, f) =
  addto v doublepath f
    withpen q withcolor clr;
enddef;

% draw path d safely, return picture of drawing,
% where d is in _device_ coordinates.
% (Courtesy of Uwe Bonnes.)

numeric minpenwd;
minpenwd := 0.01pt;

vardef picpath expr d =
 save vs; picture vs; vs=nullpicture;
 if penwd > minpenwd:
   shpath (vs, drawpen) (d);
 fi
 vs
enddef;

% drawing a path d safely,
% where d is in _device_ coordinates.

def safedraw  = colorsafedraw (drawcolor) enddef;

def colorsafedraw (expr clr) expr d =
  addto active_plane also (picpath d) withcolor clr;
  if ClipOn:
    clipsto (active_plane, ClipPath);
  fi
enddef;

% filling a region, the interior of c, safely,
% where c is in _device_ coordinates.

def safefill = colorsafefill (fillcolor) enddef;

% erasing a region (the interior of c) safely,
% where c is in _device_ coordinates.

def safeunfill = colorsafefill (background) enddef;

def colorsafefill (expr clr) expr c = % clr is a color.
  if cycle c:
    addto active_plane contour c withpen nullpen withcolor clr;
    if ClipOn:
      clipsto (active_plane, ClipPath);
    fi
  else:  % so we can see something
    safedraw c;
  fi
enddef;

% clipping to a region (the interior of c) safely,
def safeclip expr c =
  if cycle c:
    clipto (active_plane) c;
  else:  % so we can see something
    safedraw c;
  fi
enddef;

% draw path f and return path f,
% where f is in _graph_ coordinates.

def drawn = colordrawn (drawcolor) enddef;

vardef colordrawn (expr clr) expr f =
 colorsafedraw (clr) (zconv (f));
 f
enddef;

% fill contour c and return contour c,
% where c is in _graph_ coordinates.

def filled = colorfilled (fillcolor) enddef;

vardef colorfilled (expr clr) expr c =
 colorsafefill (clr) zconv (c);
 c
enddef;

% unfill contour c and return contour c,
% where c is in _graph_ coordinates.

vardef unfilled expr c =
 safeunfill zconv (c);
 c
enddef;

% clip to a contour c and return contour c,
% where c is in _graph_ coordinates.

vardef CLIP expr c =
  safeclip zconv(c);
  c
enddef;


%% Shading and Hatching.

% Shading
% - expressed mainly in _device_ coordinates.

% A closed path representing one dot of unit size
% (one pixel across is recommended).
path dotpath;

% Initially, set dotpath to a circle.
dotpath := fullcircle;

% Return picture that renders specified apath path to specified scale.
% We use mindiam to ensure visibility.

numeric mindiam; mindiam=.1pt; % one pixel at 720 dpi
vardef setdot (expr apath) (expr scale) =
  if cycle apath:
    interior
  else:
    picpath
  fi
  (apath scaled max(scale,mindiam))
enddef;

% Picture containing one dot.
picture onedot;

% Initially, set onedot to picture of initial dotpath scaled 0.5pt.
onedot := setdot (dotpath, 0.5pt);

% Draw picture w at position p in picture v,
% where p is in _device_ coordinates.

def picdot (suffix v) (expr w) (expr p) =
  addto v also (w shifted p) wd_;
enddef;


% Whether to show bounding boxes (for debugging purposes).

boolean showbbox;
showbbox := false;


% Calculate tight bounding box points (ll, ur) for path g.
% g is in _device_ coordinates.
% (Metapost version greater than 0.3 has a primitive for this)

vardef tightbbox (expr g) (suffix ll, ur) = % g can be picture or path
  ll:=llcorner g;
  ur:=urcorner g;
  if showbbox:
    safedraw (rect (ll, ur));
  fi
enddef;

% calculate tight bounding box points (ll, ur) for path array g.
% g is in _device_ coordinates. Not used.
%
vardef tbbox (suffix g) (suffix ll, ur) =
  save gll_, gur_;
  pair gll_[], gur_[];
  for i_=1 upto g:
    tightbbox (g[i_], gll_[i_], gur_[i_]);
  endfor
  gll_ = gur_ = g;
  ll := minpair (gll_);
  ur := maxpair (gur_);
  if showbbox:
    safedraw (rect (ll, ur));
  fi
enddef;

% Fill a region with (relatively large) dots.
% If dot spacing and size are too small, and region
% is too large, then capacity is easily exceeded.

polkadotwd=5pt;

picture thepolkadot;
thepolkadot=setdot(dotpath,polkadotwd);

% Based on original version of macro shade (in grafbase.mf)
% Except a hexagonal packing is used.
vardef polkadot (expr sp) expr f =
 save g; path g; g := zconv (f);
 polkadotwd := max(polkadotwd,1);
 if not cycle g:  % so we can see something
  safedraw g;
  message "attempt to polkadot open curve; drawing curve instead";
 % Spacing is between _centers_
 % fill unless spacing allows some background to show
 elseif sp<= 2*polkadotwd/3 :
  safefill g;
 else:
  save ll, ur; pair ll, ur;
  tightbbox (g, ll, ur);
  save dx,dy;
  % dx will be horizontal shift of odd rows
  dx := sp/2;
  % dy will be spacing between rows
  dy := (sqrt(3))*sp/2;
  % make an effort to limit occurance of partial dots,
  % Therefore, only draw a dot if its center is in the bbox,
  % and equalize any extra space on sides and top with this shift
  hshift := ((xpart (ur - ll)) mod sp)/2;
  vshift := ((ypart (ur - ll)) mod dy)/2;
  save p; pair p[];
  p0 := ll + (hshift + dx,vshift);
  p1 := p0 + (-dx,dy);
  save v; picture v[];
  v0 := nullpicture;
  for s = xpart p0 step 2dx until xpart ur:
    addto v0 also thepolkadot shifted (s,0);
  endfor % v0 is now the bottom row (but not yet raised into place)
  v1 := thepolkadot shifted (xpart p1, 0);
  addto v1 also v0 shifted (dx, 0); % second row has extra dot
  save vv; picture vv;
  vv := nullpicture;
  for s = ypart p0 step 2dy until ypart ur:
    addto vv also v0 shifted (0,s); % add copies of v0
  endfor
  for s = ypart p1 step 2dy until ypart ur :
    addto vv also v1 shifted (0,s); % add copies of v1
  endfor
  addto active_plane also (clipped (vv) g) wf_;
 fi
 if ClipOn :
   clipsto (active_plane, ClipPath);
 fi
 f
enddef;

%%  shading by filling with gray.
% Mostly for MF: to try to make the \shadewd macro actually work
picture shadedot; shadedot=onedot;
numeric shadewd; shadewd:=0.5pt;

% Here in grafbase.mp, ignore dots, shade by filling with gray.
vardef shade (expr sp) expr f =
 save g; path g; g := zconv (f);
 save gr; numeric gr;
 if not cycle g:  % so we can see something
  safedraw g;
  message "attempt to shade open curve; drawing curve instead";
 elseif sp<=max(shadewd,0):
  safefill g;
 else: % sp > shadewd, sp > 0
%% DHL: translate dots to gray level:
%%    sp=shadewd is black, sp=2shadewd is .75white (the default),
%%    On my 300dpi HP's this gray level comes close to the mf version.
%%    almost surely printer dependent.
  gr:= (1 -(max(shadewd,0)/sp)**2);
  colorsafefill (gr*white) g;
 fi
 f
enddef;

% Given:
% a picture variable v_c,
% an affine transform CT,
% a spacing sp,
% and an upright box (with edges xa, xb, ya, yb)
% specified in the coordinate system that CT determines.
% Then:
% hatch the interior of the box in the picture,
% using lines that in that coordinate system are horizontal
% and spaced sp units apart.
% Notes:
% 1.  sp must be strictly positive.
% 2.  CT, xa, xb, ya, yb, and sp are in _device_ coordinates.
% 3.  xa, xb, ya, and yb are the left, right, bottom and top
%     edges of the box.

vardef thatchf (suffix v_c) (expr clr, CT, sp, xa, xb, ya, yb) =
  % Number of hatch lines. Called by other hatching
  % macros. THEY check that sp is not 0
  save m_; numeric m_;
  m_ := ceiling (abs (yb - ya) / sp);
  save p_; pair p_[];  % In device coords.
  % Hatch lines have same alignment, spacing and orientation.
  % Displacement from one hatch line to the next.
  p_1 := sp * up;
  % Endpoints of first hatch line.
  p_2 := (xa, ya);
  p_3 := (xb, ya);
  for i_=1 upto m_:
   % The next source line does all the drawing.
   colorshpath (v_c, clr, hatchpen) ((p_2--p_3) transformed CT);
   p_2 := p_2+p_1;
   p_3 := p_3+p_1;
  endfor
enddef;

% Hatch interior of path f
% with lines at angle theta,
% spaced sp apart.
% where f is in _graph_ coordinates,
% and sp is in _device_ coordinates.

def thatch (expr sp, theta) =
  colorthatch (hatchcolor) (sp, theta)
enddef;
vardef colorthatch (expr clr) (expr sp, theta) expr f =
 % g is in _device_ coordinates.
 save g; path g; g := zconv (f);
 if not cycle g:  % so we can see something
  message "attempt to hatch open curve; drawing curve instead";
  safedraw g;
 elseif sp<=0:
  colorsafefill (clr) g;
 else:
  save v_c; picture v_c;
  v_c := nullpicture;
  save CT; transform CT;
  CT := identity rotated theta;
  save ll, ur; pair ll, ur;
  tightbbox (g transformed inverse CT, ll, ur);
  save xa, xb, ya, yb; numeric xa, xb, ya, yb;
  (xa, ya) = ll;
  (xb, yb) = ur;
  % Hatch interior of bounding box.
  thatchf (v_c, clr, CT, sp, xa, xb, ya, yb);
  % Clip to interior of path g.
  addto active_plane also clipped(v_c) g;
  if ClipOn :
    clipsto (active_plane, ClipPath);
  fi
 fi
 f
enddef;

% Hatch interior of path f with horizontal lines spaced sp apart.
% where f is in _graph_ coordinates,
% and sp is in _device_ coordinates.

vardef hhatch (expr sp) expr f =
  colorthatch (hatchcolor) (sp, 0 deg) f
enddef;

% Hatch interior of path f with vertical lines spaced sp apart.
% where f is in _graph_ coordinates,
% and sp is in _device_ coordinates.

vardef vhatch (expr sp) expr f =
  colorthatch (hatchcolor) (sp, 90 deg) f
enddef;

% Left-hatch interior of path f with lines spaced sp apart,
% where f is in _graph_ coordinates,
% and sp is in _device_ coordinates.
% Return f.

vardef lhatch (expr sp) expr f =
  colorthatch (hatchcolor) (sp, -45 deg) f
enddef;

% Right-hatch interior of path f with lines spaced sp apart,
% where f is in _graph_ coordinates,
% and sp is in _device_ coordinates.
% Return f.

vardef rhatch (expr sp) expr f =
  colorthatch (hatchcolor) (sp, 45 deg) f
enddef;

% Cross-hatch interior of path f with lines spaced sp apart,
% where f is in _graph_ coordinates,
% and sp is in _device_ coordinates.

vardef xhatch (expr sp) expr f =
 colorthatch (hatchcolor) (sp, 45deg) colorthatch (hatchcolor) (sp, -45deg) f
enddef;


%% Tiles.

% environment for a tile `atile', drawn with unit length
% `unit', with width `width' units and height `height' units.
% If `clipit' is true, clip the drawing to the tile's
% (width x height) boundary.

% DHL: Convert dimensions of tile to device units,
%      and change ztr to use tile units.

def tile (suffix atile) (expr unit, width, height, clipit) =
 picture atile.pic; atile.pic:=nullpicture;
 % for reference by `tess' macro:
 numeric atile.wd, atile.ht;
 atile.wd:=width*unit; % device coordinates
 atile.ht:=height*unit;
 boolean atile.clipon; atile.clipon:=clipit;
 begingroup
  save active_plane;
  def active_plane = atile.pic enddef;
  save ztr, vtr; transform ztr, vtr;
  ztr := identity scaled unit; %Drawing should now be relative to tile
                               %boundaries.
  vtr := ztr;
  save ClipOn; boolean ClipOn;
  if clipit:
   ClipOn := true;
   save ClipPath; path ClipPath[];
   ClipPath = 1;
   ClipPath[1] =
     zconv ((0,0)--(width,0)--(width,height)--(0,height)--cycle);
  else:
   ClipOn := false;
  fi
enddef;

def endtile =
 endgroup
enddef;

% test whether atile is really a tile.

def is_tile (suffix atile) =
  ((picture atile.pic) and
   (numeric atile.wd) and
   (numeric atile.ht) and
   (boolean atile.clipon))
enddef;

% tesselation of interior of closed path c,
% using tile `atile'.
% c is in _graph_ units.
% return c.

vardef tess (suffix atile) expr c =
 save g_; path g_; g_ := zconv (c);
 if not is_tile (atile):
  errhelp ("Please check the faulty definition of the `tile'");
  errmessage ("Parameter of tess() is invalid tile.");
  safedraw g_;
 elseif not cycle g_:  % so we can see something
  safedraw g_;
 else:
  save ll_, ur_; pair ll_, ur_;
  tightbbox (g_, ll_, ur_);
  save a_, b_; numeric a_, b_;
  a_ = atile.wd;
  b_ = atile.ht;
  save v_, vs_;
  picture v_, vs_;
  v_  := atile.pic shifted ll_;
  vs_ := v_;
  % fill vs_ with rectangular tesselation large enough
  % to cover interior of g_:
  for sh_= a_ step a_ until xpart (ur_ - ll_): %lay down a hoizontal strip
    addto vs_ also (v_ shifted (sh_, 0));
  endfor
  v_:= vs_;
  for sh_ = b_ step b_ until ypart (ur_ - ll_): % now stack the strips
    addto vs_ also (v_ shifted (0, sh_));
  endfor
  % trim tesselation to interior of g:
  clipto (vs_) g_;
  addto active_plane also vs_;
 fi
 if ClipOn :
   clipsto (active_plane, ClipPath);
 fi
 c
enddef;

%% Dots and Dashes.

% Draw dashed curve along path f;
% f is in _graph_ coordinates;
% return f.

if unknown segment_split: segment_split:= 8; fi
if unknown dashsize:     dashsize  := 3pt; fi
if unknown dashgap:      dashgap := 1dashsize + 2penwd; fi
if unknown dash_finish:   dash_finish := .5; fi
if unknown dash_start:    dash_start := .5; fi
if unknown unit_of_length: unit_of_length:= 0.1in; fi

% The main idea is to have a list of lengths represent the repeating
% pattern of dashes and dots. These lengths represent a dash length,
% followed by a gap length, etc., so there are an even number. To start
% dashing a path, we normally take a fraction (dash_start) of the first
% dash, then the rest of the pattern. We continue by repeating the pattern
% as many times as will fit, then we finish off with a fraction
% (dash_finish) of the first dash. A dash of length 0 is a dot. A gap of
% length 0 is OK, but useless unless it's between a dot and a dash, and
% you arrange for the dotsize to be different from penwd.

% We generalize so that pat.start and pat.finish can be any patterns,
% not necessarily related to pat.rep. Also "dots" can be symbols like
% Triangle.

vardef gendashed (suffix pat) expr f =
  if (unknown pat.rep) or (pat.rep<2) : % no "pattern"
    safedraw zconv (f);
  else :
    % Note: can't "save pat" in case pat has a suffix, so we
    % save temp_ and copy pat to temp later:
    save dl_, temp_, need_dots;
    boolean need_dots;
    need_dots:=false;
    % Compute lengths of patterns. Scale by 1/unit_of_length to avoid
    % overflow. Also set flag if any dots are needed, since they are done
    % differently than dashes (though perhaps they ought not be in MP):
    forsuffixes s = start, rep, finish :
      dl_.s:=0;
      temp_.s:=pat.s;
      for i=1 upto pat.s:
          if (odd i) and (pat.s[i] = 0):
            need_dots:=true;
          fi
          temp_.s[i]:= pat.s[i]/unit_of_length;
          dl_.s:= dl_.s + temp_.s[i];
      endfor
    endfor
    if dl_.rep = 0:
      message "You can't dash a curve if the dash pattern has 0 length!";
      message "Drawing it instead.";
      safedraw zconv (f);
    else:
      if need_dots:
      % dotpath is a grafbase global variable, defaulting to fullcircle.
      % Calling routine may define plot_pic
        save dashingdot; picture dashingdot;
        if picture plot_pic :
          dashingdot := plot_pic;
        else:
          dashingdot := setdot (dotpath, penwd); % same width as dashes
        fi
      fi
      save g_, p_;
      path g_, p_;
      g_ := zconv (f);
      p_ := g_ scaled (1/unit_of_length);

      % First calculate array of lengths of approximating polygon:
      save cur_len, tot_len, n_, sf_;
      tot_len:=makelengtharray(cur_len) p_;

      % now scale the dashes so a whole number make up
      % the approximating polygon:
      sf_:=scale_adjust (n_, dl_)(tot_len);

      % if the path is too short, we do not dash it.
      if n_ < 0 : % Now n_ is # of _repeated_ dashes (one smaller than before)
        safedraw g_;
      else:
        save ct_, _t_, _d_;
        % rescale the patterns:
        forsuffixes s = start, rep, finish :
          for i = 1 upto temp_.s :
            temp_.s[i] := temp_.s[i]*sf_;
          endfor
        dl_.s:=dl_.s*sf_;
        endfor

        % Find times for endpoints of the dashes and draw them:
        ct_:=0;
        % Begin with start pattern
        _d_0:=0;
        _t_0 := 0;
        dashit (temp_.start);

        % And now the repeating pattern
        % _d_0 now points to the end of pat.start, but to be on the safe side
        % we initialize anyway:
        _d_0:= dl_.start;

        % We now just loop over the number of repetitions. BUT to reduce
        % accumulated roundoff error, use dl to re-initialize _d_0.
        for i = 0 upto n_-1 :
          _d_0:= dl_.start+i*dl_.rep;
          _t_0 := gettime(cur_len, ct_) (_d_0);
          dashit(temp_.rep);
        endfor

        % and finally, pat.finish
        % At this point d0 ought to point to the start of
        % pat.finish, but force it there anyway (avoid rounding problems).
        _d_0:= tot_len - dl_.finish;
        _t_0 := gettime(cur_len, ct_) (_d_0);
        dashit(temp_.finish);
        if ClipOn :
          clipsto (active_plane, ClipPath);
        fi
      fi
    fi
  fi
  f
enddef;

% Takes an array name, computes the array of partial lengths,
% returns the total length.
vardef makelengtharray (suffix clen) expr p =
  clen:=segment_split*length p;
  clen[0] := 0;
  for i_ = 1 upto clen :
    clen[i_] := clen[i_-1] + abs ( ( point (i_/segment_split) of p )
                - ( point ((i_-1)/segment_split) of p ) );
  endfor
  clen[clen]
enddef;

% n is an integer suffix defined by the calling routine
% pl.{start,rep,finish} are arrays of lengths of dashing patterns
% lngth is (normalized) length of some path.
% rounds n  to an integer, and returns the scaling factor sf require
% to make  sf*(pl.start + n*pl.rep + pl.finish) equal to lngth.
vardef scale_adjust (suffix n, pl) (expr lngth) =
  n:= (lngth - pl.start - pl.finish)/pl.rep;
  % -1 is a flag
  n:= if n < 0 : -1 else : round(n) fi; % avoids scaling too much.
  lngth/(pl.start + max(n,0)*pl.rep + pl.finish)
enddef;

% arr is an array of lengths , defined by the calling routine
% ct is current index into that array
% lngth is a length
% returns the (normalized) time on a polygonal path with segments of
% length arr where the accumulated length is lngth.
vardef gettime (suffix arr, ct) (expr lngth) =
  save _d_; numeric _d_;
  % force lngth into range [ arr[ct], arr[arr] ]
  _d_:= max( arr[ct], min ( arr[arr], lngth ) );
  % find segment in which lngth is achieved
  forever:
    exitif ( (_d_ >= arr[ct]) and (_d_ <= arr[ct+1]) );
    ct:=ct+1;
  endfor
  % Within that segment, by linear interpolation.
  % (Could we divide by 0 here? Yes, curve could have a small loop.)
  if arr[ct+1] = arr[ct] :
    ct/segment_split
  else :
    (ct + (_d_ - arr[ct]) / (arr[ct+1] - arr[ct]))/ segment_split
  fi
enddef;

def dashit (suffix pos) =
% gendashed defines array cur_len, path g_, and initializes _d_0, _t_0 and ct_.
  for j_=1 upto pos:
    if odd j_: % draw a dash of length pos[j_]
      if pos[j_]=0 :
        _d_1:= _d_0;
        _t_1:= _t_0;
        picdot (active_plane, dashingdot, point _t_0 of g_);
      else:
        _d_1:= _d_0 + pos[j_];
        _t_1:= gettime (cur_len, ct_) (_d_1);
        safedraw(subpath (_t_0,_t_1) of g_);
      fi
    else: % j_ is even, find the start of the next dash:
      _d_0 := _d_1 + pos[j_];
      _t_0 := gettime(cur_len, ct_) (_d_0);
    fi
  endfor
enddef;

% Convert a text list of lengths to three dash pattern arrays.
% Should we also scale by unit_of_length to avoid doing it for every
% call to gendashed?
def dashpat (suffix pat) (text t) =
  % calling routine is responsible for saving, declaring, etc

  pat.rep := 0;
  for item_ = t:
    pat.rep[incr pat.rep]:= item_;
  endfor;

  % enforce an even number if > 2 in the list:
  if (odd pat.rep) and (pat.rep > 2) : pat.rep:= pat.rep - 1; fi

  % calculate start and finish patterns from repeating pattern
  pat.start:=1;
  pat.start[1]:=pat.rep[1]*dash_start;
  for j=2 upto pat.rep :
    pat.start[incr pat.start]:=pat.rep[j];
  endfor

  pat.finish:=1;
  pat.finish[1]:=pat.rep[1]*dash_finish;
enddef;

vardef DASHED (expr dlen,dgap) expr f =
  save dashes;
  dashpat (dashes) (dlen,dgap);
  gendashed (dashes) f
enddef;

let dashed_=dashed;
def dashed = DASHED enddef;

% Draw dotted curve along path f;
% f is in _graph_ coordinates;
% return f.

% Generalized to other shapes besides dots.
%%% Try to allow pictures as well as paths:

vardef doplot (expr spath,sc,dgap) expr f =
  save dots;
  dashpat (dots) (0,dgap);
  save plot_pic; picture plot_pic;
  plot_pic := makesymbol (spath, sc);
  gendashed (dots) f
enddef;

vardef dotted (expr dsize, dgap) expr f =
  doplot(dotpath,dsize,dgap) f
enddef;

% Accepts a pic and returns the same pic centered
vardef centerit (expr pic) =
  save ur_, ll_; pair ur_, ll_;
  ur_ = urcorner pic;
  ll_ = llcorner pic;
  pic shifted -(0.5[ll_,ur_])
enddef;

% Utility: takes an expression of *any* type and returns a picture
vardef makesymbol (expr  spath, sc) =
  if picture spath:
    % do we want to allow a symbol not centered?
    centerit(spath)
  elseif string spath:
    save vs; picture vs; vs:=nullpicture;
    addto vs centerit (spath infont defaultfont scaled defaultscale)
        withcolor tlabelcolor;
    vs
  elseif path spath:
    setdot(spath, sc)
  else:
    message "Undefinable plotting symbol, using dotpath instead.";
    setdot(dotpath, sc)
  fi
enddef;

vardef plotsymbol (expr spath, sc) (text t) =
  save one_symbol; picture one_symbol;
  one_symbol := makesymbol (spath, sc);
  for a_ = t:
    addto active_plane also (one_symbol shifted zconv(a_));
  endfor
  if ClipOn :
    clipsto (active_plane, ClipPath);
  fi
enddef;

% Symbols to plot, Taco requested 6 types, here's 9:
% Use common names. (Square is an open square, SolidSquare a solid one.)

path Triangle, Square, Circle, Diamond, Plus, Cross, Star, Asterisk,
     SolidTriangle, SolidSquare, SolidCircle, SolidDiamond, SolidStar;

% Differing scales attempt to equalize area:
Triangle:= (for n=0 upto 2: (up rotated 120n)-- endfor up) scaled .675;
  SolidTriangle := (Triangle & cycle) scaled 1.1;
Square := (for n = 0 upto 3: dir (90n + 45)-- endfor dir 45) scaled .625;
  SolidSquare := (Square & cycle) scaled 1.1;
Circle := (halfcircle & halfcircle rotated 180); % diameter = 1
  SolidCircle := (Circle & cycle) scaled 1.1;
Diamond := (for n = 0 upto 3: dir 90n-- endfor dir 0) yscaled .75 xscaled .52;
  SolidDiamond := (Diamond & cycle) scaled 1.1;
Plus := ( origin for n=0 upto 3: --(up rotated 90n)--origin endfor ) scaled .625;
Cross := Plus rotated 45;
Asterisk := ( origin for n=0 upto 4: --(up rotated 72n)--origin endfor ) scaled .575;
pair zz;
zz = (whatever)[up, up rotated 144];
zz = (whatever)[up rotated 72, up rotated -72];
Star := ( for n=0 upto 4: (up rotated 72n)--(zz rotated 72n)-- endfor up) scaled .675;
SolidStar := (Star & cycle) scaled 1.2;
save zz;
% The above scheme depends on the fact that safefill fills cycles
% and draws noncycles. Only the "Solid..." paths are cycles.


% set up scheme to plot several curves with automatically changing line
% types:

forsuffixes s = start, rep, finish :
  numeric dashtype[].s, dashtype[].s[];
endfor
% gaps exceed dash lengths because circular pens increase the lengths
% by penwd (normally .5pt) and decrease the gaps by penwd.
dashpat (dashtype0) (0);                        % solid
dashpat (dashtype1) (3pt,4pt);                  % dashed
dashpat (dashtype2) (0,4pt);                    % dotted
dashpat (dashtype3) (0pt,4pt,3pt,4pt);          % dot-dash
dashpat (dashtype4) (0pt,4pt,3pt,4pt,0pt,4pt);  % dot-dash-dot
dashpat (dashtype5) (0pt,4pt,3pt,4pt,3pt,4pt);  % dot-dash-dash

path pointtype[];
pointtype0:=Star;
pointtype1:=Triangle;
pointtype2:=SolidCircle;
pointtype3:=Plus;
pointtype4:=Square;
pointtype5:=SolidDiamond;
pointtype6:=Cross;
pointtype7:=Circle;
pointtype8:=SolidTriangle;

% These are compromises between what looks best on my screen
% and what looks best on my printer.
color colortype[];
colortype0:=black;              % black
colortype1:=red;                % red
colortype2:=.8blue + .2white;   % blue
colortype3:=.66yellow + .34red; % orange
colortype4:=.8green;            % green
colortype5:=.85magenta;         % magenta
colortype6:=.85cyan;            % cyan
colortype7:=.85yellow;          % yellow

%% Points.

% ptwd is a numeric scale factor,
% b is a pair;
% return a path representing
% a point
% with diameter ptwd
% located at b.
% All quantities are in _device_ units.

vardef bpoint (expr ptwd, b) =
  fullcircle scaled ptwd shifted b
enddef;

% Draw discs with _device_ diameter ptwd
% at the _graph_ locations listed in the text t.
% If filled is true, make the discs black,
% otherwise make them white with a black boundary.

vardef pointd (expr ptwd, filled) (text t) =
 save f_; path f_;  % in _graph_ coords.
 for a_=t:
  f_ := bpoint (ptwd, zconv (a_));
  if filled:
    safefill f_;
  else:
    safeunfill f_;
    safedraw f_;
  fi
 endfor
enddef;

%% Arrows.
% arrowheads are in _device_ coordinates.

newinternal hdwdr, hdten;
interim hdwdr := 1;
interim hdten := 1;

boolean hfilled;
hfilled := false;

% wr is in _device_ coordinates;
% tens is a pure number;
% fil is _boolean_.

def headshape (expr wr, tens, fil) =
 interim hdwdr := wr;
 interim hdten := tens;
 save hfilled; boolean hfilled; hfilled := fil;
enddef;

% front, back and hdwdr are in _device_ coords.

vardef head (expr clr, front, back, hdwdr, tens, filled) =
 if front <> back:
   save p, side;
   pair p[], side;
   side := (hdwdr/2) *
     ((front-back) rotated 90);
   p1 := back + side;
   p2 := back - side;
   save f; path f;
   f :=
     p1..tension tens..{front-back}front{back-front}..tension tens..p2;
   if filled:
     colorsafefill (clr) (f--cycle);
   fi
   colorsafedraw (clr) f;
 fi
enddef;

% hlen, hrot and hback are in _device_ coords.
% f is in _graph_ coords.
% return f.

def headpath (expr hlen, hrot, hback) =
  colorheadpath (headcolor, hlen, hrot, hback)
enddef;
vardef colorheadpath (expr clr, hlen, hrot, hback) expr f =
 if hlen <> 0:
   save g; path g; g := zconv (f);
   save P; pair P[];
   P2 := point     (length g) of g;
   P1 := direction (length g) of g;
   if P1 <> (0,0) :
     P3 := ((unitvector (P1)) rotated hrot);
     P4 := P2 - (hback * P3);
     P5 := P4 - (hlen  * P3);
     head (clr, P4, P5, hdwdr, hdten, hfilled);
   fi
 fi
 f
enddef;

% hlen is in _device_ coordinates;
% f is in _graph_ coords.

%% Changed to test for positivity of hlen, 05/15/2001:
def arrowdraw (expr hlen) (expr f) =
 store (trash) drawn headpath (hlen, 0, 0pt) f;
enddef;

%% Axes, Axis Tic Marks, and Grid.

% hlen is in _device_ coords.
% axes are described by xneg, xpos, yneg and ypos,
% which are in _graph_ coords.

def xaxis (expr hlen) =
 arrowdraw (hlen) ((xneg, 0)--(xpos, 0));
enddef;

def yaxis (expr hlen) =
 arrowdraw (hlen) ((0, yneg)--(0, ypos));
enddef;

def axes (expr hlen) =
 xaxis (hlen);
 yaxis (hlen);
enddef;

%%%%%%  Axes at borders
% amounts to shift border axes inward, default 0.
laxis:=0; baxis:=0; raxis:=0; taxis:=0;

vardef axisline.x = (xneg + laxis, 0)--(xpos - raxis, 0) enddef;
vardef axisline.y = (0, yneg + baxis)--(0, ypos - taxis) enddef;
vardef axisline.l = axisline.y shifted (xneg + laxis, 0) enddef;
vardef axisline.b = axisline.x shifted (0, yneg + baxis) enddef;
vardef axisline.r = axisline.y shifted (xpos - raxis, 0) enddef;
vardef axisline.t = axisline.x shifted (0, ypos - taxis) enddef;

vardef axis@# (expr len) =
  headpath (len, 0, 0pt) axisline@#
enddef;
%%%%%% End axes at borders

%%%%%% Tick marks on all axes
% flags for changing tick marks from default.
numeric inside, outside, centered, onleft, onright, ontop, onbottom;
% sign of flag is used to determine if direction is relative
% (inside/outside) or absolute
inside:=-2; outside:=-1;
onright:=1; centered:=1.5; onleft:=2;
onbottom:=1; ontop:=2;

% defaults:
ltick:=inside; rtick:=inside; ttick:=inside; btick:=inside;
xtick:=centered; ytick:=centered;

% Utility macro to draw tick marks on an arbitrary axis.
% It needs info about the marks and the axis:
% inang is angle one must rotate the axis to point inside.
% tp    is the tick position (normally inside, outside, etc).
% loc   is the location of the 0-point of the axis (graph coordinates).
% pdir  is the positive direction on the axis (right or up).
% len   is length of tick mark.
% t     is the list of positions.

vardef axismarks (expr inang, tp, loc, pdir) (expr len) (text t) =
 save tp_, U_, P_, tic_, ticang_;
 pair U_, P_, tic_[];
 % if onleft, onright, ontop or onbottom, don't examine inang
 % otherwise use it to determine what inside/outside mean.
 %
 ticang_ := if tp<0: inang else: 90 fi;
 tp_:=abs(tp) - 1;
 % Axis Tic Marks are at right angles to axes,
 % U_ should point in direction of inside, onleft or ontop.
 U_:=unitvector (vconv (pdir)) rotated ticang_;
 tic_1:= (tp_ - 1) * len * U_; % start of mark
 tic_2:= tp_ * len * U_;       % end of mark
 for a_=t:
   P_ := zconv (loc + a_*pdir);
   safedraw ((tic_1--tic_2) shifted P_);
 endfor
enddef;

def xmarks =
  axismarks (90,xtick,(0,0),right)  % 90, same as bottom axis
enddef;

def ymarks =
  axismarks (-90,ytick,(0,0),up)    % -90, same as left axis
enddef;

def lmarks =
  axismarks (-90,ltick,(xneg+laxis,0),up)
enddef;

def bmarks =
  axismarks (90,btick,(0,yneg+baxis),right)
enddef;

def rmarks =
  axismarks (90,rtick,(xpos-raxis,0),up)
enddef;

def tmarks =
  axismarks (-90,ttick,(0,ypos-taxis),right)
enddef;
%%%%%% End tick marks on axes

% Grid.

% Draw dots at each grid coordinate.
% xspace, yspace = spacings between grid coordinates, in _graph_ coords.

def grid = vgrid (0.5pt) enddef;
vardef vgrid (expr dsize) (expr xspace, yspace) =
 save griddot; picture griddot; griddot:=setdot(dotpath,dsize);
 for n = ceiling(xneg/xspace) upto floor(xpos/xspace):
   for m = ceiling(yneg/yspace) upto floor(ypos/yspace):
     picdot (active_plane, griddot, zconv((n*xspace,m*yspace)));
   endfor
 endfor
 if ClipOn :
   clipsto (active_plane, ClipPath);
 fi
enddef;

% Grid of lines.
vardef gridlines (expr xspace, yspace) =
 for n = ceiling(xneg/xspace) upto floor(xpos/xspace):
   safedraw zconv((n*xspace,yneg)--(n*xspace,ypos));
 endfor
 for n = ceiling(yneg/yspace) upto floor(ypos/yspace) :
   safedraw zconv((xneg,n*yspace)--(xpos,n*yspace));
 endfor
enddef;
%%%%%% End grid of lines.

%%%%%% Polar grid:
% plrpatch draws arcs with radii beginning rstart, stepping by rstep,
% and with starting angle tstart and ending angle tstop. Also draws
% radial lines from rstart to rstop, at angles beginning with tstart and
% stepping by tstep. All 6 parameters in graph coordinates.

vardef plrpatch (expr rstart,rstop,rstep,tstart,tstop,tstep) =
  save rs_;
  rs_:= if rstart = 0 : rstep else: rstart fi;
  for rad=rs_ step rstep until rstop:
    store (trash) drawn arcplr ((0,0),tstart,tstop,rad);
  endfor
  for ang=tstart step tstep until tstop:
    store (trash) drawn (((rstart,0)--(rstop,0)) rotated ang);
  endfor
enddef;

% polargrid: Draws the smallest plrpatch that covers the whole graph,
% where arcs have radii that are multiples of rstep and radial lines
% have angles that are multiples of tstep. Then clips it to the graph
% boundaries. Both parameters in graph coordinates.

vardef polargrid (expr rstep,tstep) =
  save p,r,t,rmax,rmin,tmax,tmin;
  pair p[];
  % Four corners:
  p0:=(xneg,yneg); p1:=(xneg,ypos); p2:=(xpos,ypos); p3:=(xpos,yneg);
  % Find max and min of radius and angles.
  r0:=abs(p0); rmax:=r0;
  for j = 1 upto 3 :
    r[j]:=abs(p[j]);
    if rmax < r[j] : rmax:=r[j]; fi
  endfor
  % If origin is in graph do rmin and angles one way:
  if (xneg <=0) and (xpos>=0) and (yneg <=0) and (ypos>=0):
    tmin:=0; tmax:=360; rmin:=0;
  else:
    t0:= angle(p0);
    rmin:=r0; tmax:=t0; tmin:=t0;
    for j=1 upto 3:
      t[j]:=t0 + angle (p[j] rotated -t0)
      if rmin > r[j] : rmin:=r[j]; fi
      if tmax < t[j] : tmax:=t[j]; fi
      if tmin > t[j] : tmin:=t[j]; fi
    endfor
  fi
  % force rmin and tmin to be multiples of rstep and tstep
  rmin:=rstep*floor(rmin/rstep);
  tmin:=tstep*floor(tmin/tstep);
  save v; picture v;
  v:= image(plrpatch( rmin, rmax, rstep, tmin, tmax, tstep ));
  clipto (v) zconv(rect((xneg,yneg),(xpos,ypos)));
  addto active_plane also v;
enddef;

vardef polarpatch (expr rstart, rstop, rstep, tstart, tstop, tstep) =
  plrpatch (rstart, rstop, rstep, tstart, tstop, tstep)
  % make sure the ending boundaries are drawn, too:
  store (trash) drawn arcplr ((0,0),tstart,tstop,rstop);
  store (trash) drawn (((rstart,0)--(rstop,0)) rotated tstop);
enddef;

%% Upright Rectangles.
% coordinate-independent.

vardef rect (expr ll, ur) =
  ll--(xpart ur, ypart ll)--
    ur--(xpart ll,ypart ur)--cycle
enddef;


%% Path Construction.
% coordinate-independent.

% general path construction

vardef mkpath (expr smooth,cyclic) (suffix pts) =
 if smooth:
  mksmooth (cyclic, pts)
 else:
  mkpoly (cyclic, pts)
 fi
enddef;


%% Polylines, including Polygons.
% coordinate-independent.

% polyline construction
% pts is both an array of pairs or paths,
% and a count of its members (from 1 to pts).

vardef mkpoly (expr cyclic) (suffix pts) =
 for i_=1 upto pts-1:
  pts[i_]--
 endfor
 pts[pts]
 if cyclic:
  --cycle
 fi
enddef;

vardef polyline (expr cyclic) (text t) =
 save p_;textpairs (p_,t);
 mkpoly (cyclic, p_)
enddef;

%% Smooth Curves.
% coordinate-independent.

% smooth path construction

numeric curve_tension; curve_tension := 1;

vardef mksmooth (expr cyclic) (suffix pts) =
 if cyclic:
  pts[1]{pts[2]-pts[pts]}
 else:
  pts[1]
 fi
 for i_=2 upto pts-1:
  ..tension curve_tension..pts[i_]{pts[i_+1]-pts[i_-1]}
 endfor
 ..tension curve_tension..pts[pts]
 if cyclic:
  {pts[1]-pts[pts-1]}..tension curve_tension..cycle
 fi
enddef;

% The following for backward compatability:
def curve (expr cyclic) = tcurve (curve_tension,cyclic) enddef;

vardef tcurve  (expr tens, cyclic) (text t) =
 save p_;textpairs (p_,t);
 save curve_tension; curve_tension:=tens;
 mksmooth (cyclic, p_)
enddef;

% quadratic B-splines.
% p[] == B-spline control points,
% p in number.

vardef openqbs (text t) =
  save p_;textpairs (p_,t);
  for i=1 upto p_-2:
    0.5[p_[i],p_[i+1]]
    ..controls 1/6[p_[i+1],p_[i]]
           and 1/6[p_[i+1],p_[i+2]]..
  endfor
  0.5[p_[p_-1],p_[p_]]
enddef;

vardef closedqbs (text t) =
  save p_;textpairs (p_,t);
  p_[p_+1]:=p_1;
  p_[p_+2]:=p_2;
  for i=1 upto p_:
    0.5[p_[i],p_[i+1]]
    ..controls 1/6[p_[i+1],p_[i]]
           and 1/6[p_[i+1],p_[i+2]]..
  endfor
  cycle
enddef;

% cubic B-splines.

vardef mkopencbs (suffix b) =
  for i_=1 upto b-3:
    (b[i_]+4b[i_+1]+b[i_+2])/6
    ..controls 1/3[b[i_+1],b[i_+2]]
           and 2/3[b[i_+1],b[i_+2]]..
  endfor
  (b[b-2]+4b[b-1]+b[b])/6
enddef;

vardef opencbs (text t) =
  save p_;textpairs (p_,t);
  mkopencbs (p_)
enddef;

vardef mkclosedcbs (suffix b) =
  b[b+1]:=b1;
  b[b+2]:=b2;
  for i_=1 upto b:
    (b[i_]+4b[i_+1]+b[i_+2])/6
    ..controls 1/3[b[i_+1],b[i_+2]]
           and 2/3[b[i_+1],b[i_+2]]..
  endfor
  cycle
enddef;

vardef closedcbs (text t) =
  save b_;textpairs (b_,t);
  mkclosedcbs (b_)
enddef;

%% Path Closure.

% With uclosed, the original curve is drawn unchanged. Only a closing
% Bezier is added that meets the endpoints smoothly.
vardef uclosed expr f =
 f
 if not cycle f:
 & (point infinity of f){direction infinity of f}..
    {direction 0 of f}(point 0 of f)&cycle
 fi
enddef;

% path f closed by line segment.
vardef lclosed expr f =
 f
 if not cycle f:
   --cycle
 fi
enddef;

% close path f in manner of mksmooth.

vardef sclosed expr f =
 if cycle f:
  f
 else:
  save n; numeric n;
  n := length f;  % number of Bezier segments in f
  if n <= 1:  % single Bezier segment (n=1), or single point (n=0)
   f..cycle
  else:  % (n >= 2)
   (point 0 of f){(point 1 of f)-(point n of f)}
   ..(subpath (1,n-1) of f)
   ..(point n of f){(point 0 of f)-(point (n-1) of f)}
   ..cycle
  fi
 fi
enddef;

% path f closed by bezier.

vardef bclosed expr f =
 f
 if not cycle f:
   ..cycle
 fi
enddef;

% conversion of Bezier segment key points, z,
% to cubic B-spline control points, b.

def ztob (suffix z, b) =
  save b; pair b[];
  b := 4;
  b1 = 6z1-7z2+2z3;
  b2 =     2z2- z3;
  b3 =    - z2+2z3;
  b4 =     2z2-7z3+6z4;
enddef;

% closure of path f by a cubic B-spline.

vardef cbclosed expr f =
 if cycle f:
  f
 else:
  save p, n; pair p[];
  p1 := point 0 of f;
  p2 := postcontrol 0 of f;
  p3 := precontrol 1 of f;
  p4 := point 1 of f;
  ztob (p,a);
  n := length f;
  p1 := point (n-1) of f;
  p2 := postcontrol (n-1) of f;
  p3 := precontrol n of f;
  p4 := point n of f;
  ztob (p,b);
  b1 := b3;
  b2 := b4;
  b3 := a1;
  b4 := a2;
  f..mkopencbs(b)..cycle
 fi
enddef;


%% Circles and Ellipses.
% coordinate-independent.

vardef ellipse (expr center, radx, rady, angle) =
 save t;
 transform t;
 t:=identity xscaled (2*radx)
   yscaled (2*rady) rotated angle
   shifted center;
 fullcircle transformed t
enddef;

vardef circle (expr center,rad) =
 ellipse (center,rad,rad,0)
enddef;

% Smallest circle containing the three distinct points.
%vardef circumcircle (expr A,B,C) =
%
%enddef;


%% Circular Arcs.
% coordinate-independent.

% center-point-sweep form of arc

vardef arc (expr center, from, sweep) =
 save f; path f;
 f := from;
 if (center<>from) and (sweep<>0):
   n := ceiling (abs(sweep)/45) + 1;
   if n<3: n := 3; fi
   theta := sweep/(n-1);
   save p,q; pair p,q;
   p := from;
   for i=2 upto n:
     p := p rotatedabout (center, theta);
     q := (p-center) rotated if theta<0: - fi 90;
     f := f..p{unitvector q};
   endfor
 fi
 f
enddef;

vardef arccenter (expr from, to, sweep) =
  if (sweep mod 360) = 0:
  message "The central point of this arc is undefined.";
  message "Using midpoint of chord.";
  fi
 ((0.5)[from,to]
  if (sweep mod 360) <> 0:
    + ((cotd (sweep/2)) * ((to-from) rotated 90))/2
  fi)
enddef;

% point-point-sweep form of arc

vardef arcpps (expr from, to, sweep) =
 if ((sweep mod 360)=0) or (from=to):
   message "Undefined arc. A line segment will be used.";
   from--to
 else:
   save center; pair center;
   center := arccenter (from,to,sweep);
   arc (center, from, sweep)
 fi
enddef;

% modified polar --

% center, angle, angle, radius

vardef arcplr (expr center, frtheta, totheta, rad) =
 save from; pair from;
 from:=center+rad*(dir frtheta);
 arc (center, from, totheta-frtheta)
enddef;

% center-point-sweep form

vardef arccps (expr center, from, theta) =
 arc (center, from, theta)
enddef;

% point-point-point form
% new definition changes point of view: draw arc of smallest circle
% containing all three points.
vardef arcppp (expr first, second, third) =
 if (first=second) and (second=third):
   message "Undefined arc. ";
   first--third % trivial path.
 elseif (first=second) or (second=third): % but first <> third
   arc(0.5[first,third],first, 180)
 elseif first = third: % but second is different
   arc(0.5[first,second],first, 360)
 else: % all three different
   save w, sweep, hs, critical;
   pair w[]; numeric sweep, hs[], critical;
   w1 := first - third;
   w2 := second - first;
   w3 := second - third;
   % new code for finding sweep, never goes over 360
   hs1:=angle( w3 rotated (-angle w1)); % angle of corner at third
   hs2:=angle(-w1 rotated (-angle w2)); % angle of corner at first
   sweep:=2(hs1+hs2);                   % between -360 and 360
   critical:=5;
   if abs(sweep) <= critical:  % center may blow out
    % new code is actually a circular arc (almost)
    first{w2 rotated -hs1}..second{w2 rotated hs1}..third{-w3 rotated hs2}
   else:
    save m, d, center; pair m[], d[], center;
    m1:=(0.5)[first,second]; d1:=(second-first) rotated 90;
    m2:=(0.5)[second,third]; d2:=(third-second) rotated 90;
    center = m1+whatever*d1 = m2+whatever*d2;
    arc (center,first,sweep)
   fi
 fi
enddef;

%% Polar Coordinates.
% coordinate-independent.

% polar to rectangular conversion:  (r,th) -> (x,y) = r * dir (th).

vardef polar (expr p) =
  (xpart p) * dir (ypart p)
enddef;


%% Turtle.
% coordinate-independent.

% Argument t is a list of graph coord. pairs.
% First item in list t is starting point,
% rest are successive moves.

vardef turtle (text t) =
 save p_; pair p_[]; p_:=0;
 for a_=t:
  p_[incr p_] := if p_<>1:  p_[p_-1] +  fi  a_;
 endfor
 mkpoly (false, p_)
enddef;


%% Sectors.
% coordinate-independent.

vardef sector (expr center, rad, frtheta, totheta) =
 save from; pair from;
 from:=center+rad*(dir frtheta);
 center -- arc (center,from,totheta-frtheta) -- cycle
enddef;

% This is just for fun.
% piechart calculates the wedges from the text parameter data (a list
% of positive numerics)
% sgn    is +/-1, -1 means to proceed clockwise to the next wedge
% angle  is the starting angle of the starting wedge.
% cent   is the center of the pie
% rad    is its radius
% the value of the center is left in piecenter, the wedges are paths
% piewedge1, piewedge2, etc. The direction down the center of
% piewedge[j] is piedirection[j] and the starting angle is pieangle[j].
vardef piechart (expr sgn, ang, cent, rad) (text data) =
 save sum_, tot_;
 numeric piewedge; piewedge:=0;
 numeric sum_[]; sum_[0]:=0;
 % Get the partial sums. These will be converted to angles, so if
 % the piechart goes clockwise, use negative values
 for val_ = data :
   sum_[incr piewedge] := sum_[piewedge-1] + sgn*val_;
 endfor
 tot_ := sgn*sum_[piewedge]; % tot_ is positive.
 pair piecenter;
 piecenter:=cent;
 % Now define the wedges and record the direction of each:
 path piewedge[];
 pair piedirection[];
 numeric pieangle[]; pieangle1:=ang;
 for n_ = 1 upto piewedge:
   pieangle[n_ + 1] := ang + sum_[n_]/tot_*360;
   piewedge[n_] =
       sector(cent, rad, pieangle[n_], pieangle[n_+1]);
   piedirection[n_] := dir(0.5[ pieangle[n_], pieangle[n_+1] ]);
 endfor
enddef;

% I'm told that there are better ways (than piecharts) to represent
% quantitative data. Perhaps barcharts are what they meant.
% barchart calculates the bars from the text parameter, data.
% start  is the location (on the axis) of the start of the first bar.
%  sep   is the separation between bar centers.
%   r    is barwidth/sep
% vert   is boolean: true if bars are vertical, growing up from x-axis.
% data   is a list of heights.
def barchart (expr start, sep, r, vert)(text data) =
 numeric barlength[], barstart[], chartbar, barwd;
 path chartbar[];
 chartbar := 0;
 barwd := r*sep;
 for item_ = data :
   barlength[incr chartbar] := item_;
   barstart[chartbar] := start + sep*(chartbar-1);
   if vert :
     chartbar[chartbar] :=
       rect ((barstart[chartbar], 0),
             (barstart[chartbar] + barwd, barlength[chartbar]) );
   else :
     chartbar[chartbar] :=
       rect ( (0, barstart[chartbar]),
              (barlength[chartbar], barstart[chartbar] + barwd) );
   fi
 endfor
enddef;

%% Utility Functions.
% coordinate-independent.

% the identity function.
def id (expr x) =
  x
enddef;


%% Plotting of functions.
% coordinate-independent.

% In these macros, if the boolean argument `smooth' then the path
% returned will be a B\'ezier, otherwise it will be a polyline.
% If a tens parameter exists, then the smooth version will have
% that value of tension, otherwise the MF default of 1 is used.

% In mkfcn, for example, the parameter values determining the points
% through which the path interpolates begin at bmin, and step by bst
% until bmax is reached or exceeded. If bmax - bmin is not a whole
% number of bst, then bst is adjusted so it is.

% mkfcn: convert function `pf : (x, y) = pf (t)' to a path.
% pf : numeric -> pair.
% domain interpreted as parameter values,
% codomain as cartesian coordinate pairs.

vardef mkfcn (expr smooth, tens) (expr bmin, bmax, bst) (text pf) =
 save p_; pair p_[]; p_ := 0;
 save dx_,n_,bv_; numeric dx_,n_,bv_;
 % Handle errors and ensure p doesn't exceed infinity:
 if bmax > bmin :
    dx_ := max(abs(bst), nottoosmall*(bmax - bmin));
    dx_ := min(dx_, bmax - bmin);
 elseif bmax < bmin : % dx_ will be negative
    dx_ := min(-abs(bst), nottoosmall*(bmax - bmin));
    dx_ := max(dx_, bmax - bmin);
 else :
    dx_ := 1;
 fi
 n_:=round((bmax-bmin)/dx_);
 if not (n_=0) :
   dx_:=(bmax-bmin)/n_;
   bv_:=bmin;
 fi
 for i = 0 upto n_-1 :
   p_[incr p_] := pf(bv_);
   bv_:= bv_ + dx_;
 endfor
 % force n_th iteration to be at bmax (dx_ could be rounded up)
 p_[incr p_] := pf(bmax);
 save curve_tension; curve_tension:=tens;
 mkpath (smooth, false, p_)
enddef;

let tfcn = mkfcn;

% parafcn: like tfcn, but the text argument is a literal pair
% expression `(fa(t), fb(t))' in the literal `t'.

def parafcn (expr smooth) =
  tparafcn (smooth, cur_tension)
enddef;
vardef tparafcn (expr smooth, tens) (expr bmin, bmax, bst) (text pf) =
   save fp_;
   vardef fp_ (expr t) =
     pf
   enddef;
   mkfcn (smooth,tens) (bmin, bmax, bst) (fp_)
enddef;

% xfcn: convert function `fx : y = f (x)' to a path.
% fx : numeric -> numeric.
% domain interpreted as X coordinate,
% codomain as Y coordinate.
% Return the path.

vardef xfcn (expr smooth) (expr xmin, xmax, st) (text fx) =
   save fp_;
   vardef fp_ (expr x) =
     (x, fx(x))
   enddef;
   mkfcn (smooth,cur_tension) (xmin, xmax, st) (fp_)
enddef;

% function: like xfcn, but the text argument is a literal numeric
% expression `fx(x)' in the literal `x'.

def function (expr smooth) =
  tfunction (smooth, cur_tension)
enddef;
vardef tfunction (expr smooth, tens) (expr xmin, xmax, st) (text fx) =
   save fp_;
   vardef fp_ (expr x) =
     (x, fx)
   enddef;
   mkfcn (smooth,tens) (xmin, xmax, st) (fp_)
enddef;

% rfcn: convert function `ft : r = ft (\theta)' to a path.
% ft : numeric -> numeric.
% domain interpreted as angle in degrees,
% codomain as radial distance from origin.

vardef rfcn (expr smooth) (expr tmin, tmax, st) (text ft) =
   save fq_;
   vardef fq_ (expr t) =
     (ft(t)) * (dir t)
   enddef;
   mkfcn (smooth,cur_tension) (tmin, tmax, st) (fq_)
enddef;

% plrfcn: like rfcn, but the text argument is a literal numeric
% expression `ft(t)' in the literal `t' which represents the
% angle \theta.

def plrfcn (expr smooth) =
   tplrfcn (smooth, cur_tension)
enddef;
vardef tplrfcn (expr smooth, tens) (expr tmin, tmax, st) (text ft) =
   save fq_;
   vardef fq_ (expr t) =
     (ft) * (dir t)
   enddef;
   mkfcn (smooth, tens) (tmin, tmax, st) (fq_)
enddef;

% Creating a path with increasing x from a list of points
% with increasing x coordinates.

numeric fcn_tension; fcn_tension:=1;

vardef fcncontrol (expr X,Y,Z) =
  save dl, dr, U; pair U;
  dl:= xpart (Y - X);
  dr:= xpart (Z - Y);
  % U is the direction the curve is to travel at Y,
  % it is a weighted average of the slopes on either side,
  % the shorter x-interval given more weight:

  U := unitvector ((Z - Y)*dl/dr + (Y - X)*dr/dl);

  % The following is sufficient to keeps the curve moving right:
  % The x-values of the control points lie in the interval
  % xpart Y to xpart Z.
  %
  % Since U is a unit vector, the following puts the
  % control point not more than xpart(Z-Y) from Y, provided
  % fcn_tension >= 1.
  % This also keeps extreme points of the curve within a similar
  % distance of the extreme points of the list of points.

  Y + U*abs(xpart (Z - Y))/fcn_tension
enddef;

vardef functioncurve (expr ftens)(text t) =
  save fcn_tension; fcn_tension:=max(ftens, eps);
  save p_;textpairs (p_,t);
  mkcontrolledfcn (p_)
enddef;

vardef  mkcontrolledfcn (suffix pts) =
  % Extend pts by one on each side by symmetry
  pts[0] := 2pts[1] - pts[2];
  pts[pts+1] := 2pts[pts] - pts[pts-1];
  for i_ = 1 upto pts - 1 :
    pts[i_]..controls fcncontrol (pts[i_-1], pts[i_], pts[i_+1]) and
    fcncontrol (pts[i_+2],pts[i_+1], pts[i_])..
    endfor pts[pts]
enddef;

% This is a useful little utility to draw the points on top of the
% curve through them.

vardef plotnodes (expr symbol, size) expr f =     % f is a path
  save one_symbol; picture one_symbol;
  one_symbol := makesymbol (symbol, size);
  for i = 0 upto (length f) if cycle f: -1 fi :
    picdot (active_plane, one_symbol, zconv(point i of f));
  endfor
  if ClipOn:
    clipsto (active_plane, ClipPath);
  fi
  f
enddef;

%% Overlays - taken from MFbook, p 295.  (Bruce Leban)

picture totalpicture;
boolean totalnull, currentnull;

def clearit =
  currentpicture := totalpicture := nullpicture;
  currentnull := totalnull := true;
enddef;

def keepit =
  addto totalpicture also currentpicture;
  currentpicture := nullpicture;
  totalnull := currentnull;
  currentnull := true;
enddef;

def addto_currentpicture =
  currentnull := false;
  addto currentpicture
enddef;

def mergeit (text do) =
  if totalnull:
    do currentpicture
  elseif currentnull:
    do totalpicture
  else:
    begingroup
      save v_; picture v_;
      v_:=currentpicture;
      addto v_ also totalpicture;
      do v_
    endgroup
  fi
enddef;

def shipit =
  mergeit (shipout)
enddef;

%def showit_ =
%  mergeit (show_)
%enddef;

%def show_ suffix v =
%  display v inwindow currentwindow
%enddef;

% initialize gcode for mfpic files,
numeric gcode; gcode:=0;

%%%
%%%  end  grafbase.mp
%%%
