/*
 * Listener loop for subsystem library libss.a.
 *
 *	$Header: /afs/dev.mit.edu/source/repository/athena/lib/ss/listen.c,v 1.8 1998-01-20 23:10:27 ghudson Exp $
 *	$Locker:  $
 * 
 * Copyright 1987, 1988 by MIT Student Information Processing Board
 *
 * For copyright information, see copyright.h.
 */

#include "copyright.h"
#include "ss_internal.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

static const char rcsid[] = "$Id: listen.c,v 1.8 1998-01-20 23:10:27 ghudson Exp $";

static ss_data *current_info;
static jmp_buf listen_jmpb;

static void print_prompt()
{
    struct termios termbuf;

    if (tcgetattr(STDIN_FILENO, &termbuf) == 0) {
	termbuf.c_lflag |= ICANON|ISIG|ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &termbuf);
    }
    (void) fputs(current_info->prompt, stdout);
    (void) fflush(stdout);
}

static void listen_int_handler()
{
    putc('\n', stdout);
    longjmp(listen_jmpb, 1);
}

int ss_listen (sci_idx)
    int sci_idx;
{
    register char *cp;
    register ss_data *info;
    char input[BUFSIZ];
    char expanded_input[BUFSIZ];
    char buffer[BUFSIZ];
    char *end = buffer;
    int code;
    jmp_buf old_jmpb;
    ss_data *old_info = current_info;
    struct sigaction isig, csig, nsig, osig;
    sigset_t nmask, omask;

    current_info = info = ss_info(sci_idx);
    info->abort = 0;

    csig.sa_handler = SIG_IGN;
    
    sigemptyset(&nmask);
    sigaddset(&nmask, SIGINT);
    sigprocmask(SIG_BLOCK, &nmask, &omask);

    memmove(old_jmpb, listen_jmpb, sizeof(jmp_buf));

    nsig.sa_handler = listen_int_handler;
    sigemptyset(&nsig.sa_mask);
    nsig.sa_flags = 0;
    sigaction(SIGINT, &nsig, &isig);

    setjmp(listen_jmpb);

    sigprocmask(SIG_SETMASK, &omask, NULL);

    while(!info->abort) {
	print_prompt();
	*end = '\0';

	nsig.sa_handler = listen_int_handler;	/* fgets is not signal-safe */
	osig = csig;
	sigaction(SIGCONT, &nsig, &csig);
	if (csig.sa_handler == listen_int_handler)
	    csig = osig;

	if (fgets(input, BUFSIZ, stdin) != input) {
	    code = SS_ET_EOF;
	    goto egress;
	}
	cp = strchr(input, '\n');
	if (cp) {
	    *cp = '\0';
	    if (cp == input)
		continue;
	}
	sigaction(SIGCONT, &csig, NULL);
	for (end = input; *end; end++)
	    ;

	code = ss_execute_line (sci_idx, input);
	if (code == SS_ET_COMMAND_NOT_FOUND) {
	    register char *c = input;
	    while (*c == ' ' || *c == '\t')
		c++;
	    cp = strchr (c, ' ');
	    if (cp)
		*cp = '\0';
	    cp = strchr (c, '\t');
	    if (cp)
		*cp = '\0';
	    ss_error (sci_idx, 0,
		    "Unknown request \"%s\".  Type \"?\" for a request list.",
		       c);
	}
    }
    code = 0;
egress:
    sigaction(SIGINT, &isig, NULL);
    memmove(listen_jmpb, old_jmpb, sizeof(jmp_buf));
    current_info = old_info;
    return code;
}

void ss_abort_subsystem(sci_idx, code)
    int sci_idx;
{
    ss_info(sci_idx)->abort = 1;
    ss_info(sci_idx)->exit_status = code;
    
}

int ss_quit(argc, argv, sci_idx, infop)
    int argc;
    char **argv;
    int sci_idx;
    void *infop;
{
    ss_abort_subsystem(sci_idx, 0);
}
