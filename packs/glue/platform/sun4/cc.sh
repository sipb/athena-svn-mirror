#! /bin/sh
echo 'At the present time Sun Microsystems does not ship a C compiler'
echo 'with its Solaris operating system.'
echo
echo 'There is no command "cc".'
echo
echo 'The various C compilers available are so different in the'
echo 'arguments they take that we felt it would be misleading to'
echo 'arbitrarily choose a C compiler and argument set and install it'
echo 'as cc. So instead you are getting this printout.'
echo
echo 'The SunSoft C compiler is now available. You may access it'
echo 'by typing "add sunsoft" at the athena prompt. The compiler'
echo 'is called suncc.'
echo
echo 'The Cygnus gcc compiler is available. You may access it'
echo 'by typing "add cygnus" at the athena prompt. The compiler is called'
echo 'gcc, and may be accessed by runnig /mit/cygnus/sun4bin/gcc.'
echo
echo 'The system was built with a script:'
echo
echo '#!/bin/sh'
echo '/usr/gcc/bin/gcc -traditional -B/usr/gcc/lib/ -DSOLARIS -I/usr/gcc/include $@'
echo
echo 'If your application requires ANSI C, eliminate the flag'
echo '" -traditional".'
echo
echo 'We do not recommend this script. The next release will be built'
echo 'using the Sun compiler.'
echo
exit 1
