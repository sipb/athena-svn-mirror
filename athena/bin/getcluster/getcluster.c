/*
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/getcluster.c,v $
 *      $Author: ens $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/getcluster.c,v 1.1 1987-07-23 09:41:04 ens Exp $
 */

#ifndef lint
static char rcsid_test2_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/getcluster.c,v 1.1 1987-07-23 09:41:04 ens Exp $";
#endif

#include <stdio.h>
#include <ctype.h>
#include <hesiod.h>

/*
 * Make a hesiod cluster query
 * for the machine you are on
 * and produce a set of environment variable
 * assignments for the C shell or the Bourne shell,
 * depending on the '-b' flag
 *
 * If any stdio errors, truncate standard output to 0
 * and return an exit status.
 */

main(argc, argv)
char *argv[];
{
	register char **hp;
	int bourneshell = 0;
	char myself[80];

	if (argc == 2 && strcmp(argv[1], "-b") == 0)
		bourneshell++;
	if (gethostname(myself, 80) < 0) {
		perror("Can't get my own hostname");
		exit(-1);
	}
	hp = hes_resolve(myself, "cluster");
	if (hp == NULL) {
		fprintf(stderr, "No Hesiod information available\n");
		exit(-1);
	}
	shellenv(hp, bourneshell);
}

shellenv(hp, bourneshell)
char **hp;
int bourneshell;
{
	char var[80], val[80];

	if (bourneshell) {
		while(*hp) {
			sscanf(*hp++, "%s %s", var, val);
			upper(var);
			printf("%s=%s ; export %s\n", var, val, var);
		}
	} else
		while(*hp) {
			sscanf(*hp++, "%s %s", var, val);
			upper(var);
			printf("setenv %s %s\n", var, val);
		}
	if (ferror(stdout)) {
		ftruncate(fileno(stdout), 0L);
		exit(-1);
	}
}

upper(v)
register char *v;
{
	while(*v) {
		*v = toupper(*v);
		v++;
	}
}
