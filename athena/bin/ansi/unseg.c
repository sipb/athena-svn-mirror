/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/ansi/unseg.c,v $
 *	$Author: vrt $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/ansi/unseg.c,v 1.2 1994-06-28 16:36:28 vrt Exp $
 */

#ifndef lint
static char *rcsid_unseg_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/ansi/unseg.c,v 1.2 1994-06-28 16:36:28 vrt Exp $";
#endif	lint

#include <stdio.h>
/* written by Jim Gettys, The Institute for Advanced Study, Princeton, NJ.
	May, 1983							*/

/*	This program will translate a file created originally on VMS
	as a sequential "segmented" unformatted fortran file and read in
	to Unix by the "ansi" program which translates variable record
	ansi tape files (internal forms control) into Unix f77 compatable
	variable length records. 					*/

/*	a fortran "segmented" file is a sequence of variable length records
	with a two byte header on each record used to encode information to
	allow fortran writes and reads to span multiple records of data.
	See the VMS fortran User's Guide for details.
	This header can have four values, documented below:
		0 - this is in the middle of the record
		1 - this is the beginning of a record
		2 - this is the end of a record
		3 - this is the entire record.
	If you think this is a crock, I think you are right!		*/

#define BUFFERSIZE 512
#define MIDDLE 0
#define START 1
#define END 2
#define ALL 3

long bc;	/* position in the file of the beginning of the record for
		   backpatching */
int rlength;	/* Unix fortran record length, which will be sum of all
		   segments*/
struct record {
		int length;		/* length of record */
		short seg;		/* segment of record */
		} rec;

char buffer[BUFFERSIZE];

main() {
	int nitems;			/* number items read in read*/

	while(nitems = fread(&rec,sizeof(int) + sizeof(short),1,stdin)) {
	    rec.length -= sizeof(rec.seg);	/*reduce count by seg. length*/
	    if((rec.length + sizeof(rec.length)) > BUFFERSIZE) {
		fprintf(stderr,"unseg: segment too long for buffer!\n");
		exit(1);
		}
	    fread(buffer,rec.length + sizeof(rec.length),1,stdin);
			/* read data + trailing length */
	    switch(rec.seg) {
		case START:
			bc = ftell(stdout);
			rlength = rec.length;	/* set to first seg. length*/
			fwrite(&rlength,sizeof(rlength),1,stdout);
					/* write out dummy record length*/
			fwrite(buffer,rec.length,1,stdout);
					/* write out data */
			break;
		case MIDDLE:
			rlength += rec.length ;	/* add it current data */
			fwrite(buffer,rec.length,1,stdout);
			break;
		case END:
			rlength += rec.length;	/*add in current data */
			fwrite(buffer,rec.length,1,stdout);
					/* write out data */
			fwrite(&rlength,sizeof(rlength),1,stdout);
					/* write out real record length*/
			if(fseek(stdout,bc,0) == 1) {
				fprintf(stderr,"unseg: can't seek on stdout!\n");
				exit(1);
				}
			/* patch forward pointer*/
			fwrite(&rlength,sizeof(rlength),1,stdout);
			fseek(stdout,0L,2);	/* back for next record*/
			break;
		case ALL:
			rlength = rec.length;	/* its ok this time*/
			fwrite(&rlength,sizeof(rlength),1,stdout);
			fwrite(buffer,rec.length,1,stdout);
					/* write out data */
			fwrite(&rlength,sizeof(rlength),1,stdout);
			break;
		default:
			fprintf(stderr,"unseg: bad record format!\n");
			exit(1);
			}
		}
	exit(0);
	}
