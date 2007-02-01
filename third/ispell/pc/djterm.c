#ifndef lint
static char DJGPP_Rcs_Id[] =
    "$Id";
#endif

/*
 * djterm.c - DJGPP-specific terminal driver for Ispell
 *
 * Eli Zaretskii <eliz@is.elta.co.il>, 1996, 2001
 *
 * Copyright 1996, Geoff Kuenning, Granada Hills, CA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. The code that causes the 'ispell -v' command to display a prominent
 *    link to the official ispell Web site may not be removed.
 * 5. The name of Geoff Kuenning may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GEOFF KUENNING AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL GEOFF KUENNING OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/05/01 23:03:25  geoff
 * Updates from Eli Zaretskii.
 *
 * Revision 1.3  2005/04/13 23:54:23  geoff
 * Update license.
 *
 * Revision 1.2  2001/09/06 00:33:35  geoff
 * Make the license consistent, and do some style cleanups.
 *
 * Revision 1.1  2001/09/01 06:40:21  geoff
 * As received from Eli
 *
 */

/*
** DJGPP currently doesn't support Unixy ioctl directly, so we
** have to define minimal support here via the filesystem extensions.
*/

#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <conio.h>
#include <sys/fsext.h>

static struct text_info txinfo;
static unsigned char * saved_screen;
static unsigned char ispell_norm_attr, ispell_sout_attr;

/* These declarations are on <sys/ioctl.h>, but as of DJGPP v2.03 they
   are ifdefed away.  To accomodate for both old and new versions,
   where some of the TIOC* commands might be supported, we override
   any possible definitions of those commands, but provide
   declarations of structures they use only if they are not provided
   by the library.  */
#ifdef TIOCGWINSZ
#undef TIOCGWINSZ
#else
struct winsize
    {
    unsigned short	ws_row;
    unsigned short	ws_col;
    unsigned short	ws_xpixel;
    unsigned short	ws_ypixel;
    };
#endif

#ifdef TIOCGETP
#undef TIOCGETP
#else
struct sgttyb
    {
    char		sg_ispeed;
    char		sg_ospeed;
    char		sg_erase;
    char		sg_kill;
    short		sg_flags;
    };
#endif

#undef IOCPARM_MASK
#undef IOC_OUT
#undef IOC_IN
#undef IOC_INOUT

#define IOCPARM_MASK    0x7f
#define IOC_OUT         0x40000000
#define IOC_IN          0x80000000
#define IOC_INOUT       (IOC_IN|IOC_OUT)

#undef _IOR
#undef _IOW
#undef _IOWR

#define _IOR(x,y,t)     (IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)
#define _IOW(x,y,t)     (IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)
#define _IOWR(x,y,t)    (IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|(x<<8)|y)

#undef TIOCGPGRP
#undef TIOCSETP
#undef CBREAK
#undef ECHO

/* These are the only ones we support here.  */
#define TIOCGWINSZ  _IOR('t', 104, struct winsize)
#define TIOCGPGRP   _IOR('t', 119, int)
#define TIOCGETP    _IOR('t', 8, struct sgttyb)
#define TIOCSETP    _IOW('t', 9, struct sgttyb)
#define CBREAK      0x00000002
#define ECHO        0x00000008

/* This will be called by low-level I/O functions.  */
static int djgpp_term (__FSEXT_Fnumber func, int *retval, va_list rest_args)
    {
    int fhandle = va_arg (rest_args, int);

    /*
    ** We only support ioctl on STDIN and write on STDOUT/STDERR.
    */
    if (func == __FSEXT_ioctl  &&  fhandle == fileno (stdin)
      && isatty (fhandle))
	{
	int cmd = va_arg (rest_args, int);

	switch (cmd)
	    {
	    case TIOCGWINSZ:
		{
		struct winsize *winfo = va_arg (rest_args, struct winsize *);

		winfo->ws_row = ScreenRows ();
		winfo->ws_col = ScreenCols ();
		winfo->ws_xpixel = 1;
		winfo->ws_ypixel = 1;
		*retval = 0;
		break;
		}
	    case TIOCGPGRP:
		*retval = 0;
		break;
	    case TIOCGETP:
		{
		struct sgttyb * gtty = va_arg (rest_args, struct sgttyb *);
		gtty->sg_ispeed = gtty->sg_ospeed = 0; /* unused */
		gtty->sg_erase = K_BackSpace;
		gtty->sg_kill  = K_Control_U;
		gtty->sg_flags = 0; /* unused */
		*retval = 0;
		break;
		}
	    case TIOCSETP:
		*retval = 0;
		break;
	    default:
		*retval = -1;
		break;
	    }
	return 1;
	}
    else if (func == __FSEXT_write
      &&  (fhandle == fileno (stdout)  ||  fhandle == fileno (stderr))
      &&  isatty (fhandle) &&  termchanged)
	{
	/*
	** Cannot write the output as is, because it might include
	** TABS.  We need to expand them into suitable number of spaces.
	*/
	int	col;
	int	dummy;
	char *	buf = va_arg (rest_args, char *);
	size_t	buflen = va_arg (rest_args, size_t);
	char *	local_buf;
	char *	s;
	char *	d;

	if (!buf)
	    {
	    errno = EINVAL;
	    *retval = -1;
	    return 1;
	    }

	*retval = buflen;	/* `_write' expects number of bytes written */
	local_buf = (char *) alloca (buflen+8+1); /* 8 for TAB, 1 for '\0' */
	ScreenGetCursor (&dummy, &col);

	for (s = buf, d = local_buf;  buflen--;  s++)
	    {
	    if (*s == '\0')	/* `cputs' treats '\0' as end of string */
		{
		*d = *s;
		cputs (local_buf);
		putch (*s);
		d = local_buf;
		col++;
		}
	    else if (*s == '\t')
		{
		*d++ = ' ';
		col++;
		while (col % 8)
		    {
		    *d++ = ' ';
		    col++;
		    }
		*d = '\0';
		cputs (local_buf);
		d = local_buf;
		}
	    else
	        {
		*d++ = *s;
		if (*s == '\r')
		    col = 0;
		else if (*s != '\n')
		    col++;
		}
	    }
	if (d > local_buf)
	    {
	    *d = '\0';
	    cputs (local_buf);
	    }

	return 1;
	}
    else
        return 0;
    }


/* This is called before `main' to install our terminal handler. */
static void __attribute__((constructor))
djgpp_ispell_startup (void)
{
  __FSEXT_set_function (fileno (stdin), djgpp_term);
  __FSEXT_set_function (fileno (stdout), djgpp_term);
  __FSEXT_set_function (fileno (stderr), djgpp_term);
}

/* DJGPP-specific screen initialization and deinitialization.  */
static void djgpp_init_terminal (void)
    {
    if (li == 0)
	{
	/*
	** On MSDOS/DJGPP platforms, colors are used for normal and
	** inverse-video displays.  The colors and screen size seen
	** at program startup are saved, to be restored before exit.
	** The screen contents are also saved and restored.
	*/
	char *	ispell_colors;

	gettextinfo (&txinfo);
	saved_screen = (unsigned char *) malloc (
	  txinfo.screenwidth * txinfo.screenheight * 2);
	if (saved_screen)
	    ScreenRetrieve (saved_screen);

	/*
	** Let the user specify their favorite colors for normal
	** and standout text, like so:
	**
	**         set ISPELL_COLORS=0x1e.0x74
	**                            se   so
	*/
	ispell_colors = getenv ("ISPELL_COLORS");
	if (ispell_colors != NULL)
	    {
	    char *	next;
	    unsigned long coldesc = strtoul (ispell_colors, &next, 0);

	    if (next == ispell_colors  ||  coldesc > UCHAR_MAX)
	        ispell_colors = NULL;
	    else
		{
		char *	endp;

		ispell_norm_attr = (unsigned char) coldesc;
		coldesc = strtoul (next + 1, &endp, 0);
		if (endp == next + 1  ||  coldesc > UCHAR_MAX)
		    ispell_colors = NULL;
		else
		    ispell_sout_attr = (unsigned char) coldesc;
		}
	    }
	if (ispell_colors == NULL)
	    {
	    /* Use dull B&W color scheme */
	    ispell_norm_attr = LIGHTGRAY + (BLACK << 4);
	    ispell_sout_attr  = BLACK + (LIGHTGRAY << 4);
	    }
	}
    }

static void djgpp_restore_screen (void)
    {
    if (li != txinfo.screenheight)
        _set_screen_lines (txinfo.screenheight);
    textmode (txinfo.currmode);
    textattr (txinfo.attribute);
    gotoxy (1, txinfo.screenheight);
    clreol ();
    }

static void djgpp_deinit_term (void)
    {
    termchanged = 0;	/* so output uses stdio again */
    printf ("\n");	/* in case some garbage is pending */
    fflush (stdout);
    if (saved_screen)
	{
	ScreenUpdate (saved_screen);
	gotoxy (txinfo.curx, txinfo.cury);
	}
    }

static void djgpp_ispell_screen ()
    {
    fflush (stdout);
    if (li != txinfo.screenheight)
	_set_screen_lines (li);
    textattr (ispell_norm_attr);
    }

static int djgpp_column;
static int djgpp_row;

char * tgoto (char * cmd, int col, int row)
    {
    djgpp_column = col;
    djgpp_row = row;
    return "\2";
    }

char * tputs (const char * cmd, int cnt, int (*func)(int))
    {
    fflush (stdout);
    if (!cmd)
	abort ();
    switch (*cmd)
	{
	case '\1':		/* erase */
	    clrscr ();
	    break;
	case '\2':		/* move */
	    gotoxy (djgpp_column + 1, djgpp_row + 1);
	    break;
	case '\3':		/* stand-out */
	    textattr (ispell_sout_attr);
	    break;
	case '\4':		/* end stand-out */
	    textattr (ispell_norm_attr);
	    break;
	case '\5':		/* backup */
	    gotoxy (wherex () - cnt, wherey ());
	    break;
	case '\6':		/* terminal init */
	    djgpp_ispell_screen ();
	    clrscr ();
	    break;
	case '\7':		/* terminal termination */
	    djgpp_restore_screen ();
	    djgpp_deinit_term ();
	    break;
	default:
	    abort ();
	}
    }

int tgetent (char * buf, const char * term_name)
    {
    djgpp_init_terminal ();
    }

char * tgetstr (const char * cmd, char ** buf)
    {
    static struct emulated_cmd
	{
	char *external_name;
	char *internal_code;
	}
		commands[] =
        {
	    { "cl", "\1" },
	    { "cm", "\2" },
	    { "so", "\3" },
	    { "se", "\4" },
	    { "bc", "\5" },
	    { "ti", "\6" },
	    { "te", "\7" },
	};
		int i;

    for (i = 0; i < sizeof (commands) / sizeof (commands[0]); i++)
	{
	if (strcmp (cmd, commands[i].external_name) == 0)
	    return commands[i].internal_code;
	}

    return NULL;
    }

int tgetnum (const char * cmd)
    {
    if (cmd  &&  strcmp (cmd, "co") == 0)
	return 80;
    else if (cmd  &&  strcmp (cmd, "li") == 0)
	return 24;
    return -1;
    }
