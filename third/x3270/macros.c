/*
 * Copyright 1993, 1994, 1995, 1996 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *      macros.c
 *              This module handles string, macro and script (sms) processing.
 */

#include "globals.h"
#include <X11/StringDefs.h>
#include <X11/Xaw/Dialog.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "3270ds.h"
#include "appres.h"
#include "ctlr.h"
#include "screen.h"
#include "resources.h"

#include "actionsc.h"
#include "ctlrc.h"
#include "kybdc.h"
#include "macrosc.h"
#include "mainc.h"
#include "menubarc.h"
#include "popupsc.h"
#include "screenc.h"
#include "statusc.h"
#include "tablesc.h"
#include "trace_dsc.h"
#include "utilc.h"

#define ANSI_SAVE_SIZE	4096

/* Externals */
extern int      linemode;
extern FILE    *tracef;

/* Globals */
struct macro_def *macro_defs = (struct macro_def *)NULL;

/* Statics */
typedef struct sms {
	struct sms *next;	/* next sms on the stack */
	char	msc[1024];	/* input buffer */
	int	msc_len;	/* length of input buffer */
	char   *dptr;		/* data pointer (macros only) */
	enum sms_state {
		SS_IDLE,	/* no command active (scripts only) */
		SS_INCOMPLETE,	/* command(s) buffered and ready to run */
		SS_RUNNING,	/* command executing */
		SS_KBWAIT,	/* command awaiting keyboard unlock */
		SS_CONNECT_WAIT,/* command awaiting connection to complete */
		SS_WAIT,	/* awaiting completion of Wait() */
		SS_EXPECTING,	/* awaiting completion of Expect() */
		SS_CLOSING	/* awaiting completion of Close() */
	} state;
	enum sms_type {
		ST_STRING,	/* string */
		ST_MACRO,	/* macro */
		ST_CHILD,	/* child process */
		ST_PEER		/* peer (external) process */
	} type;
	Boolean	success;
	Boolean	need_prompt;
	Boolean	is_login;
	FILE   *outfile;
	int	infd;
	int	pid;
	XtIntervalId expect_id;
} sms_t;
#define SN	((sms_t *)NULL)
static sms_t *sms = SN;
static int sms_depth = 0;

static struct macro_def *macro_last = (struct macro_def *) NULL;
static XtInputId stdin_id = (XtInputId)NULL;
static unsigned char *ansi_save_buf;
static int      ansi_save_cnt = 0;
static int      ansi_save_ix = 0;
static char    *expect_text = CN;
int		expect_len = 0;
static Boolean	hotwire = False;
static char *st_name[] = { "String", "Macro", "ChildScript", "PeerScript" };
#define ST_sNAME(s)	st_name[(int)(s)->type]
#define ST_NAME		ST_sNAME(sms)

static void     script_prompt();
static void	script_input();

/* Macro that defines that the keyboard is locked due to user input. */
#define KBWAIT	(kybdlock & (KL_OIA_LOCKED|KL_OIA_TWAIT|KL_DEFERRED_UNLOCK))

/* Macro that defines when it's safe to continue a Wait()ing sms. */
#define CAN_PROCEED ( \
    (IN_3270 && formatted && cursor_addr && !KBWAIT) || \
    (IN_ANSI && !(kybdlock & KL_AWAITING_FIRST)) \
)


/* Parse the macros resource into the macro list */
void
macros_init()
{
	char *s = CN;
	char *name, *action;
	struct macro_def *m;
	int ns;
	int ix = 1;
	static char *last_s = CN;

	/* Free the previous macro definitions. */
	while (macro_defs) {
		m = macro_defs->next;
		XtFree((XtPointer)macro_defs);
		macro_defs = m;
	}
	macro_defs = (struct macro_def *)NULL;
	macro_last = (struct macro_def *)NULL;
	if (last_s) {
		XtFree((XtPointer)last_s);
		last_s = CN;
	}

	/* Search for new ones. */
	if (PCONNECTED) {
		char *rname;
		char *space;

		rname = XtNewString(current_host);
		if ((space = strchr(rname, ' ')))
			*space = '\0';
		s = xs_buffer("%s.%s", ResMacros, rname);
		XtFree(rname);
		rname = s;
		s = get_resource(rname);
		XtFree(rname);
	}
	if (!s) {
		if (!(s = appres.macros))
			return;
		s = XtNewString(s);
	}
	last_s = s;

	while ((ns = split_dresource(&s, &name, &action)) == 1) {
		m = (struct macro_def *)XtMalloc(sizeof(*m));
		m->name = name;
		m->action = action;
		if (macro_last)
			macro_last->next = m;
		else
			macro_defs = m;
		m->next = (struct macro_def *)NULL;
		macro_last = m;
		ix++;
	}
	if (ns < 0) {
		char buf[256];

		(void) sprintf(buf, "Error in macro %d", ix);
		XtWarning(buf);
	}
}

/*
 * Enable input from a script.
 */
static void
script_enable()
{
	if (sms->infd >= 0 && stdin_id == (XtInputId)NULL) {
		if (toggled(DS_TRACE))
			(void) fprintf(tracef, "Enabling input for %s[%d]\n",
			    ST_NAME, sms_depth);
		stdin_id = XtAppAddInput(appcontext, sms->infd,
		    (XtPointer)XtInputReadMask,
		    (XtInputCallbackProc)script_input,
		    (XtPointer)sms->infd);
	}
}

/*
 * Disable input from a script.
 */
static void
script_disable()
{
	if (stdin_id != (XtInputId)NULL) {
		if (toggled(DS_TRACE))
			(void) fprintf(tracef, "Disabling input for %s[%d]\n",
			    ST_NAME, sms_depth);
		XtRemoveInput(stdin_id);
		stdin_id = (XtInputId)NULL;
	}
}

/* Allocate a new sms. */
static sms_t *
new_sms(type)
enum sms_type type;
{
	sms_t *s;

	s = (sms_t *)XtCalloc(1, sizeof(sms_t));

	s->state = SS_IDLE;
	s->type = type;
	s->dptr = s->msc;
	s->success = True;
	s->need_prompt = False;
	s->is_login = False;
	s->outfile = (FILE *)NULL;
	s->infd = -1;
	s->pid = -1;
	s->expect_id = (XtIntervalId)NULL;

	return s;
}

/*
 * Push an sms definition on the stack.
 * Returns whether or not that is legal.
 */
static Boolean
sms_push(type)
enum sms_type type;
{
	sms_t *s;

	/* Preempt any running sms. */
	if (sms != SN) {
		/* User-generated nesting is illegal. */
		if (sms->state != SS_RUNNING) {
			hotwire = True;
			popup_an_error("Can't start a second script, macro or string");
			hotwire = False;
			return False;
		}

		/* Remove the running sms's input. */
		script_disable();
	}

	s = new_sms(type);
	if (sms != SN)
		s->is_login = sms->is_login;	/* propagate from parent */
	s->next = sms;
	sms = s;

	/* Enable the abort button on the menu and the status indication. */
	if (++sms_depth == 1) {
		menubar_as_set(True);
		status_script(True);
	}

	if (ansi_save_buf == (unsigned char *)NULL)
		ansi_save_buf = (unsigned char *)XtMalloc(ANSI_SAVE_SIZE);
	return True;
}

/*
 * Add an sms definition to the _bottom_ of the stack.
 */
static sms_t *
sms_enqueue(type)
enum sms_type type;
{
	sms_t *s, *t, *t_prev = SN;

	/* Allocate and initialize a new structure. */
	s = new_sms(type);

	/* Find the bottom of the stack. */
	for (t = sms; t != SN; t = t->next)
		t_prev = t;

	if (t_prev == SN) {	/* Empty stack. */
		s->next = sms;
		sms = s;

		/*
		 * Enable the abort button on the menu and the status
		 * line indication.
		 */
		menubar_as_set(True);
		status_script(True);
	} else {			/* Add to bottom. */
		s->next = SN;
		t_prev->next = s;
	}

	sms_depth++;

	if (ansi_save_buf == (unsigned char *)NULL)
		ansi_save_buf = (unsigned char *)XtMalloc(ANSI_SAVE_SIZE);

	return s;
}

/* Pop an sms definition off the stack. */
static void
sms_pop(can_exit)
Boolean can_exit;
{
	sms_t *s;

	if (toggled(DS_TRACE))
		(void) fprintf(tracef, "%s[%d] complete\n",
		    ST_NAME, sms_depth);

	/* When you pop the peer script, that's the end of x3270. */
	if (sms->type == ST_PEER && can_exit)
		x3270_exit(0);

	/* Remove the input event. */
	script_disable();

	/* Close the files. */
	if (sms->type == ST_CHILD) {
		(void) fclose(sms->outfile);
		(void) close(sms->infd);
	}

	/* Cancel any pending Expect() timeout. */
	if (sms->expect_id != (XtIntervalId)NULL)
		XtRemoveTimeOut(sms->expect_id);

	/* Release the memory. */
	s = sms;
	sms = s->next;
	XtFree((char *)s);
	sms_depth--;

	/*
	 * If this was the last thing on the stack, disable the "Abort Script"
	 * menu option and update the status line.
	 *
	 * Otherwise, see if the new top-of-stack should be blocked because
	 * its child locked the keyboard.
	 */
	/* Update the menu option. */
	if (sms == SN) {
		menubar_as_set(False);
		status_script(False);
	} else if (KBWAIT) {
		sms->state = SS_KBWAIT;
		if (toggled(DS_TRACE))
			(void) fprintf(tracef, "%s[%d] implicitly paused %d\n",
			    ST_NAME, sms_depth, sms->state);
	}
}

/*
 * Peer script initialization.
 *
 * Must be called after the initial call to connect to the host from the
 * command line, so that the initial state can be set properly.
 */
void
peer_script_init()
{
	sms_t *s;
	Boolean on_top;

	if (!appres.scripted)
		return;

	if (sms == SN) {
		/* No login script running, simply push a new sms. */
		(void) sms_push(ST_PEER);
		s = sms;
		on_top = True;
	} else {
		/* Login script already running, pretend we started it. */
		s = sms_enqueue(ST_PEER);
		s->state = SS_RUNNING;
		on_top = False;
	}

	s->infd = fileno(stdin);
	s->outfile = stdout;
	(void) SETLINEBUF(s->outfile);	/* even if it's a pipe */

	if (on_top) {
		if (HALF_CONNECTED ||
		    (CONNECTED && (kybdlock & KL_AWAITING_FIRST)))
			s->state = SS_CONNECT_WAIT;
		else
			script_enable();
	}
}


/*
 * Interpret and execute a script or macro command.
 */
enum em_stat { EM_CONTINUE, EM_PAUSE, EM_ERROR };
static enum em_stat
execute_command(cause, s, np)
enum iaction cause;
char *s;
char **np;
{
	enum {
		ME_GND,		/* before action name */
		ME_FUNCTION,	/* within action name */
		ME_FUNCTIONx,	/* saw whitespace after action name */
		ME_LPAREN,	/* saw left paren */
		ME_PARM,	/* within unquoted parameter */
		ME_QPARM,	/* within quoted parameter */
		ME_BSL,		/* after backslash in quoted parameter */
		ME_PARMx	/* saw whitespace after parameter */
	} state = ME_GND;
	char c;
	char aname[64+1];
	char parm[1024+1];
	int nx;
	Cardinal count = 0;
	String params[64];
	int i;
	int failreason = 0;
	static char *fail_text[] = {
		/*1*/ "Action name must begin with an alphanumeric character",
		/*2*/ "Syntax error, \"(\" expected",
		/*3*/ "Syntax error, \")\" expected",
		/*4*/ "Extra data after parameters"
	};
#define fail(n) { failreason = n; goto failure; }

	parm[0] = '\0';
	params[count] = parm;

	while ((c = *s++)) switch (state) {
	    case ME_GND:
		if (isspace(c))
			continue;
		else if (isalnum(c)) {
			state = ME_FUNCTION;
			nx = 0;
			aname[nx++] = c;
		} else
			fail(1);
		break;
	    case ME_FUNCTION:
		if (c == '(' || isspace(c)) {
			aname[nx] = '\0';
			if (c == '(') {
				nx = 0;
				state = ME_LPAREN;
			} else
				state = ME_FUNCTIONx;
		} else if (isalnum(c)) {
			if (nx < 64)
				aname[nx++] = c;
		} else
			fail(2);
		break;
	    case ME_FUNCTIONx:
		if (isspace(c))
			continue;
		else if (c == '(') {
			nx = 0;
			state = ME_LPAREN;
		} else
			fail(2);
		break;
	    case ME_LPAREN:
		if (isspace(c))
			continue;
		else if (c == '"')
			state = ME_QPARM;
		else if (c == ',') {
			parm[nx++] = '\0';
			params[++count] = &parm[nx];
		} else if (c == ')')
			goto success;
		else {
			state = ME_PARM;
			parm[nx++] = c;
		}
		break;
	    case ME_PARM:
		if (isspace(c)) {
			parm[nx++] = '\0';
			params[++count] = &parm[nx];
			state = ME_PARMx;
		} else if (c == ')') {
			parm[nx] = '\0';
			++count;
			goto success;
		} else if (c == ',') {
			parm[nx++] = '\0';
			params[++count] = &parm[nx];
			state = ME_LPAREN;
		} else {
			if (nx < 1024)
				parm[nx++] = c;
		}
		break;
	    case ME_BSL:
		if (c == 'n' && nx < 1024)
			parm[nx++] = '\n';
		else {
			if (c != '"' && nx < 1024)
				parm[nx++] = '\\';
			if (nx < 1024)
				parm[nx++] = c;
		}
		state = ME_QPARM;
		break;
	    case ME_QPARM:
		if (c == '"') {
			parm[nx++] = '\0';
			params[++count] = &parm[nx];
			state = ME_PARMx;
		} else if (c == '\\') {
			state = ME_BSL;
		} else if (nx < 1024)
			parm[nx++] = c;
		break;
	    case ME_PARMx:
		if (isspace(c))
			continue;
		else if (c == ',')
			state = ME_LPAREN;
		else if (c == ')')
			goto success;
		else
			fail(3);
		break;
	}

	/* Terminal state. */
	switch (state) {
	    case ME_FUNCTION:
		aname[nx] = '\0';
		break;
	    case ME_FUNCTIONx:
		break;
	    case ME_GND:
		if (np)
			*np = s - 1;
		return EM_CONTINUE;
	    default:
		fail(3);
	}

    success:
	if (c) {
		while (*s && isspace(*s))
			s++;
		if (*s) {
			if (np)
				*np = s;
			else
				fail(4);
		} else if (np)
			*np = s;
	} else if (np)
		*np = s-1;

	/* If it's a macro, do variable substitutions. */
	if (cause == IA_MACRO) {
		int j;

		for (j = 0; j < count; j++)
			params[j] = do_subst(params[j], True, False);
	}

	/* Search the action list. */
	for (i = 0; i < actioncount; i++) {
		if (!strcmp(aname, actions[i].string)) {
			ia_cause = cause;
			(*actions[i].proc)((Widget)NULL, (XEvent *)NULL,
				count ? params : (String *)NULL,
				&count);
			screen_disp();
			break;
		}
	}

	/* If it's a macro, undo variable substitutions. */
	if (cause == IA_MACRO) {
		int j;

		for (j = 0; j < count; j++)
			XtFree(params[j]);
	}

	if (i >= actioncount) {
		popup_an_error("Unknown action: %s", aname);
		return EM_ERROR;
	}

	if (KBWAIT)
		return EM_PAUSE;
	else
		return EM_CONTINUE;

    failure:
	popup_an_error(fail_text[failreason-1]);
	return EM_ERROR;
#undef fail
}

/* Run the string at the top of the stack. */
static void
run_string()
{
	int len;
	int len_left;

	if (toggled(DS_TRACE))
		(void) fprintf(tracef, "%s[%d] running\n",
		    ST_NAME, sms_depth);

	sms->state = SS_RUNNING;
	len = strlen(sms->dptr);
	if (toggled(DS_TRACE))
		(void) fprintf(tracef, "String[%d]: '%s'\n", sms_depth,
		    sms->dptr);
	if ((len_left = emulate_input(sms->dptr, len, False))) {
		sms->dptr += len - len_left;
		if (KBWAIT) {
			sms->state = SS_KBWAIT;
			if (toggled(DS_TRACE))
				(void) fprintf(tracef, "%s[%d] paused %d\n",
				    ST_NAME, sms_depth, sms->state);
		}
	} else {
		sms_pop(False);
	}
}

/* Run the macro at the top of the stack. */

static void
run_macro()
{
	char *a = sms->dptr;
	char *nextm;
	enum em_stat es;
	sms_t *s;

	if (toggled(DS_TRACE))
		(void) fprintf(tracef, "%s[%d] running\n",
		    ST_NAME, sms_depth);

	/*
	 * Keep executing commands off the line until one pauses or
	 * we run out of commands.
	 */
	while (*a) {
		/*
		 * Check for command failure.
		 */
		if (!sms->success) {
			if (toggled(DS_TRACE))
				(void) fprintf(tracef, "Macro[%d] failed\n",
				    sms_depth);
			/* Propogate it. */
			if (sms->next != SN)
				sms->next->success = False;
			break;
		}

		sms->state = SS_RUNNING;
		if (toggled(DS_TRACE))
			(void) fprintf(tracef, "Macro[%d]: '%s'\n",
			    sms_depth, a);
		s = sms;
		s->success = True;
		es = execute_command(IA_MACRO, a, &nextm);
		s->dptr = nextm;

		/*
		 * If a new sms was started, we will be resumed
		 * when it completes.
		 */
		if (sms != s) {
			return;
		}

		/* Macro could not execute.  Abort it. */
		if (es == EM_ERROR) {
			if (toggled(DS_TRACE))
				(void) fprintf(tracef, "Macro[%d] error\n",
				    sms_depth);
			/* Propogate it. */
			if (sms->next != SN)
				sms->next->success = False;
			break;
		}

		/* Macro paused, implicitly or explicitly.  Suspend it. */
		if (es == EM_PAUSE || (int)sms->state >= (int)SS_KBWAIT) {
			if (sms->state == SS_RUNNING)
				sms->state = SS_KBWAIT;
			if (toggled(DS_TRACE))
				(void) fprintf(tracef, "%s[%d] paused %d\n",
				    ST_NAME, sms_depth, sms->state);
			sms->dptr = nextm;
			return;
		}

		/* Macro ran. */
		a = nextm;
	}

	/* Finished with this macro. */
	sms_pop(False);
}

/* Push a macro on the stack. */
static void
push_macro(s, is_login)
char *s;
Boolean is_login;
{
	if (!sms_push(ST_MACRO))
		return;
	(void) strncpy(sms->msc, s, 1023);
	sms->msc[1023] = '\0';
	sms->msc_len = strlen(s);
	if (sms->msc_len > 1023)
		sms->msc_len = 1023;
	if (is_login) {
		sms->state = SS_WAIT;
		sms->is_login = True;
	} else
		sms->state = SS_INCOMPLETE;
	if (sms_depth == 1)
		sms_continue();
}

/* Push a string on the stack. */
static void
push_string(s, is_login)
char *s;
Boolean is_login;
{
	if (!sms_push(ST_STRING))
		return;
	(void) strncpy(sms->msc, s, 1023);
	sms->msc[1023] = '\0';
	sms->msc_len = strlen(s);
	if (sms->msc_len > 1023)
		sms->msc_len = 1023;
	if (is_login) {
		sms->state = SS_WAIT;
		sms->is_login = True;
	} else
		sms->state = SS_INCOMPLETE;
	if (sms_depth == 1)
		sms_continue();
}

/* Set a pending string. */
void
ps_set(s)
char *s;
{
	push_string(s, False);
}

/* Callback for macros menu. */
void
macro_command(m)
struct macro_def *m;
{
	push_macro(m->action, False);
}

/*
 * If the string looks like an action, e.g., starts with "Xxx(", run a login
 * macro.  Otherwise, set a simple pending login string.
 */
void
login_macro(s)
char *s;
{
	char *t = s;
	Boolean looks_right = False;

	while (isspace(*t))
		t++;
	if (isalnum(*t)) {
		while (isalnum(*t))
			t++;
		while (isspace(*t))
			t++;
		if (*t == '(')
			looks_right = True;
	}

	if (looks_right)
		push_macro(s, True);
	else
		push_string(s, True);
}

/* Run the first command in the msc[] buffer. */
static void
run_script()
{
	if (toggled(DS_TRACE))
		(void) fprintf(tracef, "%s[%d] running\n",
		    ST_NAME, sms_depth);

	for (;;) {
		char *ptr;
		int cmd_len;
		char *cmd;
		sms_t *s;
		enum em_stat es;

		/* If the script isn't idle, we're done. */
		if (sms->state != SS_IDLE)
			break;

		/* If a prompt is required, send one. */
		if (sms->need_prompt) {
			script_prompt(sms->success);
			sms->need_prompt = False;
		}

		/* If there isn't a pending command, we're done. */
		if (!sms->msc_len)
			break;

		/* Isolate the command. */
		ptr = memchr(sms->msc, '\n', sms->msc_len);
		if (!ptr)
			break;
		*ptr++ = '\0';
		cmd_len = ptr - sms->msc;
		cmd = sms->msc;

		/* Execute it. */
		sms->state = SS_RUNNING;
		sms->success = True;
		if (toggled(DS_TRACE))
			(void) fprintf(tracef, "%s[%d]: '%s'\n", ST_NAME,
			    sms_depth, cmd);
		s = sms;
		es = execute_command(IA_SCRIPT, cmd, (char **)NULL);

		/* Move the rest of the buffer over. */
		if (cmd_len < s->msc_len) {
			s->msc_len -= cmd_len;
			MEMORY_MOVE(s->msc, ptr, s->msc_len);
		} else
			s->msc_len = 0;

		/*
		 * If a new sms was started, we will be resumed
		 * when it completes.
		 */
		if (sms != s) {
			s->need_prompt = True;
			return;
		}

		/* Handle what it did. */
		if (es == EM_PAUSE || (int)sms->state >= (int)SS_KBWAIT) {
			if (sms->state == SS_RUNNING)
				sms->state = SS_KBWAIT;
			script_disable();
			if (sms->state == SS_CLOSING) {
				sms_pop(False);
				return;
			}
			sms->need_prompt = True;
		} else if (es == EM_ERROR) {
			if (toggled(DS_TRACE))
				(void) fprintf(tracef, "%s[%d] error\n",
				    ST_NAME, sms_depth);
			script_prompt(False);
		} else
			script_prompt(sms->success);
		if (sms->state == SS_RUNNING)
			sms->state = SS_IDLE;
		else {
			if (toggled(DS_TRACE))
				(void) fprintf(tracef, "%s[%d] paused %d\n",
				    ST_NAME, sms_depth, sms->state);
		}
	}
}

/* Handle an error generated during the execution of a script or macro. */
void
sms_error(msg)
char *msg;
{
	/* Print the error message. */
	(void) fprintf(stderr, "%s\n", msg);

	/* Fail the command. */
	sms->success = False;

	/* Cancel any login. */
	if (sms->is_login)
		x_disconnect(True);
}

/* Process available input from a script. */
/*ARGSUSED*/
static void
script_input(fd)
XtPointer fd;
{
	char buf[128];
	int nr;
	char *ptr;
	char c;

	if (toggled(DS_TRACE))
		(void) fprintf(tracef, "Input for %s[%d] %d\n",
		    ST_NAME, sms_depth, sms->state);

	/* Read in what you can. */
	nr = read((int)fd, buf, sizeof(buf));
	if (nr < 0) {
		popup_an_errno(errno, "%s[%d] read", ST_NAME, sms_depth);
		return;
	}
	if (nr == 0) {	/* end of file */
		sms_pop(True);
		sms_continue();
		return;
	}

	/* Append to the pending command, ignoring returns. */
	ptr = buf;
	while (nr--)
		if ((c = *ptr++) != '\r')
			sms->msc[sms->msc_len++] = c;

	/* Run the command(s). */
	sms->state = SS_INCOMPLETE;
	sms_continue();
}

/* Resume a paused sms, if conditions are now ripe. */
void
sms_continue()
{
	while (True) {
		if (sms == SN)
			return;

		switch (sms->state) {

		    case SS_IDLE:
			return;		/* nothing to do */

		    case SS_INCOMPLETE:
		    case SS_RUNNING:
			break;		/* let it proceed */

		    case SS_KBWAIT:
			if (KBWAIT)
				return;
			break;

		    case SS_WAIT:
			if (!CAN_PROCEED)
				return;
			/* fall through... */
		    case SS_CONNECT_WAIT:
			if (HALF_CONNECTED ||
			    (CONNECTED && (kybdlock & KL_AWAITING_FIRST)))
				return;
			break;

		    case SS_EXPECTING:
			return;

		    case SS_CLOSING:
			return;	/* can't happen, I hope */

		}

		/* Restart the sms. */

		sms->state = SS_IDLE;

		switch (sms->type) {
		    case ST_STRING:
			run_string();
			break;
		    case ST_MACRO:
			run_macro();
			break;
		    case ST_PEER:
		    case ST_CHILD:
			script_enable();
			run_script();
			break;
		}
	}
}

/*
 * Macro- and script-specific actions.
 */

static void
dump_range(first, len, in_ascii)
int first;
int len;
Boolean in_ascii;
{
	register int i;
	Boolean any = False;

	for (i = 0; i < len; i++) {
		unsigned char c;

		if (i && !((first + i) % COLS)) {
			(void) putc('\n', sms->outfile);
			any = False;
		}
		if (!any) {
			(void) fprintf(sms->outfile, "data: ");
			any = True;
		}
		if (in_ascii) {
			c = cg2asc[screen_buf[first + i]];
			(void) fprintf(sms->outfile, "%c", c ? c : ' ');
		} else {
			(void) fprintf(sms->outfile, "%s%02x",
				i ? " " : "",
				cg2ebc[screen_buf[first + i]]);
		}
	}
	(void) fprintf(sms->outfile, "\n");
}

static void
dump_fixed(params, count, name, in_ascii)
String params[];
Cardinal count;
char *name;
Boolean in_ascii;
{
	int row, col, len, rows, cols;

	switch (count) {
	    case 0:	/* everything */
		row = 0;
		col = 0;
		len = ROWS*COLS;
		break;
	    case 1:	/* from cursor, for n */
		row = cursor_addr / COLS;
		col = cursor_addr % COLS;
		len = atoi(params[0]);
		break;
	    case 3:	/* from (row,col), for n */
		row = atoi(params[0]);
		col = atoi(params[1]);
		len = atoi(params[2]);
		break;
	    case 4:	/* from (row,col), for rows x cols */
		row = atoi(params[0]);
		col = atoi(params[1]);
		rows = atoi(params[2]);
		cols = atoi(params[3]);
		len = 0;
		break;
	    default:
		popup_an_error("%s requires 0, 1, 3 or 4 arguments", name);
		return;
	}

	if (
	    (row < 0 || row > ROWS || col < 0 || col > COLS || len < 0) ||
	    ((count < 4)  && ((row * COLS) + col + len > ROWS * COLS)) ||
	    ((count == 4) && (cols < 0 || rows < 0 ||
			      col + cols > COLS || row + rows > ROWS))
	   ) {
		popup_an_error("%s: Invalid argument", name);
		return;
	}
	if (count < 4)
		dump_range((row * COLS) + col, len, in_ascii);
	else {
		int i;

		for (i = 0; i < rows; i++)
			dump_range(((row+i) * COLS) + col, cols, in_ascii);
	}
}

static void
dump_field(count, name, in_ascii)
Cardinal count;
char *name;
Boolean in_ascii;
{
	unsigned char *fa;
	int start, baddr;
	int len = 0;

	if (count != 0) {
		popup_an_error("%s requires 0 arguments", name);
		return;
	}
	if (!formatted) {
		popup_an_error("%s: Screen is not formatted", name);
		return;
	}
	fa = get_field_attribute(cursor_addr);
	start = fa - screen_buf;
	INC_BA(start);
	baddr = start;
	do {
		if (IS_FA(screen_buf[baddr]))
			break;
		len++;
		INC_BA(baddr);
	} while (baddr != start);
	dump_range(start, len, in_ascii);
}

/*ARGSUSED*/
void
Ascii_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	dump_fixed(params, *num_params, action_name(Ascii_action), True);
}

/*ARGSUSED*/
void
AsciiField_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	dump_field(*num_params, action_name(AsciiField_action), True);
}

/*ARGSUSED*/
void
Ebcdic_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	dump_fixed(params, *num_params, action_name(Ebcdic_action), False);
}

/*ARGSUSED*/
void
EbcdicField_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	dump_field(*num_params, action_name(EbcdicField_action), False);
}

/*
 * The sms prompt is preceeded by a status line with 12 fields:
 *
 *  1 keyboard status
 *     U unlocked
 *     L locked, waiting for host response
 *     E locked, keying error
 *  2 formatting status of screen
 *     F formatted
 *     U unformatted
 *  3 protection status of current field
 *     U unprotected (modifiable)
 *     P protected
 *  4 connect status
 *     N not connected
 *     C(host) connected
 *  5 emulator mode
 *     N not connected
 *     C connected in ANSI character mode
 *     L connected in ANSI line mode
 *     P 3270 negotiation pending
 *     I connected in 3270 mode
 *  7 model number
 *  8 max rows
 *  9 max cols
 * 10 cursor row
 * 11 cursor col
 * 12 main window id
 */

/*ARGSUSED*/
static void
script_prompt(success)
Boolean success;
{
	char kb_stat;
	char fmt_stat;
	char prot_stat;
	char *connect_stat;
	Boolean free_connect = False;
	char em_mode;

	if (!kybdlock)
		kb_stat = 'U';
	else if (!CONNECTED || KBWAIT)
		kb_stat = 'L';
	else
		kb_stat = 'E';

	if (formatted)
		fmt_stat = 'F';
	else
		fmt_stat = 'U';

	if (!formatted)
		prot_stat = 'U';
	else {
		unsigned char *fa;

		fa = get_field_attribute(cursor_addr);
		if (FA_IS_PROTECTED(*fa))
			prot_stat = 'P';
		else
			prot_stat = 'U';
	}

	if (CONNECTED) {
		connect_stat = xs_buffer("C(%s)", current_host);
		free_connect = True;
	} else
		connect_stat = "N";

	if (CONNECTED) {
		if (IN_ANSI) {
			if (linemode)
				em_mode = 'L';
			else
				em_mode = 'C';
		} else if (IN_3270)
			em_mode = 'I';
		else
			em_mode = 'P';
	} else
		em_mode = 'N';

	(void) fprintf(sms->outfile,
	    "%c %c %c %s %c %d %d %d %d %d 0x%lx\n%s\n",
	    kb_stat,
	    fmt_stat,
	    prot_stat,
	    connect_stat,
	    em_mode,
	    model_num,
	    ROWS, COLS,
	    cursor_addr / COLS, cursor_addr % COLS,
	    XtWindow(toplevel),
	    success ? "ok" : "error");

	if (free_connect)
		XtFree(connect_stat);

	(void) fflush(sms->outfile);
}

/*
 * Wait for the host to let us write onto the screen.
 */
/*ARGSUSED*/
void
Wait_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	if (check_usage(Wait_action, *num_params, 0, 0) < 0)
		return;
	if (sms == SN || sms->state != SS_RUNNING) {
		popup_an_error("%s: can only be called from scripts",
		    action_name(Wait_action));
		return;
	}
	if (!(CONNECTED || HALF_CONNECTED)) {
		popup_an_error("%s: not connected", action_name(Wait_action));
		return;
	}

	/* Is it already okay? */
	if (CAN_PROCEED)
		return;

	/* No, wait for it to happen. */
	sms->state = SS_WAIT;
}

/*
 * Callback from Connect() and Reconnect() actions, to minimally pause a
 * running sms.
 */
void
sms_connect_wait()
{
	if (sms != SN &&
	    (int)sms->state >= (int)SS_RUNNING &&
	    sms->state != SS_WAIT) {
		if (HALF_CONNECTED ||
		    (CONNECTED && (kybdlock & KL_AWAITING_FIRST)))
			sms->state = SS_CONNECT_WAIT;
	}
}

/* Return whether error pop-ups should be short-circuited. */
Boolean
sms_redirect()
{
	return !hotwire &&
	       sms != SN &&
	       (sms->type == ST_CHILD || sms->type == ST_PEER);
}

/* Return whether any scripts are active. */
Boolean
sms_active()
{
	return sms != SN;
}

/* Translate an expect string (uses C escape syntax). */
static void
expand_expect(s)
char *s;
{
	char *t = XtMalloc(strlen(s) + 1);
	char c;
	enum { XS_BASE, XS_BS, XS_O, XS_X } state = XS_BASE;
	int n;
	int nd;
	static char hexes[] = "0123456789abcdef";

	expect_text = t;

	while ((c = *s++)) {
		switch (state) {
		    case XS_BASE:
			if (c == '\\')
				state = XS_BS;
			else
				*t++ = c;
			break;
		    case XS_BS:
			switch (c) {
			    case 'x':
				nd = 0;
				n = 0;
				state = XS_X;
				break;
			    case 'r':
				*t++ = '\r';
				state = XS_BASE;
				break;
			    case 'n':
				*t++ = '\n';
				state = XS_BASE;
				break;
			    case 'b':
				*t++ = '\b';
				state = XS_BASE;
				break;
			    default:
				if (c >= '0' && c <= '7') {
					nd = 1;
					n = c - '0';
					state = XS_O;
				} else {
					*t++ = c;
					state = XS_BASE;
				}
				break;
			}
			break;
		    case XS_O:
			if (nd < 3 && c >= '0' && c <= '7') {
				n = (n * 8) + (c - '0');
				nd++;
			} else {
				*t++ = n;
				*t++ = c;
				state = XS_BASE;
			}
			break;
		    case XS_X:
			if (isxdigit(c)) {
				n = (n * 16) +
				    strchr(hexes, tolower(c)) - hexes;
				nd++;
			} else {
				if (nd) 
					*t++ = n;
				else
					*t++ = 'x';
				*t++ = c;
				state = XS_BASE;
			}
			break;
		}
	}
	expect_len = t - expect_text;
}

/* 'mem' version of strstr */
static char *
memstr(s1, s2, n1, n2)
char *s1;
char *s2;
int n1;
int n2;
{
	int i;

	for (i = 0; i <= n1 - n2; i++, s1++)
		if (*s1 == *s2 && !memcmp(s1, s2, n2))
			return s1;
	return CN;
}

/* Check for a match against an expect string. */
static Boolean
expect_matches()
{
	int ix, i;
	unsigned char buf[ANSI_SAVE_SIZE];
	char *t;

	ix = (ansi_save_ix + ANSI_SAVE_SIZE - ansi_save_cnt) % ANSI_SAVE_SIZE;
	for (i = 0; i < ansi_save_cnt; i++) {
		buf[i] = ansi_save_buf[(ix + i) % ANSI_SAVE_SIZE];
	}
	t = memstr((char *)buf, expect_text, ansi_save_cnt, expect_len);
	if (t != CN) {
		ansi_save_cnt -= ((unsigned char *)t - buf) + expect_len;
		XtFree(expect_text);
		expect_text = CN;
		return True;
	} else
		return False;
}

/* Store an ANSI character for use by the Ansi action. */
void
sms_store(c)
unsigned char c;
{
	if (sms == SN)
		return;

	/* Save the character in the buffer. */
	ansi_save_buf[ansi_save_ix++] = c;
	ansi_save_ix %= ANSI_SAVE_SIZE;
	if (ansi_save_cnt < ANSI_SAVE_SIZE)
		ansi_save_cnt++;

	/* If a script or macro is waiting to match a string, check now. */
	if (sms->state == SS_EXPECTING && expect_matches()) {
		XtRemoveTimeOut(sms->expect_id);
		sms->expect_id = (XtIntervalId)NULL;
		sms->state = SS_INCOMPLETE;
		sms_continue();
	}
}

/* Dump whatever ANSI data has been sent by the host since last called. */
/*ARGSUSED*/
void
AnsiText_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	register int i;
	int ix;
	unsigned char c;

	if (!ansi_save_cnt)
		return;
	(void) fprintf(sms->outfile, "data: ");
	ix = (ansi_save_ix + ANSI_SAVE_SIZE - ansi_save_cnt) % ANSI_SAVE_SIZE;
	for (i = 0; i < ansi_save_cnt; i++) {
		c = ansi_save_buf[(ix + i) % ANSI_SAVE_SIZE];
		if (!(c & ~0x1f)) switch (c) {
		    case '\n':
			(void) fprintf(sms->outfile, "\\n");
			break;
		    case '\r':
			(void) fprintf(sms->outfile, "\\r");
			break;
		    case '\b':
			(void) fprintf(sms->outfile, "\\b");
			break;
		    default:
			(void) fprintf(sms->outfile, "\\%03o", c);
			break;
		} else if (c == '\\')
			(void) fprintf(sms->outfile, "\\\\");
		else
			(void) putc((char)c, sms->outfile);
	}
	(void) putc('\n', sms->outfile);
	ansi_save_cnt = 0;
	ansi_save_ix = 0;
}

/* Stop listening to stdin. */
/*ARGSUSED*/
void
CloseScript_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	if (sms != SN &&
	    (sms->type == ST_PEER || sms->type == ST_CHILD)) {

		/* Close this script. */
		sms->state = SS_CLOSING;
		script_prompt(True);

		/* If nonzero status passed, fail the calling script. */
		if (*num_params > 0 &&
		    atoi(params[0]) != 0 &&
		    sms->next != SN) {
			sms->next->success = False;
			if (sms->is_login)
				x_disconnect(True);
		}
	} else
		popup_an_error("%s can only be called from a script",
		    action_name(CloseScript_action));
}

/* Execute an arbitrary shell command. */
/*ARGSUSED*/
void
Execute_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	if (check_usage(Execute_action, *num_params, 1, 1) < 0)
		return;
	(void) system(params[0]);
}

/* Timeout for Expect action. */
/*ARGSUSED*/
static void
expect_timed_out(closure, id)
XtPointer closure;
XtIntervalId *id;
{
	if (sms == SN || sms->state != SS_EXPECTING)
		return;

	XtFree(expect_text);
	expect_text = CN;
	popup_an_error("%s(): Timed out", action_name(Expect_action));
	sms->expect_id = (XtIntervalId)NULL;
	sms->state = SS_INCOMPLETE;
	sms->success = False;
	if (sms->is_login)
		x_disconnect(True);
	sms_continue();
}

/* Wait for a string from the host (ANSI mode only). */
/*ARGSUSED*/
void
Expect_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int tmo;

	/* Verify the environment and parameters. */
	if (sms == SN || sms->state != SS_RUNNING) {
		popup_an_error("%s can only be called from a script or macro",
		    action_name(Expect_action));
		return;
	}
	if (check_usage(Expect_action, *num_params, 1, 2) < 0)
		return;
	if (!IN_ANSI) {
		popup_an_error("%s() is valid only when connected in ANSI mode",
		    action_name(Expect_action));
	}
	if (*num_params == 2) {
		tmo = atoi(params[1]);
		if (tmo < 1 || tmo > 600) {
			popup_an_error("%s(): Invalid timeout: %s",
			    action_name(Expect_action), params[1]);
			return;
		}
	} else
		tmo = 30;

	/* See if the text is there already; if not, wait for it. */
	expand_expect(params[0]);
	if (!expect_matches()) {
		sms->expect_id = XtAppAddTimeOut(appcontext, tmo * 1000,
		    expect_timed_out, 0);
		sms->state = SS_EXPECTING;
	}
	/* else allow sms to proceed */
}


/* "Execute an Action" menu option */

static Widget execute_action_shell = (Widget)NULL;

/* Callback for "OK" button on execute action popup */
/*ARGSUSED*/
static void
execute_action_callback(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	char *text;

	text = XawDialogGetValueString((Widget)client_data);
	XtPopdown(execute_action_shell);
	if (!text)
		return;
	push_macro(text, False);
}

/*ARGSUSED*/
void
execute_action_option(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (execute_action_shell == NULL)
		execute_action_shell = create_form_popup("ExecuteAction",
		    execute_action_callback, (XtCallbackProc)NULL, False);

	popup_popup(execute_action_shell, XtGrabExclusive);
}

/* "Script" action, runs a script as a child process. */
/*ARGSUSED*/
void
Script_action(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
	int inpipe[2];
	int outpipe[2];
	extern int children;

	if (*num_params < 1) {
		popup_an_error("%s() requires at least one argument",
		    action_name(Script_action));
		return;
	}

	/* Create a new script description. */
	if (!sms_push(ST_CHILD))
		return;

	/*
	 * Create pipes and stdout stream for the script process.
	 *  inpipe[] is read by x3270, written by the script
	 *  outpipe[] is written by x3270, read by the script
	 */
	if (pipe(inpipe) < 0) {
		sms_pop(False);
		popup_an_error("pipe() failed");
		return;
	}
	if (pipe(outpipe) < 0) {
		(void) close(inpipe[0]);
		(void) close(inpipe[1]);
		sms_pop(False);
		popup_an_error("pipe() failed");
		return;
	}
	if ((sms->outfile = fdopen(outpipe[1], "w")) == (FILE *)NULL) {
		(void) close(inpipe[0]);
		(void) close(inpipe[1]);
		(void) close(outpipe[0]);
		(void) close(outpipe[1]);
		sms_pop(False);
		popup_an_error("fdopen() failed");
		return;
	}
	(void) SETLINEBUF(sms->outfile);

	/* Fork and exec the script process. */
	if ((sms->pid = fork()) < 0) {
		(void) close(inpipe[0]);
		(void) close(inpipe[1]);
		(void) close(outpipe[0]);
		sms_pop(False);
		popup_an_error("fork() failed");
		return;
	}

	/* Child processing. */
	if (sms->pid == 0) {
		char **argv;
		int i;
		char env_buf[2][32];

		/* Clean up the pipes. */
		(void) close(outpipe[1]);
		(void) close(inpipe[0]);

		/* Export the names of the pipes into the environment. */
		(void) sprintf(env_buf[0], "X3270OUTPUT=%d", outpipe[0]);
		(void) putenv(env_buf[0]);
		(void) sprintf(env_buf[1], "X3270INPUT=%d", inpipe[1]);
		(void) putenv(env_buf[1]);

		/* Set up arguments. */
		argv = (char **)XtMalloc((*num_params + 1) * sizeof(char *));
		for (i = 0; i < *num_params; i++)
			argv[i] = params[i];
		argv[i] = CN;

		/* Exec. */
		(void) execvp(params[0], argv);
		(void) fprintf(stderr, "exec(%s) failed\n", params[0]);
		(void) _exit(1);
	}

	/* Clean up our ends of the pipes. */
	sms->infd = inpipe[0];
	(void) close(inpipe[1]);
	(void) close(outpipe[0]);

	/* Enable input. */
	script_enable();

	/* Set up to reap the child's exit status. */
	++children;
}

/* Abort all running scripts. */
void
abort_script()
{
	while (sms != SN) {
		if (sms->type == ST_CHILD && sms->pid > 0)
			(void) kill(sms->pid, SIGTERM);
		sms_pop(True);
	}
}

/* Abort all login scripts. */
void
sms_cancel_login()
{
	while (sms != SN && sms->is_login) {
		if (sms->type == ST_CHILD && sms->pid > 0)
			(void) kill(sms->pid, SIGTERM);
		sms_pop(False);
	}
}
