% Psnup prolog 
% Copyright (c) 1990,1992 Adobe Systems Incorporated. All Rights Reserved. 
% GOVERNMENT END USERS: See Notice file in TranScript library directory
% -- probably /usr/lib/ps/Notice
% RCS: $Header: /afs/dev.mit.edu/source/repository/third/transcript/lib/psnup.pro,v 1.1.1.1 1996-10-07 20:25:32 ghudson Exp $
/PNSP { % scalefactor PNSP
    /PN save def
    dup scale
} bind def

/PNEP {
    PN restore
    PNshowpage
} bind def

/PNBOX { % width height PNBOX
    gsave
    1 setlinewidth
    exch /width exch def
    0 0 moveto
    width 0 rlineto
    0 exch rlineto
    0 width sub 0 rlineto
    closepath
    stroke
    grestore
} bind def

/PNLS { % scalefactor -width PNLS
    /PN save def
    90 rotate
    0 exch translate
    dup scale
} bind def
