#include "xdvi-config.h"
/*****************************************************************************
Help window for xdvi using Xt/Xaw.

written by Stefan Ulrich (stefanulrich@users.sourceforge.net) 2000/02/05

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL PAUL VOJTA OR ANY OTHER AUTHOR OF THIS SOFTWARE BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*****************************************************************************/

#ifdef TOOLKIT
/* Xaw specific stuff */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/AsciiText.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include "version.h"

/* mappings of help texts to resources */
#define help_topics_button_label	resource._help_topics_button_label
#define help_quit_button_label		resource._help_quit_button_label
#define help_intro			resource._help_intro
#define help_general_menulabel		resource._help_general_menulabel
#define help_general			resource._help_general
#define help_hypertex_menulabel		resource._help_hypertex_menulabel
#define help_hypertex			resource._help_hypertex
#define help_othercommands_menulabel	resource._help_othercommands_menulabel
#define help_othercommands		resource._help_othercommands
#define help_pagemotion_menulabel	resource._help_pagemotion_menulabel
#define help_pagemotion			resource._help_pagemotion
#define help_mousebuttons_menulabel	resource._help_mousebuttons_menulabel
#define help_mousebuttons		resource._help_mousebuttons
#define help_sourcespecials_menulabel	resource._help_sourcespecials_menulabel
#define help_sourcespecials		resource._help_sourcespecials

#define WIDGET_INFO_LIST_LEN 10 /* number of help texts */

/*
  ========================================================================
  Global Variables for communication with callbacks
  ========================================================================
*/

static int help_active = 0;
static Widget helptext, help;


/*
 * Altarnative strategy: make helptexts more dynamic.
 * E.g. by using a config file
 *
 * topics.help
 *
 * containing lines like
 *
 * Page Motion<TAB>page_motion
 *
 * which would then make xdvi create a widget
 *
 * page_motion
 *
 * with title
 *
 * Page Motion
 *
 * and search for a file named
 *
 * page_motion.help
 *
 * etc.; this way, topics could easily be added/removed.
 *
 */


/*
  ========================================================================
  Private Functions
  ========================================================================
*/

/*------------------------------------------------------------
 *  popdown_help_callback
 *
 *  Arguments:
 *	Widget w	- unused
 *	XtPointer a, b  - unused
 *	 
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Help popdown callback function
 *------------------------------------------------------------*/

/* ARGSUSED */
static void
popdown_help_callback(Widget w, XtPointer a, XtPointer b)
{
    UNUSED(w);
    UNUSED(a);
    UNUSED(b);
    
    if (!help_active) {
	return;
    }
    help_active = 0;
    XtPopdown(help);
}


/*------------------------------------------------------------
 *  select_help_topic_callback
 *
 *  Arguments:
 *	Widget w	      - help menu currently selected
 *	XtPointer client_data - contains the help text as string
 *	XtPointer call_data   - unused
 *	 
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Print the contents of the help associated with
 *	menu entry w into the helptext widget.
 *------------------------------------------------------------*/

/* ARGSUSED */
static void
select_help_topic_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    UNUSED(call_data);
    
    if (client_data == NULL) {
	fprintf(stderr, "shouldn't happen - resource `%s' is NULL!\n",
		XtName(w));
	print_statusline(STATUS_LONG,
			 "shouldn't happen - resource `%s' is NULL!\n",
			 XtName(w));
	return;
    }
    XtVaSetValues(helptext, XtNstring, client_data, NULL);
}
#endif /* TOOLKIT */

/*
  ========================================================================
  Public Functions
  ========================================================================
*/

/*------------------------------------------------------------
 *  show_help
 *
 *  Arguments:
 *	toplevel - parent of the help window if compiled with TOOLKIT; else: void
 *	 
 *  Returns:
 *	void
 *		 
 *  Purpose:
 *	Called from events.c when user invokes `help'.
 *      With Toolkit, pops up help window; without TOOLKIT,
 *      simply dumps help strings to stdout.
 *------------------------------------------------------------*/

#ifdef TOOLKIT
void
#else
void
show_help ()
#endif /* TOOLKIT */
show_help(Widget toplevel)
{
#ifdef TOOLKIT
    Widget topicsmenu, helpclose, helppaned, helppanel, topicsbutton, w;
    int i, j, size, offset, alloc_len;
    _Xconst int alloc_step = 1024;
    static Boolean help_created = False;
#else
    int j, i;
#endif /* TOOLKIT */


    /*
     * Define fallbacks: default_xyz is used as fallback text if X
     * resource xyz isn't specified.
     *
     * Use arrays of strings rather than simple strings because of C
     * limitations on max string length; but since the resource needs to
     * be a simple `char *', these have to be copied into larger buffers
     * later on (which is a bit wasteful to space). But splitting the
     * strings into smaller pieces would make them hard to deal with as
     * X resources. They are defined as static so that they are initialized
     * only once.
     *
     * Last elem of each array is NULL for ease of looping through it.
     */

    static _Xconst char *default_help_intro[] = {
	"\n",
	"Please choose a topic from the menu above,\n",
	"or `Close' to exit help.\n",
	"More detailed help is available with `man xdvi'.\n",
	NULL
    };

    static _Xconst char *default_help_general[] = {
	"\n",
	"This is xdvik, version ",
	TVERSION
	".\nThe xdvik project homepage is located at\n",
	"http://sourceforge.net/projects/xdvi\n",
	"where you can find updates, report bugs and submit feature requests.\n",
	"\n",
	"Getting help and exiting xdvi:\n",
	"\n",
	"h or ? or `Help' button\n",
	"     Displays this help text.\n",
	"\n",
	"q or Control-C or Control-D or Cancel or Stop or Control-Z (VAX VMS)\n",
	"     Quits the program.\n",
	"\n",
	"\n",
	"\n",
	"Configuration options used for this build:\n",
#ifdef HTEX
	"- htex enabled\n",
#endif
#ifdef A4
	"- paper: a4/cm\n",
#else
	"- paper: letter/inch\n",
#endif
#ifdef GREY
	"- anti-aliasing (grey) enabled\n",
#endif
#ifdef STATUSLINE
	"- statusline enabled\n",
#endif
#ifdef BUTTONS
	"- buttons enabled\n",
#endif
#ifdef TEXXET
	"- left-to-right typesetting support\n",
#endif
#ifdef USE_GF
	"- gf file support\n",
#endif
	NULL
    };
    
#ifdef HTEX
    static _Xconst char *default_help_hypertex[] = {
	"\n",
	"HyperTeX Commands\n",
	"\n",
	"Xdvi can evaluate HyperTeX specials. The hyperlinks are\n",
	"underlined using the \"highlight\" colour (see the \"-hl\" option).\n",
	"Whenever the mouse is positioned on such a link, the cursor\n",
	"changes to a `hand' shape an the target of the link is displayed\n",
	"in the statusline at the bottom.\n",
	"\n",
	"The following keybindings are pre-configured:\n",
	"\n",
	"Mouse-1	  Follow the link at the cursor position.\n",
	"Mouse-1	  Open a new xdvi window displaying the link\n",
	"           at the cursor position.\n",
	"B or \"Back\" button\n",
	"           Go back to the previous anchor.\n",
	"\n",
	NULL
    };
#endif
    
    static _Xconst char *default_help_othercommands[] = {
	"\n",
	"Other Commands\n",
	"\n",
	"\n",
#ifdef SELFILE
	"Control-F\n",
	"     Find another DVI file.\n",
	"\n",
#endif
	"Control-L or Clear\n",
	"     Redisplays the current page.\n",
	"\n",
	"Control-P\n",
	"     Prints bitmap unit, bit order, and byte order.\n",
	"\n",
	"^ or Home\n",
	"     Move to the ``home'' position of the page.  This is\n",
	"     normally the upper left-hand corner of the page,\n",
	"     depending on the margins as described in the -margins\n",
	"     option, above.\n",
	"\n",
	"c    Moves the page so that the point currently beneath the\n",
	"     cursor is moved to the middle of the window.  It also\n",
	"     (gasp!) warps the cursor to the same place.\n",
	"\n",
	"G    This key toggles the use of greyscale anti-aliasing for\n",
	"     displaying shrunken bitmaps.  In addition, the key\n",
	"     sequences `0G' and `1G' clear and set this flag,\n",
	"     respectively.  See also the -nogrey option.\n",
	"\n",
	"k    Normally when xdvi switches pages, it moves to the home\n",
	"     position as well.  The `k' keystroke toggles a `keep-\n",
	"     position' flag which, when set, will keep the same\n",
	"     position when moving between pages.  Also `0k' and `1k'\n",
	"     clear and set this flag, respectively.  See also the\n",
	"     -keep option.\n",
	"\n",
	"M    Sets the margins so that the point currently under the\n",
	"     cursor is the upper left-hand corner of the text in the\n",
	"     page.  Note that this command itself does not move the\n",
	"     image at all.  For details on how the margins are used,\n",
	"     see the -margins option.\n",
	"\n",
	"P    ``This is page number n.''  This can be used to make\n",
	"     the `g' keystroke refer to actual page numbers instead\n",
	"     of absolute page numbers.\n",
	"\n",
	"R    Forces the dvi file to be reread.  This allows you to\n",
	"     preview many versions of the same file while running\n",
	"     xdvi only once.\n",
	"\n",
	"s    Changes the shrink factor to the given number.  If no\n",
	"     number is given, the smallest factor that makes the\n",
	"     entire page fit in the window will be used.  (Margins\n",
	"     are ignored in this computation.)\n",
	"\n",
	"S    Sets the density factor to be used when shrinking\n",
	"     bitmaps.  This should be a number between 0 and 100;\n",
	"     higher numbers produce lighter characters.\n",
	"\n",
	"t    Toggles to the next unit in a sorted list of TeX dimension\n",
	"     units for the popup magnifier ruler.\n",
	"\n",
	"V    Toggles Ghostscript anti-aliasing.  Also `0V' and `1V' clear\n",
	"     and enables this mode, respectively.  See also the the\n",
	"     -gsalpha option.\n",
	"\n",
	"x    Toggles expert mode (in which the buttons do not appear).\n",
	"     `1x' toggles display of the statusline at the bottom of\n",
	"     the window.\n",
	"\n",
	NULL
    };
    
    static _Xconst char *default_help_pagemotion[] = {
	"\n",
	"Moving around in the document\n",
	"\n",
	"\n",
	"\n",
	"n or f or Space or Return or LineFeed or PgDn\n",
	"     Moves to the next page (or to the nth next page if a\n",
	"     number is given).\n",
	"\n",
	"p or b or Control-H or BackSpace or DELete or PgUp\n",
	"     Moves to the previous page (or back n pages).\n",
	"\n",
	"Up-arrow\n",
	"     Scrolls page up.\n",
	"\n",
	"Down-arrow\n",
	"     Scrolls page down.\n",
	"u\n",
	"     Moves page up two thirds of a window-full.\n",
	"\n",
	"d\n",
	"     Moves page down two thirds of a window-full.\n",
	"\n",
	"Left-arrow\n",
	"     Scrolls page left.\n",
	"\n",
	"Right-arrow\n",
	"     Scrolls page right.\n",
	"\n",
	"l\n",
	"     Moves page left two thirds of a window-full.\n",
	"\n",
	"r\n",
	"     Moves page right two thirds of a window-full.\n",
	"\n",
	"g or j or End\n",
	"     Moves to the page with the given number.  Initially,\n",
	"     the first page is assumed to be page number 1, but this\n",
	"     can be changed with the `P' keystroke, below.  If no\n",
	"     page number is given, then it goes to the last page.\n",
	"\n",
	"<    Move to first page in document.\n",
	">    Move to last page in document.\n",
	"\n",
	NULL
    };

    static _Xconst char *default_help_mousebuttons[] = {
	"\n",
	"Mouse bindings\n",
	"\n",
	"\n",
	"The mouse buttons can be customized just like the keys;\n",
	"however the bindings cannot be intermixed (since\n",
	"a mouse event always requires the pointer location\n",
	"to be present, which a key event doesn't).\n",
        "The default bindings are as follows:\n"
	"\n",
	"Buttons 1-3\n",
	"     Pops up a magnifier window at different sizes.\n",
	"     When the mouse is over a hyperlink, the link overrides\n",
	"     the magnifier. In that case, Button 1 jumps to the link\n",
	"     in the current Xdvi window, Button 2 opens the link target\n",
	"     in a new instance of Xdvik.\n"
	"\n",
	"Shift-Button1 to Shift-Button3\n",
	"     Drag the page in each direction (Button 1), vertically\n",
	"     only (Button 2) or horizontally only (Button 3).\n",
	"\n",
	"Ctrl-Button1\n",
	"     Invoke a reverse search for the text on the cursor\n",
	"     location (see the section SOURCE SPECIALS for more\n",
	"     information on this).\n",
	"\n",
	NULL
    };
    
    static _Xconst char *default_help_sourcespecials[] = {
	"\n",
	"Source Special Commands\n",
	"\n",
	"Some TeX implementations have an option to automatically\n",
	"include so-called `source specials' into a .dvi file. These\n",
	"contain the line number and the filename of the .tex source\n",
	"and make it possible to go from a .dvi file to the\n",
	"corresponding place in the .tex source and back (also called\n",
	"`reverse search' and `forward search').\n",
	"\n",
	"On the TeX side, you need a TeX version or a macro package\n",
	"to insert the specials in the dvi file.\n",
	"\n",
	"Source special mode can be adapted for use with various editors\n",
	"by using the command line option \"-editor\" or one of the\n",
	"environment variables \"XEDITOR\", \"VISUAL\" or \"EDITOR\".\n",
	"See the xdvi man page on the \"-editor\" option for details\n",
	"and examples.\n",
	"\n",
	"Forward search can be performed by a program (most probably\n",
	"your editor) calling xdvi with the \"-sourceposition\" option\n",
	"like this:\n",
	"xdvi -sourceposition \"<line> <filename>\" <main file>\n",
	"If there is already an instance of xdvi running that displays\n",
	"<main file>, it will try to open the page specified by\n",
	"<line> and <filename> an highlight this location on the page.\n",
	"Else, a new instance of xdvi will be started that will try to\n",
	"do the same.\n",
	"\n",
	"The following keybindings are pre-configured:\n",
	"\n",
	"Control-Mouse1\n",
	"     [source-special()] Invoke the editor (the value\n",
	"     of the \"editor\" resource ) to display the line in the\n",
	"     .tex file corresponding to special at cursor position.\n",
	"\n",
	"Control-v\n",
	"     [show-source-specials()]  Show bounding boxes for every\n",
	"     source special on the current page, and print the strings\n",
	"     contained in these specials to  stderr. With prefix 1,\n",
	"     show every bounding box on the page (for debugging purposes).\n",
	"\n",
	"Control-x\n",
	"     [source-what-special()]  Display information about the\n",
	"     source special next to the cursor, similar to\n",
	"     \"source-special()\", but without actually invoking\n",
	"     the editor (for debugging purposes).\n",
	"\n",
	NULL
    };

    struct widget_info {
	char *name;	/* name of widget */
	char *title_string;	/* string used for menu entry; NULL if widget is not a menu entry */
	char *resource_string;	/* value of X resource */
	_Xconst char **default_resource_string;	/* fallback if resource is unspecified */
    } widget_info_list[WIDGET_INFO_LIST_LEN];

    /*
     * store info on all widgets into array so that we can initialize them within a loop;
     * `name' == NULL indicates end of array
     * Aaarghh! K&R C doesn't allow initializing automatic structures; OTOH,
     * the struct can't be fixed either, since the resource strings are only known at
     * run-time. Hence the following kludge:
     */

    int k = 0;

    widget_info_list[k].name = "helpIntro";
    widget_info_list[k].title_string = NULL;
    widget_info_list[k].resource_string = help_intro;
    widget_info_list[k].default_resource_string = default_help_intro;

    k++;
	
    widget_info_list[k].name = "helpGeneral";
    widget_info_list[k].title_string = help_general_menulabel;
    widget_info_list[k].resource_string = help_general;
    widget_info_list[k].default_resource_string = default_help_general;

    k++;
	
    widget_info_list[k].name = "helpPagemotion";
    widget_info_list[k].title_string = help_pagemotion_menulabel;
    widget_info_list[k].resource_string = help_pagemotion;
    widget_info_list[k].default_resource_string = default_help_pagemotion;

    k++;
	
    widget_info_list[k].name = "helpOthercommands";
    widget_info_list[k].title_string = help_othercommands_menulabel;
    widget_info_list[k].resource_string = help_othercommands;
    widget_info_list[k].default_resource_string = default_help_othercommands;

    k++;
	
#ifdef HTEX
    widget_info_list[k].name = "helpHypertex";
    widget_info_list[k].title_string = help_hypertex_menulabel;
    widget_info_list[k].resource_string = help_hypertex;
    widget_info_list[k].default_resource_string = default_help_hypertex;

    k++;
#endif
	
    widget_info_list[k].name = "helpMousebuttons";
    widget_info_list[k].title_string = help_mousebuttons_menulabel;
    widget_info_list[k].resource_string = help_mousebuttons;
    widget_info_list[k].default_resource_string = default_help_mousebuttons;

    k++;
	
    widget_info_list[k].name = "helpSourcespecials";
    widget_info_list[k].title_string = help_sourcespecials_menulabel;
    widget_info_list[k].resource_string = help_sourcespecials;
    widget_info_list[k].default_resource_string = default_help_sourcespecials;

    k++;
	
    widget_info_list[k].name = NULL;
    widget_info_list[k].title_string = NULL;
    widget_info_list[k].resource_string = NULL;
    widget_info_list[k].default_resource_string = NULL;

    if (k >= WIDGET_INFO_LIST_LEN)
	oops("BUG: k (%d) not smaller than WIDGET_INFO_LIST_LEN (%d)!", k, WIDGET_INFO_LIST_LEN);

#ifdef TOOLKIT
    if (!help_created) {
	/* called 1st time; create all widgets. Note: widgets are not
	 * destroyed by popdown_help_callback. */
	help = XtVaCreatePopupShell("helpwindow", transientShellWidgetClass, toplevel,
				    XtNx, 60,
				    XtNy, 80,
				    XtNvisual, our_visual,
				    XtNcolormap, our_colormap,
				    NULL);

	helppaned = XtCreateManagedWidget("helppaned", panedWidgetClass, help, NULL, 0);
	helppanel = XtCreateManagedWidget("helppanel", formWidgetClass, helppaned, NULL, 0);

	topicsbutton = XtVaCreateManagedWidget(help_topics_button_label, menuButtonWidgetClass, helppanel,
					       XtNtop, XtChainTop,
					       XtNbottom, XtChainBottom,
					       XtNleft, XtChainLeft,
					       XtNright, XtChainLeft,
					       XtNmenuName, "topicsmenu",
					       NULL);
	topicsmenu = XtCreatePopupShell("topicsmenu", simpleMenuWidgetClass, topicsbutton,
					NULL, 0);

	helpclose = XtVaCreateManagedWidget(help_quit_button_label, commandWidgetClass, helppanel,
					    XtNaccelerators, accelerators,
					    XtNtop, XtChainTop,
					    XtNfromHoriz, topicsbutton,
					    XtNbottom, XtChainBottom,
					    XtNleft, XtChainRight,
					    XtNright, XtChainRight,
					    XtNjustify, XtJustifyRight,
					    NULL);
	XtAddCallback(helpclose, XtNcallback, popdown_help_callback, NULL);
	XtInstallAccelerators(helppanel, helpclose);

	/*
	 * Initialize remaining widgets: loop through widget array and check
	 * if resources are set; use the default strings otherwise.
	 * Create widgets for menu entries.
	 */

	for (j = 0; widget_info_list[j].name != NULL; j++) {
	    if (widget_info_list[j].resource_string == NULL) {
		/*
		 * resource not set; loop through the `default_*' strings
		 * and copy them into malloc'ed resource:
		 */
		size = alloc_len = 0;
		for (i = 0; widget_info_list[j].default_resource_string[i]; i++) {
		    offset = size;
		    size += strlen(widget_info_list[j].default_resource_string[i]);
		    /*
		     * allocate chunks of `alloc_step' to avoid frequent calls to malloc.
		     * `alloc_len' is always 1 more than `size', for the terminating '\0'.
		     */
		    while (size + 1 > alloc_len) {
			alloc_len += alloc_step;
			widget_info_list[j].resource_string = xrealloc(widget_info_list[j].resource_string,
								       alloc_len);
		    }
		    memcpy(widget_info_list[j].resource_string + offset,
			   widget_info_list[j].default_resource_string[i],
			   size - offset);
		}
		/* null-terminate string */
		memset(widget_info_list[j].resource_string + size, '\0', 1);
	    }

	    if (widget_info_list[j].title_string != NULL) {
		/* title_string != NULL indicates that widget is a menu entry; create widget for it: */
		w = XtVaCreateManagedWidget(widget_info_list[j].name,
					    smeBSBObjectClass, topicsmenu,
					    XtNlabel, widget_info_list[j].title_string,
					    NULL);
		XtAddCallback(w, XtNcallback, select_help_topic_callback,
			      (XtPointer) widget_info_list[j].resource_string);
	    }
	}

	helptext = XtVaCreateManagedWidget("helptext", asciiTextWidgetClass, helppaned,
					   XtNstring, widget_info_list[0].resource_string,
					   XtNheight, 500,
					   XtNwidth, 500,
					   /* resizing of pane by user isn't needed */
					   XtNshowGrip, False,
					   XtNscrollVertical, XAW_SCROLL_ALWAYS,
					   XtNscrollHorizontal, XawtextScrollNever,
					   XtNeditType, XawtextRead,
					   XtNleftMargin, 5,
					   NULL);
	XawTextDisplayCaret(helptext, False);
	help_created = True;
    }

    /* finally, pop up help window (let the window manager position it) */
    if (help_active) {
	return;
    }
    help_active = 1;
    XtPopup(help, XtGrabNone);

#else /* TOOLKIT */

    /*
     * non-toolkit version simply dumps everything to stdout. Actually
     * it might be argued that giving a pointer to `man xdvi(1)' would be more
     * helpful for users; the only excuse is that it uses the X resource
     * strings, so it's -- at least in theory -- localizeable ...
     */

    for (j = 0; widget_info_list[j].name != NULL; j++) {
	if (widget_info_list[j].title_string != NULL) {
	    fprintf(stdout, "\n-------------------- %s --------------------\n",
		    widget_info_list[j].title_string);

	    /* print X resource string if available, else default string */

	    if (widget_info_list[j].resource_string != NULL) {
		fputs(widget_info_list[j].resource_string, stdout);
	    }
	    else {
		for (i = 0; widget_info_list[j].default_resource_string[i]; i++) {
		    fputs(widget_info_list[j].default_resource_string[i],
			  stdout);
		}
	    }
	}
    }
#endif /* TOOLKIT */
}
