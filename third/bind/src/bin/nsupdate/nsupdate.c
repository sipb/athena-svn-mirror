#if !defined(lint) && !defined(SABER)
static char rcsid[] = "$Id: nsupdate.c,v 1.1.1.1 1998-05-04 22:23:38 ghudson Exp $";
#endif /* not lint */

/*
 * Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "port_before.h"
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "port_after.h"

/* XXX all of this stuff should come from libbind.a */

/*
 * Map class and type names to number
 */
struct map {
	char    token[10];
	int     val;
};

struct map class_strs[] = {
	{ "in",         C_IN },
	{ "chaos",      C_CHAOS },
	{ "hs",         C_HS },
};
#define M_CLASS_CNT (sizeof(class_strs) / sizeof(struct map))

struct map type_strs[] = {
	{ "a",          T_A },
	{ "ns",         T_NS },
	{ "cname",      T_CNAME },
	{ "soa",        T_SOA },
	{ "mb",         T_MB },
	{ "mg",         T_MG },
	{ "mr",         T_MR },
	{ "null",       T_NULL },
	{ "wks",        T_WKS },
	{ "ptr",        T_PTR },
	{ "hinfo",      T_HINFO },
	{ "minfo",      T_MINFO },
	{ "mx",         T_MX },
	{ "txt",        T_TXT },
	{ "rp",         T_RP },
	{ "afsdb",      T_AFSDB },
	{ "x25",        T_X25 },
	{ "isdn",       T_ISDN },
	{ "rt",         T_RT },
	{ "nsap",       T_NSAP },
	{ "nsap_ptr",   T_NSAP_PTR },
	{ "px",         T_PX },
	{ "loc",        T_LOC },
};
#define M_TYPE_CNT (sizeof(type_strs) / sizeof(struct map))

struct map section_strs[] = {
	{ "zone",	S_ZONE },
	{ "prereq",	S_PREREQ },
	{ "update", 	S_UPDATE },
	{ "reserved",	S_ADDT },
};
#define M_SECTION_CNT (sizeof(section_strs) / sizeof(struct map))

struct map opcode_strs[] = {
	{ "nxdomain",	NXDOMAIN },
	{ "yxdomain",	YXDOMAIN },
	{ "nxrrset", 	NXRRSET },
	{ "yxrrset",	YXRRSET },
	{ "delete",	DELETE },
	{ "add",	ADD },
};
#define M_OPCODE_CNT (sizeof(opcode_strs) / sizeof(struct map))

static char *progname;
static FILE *log;

static void usage(void);
static int getword_str(char *, int, char **, char *);

/*
 * format of file read by nsupdate is kept the same as the log
 * file generated by updates, so that the log file can be fed
 * to nsupdate to reconstruct lost updates.
 * 
 * file is read on line at a time using fgets() rather than
 * one word at a time using getword() so that it is easy to
 * adapt nsupdate to read piped input from other scripts
 *
 * overloading of class/type has to be deferred to res_update()
 * because class is needed by res_update() to determined the
 * zone to which a resource record belongs
 */
int
main(argc, argv)
	int argc;
	char **argv;
{
	FILE *fp = NULL;
	char buf[BUFSIZ], buf2[BUFSIZ], hostbuf[100], filebuf[100];
	char dnbuf[MAXDNAME];
	u_char packet[PACKETSZ], answer[PACKETSZ];
	char *host = hostbuf, *batchfile = filebuf;
	char *r_dname, *cp, *startp, *endp, *svstartp;
	char section[15], opcode[10];
	int i, c, n, n1, inside, lineno, vc = 0,
		debug = 0, r_size, r_section, r_opcode,
		prompt = 0, ret = 0;
	int16_t r_class, r_type;
	u_int32_t r_ttl;
	struct map *mp;
	ns_updrec *rrecp_start = NULL, *rrecp, *tmprrecp;
	struct in_addr hostaddr;
	extern int getopt();
	extern char *optarg;
	extern int optind, opterr, optopt;


	progname = argv[0];

	while ((c = getopt(argc, argv, "dv")) != EOF) {
		switch (c) {
		case 'v':
			vc = 1;
			break;
		case 'd':
			debug = 1;
			break;
		default:
			usage();
		}
	}

	if ((argc - optind) == 0) {
	    /* no file specified, read from stdin */
	    ret = system("tty -s");
	    if (ret == 0) /* terminal */
		prompt = 1;
	    else /* stdin redirect from a file or a pipe */
		prompt = 0;
	} else {
	    /* file specified, open it */
	    /* XXX - currently accepts only one filename */
	    if ((fp = fopen(argv[optind], "r")) == NULL) {
		fprintf(stderr, "error opening file: %s\n", argv[optind]);
		exit (1);
	    }
	}
	for (;;) {

	    inside = 1;
	    if (prompt)
		fprintf(stdout, "> ");
	    if (!fp)
		cp = fgets(buf, sizeof buf, stdin);
	    else
	        cp = fgets(buf, sizeof buf, fp);
	    if (cp == NULL) /* EOF */
		break;
	    lineno++;

	    /* get rid of the trailing newline */
	    n = strlen(buf);
	    buf[--n] = '\0';
 
	    startp = cp;
	    endp = strchr(cp, ';');
	    if (endp != NULL)
		endp--;
	    else
		endp = cp + n - 1;

	    /* verify section name */
	    if (!getword_str(section, sizeof section, &startp, endp)) {
		/* empty line */
		inside = 0;
	    }
	    if (inside) {
		/* inside the same update packet,
		 * continue accumulating records */
		r_section = -1;
		n1 = strlen(section);
		if (section[n1-1] == ':')
		    section[--n1] = '\0';
		for (mp = section_strs; mp < section_strs+M_SECTION_CNT; mp++)
		    if (!strcasecmp(section, mp->token)) {
			r_section = mp->val;
			break;
		    }
		if (r_section == -1) {
		    fprintf(stderr, "incorrect section name: %s\n", section);
		    exit (1);
		}
		if (r_section == S_ZONE) {
		    fprintf(stderr, "section ZONE not permitted\n");
		    exit (1);
		}
		/* read operation code */
		if (!getword_str(opcode, sizeof opcode, &startp, endp)) {
			fprintf(stderr, "failed to read operation code\n");
			exit (1);
		}
		r_opcode = -1;
		if (opcode[0] == '{') {
		    n1 = strlen(opcode);
		    for (i = 0; i < n1; i++)
			opcode[i] = opcode[i+1];
		    if (opcode[n1-2] == '}')
			opcode[n1-2] = '\0';
		}
		for (mp = opcode_strs; mp < opcode_strs+M_OPCODE_CNT; mp++) {
		    if (!strcasecmp(opcode, mp->token)) {
			r_opcode = mp->val;
			break;
		    }
		}
		if (r_opcode == -1) {
		    fprintf(stderr, "incorrect operation code: %s\n", opcode);
		    exit (1);
		}
		/* read owner's domain name */
		if (!getword_str(dnbuf, sizeof dnbuf, &startp, endp)) {
		    fprintf(stderr, "failed to read owner name\n");
		    exit (1);
		}
		r_dname = dnbuf;
		r_ttl = 0;
		r_type = -1;
		r_class = C_IN; /* default to IN */
		r_size = 0;

		(void) getword_str(buf2, sizeof buf2, &startp, endp);

		if (isdigit(buf2[0])) { /* ttl */
		    r_ttl = strtoul(buf2, 0, 10);
		    if (errno == ERANGE && r_ttl == ULONG_MAX) {
			fprintf(stderr, "oversized ttl: %s\n", buf2);
			exit (1);
		    }
		    (void) getword_str(buf2, sizeof buf2, &startp, endp);
		}

		if (buf2[0]) { /* possibly class */
		    for (mp = class_strs; mp < class_strs+M_CLASS_CNT; mp++) {
			if (!strcasecmp(buf2, mp->token)) {
			    r_class = mp->val;
			    (void) getword_str(buf2, sizeof buf2, &startp, endp);
			    break;
			}
		    }
		}
		/*
		 * type and rdata field may or may not be required depending
		 * on the section and operation
		 */
		switch (r_section) {
		case S_PREREQ:
		    if (r_ttl) {
			fprintf(stderr, "nonzero ttl in prereq section: %ul\n",
				r_ttl);
			r_ttl = 0;
		    }
		    switch (r_opcode) {
		    case NXDOMAIN:
		    case YXDOMAIN:
			if (buf2[0]) {
			    fprintf (stderr, "invalid field: %s, ignored\n",
				     buf2);
			    exit (1);
			}
			break;
		    case NXRRSET:
		    case YXRRSET:
			if (buf2[0])
			    for (mp = type_strs; mp < type_strs+M_TYPE_CNT; mp++)
				if (!strcasecmp(buf2, mp->token)) {
				    r_type = mp->val;
				    break;
				}
			if (r_type == -1) {
			    fprintf (stderr, "invalid type for RRset: %s\n",
				     buf2);
			    exit (1);
			}
			if (r_opcode == NXRRSET)
			    break;
			/*
			 * for RRset exists (value dependent) case,
			 * nonempty rdata field will be present.
			 * simply copy the whole string now and let
			 * res_update() interpret the various fields
			 * depending on type
			 */
			cp = startp;
			while (cp <= endp && isspace(*cp))
			    cp++;
			r_size = endp - cp + 1;
			break;
		    default:
			fprintf (stderr,
				 "unknown operation in prereq section\"%s\"\n",
				 opcode);
			exit (1);
		    }
		    break;
		case S_UPDATE:
		    switch (r_opcode) {
		    case DELETE:
			r_ttl = 0;
			r_type = T_ANY;
			/* read type, if specified */
			if (buf2[0])
			    for (mp = type_strs; mp < type_strs+M_TYPE_CNT; mp++)
				if (!strcasecmp(buf2, mp->token)) {
				    r_type = mp->val;
				    svstartp = startp;
				    (void) getword_str(buf2, sizeof buf2,
						       &startp, endp);
				    if (buf2[0]) /* unget preference */
					startp = svstartp;
				    break;
				}
			/* read rdata portion, if specified */
			cp = startp;
			while (cp <= endp && isspace(*cp))
			    cp++;
			r_size = endp - cp + 1;
			break;
		    case ADD:
			if (r_ttl == 0) {
			    fprintf (stderr,
		"ttl must be specified for record to be added: %s\n", buf);
			    exit (1);
			}
			/* read type */
			if (buf2[0])
			    for (mp = type_strs; mp < type_strs+M_TYPE_CNT; mp++)
				if (!strcasecmp(buf2, mp->token)) {
				    r_type = mp->val;
				    break;
				}
			if (r_type == -1) {
			    fprintf(stderr,
		"invalid type for record to be added: %s\n", buf2);
			    exit (1);
			}
			/* read rdata portion */
			cp = startp;
			while (cp < endp && isspace(*cp))
			    cp++;
			r_size = endp - cp + 1;
			if (r_size <= 0) {
			    fprintf(stderr,
		"nonempty rdata field needed to add the record at line %d\n",
				    lineno);
			    exit (1);
			}
			break;
		    default:
			fprintf(stderr,
		"unknown operation in update section\"%s\"\n", opcode);
			exit (1);
		    }
		    break;
		default:
		    fprintf(stderr,
			    "unknown section identifier \"%s\"\n", section);
		    exit (1);
		}

		if ( !(rrecp = res_mkupdrec(r_section, r_dname, r_class,
					    r_type, r_ttl)) ||
		     (r_size > 0 && !(rrecp->r_data = (u_char *)malloc(r_size))) ) {
			fprintf(stderr, "saverrec error\n");
			exit (1);
		}
		rrecp->r_opcode = r_opcode;
		rrecp->r_size = r_size;
		(void) strncpy((char *)rrecp->r_data, cp, r_size);
		/* append current record to the end of linked list of
		 * records seen so far */
		if (rrecp_start == NULL)
			rrecp_start = rrecp;
		else {	
			tmprrecp = rrecp_start;
			while (tmprrecp->r_next != NULL)
				tmprrecp = tmprrecp->r_next;
			tmprrecp->r_next = rrecp;
		}
	    } else { /* end of an update packet */
		(void) res_init();
		if (vc)
		    _res.options |= RES_USEVC | RES_STAYOPEN;
		if (debug)
		    _res.options |= RES_DEBUG;
		if (rrecp_start) {
		    if ((n = res_update(rrecp_start)) < 0)
			fprintf(stderr, "failed update packet\n");
	            /* free malloc'ed memory */
        	    while(rrecp_start) {
                	tmprrecp = rrecp_start;
                	rrecp_start = rrecp_start->r_next;
			free((char *)tmprrecp->r_dname);
                	free((char *)tmprrecp);
        	    }
		}
	    }
	} /* for */
}

static void
usage() {
	fprintf(stderr, "Usage: %s [-d] [-v] [file]\n",
		progname);
	exit(1);
}

/*
 * Get a whitespace delimited word from a string (not file)
 * into buf. modify the start pointer to point after the
 * word in the string.
 */
static int
getword_str(char *buf, int size, char **startpp, char *endp) {
        char *cp;
        int c;
 
        for (cp = buf; *startpp <= endp; ) {
                c = **startpp;
                if (isspace(c) || c == '\0') {
                        if (cp != buf) /* trailing whitespace */
                                break;
                        else { /* leading whitespace */
                                (*startpp)++;
                                continue;
                        }
                }
                (*startpp)++;
                if (cp >= buf+size-1)
                        break;
                *cp++ = (u_char)c;
        }
        *cp = '\0';
        return (cp != buf);
}
