/* $XConsortium: Login.c,v 1.42 94/04/17 20:03:53 gildea Exp $ */
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * Login.c
 */

# include <X11/IntrinsicP.h>
# include <X11/StringDefs.h>
# include <X11/keysym.h>
# include <X11/Xfuncs.h>

# include <stdio.h>

#ifdef sgi
#include <sys/systeminfo.h>
#endif

# include "dm.h"
# include "greet.h"
# include "LoginP.h"

#ifdef NRL
#include "xdm_bitmap.h"
#else
#define xdm_width 0
#define xdm_height 0
#endif

#define offset(field) XtOffsetOf(LoginRec, login.field)
#define goffset(field) XtOffsetOf(WidgetRec, core.field)

extern Debug(), ResetLogin(), LogOutOfMem(), RedrawFail();

static XtResource resources[] = {
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	goffset(width), XtRImmediate,	(XtPointer) 0},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	goffset(height), XtRImmediate,	(XtPointer) 0},
    {XtNx, XtCX, XtRPosition, sizeof (Position),
	goffset(x), XtRImmediate,	(XtPointer) -1},
    {XtNy, XtCY, XtRPosition, sizeof (Position),
	goffset(y), XtRImmediate,	(XtPointer) -1},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(textpixel), XtRString,	XtDefaultForeground},
    {XtNpromptColor, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(promptpixel), XtRString,	XtDefaultForeground},
    {XtNgreetColor, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(greetpixel), XtRString,	XtDefaultForeground},
    {XtNfailColor, XtCForeground, XtRPixel, sizeof (Pixel),
	offset(failpixel), XtRString,	XtDefaultForeground},

/* added by Amit Margalit */
    {XtNhiColor, XtCForeground, XtRPixel, sizeof (Pixel),
	offset(hipixel), XtRString,	XtDefaultForeground},
    {XtNshdColor, XtCForeground, XtRPixel, sizeof (Pixel),
	offset(shdpixel), XtRString,	XtDefaultForeground},
    {XtNframeWidth, XtCFrameWidth, XtRInt, sizeof(int),
        offset(outframewidth), XtRImmediate, (XtPointer) 1},
    {XtNinnerFramesWidth, XtCFrameWidth, XtRInt, sizeof(int),
        offset(inframeswidth), XtRImmediate, (XtPointer) 1},
    {XtNsepWidth, XtCFrameWidth, XtRInt, sizeof(int),
        offset(sepwidth), XtRImmediate, (XtPointer) 1},

    {XtNfont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
    	offset (font), XtRString,	"*-new century schoolbook-medium-r-normal-*-180-*"},
    {XtNpromptFont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
    	offset (promptFont), XtRString, "*-new century schoolbook-bold-r-normal-*-180-*"},
    {XtNgreetFont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
    	offset (greetFont), XtRString,	"*-new century schoolbook-bold-i-normal-*-240-*"},
    {XtNfailFont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
	offset (failFont), XtRString,	"*-new century schoolbook-bold-r-normal-*-180-*"},
    {XtNgreeting, XtCGreeting, XtRString, sizeof (char *),
    	offset(greeting), XtRString, "X Window System"},
    {XtNunsecureGreeting, XtCGreeting, XtRString, sizeof (char *),
	offset(unsecure_greet), XtRString, "This is an unsecure session"},
    {XtNnamePrompt, XtCNamePrompt, XtRString, sizeof (char *),
	offset(namePrompt), XtRString, "Login:  "},
    {XtNpasswdPrompt, XtCPasswdPrompt, XtRString, sizeof (char *),
	offset(passwdPrompt), XtRString, "Password:  "},
    {XtNfail, XtCFail, XtRString, sizeof (char *),
	offset(fail), XtRString, "Login incorrect"},
    {XtNfailTimeout, XtCFailTimeout, XtRInt, sizeof (int),
	offset(failTimeout), XtRImmediate, (XtPointer) 10},
    {XtNnotifyDone, XtCCallback, XtRFunction, sizeof (XtPointer),
	offset(notify_done), XtRFunction, (XtPointer) 0},
    {XtNsessionArgument, XtCSessionArgument, XtRString,	sizeof (char *),
	offset(sessionArg), XtRString, (XtPointer) 0 },
    {XtNsecureSession, XtCSecureSession, XtRBoolean, sizeof (Boolean),
	offset(secure_session), XtRImmediate, False },
    {XtNallowAccess, XtCAllowAccess, XtRBoolean, sizeof (Boolean),
	offset(allow_access), XtRImmediate, False }
};

#undef offset
#undef goffset

# define TEXT_X_INC(w)	((w)->login.font->max_bounds.width)
# define TEXT_Y_INC(w)	((w)->login.font->max_bounds.ascent +\
			 (w)->login.font->max_bounds.descent)
# define PROMPT_X_INC(w)	((w)->login.promptFont->max_bounds.width)
# define PROMPT_Y_INC(w)	((w)->login.promptFont->max_bounds.ascent +\
			 (w)->login.promptFont->max_bounds.descent)
# define GREET_X_INC(w)	((w)->login.greetFont->max_bounds.width)
# define GREET_Y_INC(w)	((w)->login.greetFont->max_bounds.ascent +\
			 (w)->login.greetFont->max_bounds.descent)
# define FAIL_X_INC(w)	((w)->login.failFont->max_bounds.width)
# define FAIL_Y_INC(w)	((w)->login.failFont->max_bounds.ascent +\
			 (w)->login.failFont->max_bounds.descent)

# define Y_INC(w)	max (TEXT_Y_INC(w), PROMPT_Y_INC(w))

# define LOGIN_PROMPT_W(w) (XTextWidth (w->login.promptFont,\
				 w->login.namePrompt,\
				 strlen (w->login.namePrompt)) + \
				 w->login.inframeswidth)
# define PASS_PROMPT_W(w) (XTextWidth (w->login.promptFont,\
				 w->login.passwdPrompt,\
				 strlen (w->login.passwdPrompt)) + \
				 w->login.inframeswidth)
# define PROMPT_W(w)	(max(LOGIN_PROMPT_W(w), PASS_PROMPT_W(w)))
# define GREETING(w)	((w)->login.secure_session  && !(w)->login.allow_access ?\
				(w)->login.greeting : (w)->login.unsecure_greet)
# define GREET_X(w)	((int)(w->core.width - xdm_width - XTextWidth (w->login.greetFont,\
			  GREETING(w), maxlinelen (GREETING(w)))) / 2)
# define GREET_Y(w)	(GREETING(w)[0] ? (numlines(GREETING(w)) * \
			      GREET_Y_INC(w) + GREET_Y_INC(w)) : 0)
# define GREET_W(w)	(max (XTextWidth (w->login.greetFont,\
			      w->login.greeting, maxlinelen (w->login.greeting)), \
			      XTextWidth (w->login.greetFont,\
			      w->login.unsecure_greet, maxlinelen (w->login.unsecure_greet))))
# define LOGIN_X(w)	(2 * PROMPT_X_INC(w))
# define LOGIN_Y(w)	(GREET_Y(w) + GREET_Y_INC(w) +\
			 w->login.greetFont->max_bounds.ascent + Y_INC(w))
# define LOGIN_W(w)	(w->core.width - xdm_width - 6 * TEXT_X_INC(w))
# define LOGIN_H(w)	(3 * Y_INC(w) / 2)
# define LOGIN_TEXT_X(w)(LOGIN_X(w) + PROMPT_W(w))
# define PASS_X(w)	(LOGIN_X(w))
# define PASS_Y(w)	(LOGIN_Y(w) + 10 * Y_INC(w) / 5)
# define PASS_W(w)	(LOGIN_W(w))
# define PASS_H(w)	(LOGIN_H(w))
# define PASS_TEXT_X(w)	(PASS_X(w) + PROMPT_W(w))
# define FAIL_X(w)	((int)(w->core.width - xdm_width - XTextWidth (w->login.failFont,\
				w->login.fail, strlen (w->login.fail))) / 2)
# define FAIL_Y(w)	(PASS_Y(w) + 2 * FAIL_Y_INC (w) +\
			w->login.failFont->max_bounds.ascent)
# define FAIL_W(w)	(XTextWidth (w->login.failFont,\
			 w->login.fail, strlen (w->login.fail)))

# define PAD_X(w)	(2 * (LOGIN_X(w) + max (GREET_X_INC(w), FAIL_X_INC(w))))

# define PAD_Y(w)	(max (max (Y_INC(w), GREET_Y_INC(w)),\
			     FAIL_Y_INC(w)))
	
static void Initialize(), Realize(), Destroy(), Redisplay();
static Boolean SetValues();
static void draw_it ();
static int numlines ();
static int maxlinelen ();

static int max (a,b) { return a > b ? a : b; }

static void
EraseName (w, cursor)
    LoginWidget	w;
    int		cursor;
{
    int	x;

    x = LOGIN_TEXT_X (w);
    if (cursor > 0)
	x += XTextWidth (w->login.font, w->login.data.name, cursor);
    XDrawString (XtDisplay(w), XtWindow (w), w->login.bgGC, x, LOGIN_Y(w),
		w->login.data.name + cursor, strlen (w->login.data.name + cursor));
}

static void
DrawName (w, cursor)
    LoginWidget	w;
    int		cursor;
{
    int	x;

    x = LOGIN_TEXT_X (w);
    if (cursor > 0)
	x += XTextWidth (w->login.font, w->login.data.name, cursor);
    XDrawString (XtDisplay(w), XtWindow (w), w->login.textGC, x, LOGIN_Y(w),
		w->login.data.name + cursor, strlen (w->login.data.name + cursor));
}

static void
realizeCursor (w, gc)
    LoginWidget	w;
    GC		gc;
{
    int	x, y;
    int height, width;

    switch (w->login.state) {
    case GET_NAME:
	x = LOGIN_TEXT_X (w);
	y = LOGIN_Y (w);
	height = w->login.font->max_bounds.ascent + w->login.font->max_bounds.descent;
	width = 1;
	if (w->login.cursor > 0)
	    x += XTextWidth (w->login.font, w->login.data.name, w->login.cursor);
	break;
    case GET_PASSWD:
	x = PASS_TEXT_X (w);
	y = PASS_Y (w);
	height = w->login.font->max_bounds.ascent + w->login.font->max_bounds.descent;
	width = 1;
	break;
    default:
	return;
    }
    XFillRectangle (XtDisplay (w), XtWindow (w), gc,
		    x, y+1 - w->login.font->max_bounds.ascent, width, height-1);
    XDrawPoint     (XtDisplay (w), XtWindow (w), gc,
    		    x-1 , y - w->login.font->max_bounds.ascent);
    XDrawPoint     (XtDisplay (w), XtWindow (w), gc,
    		    x+1 , y - w->login.font->max_bounds.ascent);
    XDrawPoint     (XtDisplay (w), XtWindow (w), gc,
    		    x-1 , y - w->login.font->max_bounds.ascent+height);
    XDrawPoint     (XtDisplay (w), XtWindow (w), gc,
    		    x+1 , y - w->login.font->max_bounds.ascent+height);
    XDrawPoint     (XtDisplay (w), XtWindow (w), gc,
    		    x-2 , y - w->login.font->max_bounds.ascent);
    XDrawPoint     (XtDisplay (w), XtWindow (w), gc,
    		    x+2 , y - w->login.font->max_bounds.ascent);
    XDrawPoint     (XtDisplay (w), XtWindow (w), gc,
    		    x-2 , y - w->login.font->max_bounds.ascent+height);
    XDrawPoint     (XtDisplay (w), XtWindow (w), gc,
    		    x+2 , y - w->login.font->max_bounds.ascent+height);
}

static void
EraseFail (w)
    LoginWidget	w;
{
    int x = FAIL_X(w);
    int y = FAIL_Y(w);

    XSetForeground (XtDisplay (w), w->login.failGC,
			w->core.background_pixel);
    XDrawString (XtDisplay (w), XtWindow (w), w->login.failGC,
		x, y,
		w->login.fail, strlen (w->login.fail));
    w->login.failUp = 0;
    XSetForeground (XtDisplay (w), w->login.failGC,
			w->login.failpixel);
}

static void
XorCursor (w)
    LoginWidget	w;
{
    realizeCursor (w, w->login.xorGC);
}

static void
RemoveFail (w)
    LoginWidget	w;
{
    if (w->login.failUp)
	EraseFail (w);
}

static void
EraseCursor (w)
    LoginWidget (w);
{
    realizeCursor (w, w->login.bgGC);
}

/*ARGSUSED*/
void failTimeout (client_data, id)
    XtPointer	client_data;
    XtIntervalId *	id;
{
    LoginWidget	w = (LoginWidget)client_data;

    Debug ("failTimeout\n");
    EraseFail (w);
}

int DrawFail (ctx)
    Widget	ctx;
{
    LoginWidget	w;

    w = (LoginWidget) ctx;
    XorCursor (w);
    ResetLogin (w);
    XorCursor (w);
    w->login.failUp = 1;
    RedrawFail (w);
    if (w->login.failTimeout > 0) {
	Debug ("failTimeout: %d\n", w->login.failTimeout);
	XtAppAddTimeOut(XtWidgetToApplicationContext ((Widget)w),
			w->login.failTimeout * 1000,
		        failTimeout, (XtPointer) w);
    }
  return 0;
}

int RedrawFail (w)
    LoginWidget w;
{
    int x = FAIL_X(w);
    int y = FAIL_Y(w);

    if (w->login.failUp)
        XDrawString (XtDisplay (w), XtWindow (w), w->login.failGC,
		    x, y,
		    w->login.fail, strlen (w->login.fail));
    return 0;
}

static void
draw_it (w)
    LoginWidget	w;
{
    int i,in_frame_x,in_login_y,in_pass_y,in_width,in_height;
    int gr_line_x, gr_line_y, gr_line_w;
#ifdef NRL
    int gr_line_h;
    Pixmap bitmap;
#endif /* NRL */

    EraseCursor (w);

    if( (w->login.outframewidth) < 1 )
      w->login.outframewidth = 1;
    for(i=1;i<=(w->login.outframewidth);i++)
    {
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.hiGC,
    		i-1,i-1,w->core.width-i,i-1);
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.hiGC,
    		i-1,i-1,i-1,w->core.height-i);
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.shdGC,
    		w->core.width-i,i-1,w->core.width-i,w->core.height-i);
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.shdGC,
    		i-1,w->core.height-i,w->core.width-i,w->core.height-i);
    }
    
    /* make separator line */
    gr_line_x = w->login.outframewidth;
    gr_line_y = GREET_Y(w) + GREET_Y_INC(w);
    gr_line_w = w->core.width - xdm_width - 2*(w->login.outframewidth)
#ifdef NRL
		- 2*(w->login.inframeswidth)
#endif /* NRL */
		;
 
    for(i=1;i<=(w->login.sepwidth);i++)
    {
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.shdGC,
        gr_line_x,           gr_line_y + i-1,
        gr_line_x+gr_line_w, gr_line_y + i-1);
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.hiGC,
        gr_line_x,           gr_line_y + 2*(w->login.inframeswidth) -i,
        gr_line_x+gr_line_w, gr_line_y + 2*(w->login.inframeswidth) -i);
    }
#ifdef NRL
    gr_line_y = w->login.outframewidth;
    gr_line_x = w->core.width - xdm_width - 2*(w->login.inframeswidth) -
		w->login.outframewidth;
    gr_line_h = w->core.height - 2*(w->login.outframewidth);

    for (i=1;i<=(w->login.sepwidth);i++)
    {
      XDrawLine(XtDisplay(w), XtWindow(w), w->login.shdGC,
	gr_line_x + i-1,	gr_line_y,
	gr_line_x + i-1,	gr_line_y + gr_line_h);
      XDrawLine(XtDisplay(w), XtWindow(w), w->login.hiGC,
	gr_line_x + 2*(w->login.inframeswidth) - i,	gr_line_y,
	gr_line_x + 2*(w->login.inframeswidth) - i,	gr_line_y + gr_line_h);
    }

    bitmap = XCreateBitmapFromData(XtDisplay(w), XtWindow(w), xdm_bits,
				   xdm_width, xdm_height);

    XCopyPlane(XtDisplay(w), bitmap, XtWindow(w), w->login.textGC, 0, 0,
		xdm_width, xdm_height,
		w->core.width - xdm_width - w->login.outframewidth,
		(w->core.height - xdm_height) / 2, 1);

    XFreePixmap(XtDisplay(w), bitmap);
#endif /* NRL */

    in_frame_x = LOGIN_TEXT_X(w) - w->login.inframeswidth - 3;
    in_login_y = LOGIN_Y(w) - w->login.inframeswidth - 1 - TEXT_Y_INC(w);
    in_pass_y  = PASS_Y(w) - w->login.inframeswidth - 1 - TEXT_Y_INC(w);
 
    in_width = LOGIN_W(w) - PROMPT_W(w);
    in_height = LOGIN_H(w) + w->login.inframeswidth + 2;

    for(i=1;i<=(w->login.inframeswidth);i++)
    {
      /* Make top/left sides */
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.shdGC,
	in_frame_x + i-1,             in_login_y + i-1,
	in_frame_x + in_width-i,      in_login_y + i-1); 

      XDrawLine(XtDisplay (w), XtWindow (w), w->login.shdGC,
	in_frame_x + i-1,             in_login_y + i-1,
	in_frame_x + i-1,             in_login_y + in_height-i); 

      XDrawLine(XtDisplay (w), XtWindow (w), w->login.hiGC,
        in_frame_x + in_width-i,      in_login_y + i-1,
        in_frame_x + in_width-i,      in_login_y + in_height-i); 
                
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.hiGC,
	in_frame_x + i-1,             in_login_y + in_height-i,
	in_frame_x + in_width-i,      in_login_y + in_height-i);

      /* Make bottom/right sides */
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.shdGC,
	in_frame_x + i-1,             in_pass_y + i-1,
	in_frame_x + in_width-i,      in_pass_y + i-1); 

      XDrawLine(XtDisplay (w), XtWindow (w), w->login.shdGC,
	in_frame_x + i-1,             in_pass_y + i-1,
	in_frame_x + i-1,             in_pass_y + in_height-i); 

      XDrawLine(XtDisplay (w), XtWindow (w), w->login.hiGC,
        in_frame_x + in_width-i,      in_pass_y + i-1,
        in_frame_x + in_width-i,      in_pass_y + in_height-i); 
                
      XDrawLine(XtDisplay (w), XtWindow (w), w->login.hiGC,
	in_frame_x + i-1,             in_pass_y + in_height-i,
	in_frame_x + in_width-i,      in_pass_y + in_height-i);
    }

    if (GREETING(w)[0]) {
	char line[1024];
	int i, linenum = 1;

	for (line[0] = '\0', i = 0; GREETING(w)[i]; i++) {
		switch (GREETING(w)[i]) {

		case '\n':
		     XDrawString (XtDisplay(w), XtWindow(w), w->login.greetGC,
				 (w->core.width - XTextWidth (w->login.greetFont,
				     line, strlen(line))) / 2,
				 GREET_Y_INC (w) * (linenum + 1),
				 line, strlen(line));
		     linenum++;
		     line[0] = '\0';
		     break;

		case '%':
		     i++;
		     switch (GREETING(w)[i]) {
			case '\0':
			    i--;
			    break;
			case 'H':
#ifdef sgi
			    sysinfo(SI_HOSTNAME, &line[strlen(line)], 1024);
#else
			    gethostname(&line[strlen(line)], 1024);
#endif
			    break;
			case 'R':
#ifdef sgi
			    sysinfo(SI_RELEASE, &line[strlen(line)], 1024);
#endif
			    break;
			case '%':
			    strcat(line, "%");
			default:
			    break;
		     }
		     break;

		default:
		     strncat(line, &GREETING(w)[i], 1);
		}

	}
	XDrawString (XtDisplay(w), XtWindow(w), w->login.greetGC,
		    (w->core.width - XTextWidth (w->login.greetFont,
			line, strlen(line))) / 2,
		    GREET_Y_INC (w) * (linenum + 1),
		    line, strlen(line));
    }

    XDrawString (XtDisplay (w), XtWindow (w), w->login.promptGC,
		LOGIN_X(w), LOGIN_Y(w),
		w->login.namePrompt, strlen (w->login.namePrompt));
    XDrawString (XtDisplay (w), XtWindow (w), w->login.promptGC,
		PASS_X(w), PASS_Y(w),
		w->login.passwdPrompt, strlen (w->login.passwdPrompt));
    RedrawFail (w);
    DrawName (w, 0);
    XorCursor (w);
    /*
     * The GrabKeyboard here is needed only because of
     * a bug in the R3 server -- the keyboard is grabbed on
     * the root window, and the server won't dispatch events
     * to the focus window unless the focus window is a ancestor
     * of the grab window.  Bug in server already found and fixed,
     * compatibility until at least R4.
     */
    if (XGrabKeyboard (XtDisplay (w), XtWindow (w), False, GrabModeAsync,
		       GrabModeAsync, CurrentTime) != GrabSuccess)
    {
	XSetInputFocus (XtDisplay (w), XtWindow (w),
			RevertToPointerRoot, CurrentTime);
    }
}

/*ARGSUSED*/
static void
DeleteBackwardChar (ctxw, event, params, num_params)
    Widget ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    if (ctx->login.cursor > 0) {
	ctx->login.cursor--;
	switch (ctx->login.state) {
	case GET_NAME:
	    EraseName (ctx, ctx->login.cursor);
	    strcpy (ctx->login.data.name + ctx->login.cursor,
		    ctx->login.data.name + ctx->login.cursor + 1);
	    DrawName (ctx, ctx->login.cursor);
	    break;
	case GET_PASSWD:
	    strcpy (ctx->login.data.passwd + ctx->login.cursor,
		    ctx->login.data.passwd + ctx->login.cursor + 1);
	    break;
	}
    }
    XorCursor (ctx);	
}

/*ARGSUSED*/
static void
DeleteForwardChar (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
	if (ctx->login.cursor < (int)strlen (ctx->login.data.name)) {
	    EraseName (ctx, ctx->login.cursor);
	    strcpy (ctx->login.data.name + ctx->login.cursor,
		    ctx->login.data.name + ctx->login.cursor + 1);
	    DrawName (ctx, ctx->login.cursor);
	}
	break;
    case GET_PASSWD:
    	if (ctx->login.cursor < (int)strlen (ctx->login.data.passwd)) {
	    strcpy (ctx->login.data.passwd + ctx->login.cursor,
		    ctx->login.data.passwd + ctx->login.cursor + 1);
	}
	break;
    }
    XorCursor (ctx);	
}

/*ARGSUSED*/
static void
MoveBackwardChar (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget	ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    if (ctx->login.cursor > 0)
    	ctx->login.cursor--;
    XorCursor (ctx);
}

/*ARGSUSED*/
static void
MoveForwardChar (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
    	if (ctx->login.cursor < (int)strlen(ctx->login.data.name))
	    ++ctx->login.cursor;
	break;
    case GET_PASSWD:
    	if (ctx->login.cursor < (int)strlen(ctx->login.data.passwd))
	    ++ctx->login.cursor;
	break;
    }
    XorCursor (ctx);
}

/*ARGSUSED*/
static void
MoveToBegining (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    ctx->login.cursor = 0;
    XorCursor (ctx);
}

/*ARGSUSED*/
static void
MoveToEnd (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
    	ctx->login.cursor = strlen (ctx->login.data.name);
	break;
    case GET_PASSWD:
    	ctx->login.cursor = strlen (ctx->login.data.passwd);
	break;
    }
    XorCursor (ctx);
}

/*ARGSUSED*/
static void
EraseToEndOfLine (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
	EraseName (ctx, ctx->login.cursor);
	ctx->login.data.name[ctx->login.cursor] = '\0';
	break;
    case GET_PASSWD:
	ctx->login.data.passwd[ctx->login.cursor] = '\0';
	break;
    }
    XorCursor (ctx);
}

/*ARGSUSED*/
static void
EraseLine (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    MoveToBegining (ctxw, event, params, num_params);
    EraseToEndOfLine (ctxw, event, params, num_params);
}

/*ARGSUSED*/
static void
FinishField (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
	ctx->login.state = GET_PASSWD;
	ctx->login.cursor = 0;
	break;
    case GET_PASSWD:
	ctx->login.state = DONE;
	ctx->login.cursor = 0;
	(*ctx->login.notify_done) (ctx, &ctx->login.data, NOTIFY_OK);
	break;
    }
    XorCursor (ctx);
}

/*ARGSUSED*/
static void
AllowAccess (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;
    Arg	arglist[1];
    Boolean allow;

    RemoveFail (ctx);
    XtSetArg (arglist[0], XtNallowAccess, (char *) &allow);
    XtGetValues ((Widget) ctx, arglist, 1);
    XtSetArg (arglist[0], XtNallowAccess, !allow);
    XtSetValues ((Widget) ctx, arglist, 1);
}

/*ARGSUSED*/
static void
SetSessionArgument (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    RemoveFail (ctx);
    if (ctx->login.sessionArg)
	XtFree (ctx->login.sessionArg);
    ctx->login.sessionArg = 0;
    if (*num_params > 0) {
	ctx->login.sessionArg = XtMalloc (strlen (params[0]) + 1);
	if (ctx->login.sessionArg)
	    strcpy (ctx->login.sessionArg, params[0]);
	else
	    LogOutOfMem ("set session argument");
    }
}

/*ARGSUSED*/
static void
RestartSession (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    ctx->login.state = DONE;
    ctx->login.cursor = 0;
    (*ctx->login.notify_done) (ctx, &ctx->login.data, NOTIFY_RESTART);
    XorCursor (ctx);
}

/*ARGSUSED*/
static void
AbortSession (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    ctx->login.state = DONE;
    ctx->login.cursor = 0;
    (*ctx->login.notify_done) (ctx, &ctx->login.data, NOTIFY_ABORT);
    XorCursor (ctx);
}

/*ARGSUSED*/
static void
AbortDisplay (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    XorCursor (ctx);
    RemoveFail (ctx);
    ctx->login.state = DONE;
    ctx->login.cursor = 0;
    (*ctx->login.notify_done) (ctx, &ctx->login.data, NOTIFY_ABORT_DISPLAY);
    XorCursor (ctx);
}

int ResetLogin (w)
    LoginWidget	w;
{
    EraseName (w, 0);
    w->login.cursor = 0;
    w->login.data.name[0] = '\0';
    w->login.data.passwd[0] = '\0';
    w->login.state = GET_NAME;
    return 0;
}

/* ARGSUSED */
static void
InsertChar (ctxw, event, params, num_params)
    Widget	ctxw;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    LoginWidget ctx = (LoginWidget)ctxw;

    char strbuf[128];
    int  len,pixels;

    len = XLookupString (&event->xkey, strbuf, sizeof (strbuf), 0, 0);
    strbuf[len] = '\0';

    pixels = 3 + ctx->login.font->max_bounds.width * len +
    	     XTextWidth(ctx->login.font,
    	     		ctx->login.data.name,
    	     		strlen(ctx->login.data.name));
    	     	/* pixels to be added */

    switch (ctx->login.state) {
    case GET_NAME:
	if (
	        (len + (int)strlen(ctx->login.data.name) >= NAME_LEN - 1)/* &&
		(pixels <= LOGIN_W(ctx) - PROMPT_W(ctx))*/
	   )
	    len = NAME_LEN - strlen(ctx->login.data.name) - 2;
    case GET_PASSWD:
	if (len + (int)strlen(ctx->login.data.passwd) >= NAME_LEN - 1)
	    len = NAME_LEN - strlen(ctx->login.data.passwd) - 2;
    }
    if (len == 0 || pixels >= LOGIN_W(ctx) - PROMPT_W(ctx))
	return;
    XorCursor (ctx);
    RemoveFail (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
	EraseName (ctx, ctx->login.cursor);
	memmove( ctx->login.data.name + ctx->login.cursor + len,
	       ctx->login.data.name + ctx->login.cursor,
	       strlen (ctx->login.data.name + ctx->login.cursor) + 1);
	memmove( ctx->login.data.name + ctx->login.cursor, strbuf, len);
	DrawName (ctx, ctx->login.cursor);
	ctx->login.cursor += len;
	break;
    case GET_PASSWD:
	memmove( ctx->login.data.passwd + ctx->login.cursor + len,
	       ctx->login.data.passwd + ctx->login.cursor,
	       strlen (ctx->login.data.passwd + ctx->login.cursor) + 1);
	memmove( ctx->login.data.passwd + ctx->login.cursor, strbuf, len);
	ctx->login.cursor += len;
	break;
    }
    XorCursor (ctx);
}

/* ARGSUSED */
static void Initialize (greq, gnew, args, num_args)
    Widget greq, gnew;
    ArgList args;
    Cardinal *num_args;
{
    LoginWidget w = (LoginWidget)gnew;
    XtGCMask	valuemask, xvaluemask;
    XGCValues	myXGCV;
    Arg		position[2];
    Position	x, y;

    myXGCV.foreground = w->login.hipixel;
    myXGCV.background = w->core.background_pixel;
    valuemask = GCForeground | GCBackground;
    w->login.hiGC = XtGetGC(gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->login.shdpixel;
    myXGCV.background = w->core.background_pixel;
    valuemask = GCForeground | GCBackground;
    w->login.shdGC = XtGetGC(gnew, valuemask, &myXGCV);
 
    myXGCV.foreground = w->login.textpixel;
    myXGCV.background = w->core.background_pixel;
    valuemask = GCForeground | GCBackground;
    if (w->login.font) {
	myXGCV.font = w->login.font->fid;
	valuemask |= GCFont;
    }
    w->login.textGC = XtGetGC(gnew, valuemask, &myXGCV);
    myXGCV.foreground = w->core.background_pixel;
    w->login.bgGC = XtGetGC(gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->login.textpixel ^ w->core.background_pixel;
    myXGCV.function = GXxor;
    xvaluemask = valuemask | GCFunction;
    w->login.xorGC = XtGetGC (gnew, xvaluemask, &myXGCV);

    /*
     * Note that the second argument is a GCid -- QueryFont accepts a GCid and
     * returns the curently contained font.
     */

    if (w->login.font == NULL)
	w->login.font = XQueryFont (XtDisplay (w),
		XGContextFromGC (XDefaultGCOfScreen (XtScreen (w))));

    xvaluemask = valuemask;
    if (w->login.promptFont == NULL)
        w->login.promptFont = w->login.font;
    else
	xvaluemask |= GCFont;

    myXGCV.foreground = w->login.promptpixel;
    myXGCV.font = w->login.promptFont->fid;
    w->login.promptGC = XtGetGC (gnew, xvaluemask, &myXGCV);

    xvaluemask = valuemask;
    if (w->login.greetFont == NULL)
    	w->login.greetFont = w->login.font;
    else
	xvaluemask |= GCFont;

    myXGCV.foreground = w->login.greetpixel;
    myXGCV.font = w->login.greetFont->fid;
    w->login.greetGC = XtGetGC (gnew, xvaluemask, &myXGCV);

    xvaluemask = valuemask;
    if (w->login.failFont == NULL)
	w->login.failFont = w->login.font;
    else
	xvaluemask |= GCFont;

    myXGCV.foreground = w->login.failpixel;
    myXGCV.font = w->login.failFont->fid;
    w->login.failGC = XtGetGC (gnew, xvaluemask, &myXGCV);

    w->login.data.name[0] = '\0';
    w->login.data.passwd[0] = '\0';
    w->login.state = GET_NAME;
    w->login.cursor = 0;
    w->login.failUp = 0;
    if (w->core.width == 0)
	w->core.width = max (GREET_W(w) + xdm_width, FAIL_W(w) + xdm_width) + PAD_X(w);
    if (w->core.height == 0) {
	int fy = FAIL_Y(w);
	int pady = PAD_Y(w);

	w->core.height = fy + pady;	/* for stupid compilers */
    }
    if ((x = w->core.x) == -1)
	x = (int)(XWidthOfScreen (XtScreen (w)) - w->core.width) / 2;
    if ((y = w->core.y) == -1)
	y = (int)(XHeightOfScreen (XtScreen (w)) - w->core.height) / 3;
    XtSetArg (position[0], XtNx, x);
    XtSetArg (position[1], XtNy, y);
    XtSetValues (XtParent (w), position, (Cardinal) 2);
}

 
static void Realize (gw, valueMask, attrs)
     Widget gw;
     XtValueMask *valueMask;
     XSetWindowAttributes *attrs;
{
    XtCreateWindow( gw, (unsigned)InputOutput, (Visual *)CopyFromParent,
		     *valueMask, attrs );
}

static void Destroy (gw)
     Widget gw;
{
    LoginWidget w = (LoginWidget)gw;
    bzero (w->login.data.name, NAME_LEN);
    bzero (w->login.data.passwd, NAME_LEN);
    XtReleaseGC(gw, w->login.textGC);
    XtReleaseGC(gw, w->login.bgGC);
    XtReleaseGC(gw, w->login.xorGC);
    XtReleaseGC(gw, w->login.promptGC);
    XtReleaseGC(gw, w->login.greetGC);
    XtReleaseGC(gw, w->login.failGC);
    XtReleaseGC(gw, w->login.hiGC);
    XtReleaseGC(gw, w->login.shdGC);
}

/* ARGSUSED */
static void Redisplay(gw, event, region)
     Widget gw;
     XEvent *event;
     Region region;
{
    draw_it ((LoginWidget) gw);
}

/*ARGSUSED*/
static Boolean SetValues (current, request, new, args, num_args)
    Widget  current, request, new;
    ArgList args;
    Cardinal *num_args;
{
    LoginWidget currentL, newL;
    
    currentL = (LoginWidget) current;
    newL = (LoginWidget) new;
    if (GREETING (currentL) != GREETING (newL))
	return True;
    return False;
}

static int numlines(line)
    char *line;
{
    int i;
    
    for (i = 1; *line; line++)
	if (*line == '\n')
	    i++;
	
    return i;
}

static int maxlinelen(line)
    char *line;
{
    int len = 0, i;

    for (i = 0; *line; line++, i++)
	if (*line == '\n') {
	    len = max(len, i);
	    i = -1;
	}
    
    len = max(len, i);

    return len;
}

char defaultLoginTranslations [] =
"\
Ctrl<Key>H:	delete-previous-character() \n\
Ctrl<Key>D:	delete-character() \n\
Ctrl<Key>B:	move-backward-character() \n\
Ctrl<Key>F:	move-forward-character() \n\
Ctrl<Key>A:	move-to-begining() \n\
Ctrl<Key>E:	move-to-end() \n\
Ctrl<Key>K:	erase-to-end-of-line() \n\
Ctrl<Key>U:	erase-line() \n\
Ctrl<Key>X:	erase-line() \n\
Ctrl<Key>C:	restart-session() \n\
Ctrl<Key>\\\\:	abort-session() \n\
:Ctrl<Key>plus:	allow-all-access() \n\
<Key>BackSpace:	delete-previous-character() \n\
<Key>Delete:	delete-previous-character() \n\
<Key>Return:	finish-field() \n\
<Key>:		insert-char() \
";

XtActionsRec loginActionsTable [] = {
  {"delete-previous-character",	DeleteBackwardChar},
  {"delete-character",		DeleteForwardChar},
  {"move-backward-character",	MoveBackwardChar},
  {"move-forward-character",	MoveForwardChar},
  {"move-to-begining",		MoveToBegining},
  {"move-to-end",		MoveToEnd},
  {"erase-to-end-of-line",	EraseToEndOfLine},
  {"erase-line",		EraseLine},
  {"finish-field", 		FinishField},
  {"abort-session",		AbortSession},
  {"abort-display",		AbortDisplay},
  {"restart-session",		RestartSession},
  {"insert-char", 		InsertChar},
  {"set-session-argument",	SetSessionArgument},
  {"allow-all-access",		AllowAccess},
};

LoginClassRec loginClassRec = {
    { /* core fields */
    /* superclass		*/	&widgetClassRec,
    /* class_name		*/	"Login",
    /* size			*/	sizeof(LoginRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	loginActionsTable,
    /* num_actions		*/	XtNumber (loginActionsTable),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	NULL,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	NULL,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	defaultLoginTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
    }
};

WidgetClass loginWidgetClass = (WidgetClass) &loginClassRec;
