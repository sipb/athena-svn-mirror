#include <sgtty.h>
#include <sys/file.h>
#include <stdio.h>

main(argc, argv)
char *argv[];
{
    char *login;
    char *host;
    char *accfile;
    register int eop = 0;
    int n;
    unsigned char buf[2048];

    parse_args(argc, argv, &login, &host, &accfile);
    setup_output_modes();
    n = fread(buf, 1, 2048, stdin);
    if (is_graphics_file(buf, n)) 
	{
	    while (n > 0) 
		{
		    eop = find_eop(buf, &n);
		    fwrite(buf, 1, n, stdout);
		    if (eop) break;
		    n = fread(buf, 1, 2048, stdin);
		}
	}
    else
	{
	    output_lossage_sheet(login, host);
	}
    fflush(stdout);
/*    do_accounting(accfile, login, host, eop); */ /* Lossage: see below */
    exit(0);
}

/*
 * fix output file descriptor so that it has the right modes. 
 * in particular, if it is in raw mode, flow control won't work
 */

setup_output_modes()
{
  struct sgttyb tty_buf;
  ioctl(fileno(stdout), TIOCGETP, &tty_buf);
  tty_buf.sg_flags &= ~RAW;
  ioctl(fileno(stdout), TIOCSETP, &tty_buf);
}


parse_args(argc, argv, login, host, accfile)
int argc;
register char *argv[];
char **login;
char **host;
char **accfile;
{
    argv++;
    while (argv[0][0] == '-' && argv[0][1] != 'n') argv++;
    argv++;
    *login = *argv++;
    argv++;
    *host = *argv++;
    *accfile = *argv;
}


is_graphics_file(buf, n) 
unsigned char *buf;
register int n;
{
    register unsigned char *bp;
    register int i;
    
    bp = buf;
    for (i = 0; i < n; i++) 
	{
	    if (bp[i] == 27 && bp[++i] == 'P' && bp[i+6] == 'q') return(1);
	}
    return(0);
}

find_eop(buf, n)
unsigned char *buf;
int *n;
{
    register int i = *n;
    register unsigned char *bp = buf;
    
    while (--i >= 0) if (*bp++ == '\f') {
	*bp++ = '\033';
	*bp++ = 'c';
	*n = bp - buf;
	return (1);
    }
    return(0);
}

#define IDENT "\033c\033[20d\033[30`%s:%s\n"
#define TIME "\033[25d\033[27`%s\n"
#define LOSSAGE "\033[30d\033[20`THIS PRINTER IS FOR GRAPHICS OUTPUT ONLY\f"

output_lossage_sheet(login, host)
char *login;
char *host;
{
    char *timestamp;
    int atime;
    char *ctime();
    
    atime = time(0);
    timestamp = ctime(&atime);
    printf(IDENT, host, login);
    printf(TIME, timestamp);
    printf(LOSSAGE);
}

/*
** I don't know what is wrong here.  The idea is to open accfile for append
** and write the accounting information into it.  The accfile is specified
** on the command line; lpd gets it from the "af" printcap entry.  For some
** reason the file never gets created.  I tried to create another file
** explicitly.  This file gets created, but nothing gets written into it.
** I've tried both stdio and not stdio.  I give up.
*/

do_accounting(accfile, login, host, pages)
char *accfile;
char *login;
char *host;
int pages;
{
    char acct[64];
    register FILE *f;
    register int ft;
    register int n;
    register int status;

    ft = open("/tmp/ln03acct", O_WRONLY|O_CREAT|O_APPEND, 0770);
    if (ft < 0) {
	perror(open);
	exit(2);
    }
    f = fopen(accfile, "a");
    if (f == NULL) {
	perror("fopen");
	exit(2);
    }
    sprintf(acct, "%-12s  %-8s  %d\n", host, login, pages);
    n = strlen(acct);
    status = fwrite(acct, 1, n, f);
    if (status == 0) perror("fwrite");
    status = write(ft, acct, n);
    if (status < 0) perror("write");
    close(ft);
    fclose(f);
    fflush(stderr);
}
