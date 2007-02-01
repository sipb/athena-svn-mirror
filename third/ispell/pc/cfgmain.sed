/^	  *\$\$SORTTMP/i\
	    -e 's@/munch\\$$\\$$@/mu$$$$@g' \\\
	    -e 's@/faff\\$$\\$$@/fa$$$$@g' \\\
	    -e 's@/sset\\$$\\$$\\.@/se$$$$@g' \\
/^	  *while \[ "X\$\$LANGUAGES" != X \]/i\
	    [ -r languages/english/Makefile.orig ] \\\
	      || (cd languages/english; mv -f Makefile Makefile.orig; \\\
	          ../../pc/cfglang.sed Makefile.orig > Makefile); \\
/^		cd languages\/$$dir; \\/a\
		[ -r Makefile.orig ] \\\
		   || (mv -f Makefile Makefile.orig; \\\
		       ../../pc/cfglang.sed Makefile.orig > Makefile); \\
/^ijoin.o:/i\
term.o: pc/djterm.c
