/*
 * Modifications Copyright 1993, 1994, 1995, 1996 by Paul Mattes.
 * Copyright 1990 by Jeff Sparkes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	appres.h
 *		Application resource definitions for x3270.
 */

/* Toggles */

enum toggle_type { TT_INITIAL, TT_TOGGLE, TT_FINAL };
struct toggle {
	Boolean value;		/* toggle value */
	Boolean changed;	/* has the value changed since init */
	Widget w[2];		/* the menu item widgets */
	char *label[2];		/* labels */
	void (*upcall)();	/* change value */
};
#define MONOCASE	0
#define ALT_CURSOR	1
#define CURSOR_BLINK	2
#define SHOW_TIMING	3
#define CURSOR_POS	4
#define DS_TRACE	5
#define SCROLL_BAR	6
#define LINE_WRAP	7
#define BLANK_FILL	8
#define SCREEN_TRACE	9
#define EVENT_TRACE	10
#define MARGINED_PASTE	11
#define RECTANGLE_SELECT 12

#define N_TOGGLES	13

#define toggled(ix)		(appres.toggle[ix].value)
#define toggle_toggle(t) \
	{ (t)->value = !(t)->value; (t)->changed = True; }

/* Application resources */

typedef struct {
	/* Basic colors */
	Pixel	foreground;
	Pixel	background;

	/* Options (not toggles) */
	Boolean mono;
	Boolean extended;
	Boolean m3279;
	Boolean visual_bell;
	Boolean	keypad_on;
	Boolean menubar;
	Boolean active_icon;
	Boolean label_icon;
	Boolean	apl_mode;
	Boolean	once;
	Boolean invert_kpshift;
	Boolean scripted;
	Boolean modified_sel;
	Boolean use_cursor_color;
	Boolean reconnect;
	Boolean do_confirms;
	Boolean numeric_lock;
	Boolean allow_resize;
	Boolean secure;
	Boolean no_other;
	Boolean oerr_lock;
	Boolean	typeahead;
	Boolean debug_tracing;
	Boolean attn_lock;

	/* Named resources */
	char	*keypad;
	char	*efontname;
	char	*afontname;
	char	*model;
	char	*key_map;
	char	*compose_map;
	char	*hostsfile;
	char	*port;
	char	*charset;
	char	*termname;
	char	*font_list;
	char	*debug_font;
	char	*icon_font;
	char	*icon_label_font;
	char	*macros;
	char	*trace_dir;
	int	save_lines;
	char	*normal_name;
	char	*select_name;
	char	*bold_name;
	char	*colorbg_name;
	char	*keypadbg_name;
	char	*selbg_name;
	char	*cursor_color_name;
	char    *color_scheme;
	int	bell_volume;
	char	*oversize;
	char	*char_class;
	char	*ft_command;
	int	modified_sel_color;

	/* Toggles */
	struct toggle toggle[N_TOGGLES];

	/* Simple widget resources */
	XtTranslations	base_translations;
	Cursor	wait_mcursor;
	Cursor	locked_mcursor;

	/* Line-mode TTY parameters */
	Boolean	icrnl;
	Boolean	inlcr;
	char	*erase;
	char	*kill;
	char	*werase;
	char	*rprnt;
	char	*lnext;
	char	*intr;
	char	*quit;
	char	*eof;

#if defined(USE_APP_DEFAULTS) /*[*/
	/* App-defaults version */
	char	*ad_version;
#endif /*]*/

} AppRes, *AppResptr;

extern AppRes appres;
