/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/ansi/ansi.c,v $
 *	$Author: builder $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/ansi/ansi.c,v 1.1 1985-04-12 15:28:23 builder Exp $
 */

#ifndef lint
static char *rcsid_ansi_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/ansi/ansi.c,v 1.1 1985-04-12 15:28:23 builder Exp $";
#endif	lint

#include <stdio.h>
/* Rewritten almost entirely from stolen MIT original by Jim Gettys,
   The Institute for Advanced Study, Princeton, NJ. */

/* this program written to read Ansi labeled tapes produced by Vax/VMS 2.5*/
/* results on other tapes/or versions of VMS 3.0 or later not guaranteed.*/

/* note that the variable record file may not be what you want for fortran */
/* written unformatted files, which add a 2 byte segment code at the start*/
/* of each record.  The accompanying program "unseg" will create Unix fortran*/
/* style files from a VMS fortran segmented file. Note that VMS creates these*/
/* segmented files by default.*/

#define BINARY 0		/*binary files are copied in unchanged*/
				/* .exe or Eunice stream files*/
#define CRLF 1			/*insert new line after each record*/
				/* typical .for, .lis etc text files*/
#define FORTRAN 2		/*fortran carriage control interpretation*/
				/* output of fortran programs*/
#define VARIABLE 3		/*read in variable length recs, transcribe */
				/* to unix fortran style variable records*/
				/* sequential fortran (unformatted) or .obj */
				/* files, transcribed to Unix fortran files*/

int vnos = 1;			/*restore version numbers of files*/
int verbose = 0;
int superverbose = 0;		/*super verbose mode*/
int fbinary = 0;		/*force binary input mode, debugging*/
int list = 0;			/*listing mode*/

/* the following structure definitions generated in reference to DEC VAX/VMS
	magnetic tape user's guide, appendix a */

int fbsize;			/*file block size determined from tape*/
char cbuffer[100];		/*numeric conversion buffer*/

struct volume1 {
	char v_lid[3];		/*label id*/
	char v_num;		/*label number*/
	char v_id[6];		/*volume id*/
	char v_acc;		/*volume access*/
	char v_res1[26];	/*reserved space*/
	char v_owner[13];	/*owner identifier*/
	char v_std;		/*Digital Standard*/
	char v_res2[28];	/*reseved space*/
	char v_lstd;		/*label Standard Version 1 (3)*/
	} vol1;

struct header1 {		/*hdr1 record format*/
	char h1_lid[3];		/*label id*/
	char h1_num;		/*label number (1)*/
	char h1_fname[17];	/*file identifier*/
	char h1_fset[6];	/*file set id*/
	char h1_fsec[4];	/*file section number*/
	char h1_seqno[4];	/*file sequence number*/
	char h1_genno[4];	/*generation number*/
	char h1_gverno[2];	/*version of generation number*/
	char h1_credate[6];	/*creation date*/
	char h1_exdate[6];	/*expiration date*/
	char h1_access;		/*file accessability*/
	char h1_blcount[6];	/*block count (always zero from DEC on hdr1*/
	char h1_syscode[13];	/*system code, should be DECFILE11A*/
	char h1_res1[7];	/*reserved space*/
	} hdr1;
struct header2 {		/*hdr2 record format*/
	char h2_lid[3];		/*label id*/
	char h2_num;		/*label number(should be a 2)*/
	char h2_format;		/*record format F is fixed, D is Variable*/
	char h2_bsize[5];	/*maximum number of chars/block*/
	char h2_rlength[5];	/*record length fixed, max length variable*/
	char h2_flag;		/*flag for RMS cruft*/
	char h2_rmscrud1[20];	/*for VMS 2.0 and earlier, contains RMS Crud*/
	char h2_form;		/*forms control, A means fortran,
				  M imbedded control, space means crlf needed*/
	char h2_morerms[13];	/*more rms stuff*/
	char h2_buffoff[2];	/*buffer offset, should be 00*/
	char h2_res1[28];	/*reserved for future use*/
	} hdr2;

char buffer[32768];	/* buffer for raw reads from magtape */
char nl[] = "\n";	/* carriage control characters */
char ff[] = "\f";
char cr[] = "\r";

main(argc,argv)
int argc;
char *argv[];
 {	register int i;
	register char *p;
	int version,genno,genver;
	char filename[25];		/* recreated file name */
	int bf,ef = argc;		/* beginning and ending files*/
	char *s;
	char *tfile = "/dev/rmt8";	/* pointer to tape to open */
	int match = 0;			/* file name match with argument list*/
	int first;
	int tape,file,bitbucket,c,ftype;
	for(i = 0; i < argc; i++) {
		if(argv[i][0] != '-') continue;
		bf = i + 1;		/* save last argument found*/
		for(s = argv[i] + 1; *s != '\0'; s++) 
			switch(*s) {
				case 0:
					break;
				case 'b':
					fbinary = 1;
					printf("ansi: warning, binary input forced.\n");
					break;
				case 'v':
					verbose = 1;
					break;
				case 's':
					superverbose = 1;
					verbose = 1;
					break;
				case 'f':
					bf += 1;	/*skip drive*/
					tfile = argv[i + 1];
					printf("file %s\n",argv[1]);
					break;
				case 'i':
					vnos = 0;
					break;
				case 'l':
					list = 1;
					verbose = 1;
					break;
				default:
					fprintf(stderr,
						"usage: ansi -[bfilsv tape files]\n");
					exit(1);
			}
		}

	bitbucket = open("/dev/null",1);
	tape = open(tfile,0);		/* acquire the tape drive */
	if (tape == -1) {
		printf("Can't open %s\n",tfile);
		exit(-1);
		}

	if ((c=read(tape,&vol1,80)) != 80) {	/* read in volume label */
		printf("Error in reading volume label.\n");
		exit(-1);
		}
	if(verbose) printf("Volume label found was %6.6s.\n",vol1.v_id);

	while (1) {			/* loop to read files from tape */

	/* read in hdr1 label, error => all done */
	if (read(tape,&hdr1,80) != 80) {
		exit(0);
		}

	if (superverbose) dump1(&hdr1);

	for (i=0,p=hdr1.h1_fname; i++<17 && *p!=' '; p++)
	if (*p>='A' && *p<='Z') *p |= 040;	/* look for end */
	*p = '\0';				/* terminate name with null */

	/* reextract version number from tape*/
	convert(4,hdr1.h1_genno,&genno);
	convert(2,hdr1.h1_gverno,&genver);
	version = (genno-1)*100 + genver + 1;

	if (vnos) sprintf(filename,"%s.%d",hdr1.h1_fname,version);
	else strcpy(filename,hdr1.h1_fname);

	/* see if file on list of files to be extracted */
	match = 0;
	for(i = bf; i < ef; i++)
		if(strcmp(argv[i],filename) == 0) {
				match = 1;
				break;
				}

	if(bf >= ef) match = 1;
	if(list) match = 0;			/* don't retrieve files if
						   listing volume contents*/
	first = 0;
	file = bitbucket;
	if(match) {
		if(verbose) printf("Creating file %s, ",filename) ;
		fflush(stdout);			/* let him see it */
		file = creat(filename,0666);	/* and use remainder as name */
		first = 1;
		if (file==-1) {
			printf("Can't create %s, skipping file on tape.\n"
				,filename);
			file = bitbucket;
			}
		}
	if(list) { first = 1; printf("%s, ",filename) ; }

	if (read(tape,&hdr2,80) != 80) {
		printf("Bad HDR2 record!\n");
		exit(1);
		}

	if (superverbose) dump2(&hdr2);
	convert(5,&hdr2.h2_bsize[0],&fbsize);


	ftype = BINARY;		/*default to binary input*/
	/* lets decode the record format and forms control properly*/	
	switch(hdr2.h2_format) {
	    case 'D':
		switch(hdr2.h2_form) {
		case 'A':	ftype = FORTRAN;
				break;
		case 'M':	ftype = VARIABLE;
				break;
		case ' ':	ftype = CRLF;
				break;
		default:	fprintf(stderr,
				"ansi: bad forms, binary assumed in %s.\n"
					,filename);
				ftype = BINARY;
				break;
				}
		break;
	    case 'F':
		ftype = BINARY;
		if(hdr2.h2_form != 'M')
			 fprintf(stderr,
				"ansi: unknown type, binary assumed in %s.\n",
					 filename);
		break;
	    default:
		ftype = BINARY;
		fprintf(stderr,"ansi: unknown format, binary assumed in %s.\n",
				filename);
		}
	if(fbinary) ftype = BINARY;

	if(first) {
		char *out;		/*text string to output*/
		printf("file type is ") ;
		switch(ftype) {
			case BINARY:	out = "fixed binary or stream.";break;
			case CRLF:	out = "text.";			break;
			case FORTRAN:	out = "fortran text."; 		break;
			case VARIABLE:	out = "variable binary."; 	break;
			default:	out = "unknown."; 		break;
			}
		printf("%s\n",out);
		}

	/* skip rest of header file, last read skips eof */
	while ((c = read(tape,buffer,80)) == 80);
	
	/* binary files get read as is... */
	if (ftype == BINARY) while (1) {
		c = read(tape,buffer,fbsize);
		if (c == 0) break;
		write(file,buffer,c);
		}
	else while (1) {		/* read in records of file */
		c = read(tape,buffer,fbsize);/*read in next record from tape */
	   	if (c == 0) break;	/* EOF => all done with this file */
		if (c != fbsize) printf("Record wrong size (count = %d)\n",c);
	   	p = buffer;

		/* ^ as count means end of buffer */
		while (p < &buffer[fbsize] && *p != '^') {
			int length;
			for (i=0,c=0; i<4; i++) c = c*10 + *p++ - '0';
			c -= 4;
			length = c;
			switch(ftype) {
				case FORTRAN:
					switch(*p) {	/* do fortran crock */
					    default:
					    case ' ':
						write(file,nl,1); /* NL */
						break;
					    case '0':
						write(file,nl,1); /* NL */
						write(file,nl,1); /* NL */
						break;
					    case '+':
						write(file,cr,1); /* CR */
						break;
					    case '1':
						write(file,ff,1); /* FF */
						break;
						}
					write(file,p+1,c-1);
					break;
				case VARIABLE:
					write(file,&length,sizeof(length));
					write(file,p,c);
					write(file,&length,sizeof(length));
					break;
				case CRLF:
					write(file,p,c);
					write(file,nl,1); /*add newline*/
					break;
				}
			p += c;
			}
		}	/* end of record reading loop */
		/* terminate last fortran record */
	if(ftype == FORTRAN) write(file,nl,1);

	if (file != bitbucket) close(file);

	/* skip EOF file, last read skips eof */
	while ((c = read(tape,buffer,80)) == 80);
	}					/* end of loop to read files */
}

dump1(h)
	struct header1 *h;
	{
	printf("label id %3.3s\n",h->h1_lid);
	printf("label number %c\n",h->h1_num);
	printf("name %17.17s\n",h->h1_fname);
	printf("set id %6.6s\n",h->h1_fset);
	printf("section number %4.4s\n",h->h1_fsec);
	printf("sequence number %4.4s\n",h->h1_seqno);
	printf("generation number %4.4s\n",h->h1_genno);
	printf("generation version number %2.2s\n",h->h1_gverno);
	printf("creation date %6.6s\n",h->h1_credate);
	printf("expiration date %6.6s\n",h->h1_exdate);
	printf("access code %c\n",h->h1_access);
	printf("block count %6.6s\n",h->h1_blcount);
	printf("system code %13.13s\n",h->h1_syscode);
	return;
	}
dump2(h)
	struct header2 *h;
	{
	printf("label id %3.3s\n",h->h2_lid);
	printf("label number %c\n",h->h2_num);
	printf("format %c\n",h->h2_format);
	printf("block size %5.5s\n",h->h2_bsize);
	printf("record length %5.5s\n",h->h2_rlength);
	printf("rms flag %c\n",h->h2_flag);
	printf("forms control %c\n",h->h2_form);
	printf("buffer offset %2.2s\n",h->h2_buffoff);
	return;
	}

convert(howmany,where,value)
int howmany;			/*number of characters in numeric field*/
char *where;			/*where to get the characters*/
int *value;			/*place to put the result*/
	{
	int i;
	char *cptr = &cbuffer[0];	/*pointer to temp area*/
	while(howmany--) *cptr++ = *where++;
	*cptr++ = '\0';		/*terminate the string*/
	*value = atoi(cbuffer);	/*get and store the value*/
	}
