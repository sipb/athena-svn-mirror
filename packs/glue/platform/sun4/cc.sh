#! /bin/sh
echo 'At the present time Sun Microsystems does not ship a C compiler'
echo 'with its Solaris operating system.'
echo
echo 'There is no command "cc".'
echo
echo 'The various C compilers available are so different in the'
echo 'arguments they take that we felt it would be misleading to'
echo 'arbitrarily choose a C compiler and argument set and install it'
echo 'as cc.  So instead you are getting this printout.'
echo
echo 'The SunSoft C compiler is now available. You may access it'
echo 'by typing "add sunsoft" at the athena prompt.  The compiler'
echo 'is called suncc.'
echo
echo 'The Cygnus gcc compiler is available.  You may access it'
echo 'by typing "add cygnus" at the athena prompt.  The compiler is'
echo 'called gcc, and may be accessed by running /mit/cygnus/sun4bin/gcc.'
echo
echo 'The Athena software was built with /usr/gcc/bin/gcc, which was'
echo 'built from the Cygnus 95q2 gcc source tree.'
echo
exit 1
