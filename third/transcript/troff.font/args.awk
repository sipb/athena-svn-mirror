/^@FAMILYNAME / {print "FAM=" $2;}
/^@FACENAMES /  {print "fR=" $2,"fI=" $3,"fB=" $4, "fS=" $5;}
