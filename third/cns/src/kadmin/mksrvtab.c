/* mksrvtab.c */
/* Copyright 1994 Cygnus Support */
/* Mark Eichin */
/*
 * Permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Cygnus Support makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */
/* Simple portable C program to create a srvtab file from a key. Only
   gets filename from krb.h. Uses raw key value rather than string2key. */

#include <krb.h>
#include <stdio.h>
#include <string.h>

#define FOPEN_READ "r"
#define FOPEN_WRITE "w"

#define BUFLN 1024

chop(p)
	char *p;
{
	char *q;
	q = strchr(p,'\n');
	if (q) *q = 0;
}

main() {
	FILE *sfile;
        char buf[BUFLN];
	char *p;
	char srvdata[BUFLN];
	char *output = srvdata;
	char *srvstart = srvdata;
	int n;

	printf("Name of srvtab file (hit return for default %s):\n",KEYFILE);
	fgets(buf, BUFLN, stdin);
	if (buf[0] == '\n') {
		strcpy(buf, KEYFILE);
	}

 	chop(buf);

	printf("Creating srvtab %s\n", buf);

	sfile = fopen(buf, FOPEN_READ);
	if (sfile) {
		fclose(sfile);
		fprintf(stderr, "srvtab already exists!\n");
		exit(1);
	}
	sfile = fopen(buf, FOPEN_WRITE);
	if (!sfile) {
		perror("opening srvtab file");
		exit(1);
	}
	printf("Name: "); fflush(stdout);
	fgets(buf, BUFLN, stdin);
        chop(buf);
	sprintf(output,  "%s%c", buf, 0);
	output += strlen(buf)+1;

        printf("Instance: "); fflush(stdout);
        fgets(buf, BUFLN, stdin);
        chop(buf);
        sprintf(output,  "%s%c", buf, 0);
        output += strlen(buf)+1;

        printf("Realm: "); fflush(stdout);
        fgets(buf, BUFLN, stdin);
        chop(buf);
        sprintf(output,  "%s%c", buf, 0);
        output += strlen(buf)+1;
 
	printf("Version Number: "); fflush(stdout);
	fgets(buf, BUFLN, stdin);
	sprintf(output,  "%c", atoi(buf));
        output += 1;

	printf("Key (16 hex digits): "); fflush(stdout);
	fscanf(stdin, "%02x%02x%02x%02x%02x%02x%02x%02x",
		buf, buf+1, buf+2, buf+3,
		buf+4, buf+5, buf+6, buf+7);
	sprintf(output,  "%c%c%c%c%c%c%c%c",
		buf[0], buf[1], buf[2], buf[3],
		buf[4], buf[5], buf[6], buf[7]);
	output += 8;

	n = fwrite(srvdata, 1, output-srvstart, sfile);
	if (n != output-srvstart) {
		perror("Failed writing srvtab");
		exit(1);
	}
	if (fclose(sfile)) {
		perror("Failed to close srvtab");
		exit(1);
	}
}
