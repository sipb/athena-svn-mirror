dnl ----------------- GCC Compilation ----------------------------------
AC_OUTPUT(
Makefile src/Makefile man/Makefile fonts/Makefile
Makefile.bsd src/Makefile.bsd man/Makefile.bsd fonts/Makefile.bsd
HOWTO/Makefile HOWTO/Makefile.bsd
 ,[
 echo "#define OSNAME $osword" >>config.h
 echo "#define OSVERSION $version" >>config.h
 ]
)
