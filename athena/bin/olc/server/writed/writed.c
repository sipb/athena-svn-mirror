/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
/*
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 * Note:  This is mostly fingerd.c with all occurances of
 * "finger" changed to "write".
 */
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
static char *rcsid_writed_c = "$Header: /afs/rel-eng.athena.mit.edu/project/release/current/source/bsd-4.3/common/etc/RCS/writed.c,v 1.3 90/04/05 18:31:44 epeisach Exp ";
#endif not lint

/*
 * Write server.
 */
#include <mit-copyright.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <stdio.h>
#include <ctype.h>

main(argc, argv)
	char *argv[];
{
	register char *sp;
	char line[BUFSIZ];
	struct sockaddr_in sin;
	int i;
	char *av[10]; /* space for 9 arguments, plus terminating null */


	i = sizeof (sin);
	if (getpeername(0, &sin, &i) < 0)
		fatal(argv[0], "getpeername");
	line[0] = '\0';

	fgets(line, BUFSIZ, stdin);
	sp = line;
	av[0] = "write";
	av[1] = "-f";
	i = 2;
	while (1) {
		while (isspace(*sp))
			sp++;
		if (!*sp)
			break;
		av[i++] = sp;
		if (i == 9)
			/* past end of av space -- throw out the rest */
			/* of the args				      */
			break;
		while (*sp && !isspace(*sp)) sp++;
		if (*sp) *sp++ = '\0';
	}
	av[i] = 0;
	/* Put the socket on stdin, stdout, and stderr */
	dup2(0, 1);
	dup2(0, 2);
	execv("/bin/write", av);
	_exit(1);
}

fatal(prog, s)
	char *prog, *s;
{

	fprintf(stderr, "%s: ", prog);
	perror(s);
	exit(1);
}
