:: This is a sample batch illustrating how to make color separation
:: using CMYK-HAX.TEX. The script can be easily adapted to any platform
:: Calls for `TEX' and `DVIPS' should be changed to respective local names:
::       TEX  calls tex with appropriate format (e.g., plain or MeX, or...)
::       DVIPS  calls dvips (or dvips32.exe)
:: Names SAMPLE1.TEX, SAM1-C.PS, SAM1-M.PS, SAM1-Y.PS, SAM1-K.PS
:: should be replaced appropriately by parameters of the script.
::
:: MAKE CYAN SEPARATION (PROJECTION):
call tex \def\separate{\CYAN}\input sample1.tex
call dvips sample1 -osam1-c.ps
:: 
:: MAKE MAGENTA SEPARATION (PROJECTION):
call tex \def\separate{\MAGENTA}\input sample1.tex
call dvips sample1 -osam1-m.ps
::
:: MAKE YELLOW SEPARATION (PROJECTION):
call tex \def\separate{\YELLOW}\input sample1.tex
call dvips sample1 -osam1-y.ps
::
:: MAKE BLACK SEPARATION (PROJECTION):
call tex \def\separate{\BLACK}\input sample1.tex
call dvips sample1 -osam1-k.ps
