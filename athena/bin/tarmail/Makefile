#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/tarmail/Makefile,v $
#	$Author: jtkohl $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/tarmail/Makefile,v 1.1 1985-04-25 20:34:52 jtkohl Exp $
#


install:	atob btoa tarmail untarmail
		mv atob btoa /usr/local/bin
		cp tarmail untarmail /usr/local/bin
		cp btoa.1 /usr/man/man1/btoa.1
		cp btoa.1 /usr/man/man1/tarmail.1

btoa:		btoa.c
		cc -O -s btoa.c -o btoa

atob:		atob.c
		cc -O -s atob.c -o atob

clean:		
		rm -f atob btoa *.o
