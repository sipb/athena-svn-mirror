/* 
 * $Id: from.c,v 1.1 1991-05-29 10:04:04 akajerry Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/from/from.c,v $
 * $Author: akajerry $
 *
 * This is the main source file for a KPOP version of the from command. 
 * It was written by Theodore Y. Ts'o, MIT Project Athena.  And later 
 * modified by Jerry Larivee, MIT Project Athena,  to support both KPOP
 * and the old UCB from functionality.
 */

#if !defined(lint) && !defined(SABER) && defined(RCS_HDRS)
static char *rcsid = "$Id: from.c,v 1.1 1991-05-29 10:04:04 akajerry Exp $";
#endif /* lint || SABER || RCS_HDRS */

#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <strings.h>
#ifdef HESIOD
#include <hesiod.h>
#endif
#include <ctype.h>

#define NOTOK (-1)
#define OK 0
#define DONE 1

#define MAX_HEADER_LINES 512

FILE 	*sfi, *sfo;
char 	Errmsg[80];
char	*optarg, *malloc(), *getlogin(), *strdup(), *parse_from_field();
int	optind;
struct	passwd *getpwuid();
uid_t	getuid();

int	popmail_debug, verbose, unixmail, popmail;
char	*progname, *sender, *user, *host;

char	*headers[MAX_HEADER_LINES];
int	num_headers, skip_message;

char *Short_List[] = {
	"^from$", NULL
	};

char *Verbose_List[] = {
	"^to$", "^from$", "^subject$", "^date$", NULL
	};

	
main(argc, argv)
	int	argc;
	char	**argv;
{
	int	retval = 1;

	PRS(argc,argv);
	if (popmail)
	  retval = getmail_pop(user, host);
	if (unixmail)
	  retval &= getmail_unix(user);
	exit(retval);
}

PRS(argc,argv)
	int	argc;
	char	**argv;
{
	register struct passwd *pp;
	int	c;
	extern char	*getenv();
#ifdef HESIOD
	struct hes_postoffice *p;
#endif HESIOD

	progname = argv[0];
	verbose = popmail_debug = 0;
	host = user = sender = NULL;
	unixmail = popmail = 1;
	
	optind = 1;
	while ((c = getopt(argc,argv,"vdpus:h:")) != EOF)
		switch(c) {
		case 'v':
		        /* verbose mode */
			verbose++;
			break;
		case 'd':
			/* debug mode */
			popmail_debug++;
			break;
		case 'p':
			/* check pop only */
			unixmail = 0;
			break;
		case 'u':
			/* check unix mail only */
			popmail = 0;
			break;
		case 's':
			/* check for mail from sender only */
			sender = optarg;
			break;
		case 'h':
			/* specify pobox host */
			host = optarg;
			break;
		case '?':
			lusage();
		      }
	/* check mail for user */
	if (optind < argc)
		user = argv[optind];
	else {
		user = strdup(getlogin());
		if (!user || !*user) {
			pp = getpwuid((int) getuid());
			if (pp == NULL) {
				fprintf(stderr, "Who are you?\n");
				exit(1);
			}
			user = pp->pw_name;
		}
	      }
	
	if (popmail) {
	  if (!host)
	    host = getenv("MAILHOST");
#ifdef HESIOD
	  if (!host) {
	    p = hes_getmailhost(user);
	    if (p && !strcmp(p->po_type, "POP"))
	      host = p->po_host;
	  }
#endif HESIOD
	  if (!host) {
	    if (!unixmail) {
	      fprintf(stderr, "%s: can't find post office server\n",
		      progname);
	      return(1);
	    }
	    else {
	      fprintf(stderr, "%s: can't find post office server\n",
		      progname);
	      fprintf(stderr, "Trying unix mail drop ...");
	      popmail = 0;
	    }
	  }
	}
}

lusage()
{
	printf("Usage: %s [-v] [-p | -u] [-s sender] [-h host] [user]\n", progname);
	exit(1);
}


getmail_pop(user, host)
     char	*user,*host;
{
	int nmsgs, nbytes;
	char response[128];
	register int i, j;
	int	header_scan();

	if (pop_init(host) == NOTOK) {
		error(Errmsg);
		return(1);
	}

	if ((getline(response, sizeof response, sfi) != OK) ||
	    (*response != '+')){
		error(response);
		return(1);
	}

#ifdef KPOP
	if (pop_command("USER %s", user) == NOTOK || 
	    pop_command("PASS %s", user) == NOTOK)
#else !KPOP
	if (pop_command("USER %s", user) == NOTOK || 
	    pop_command("RPOP %s", user) == NOTOK)
#endif KPOP
	{
		error(Errmsg);
		(void) pop_command("QUIT");
		return(1);
	}

	if (pop_stat(&nmsgs, &nbytes) == NOTOK) {
		error(Errmsg);
		(void) pop_command("QUIT");
		return(1);
	}
	if (verbose)
		printf("You have %d messages (%d bytes) on %s:\n",
		       nmsgs, nbytes, host);
	
	for (i = 1; i <= nmsgs; i++) {
		if (verbose && !skip_message)
			printf("\n");
		num_headers = skip_message = 0;
		if (pop_scan(i, header_scan) == NOTOK) {
			error(Errmsg);
			(void) pop_command("QUIT");
			return(1);
		}
		for (j = 0; j < num_headers; j++) {
			if (!skip_message)
				puts(headers[j]);
			free(headers[j]);
		}
        }
	
	(void) pop_command("QUIT");
	return(0);
}

header_scan(line, last_header)
	char	*line;
	int	*last_header;
{
	char	*keyword;
	register int	i;
	
	if (*last_header && isspace(*line)) {
		headers[num_headers++] = strdup(line);
		return;
	}

	for (i=0;line[i] && line[i]!=':';i++) ;
	keyword=malloc((unsigned) i+1);
	(void) strncpy(keyword,line,i);
	keyword[i]='\0';
	MakeLowerCase(keyword);
	if (sender && !strcmp(keyword,"from")) {
		char *mail_from = parse_from_field(line+i+1);
		if (strcmp(sender, mail_from))
			skip_message++;
		free (mail_from);
	}
	if (list_compare(keyword, verbose ? Verbose_List : Short_List)) {
		*last_header = 1;
		headers[num_headers++] = strdup(line);
	} else
		*last_header = 0;	
}

pop_scan(msgno,action)
	int	msgno;
	int	(*action)();
{	
	char buf[4096];
	int	headers = 1;
	int	scratch = 0;	/* Scratch for action() */

	(void) sprintf(buf, "RETR %d", msgno);
	if (popmail_debug) fprintf(stderr, "%s\n", buf);
	if (putline(buf, Errmsg, sfo) == NOTOK)
		return(NOTOK);
	if (getline(buf, sizeof buf, sfi) != OK) {
		(void) strcpy(Errmsg, buf);
		return(NOTOK);
	}
	while (headers) {
		switch (multiline(buf, sizeof buf, sfi)) {
		case OK:
			if (!*buf)
				headers = 0;
			(*action)(buf,&scratch);
			break;
		case DONE:
			return(OK);
		case NOTOK:
			return(NOTOK);
		}
	}
	while (1) {
		switch (multiline(buf, sizeof buf, sfi)) {
		case OK:
			break;
		case DONE:
			return(OK);
		case NOTOK:
			return(NOTOK);
		}
	}
}

/* Print error message.  `s1' is printf control string, `s2' is arg for it. */

/*VARARGS1*/
error (s1, s2)
     char *s1, *s2;
{
  printf ("pop: ");
  printf (s1, s2);
  printf ("\n");
}

char *re_comp();

int list_compare(s,list)
      char *s,**list;
{
      char *err;

      while (*list!=NULL) {
              err=re_comp(*list++);
              if (err) {
                      fprintf(stderr,"%s: %s - %s\n",progname,err,*(--list));
                      exit(1);
                      }
              if (re_exec(s))
                      return(1);
      }
      return(0);
}

MakeLowerCase(s)
      char *s;
{
      int i;
      for (i=0;s[i];i++)
              s[i]=isupper(s[i]) ? tolower(s[i]) : s[i];
}

/*
 * Duplicate a string in malloc'ed memory
 */
char *strdup(s)
      char    *s;
{
      register char   *cp;
      
      if (!s)
	      return(NULL);
      if (!(cp = malloc((unsigned) strlen(s)+1))) {
              printf("Out of memory!!!\n");
              abort();
      }
      return(strcpy(cp,s));
}

char *parse_from_field(str)
	char	*str;
{
	register char	*cp, *scr;
	char		*stored;
	
	stored = scr = strdup(str);
	while (*scr && isspace(*scr))
		scr++;
	if (cp = index(scr, '<'))
		scr = cp+1;
	if (cp = index(scr, '@'))
		*cp = '\0';
	if (cp = index(scr, '>'))
		*cp = '\0';
	scr = strdup(scr);
	MakeLowerCase(scr);
	free(stored);
	return(scr);
}

/*
 * Do the old unix mail lookup.
 */

getmail_unix(user)
     char *user;
{
	char lbuf[BUFSIZ];
	char lbuf2[BUFSIZ];
	int stashed = 0;
	register char *name;
	char *getlogin();

	if (sender != NULL)
	  for (name = sender; *name; name++)
	    if (isupper(*name))
	      *name = tolower(*name);

	if (chdir("/usr/spool/mail") < 0)
	  return(1);

	if (freopen(user, "r", stdin) == NULL) {
	  if(!popmail) {
	    fprintf(stderr, "Can't open /usr/spool/mail/%s\n", user);
	    exit(0);
	  }
	  else 
	    return(1);
	}

	while (fgets(lbuf, sizeof lbuf, stdin) != NULL)
		if (lbuf[0] == '\n' && stashed) {
			stashed = 0;
			printf("%s", lbuf2);
		} else if (strncmp(lbuf, "From ", 5) == 0 &&
		    (sender == NULL || match(&lbuf[4], sender))) {
			strcpy(lbuf2, lbuf);
			stashed = 1;
		}
	if (stashed)
		printf("%s", lbuf2);
	return(0);
}

match (line, str)
	register char *line, *str;
{
	register char ch;

	while (*line == ' ' || *line == '\t')
		++line;
	if (*line == '\n')
		return (0);
	while (*str && *line != ' ' && *line != '\t' && *line != '\n') {
		ch = isupper(*line) ? tolower(*line) : *line;
		if (ch != *str++)
			return (0);
		line++;
	}
	return (*str == '\0');
}
