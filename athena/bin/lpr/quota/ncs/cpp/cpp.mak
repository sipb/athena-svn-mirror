#
# MS/DOS makefile for cpp.
#

# *************************************************************************        

MSC             = \msc

MSCLIB          = $(MSC)\lib
MSCINC          = $(MSC)\include
MSCBIN          = $(MSC)\bin

CPP             = $(MSCBIN)\errout ..\cpp\cpp
CC              = $(MSCBIN)\cl
LIB             = $(MSCBIN)\lib
LINK            = $(MSCBIN)\link
ASM             = masm

# *************************************************************************        

TMP             = c:\tmp
IDL             = ..\idl
DEFS            = -DMSDOS -Di8086 -DM_I86LM -D__STDC__
DEBUGFLAG       = -DDEBUG
STATSFLAG       =
PFLAGS          = -I$(IDL) $(DEFS) $(DEBUGFLAG) $(STATSFLAG) -N

OPTFLAG         = /Od                   # Optimization: -O[dastx]
CMODELFLAG      = /AS                   # Storage model: -A[SMCLH]
CDEBUGFLAGS     =                       # Codeview stuff: -Zi
CFLAGS          = /Gs /Zp $(CMODELFLAG) $(CDEBUGFLAGS) $(OPTFLAG)

# *************************************************************************        

OBJS = cpp1.obj cpp2.obj cpp3.obj cpp4.obj cpp5.obj cpp6.obj

.c.obj:
	$(CC) $(CFLAGS) /c $*.c ; >$*.txt
	type $*.txt

cpp1.obj: $*.c

cpp2.obj: $*.c

cpp3.obj: $*.c

cpp4.obj: $*.c

cpp5.obj: $*.c

cpp6.obj: $*.c

cpp: $(OBJS)
	link cpp1+cpp2+cpp3+cpp4+cpp5+cpp6, cpp, cpp ;
