/*
 *	$Id: dent.c,v 1.3 1999-01-22 23:09:17 ghudson Exp $
 */

#ifndef lint
static char *rcsid_dent_c = "$Id: dent.c,v 1.3 1999-01-22 23:09:17 ghudson Exp $";
#endif /* lint */

#include <stdio.h>

#define MAXCHARS 1024		/* maximum length of a single line */

/*
	This routine indents the standard input.
*/

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fptr = stdin;	/* initially read standard input file */
	register int i,j;
	int nspaces = 8;
	for(i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			if(sscanf(&argv[i][1],"%d",&nspaces) == 0) {
				fprintf(stderr,
					"%s: argument must be integer.\n",
					argv[0]);
				exit(1);
			}
			continue;
		}
		if((fptr = fopen(argv[i],"r")) == NULL) {
			fprintf(stderr,"%s: no such file %s.\n",
				argv[0],argv[i]);
			exit(1);
		}
		indent(fptr,nspaces);
	fclose(fptr);
	}
	if(fptr == stdin) indent(fptr,nspaces);
	exit(0);
}

int indent(fptr,nspaces)
FILE *fptr;
int nspaces;
{
	register int i,j;
	char line[MAXCHARS];
	while(fgets(line,MAXCHARS,fptr) != NULL) {
		if(strlen(line) >= 1) {
			spaceout(nspaces);
			for (j = 0; j < strlen(line); j++) {
				putchar(line[j]);
				if(line[j] == '\r') spaceout(nspaces);
			}
		}
	}
}
spaceout(spaces)
register int spaces;
{
	while(spaces-- > 0) putchar(' ');
}
