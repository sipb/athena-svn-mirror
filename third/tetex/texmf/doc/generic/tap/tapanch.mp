defaultfont:="cmr8"; verbatimtex \everymath{\scriptstyle} etex

vardef rule (expr ll, ur) =
  image(
    save r_; path r_;
    r_=ll--(xpart ur, ypart ll)--ur--(xpart ll, ypart ur)--cycle;
    fill r_ withcolor .8white; draw r_;
  )
enddef;

vardef anchor_line@#(expr s,p) =
 image(
  if path p:
    draw p withpen pencircle scaled 3/2bp; % dashed dashpattern(on 9 off 4);
    fill fullcircle scaled 4bp shifted point 0 of p withcolor white; 
    draw fullcircle scaled 4bp shifted point 0 of p; 
    fill fullcircle scaled 4bp shifted point infinity of p;
    label@#(s,point 0 of p);
  else:
   fill fullcircle scaled 4bp shifted p; label@#(s,p);
  fi
 )
enddef;

beginfig(100)
 r=3mm; R=6mm; H=40mm; V=8mm;
 a0=0; a1=a0+R; a2=a1+H; a3=a2+r; a4=a3+H; a5=a4+R; I=5;
 b0=0; b1=b0+r; b2=b1+V; b3=b2+r; b4=b3+V; b5=b4+r; b6=b5+V; b7=b6+r; J=7;
 for i:=0 upto I-1:
  for j:=0 upto J-1:
   if ((i mod 2)<>1) or ((j mod 2)<>1):
    draw rule((a[i],b[j]), (a[i+1],b[j+1]));
   else:
    label("cell", 1/2[(a[i],b[j]), (a[i+1],b[j+1])]);
   fi
  endfor
 endfor

 draw anchor_line.llft (btex $A$ etex,  (a4,b6)--(a4+1/2r,b6+1/2r));
 draw anchor_line.ulft (btex $A'$ etex, (a4,b5)--(a4+1/2r,b5+1/2r));
 draw anchor_line.lrt  (btex $B'$ etex,  (a3,b6)--(a3-1/2r,b6-1/2r));
 draw anchor_line.urt  (btex $B$ etex,  (a3,b5)--(a3-1/2r,b5-1/2r));
 draw anchor_line.rt  (btex $C$ etex,  (a5,b4)--(a5-1/2r,b4+1/2r));
 draw anchor_line.lft  (btex $D$ etex,  (a0,b3)--(a0+1/2r,b3-1/2r));
 draw anchor_line.lrt  (btex $E$ etex,  (a1,b2));
 draw anchor_line.ulft (btex $F$ etex,  (a2,b1));

endfig;

end.
