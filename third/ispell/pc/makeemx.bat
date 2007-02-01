set CC=gcc
set CFLAGS=-O
set REGLIB=-lregexp
set TERMLIB=-ltermcap
set YACC=yacc

copy language\english\msgs.h
copy pc\local.emx local.h
copy config.x config.h
:: goto build
gcc -O -c buildhas.c
gcc -O -c correct.c
gcc -O -c defmt.c
gcc -O -c dump.c
gcc -O -c exp_table.c
gcc -O -c fields.c
gcc -O -c good.c
gcc -O -c hash.c
gcc -O -c icombine.c
gcc -O -c ijoin.c
gcc -O -c ispell.c
gcc -O -c lookup.c
gcc -O -c makedent.c
gcc -O -DUSG -c term.c
gcc -O -c tgood.c
gcc -O -c tree.c
gcc -O -c xgets.c

yacc parse.y
gcc -O -c y_tab.c
move y_tab.o parse.o
del y_tab.c

gcc -O -o buildhas buildhas.o hash.o makedent.o parse.o %LIBES%
emxbind -b buildhas
emxbind -s buildhas.exe

gcc -O -o icombine icombine.o makedent.o parse.o %LIBES%
emxbind -b icombine
emxbind -s icombine.exe

gcc -O -o ijoin ijoin.o fields.o %LIBES%
emxbind -b ijoin
emxbind -s ijoin.exe

:build
ar -q ispell.a term.o ispell.o correct.o defmt.o dump.o exp_table.o fields.o good.o lookup.o hash.o makedent.o tgood.o tree.o xgets.o
gcc -o ispell ispell.a %TERMLIB% %REGLIB% %LIBES%
:: strip ispell
emxbind -b -s ispell
:: because of use of system()
emxbind -a ispell -p
:: goto end


:end
del ispell.a
