@Rem Do not turn echo off so they will see what is going on
copy language\english\msgs.h msgs.h > nul
copy pc\local.djgpp local.h > nul
copy config.X config.h > nul
echo /* AUTOMATICALLY-GENERATED SYMBOLS */ >> config.h
echo #define SIGNAL_TYPE_STRING "void" >> config.h
echo #define MASKTYPE_STRING "long" >> config.h
@Rem
@Rem Use the following Goto when you only change a few source
@Rem files and do not want to recompile all of them
:: goto build
gcc -c -O2 -g buildhash.c
gcc -c -O2 -g hash.c
gcc -c -O2 -g makedent.c
bison -y parse.y
if exist y.tab.c ren y.tab.c y_tab.c
gcc -c -O2 -g y_tab.c -o parse.o
if exist y_tab.c del y_tab.c
gcc -o -g buildhash buildhash.o hash.o makedent.o parse.o
gcc -c -O2 -g icombine.c
gcc -o -g icombine icombine.o makedent.o parse.o
gcc -c -O2 -g ijoin.c
gcc -c -O2 -g fields.c
gcc -o -g ijoin ijoin.o fields.o
gcc -c -O2 -g term.c
gcc -c -O2 -g ispell.c
gcc -c -O2 -g correct.c
gcc -c -O2 -g defmt.c
gcc -c -O2 -g dump.c
gcc -c -O2 -g exp_table.c
gcc -c -O2 -g good.c
gcc -c -O2 -g lookup.c
gcc -c -O2 -g tgood.c
gcc -c -O2 -g tree.c
gcc -c -O2 -g xgets.c
:build
@echo ispell.o term.o correct.o defmt.o dump.o good.o lookup.o > link.lst
@echo fields.o exp_table.o hash.o makedent.o tgood.o tree.o xgets.o >> link.lst
@del link.lst
gcc -o -g ispell @link.lst
@Rem
@Rem Strip the .exe but leave the COFF images with debug info
strip *.exe
cd deformatters
gcc -O2 -g -o defmt-c defmt-c.c
gcc -O2 -g -o defmt-sh defmt-sh.c
strip *.exe
cd ..
