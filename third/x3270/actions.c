/*
 * Modifications Copyright 1993, 1994, 1995 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *  All Rights Reserved.  GTRC hereby grants public use of this software.
 *  Derivative works based on this software must incorporate this copyright
 *  notice.
 */

/*
 *	actions.c
 *		The X actions table and action debugging code.
 */

#include "globals.h"
#include "appres.h"

#include "actionsc.h"
#include "ftc.h"
#include "kybdc.h"
#include "macrosc.h"
#include "menubarc.h"
#include "popupsc.h"
#include "printc.h"
#include "screenc.h"
#include "selectc.h"
#include "trace_dsc.h"

XtActionsRec actions[] = {
	{ "AltCursor",  	(XtActionProc)AltCursor_action },
	{ "AnsiText",		(XtActionProc)AnsiText_action },
	{ "Ascii",		(XtActionProc)Ascii_action },
	{ "AsciiField",		(XtActionProc)AsciiField_action },
	{ "Attn",		(XtActionProc)Attn_action },
	{ "BackSpace",		(XtActionProc)BackSpace_action },
	{ "BackTab",		(XtActionProc)BackTab_action },
	{ "CircumNot",		(XtActionProc)CircumNot_action },
	{ "Clear",		(XtActionProc)Clear_action },
	{ "CloseScript",	(XtActionProc)CloseScript_action },
	{ "Compose",		(XtActionProc)Compose_action },
	{ "Configure",		(XtActionProc)Configure_action },
	{ "Connect",		(XtActionProc)Connect_action },
	{ "CursorSelect",	(XtActionProc)CursorSelect_action },
	{ "Cut",		(XtActionProc)Cut_action },
	{ "Default",		(XtActionProc)Default_action },
	{ "Delete", 		(XtActionProc)Delete_action },
	{ "DeleteField",	(XtActionProc)DeleteField_action },
	{ "DeleteWord",		(XtActionProc)DeleteWord_action },
	{ "Disconnect",		(XtActionProc)Disconnect_action },
	{ "Down",		(XtActionProc)Down_action },
	{ "Dup",		(XtActionProc)Dup_action },
	{ "Ebcdic",		(XtActionProc)Ebcdic_action },
	{ "EbcdicField",	(XtActionProc)EbcdicField_action },
	{ "Enter",		(XtActionProc)Enter_action },
	{ "EnterLeave",		(XtActionProc)EnterLeave_action },
	{ "Erase",		(XtActionProc)Erase_action },
	{ "EraseEOF",		(XtActionProc)EraseEOF_action },
	{ "EraseInput",		(XtActionProc)EraseInput_action },
	{ "Execute",		(XtActionProc)Execute_action },
	{ "Expect",		(XtActionProc)Expect_action },
	{ "FieldEnd",		(XtActionProc)FieldEnd_action },
	{ "FieldMark",		(XtActionProc)FieldMark_action },
	{ "Flip",		(XtActionProc)Flip_action },
	{ "FocusEvent",		(XtActionProc)FocusEvent_action },
	{ "GraphicsExpose",	(XtActionProc)GraphicsExpose_action },
	{ "HandleMenu",		(XtActionProc)HandleMenu_action },
	{ "HardPrint",		(XtActionProc)PrintText_action },
	{ "Home",		(XtActionProc)Home_action },
	{ "Info",		(XtActionProc)Info_action },
	{ "Insert",		(XtActionProc)Insert_action },
	{ "Key",		(XtActionProc)Key_action },
	{ "Keymap",		(XtActionProc)Keymap_action },
	{ "KeymapEvent",	(XtActionProc)KeymapEvent_action },
	{ "Left",		(XtActionProc)Left_action },
	{ "Left2", 		(XtActionProc)Left2_action },
	{ "MonoCase",		(XtActionProc)MonoCase_action },
	{ "MoveCursor",		(XtActionProc)MoveCursor_action },
	{ "Newline",		(XtActionProc)Newline_action },
	{ "NextWord",		(XtActionProc)NextWord_action },
	{ "PA",			(XtActionProc)PA_action },
	{ "PF",			(XtActionProc)PF_action },
	{ "PreviousWord",	(XtActionProc)PreviousWord_action },
	{ "PrintText",		(XtActionProc)PrintText_action },
	{ "PrintWindow",	(XtActionProc)PrintWindow_action },
	{ "Quit",		(XtActionProc)Quit_action },
	{ "Reconnect",		(XtActionProc)Reconnect_action },
	{ "Redraw",		(XtActionProc)Redraw_action },
	{ "Reset",		(XtActionProc)Reset_action },
	{ "Right",		(XtActionProc)Right_action },
	{ "Right2",		(XtActionProc)Right2_action },
	{ "Script",		(XtActionProc)Script_action },
	{ "SetFont",		(XtActionProc)SetFont_action },
	{ "Shift",		(XtActionProc)Shift_action },
	{ "StateChanged",	(XtActionProc)StateChanged_action },
	{ "String",		(XtActionProc)String_action },
	{ "SysReq",		(XtActionProc)SysReq_action },
	{ "Tab",		(XtActionProc)Tab_action },
	{ "ToggleInsert",	(XtActionProc)ToggleInsert_action },
	{ "ToggleReverse",	(XtActionProc)ToggleReverse_action },
	{ "Up",			(XtActionProc)Up_action },
	{ "Visible",		(XtActionProc)Visible_action },
	{ "WMProtocols",	(XtActionProc)WMProtocols_action },
	{ "Wait",		(XtActionProc)Wait_action },
	{ "confirm",		(XtActionProc)confirm_action },
	{ "dialog-next",	(XtActionProc)dialog_next_action },
	{ "dialog-focus",	(XtActionProc)dialog_focus_action },
	{ "ignore",		(XtActionProc)ignore_action },
	{ "insert-selection",	(XtActionProc)insert_selection_action },
	{ "move-select",	(XtActionProc)move_select_action },
	{ "select-end",		(XtActionProc)select_end_action },
	{ "select-extend",	(XtActionProc)select_extend_action },
	{ "select-start",	(XtActionProc)select_start_action },
	{ "set-select",		(XtActionProc)set_select_action },
	{ "start-extend",	(XtActionProc)start_extend_action }
};

int actioncount = XtNumber(actions);

enum iaction ia_cause;
char *ia_name[] = {
	"String", "Paste", "Screen redraw", "Keypad", "Default", "Key",
	"Macro", "Script", "Peek", "Typeahead", "File transfer"
};

/*
 * Return a name for an action.
 */
char *
action_name(action)
XtActionProc action;
{
	register int i;

	for (i = 0; i < actioncount; i++)
		if (actions[i].proc == (XtActionProc)action)
			return actions[i].string;
	return "(unknown)";
}

static char *
key_state(state)
unsigned int state;
{
	static char rs[64];
	char *comma = "";
	static struct {
		char *name;
		unsigned int mask;
	} keymask[] = {
		{ "Shift", ShiftMask },
		{ "Lock", LockMask },
		{ "Control", ControlMask },
		{ "Mod1", Mod1Mask },
		{ "Mod2", Mod2Mask },
		{ "Mod3", Mod3Mask },
		{ "Mod4", Mod4Mask },
		{ "Mod5", Mod5Mask },
		{ "Button1", Button1Mask },
		{ "Button2", Button2Mask },
		{ "Button3", Button3Mask },
		{ "Button4", Button4Mask },
		{ "Button5", Button5Mask },
		{ CN, 0 },
	};
	int i;

	rs[0] = '\0';
	for (i = 0; keymask[i].name; i++)
		if (state & keymask[i].mask) {
			(void) strcat(rs, comma);
			(void) strcat(rs, keymask[i].name);
			comma = "|";
			state &= ~keymask[i].mask;
		}
	if (!rs[0])
		(void) sprintf(rs, "%d", state);
	else if (state)
		(void) sprintf(strchr(rs, '\0'), "|%d", state);
	return rs;
}

/*
 * Check the number of argument to an action, and possibly pop up a usage
 * message.
 *
 * Returns 0 if the argument count is correct, -1 otherwise.
 */
int
check_usage(action, nargs, nargs_min, nargs_max)
XtActionProc action;
Cardinal nargs;
Cardinal nargs_min;
Cardinal nargs_max;
{
	if (nargs >= nargs_min && nargs <= nargs_max)
		return 0;
	if (nargs_min == nargs_max)
		popup_an_error("%s() requires %d argument%s",
		    action_name(action), nargs_min, nargs_min == 1 ? "" : "s");
	else
		popup_an_error("%s() requires %d or %d arguments",
		    action_name(action), nargs_min, nargs_max);
	return -1;
}

/*
 * Display an action debug message
 */
#define KSBUF	256
void
action_debug(action, event, params, num_params)
void (*action)();
XEvent *event;
String *params;
Cardinal *num_params;
{
	int i;
	XKeyEvent *kevent;
	KeySym ks;
	XButtonEvent *bevent;
	XMotionEvent *mevent;
	XConfigureEvent *cevent;
	XClientMessageEvent *cmevent;
	XExposeEvent *exevent;
	char *press = "Press";
	char dummystr[KSBUF+1];
	char *atom_name;

	if (!toggled(EVENT_TRACE))
		return;
	if (!event) {
		(void) fprintf(tracef, " %s", ia_name[(int)ia_cause]);
	} else switch (event->type) {
	    case KeyRelease:
		press = "Release";
	    case KeyPress:
		kevent = (XKeyEvent *)event;
		(void) XLookupString(kevent, dummystr, KSBUF, &ks, NULL);
		(void) fprintf(tracef,
		    "Key%s [state %s, keycode %d, keysym 0x%lx \"%s\"]",
			    press, key_state(kevent->state),
			    kevent->keycode, ks,
			    ks == NoSymbol ? "NoSymbol" : XKeysymToString(ks));
		break;
	    case ButtonRelease:
		press = "Release";
	    case ButtonPress:
		bevent = (XButtonEvent *)event;
		(void) fprintf(tracef,
		    "Button%s [state %s, button %d]",
		    press, key_state(bevent->state),
		    bevent->button);
		break;
	    case MotionNotify:
		mevent = (XMotionEvent *)event;
		(void) fprintf(tracef,
		    "MotionNotify [state %s]", key_state(mevent->state));
		break;
	    case EnterNotify:
		(void) fprintf(tracef, "EnterNotify");
		break;
	    case LeaveNotify:
		(void) fprintf(tracef, "LeaveNotify");
		break;
	    case FocusIn:
		(void) fprintf(tracef, "FocusIn");
		break;
	    case FocusOut:
		(void) fprintf(tracef, "FocusOut");
		break;
	    case KeymapNotify:
		(void) fprintf(tracef, "KeymapNotify");
		break;
	    case Expose:
		exevent = (XExposeEvent *)event;
		(void) fprintf(tracef, "Expose [%dx%d+%d+%d]",
		    exevent->width, exevent->height, exevent->x, exevent->y);
		break;
	    case PropertyNotify:
		(void) fprintf(tracef, "PropertyNotify");
		break;
	    case ClientMessage:
		cmevent = (XClientMessageEvent *)event;
		atom_name = XGetAtomName(display, (Atom)cmevent->data.l[0]);
		(void) fprintf(tracef, "ClientMessage [%s]",
		    (atom_name == CN) ? "(unknown)" : atom_name);
		break;
	    case ConfigureNotify:
		cevent = (XConfigureEvent *)event;
		(void) fprintf(tracef, "ConfigureNotify [%dx%d+%d+%d]",
		    cevent->width, cevent->height, cevent->x, cevent->y);
		break;
	    default:
		(void) fprintf(tracef, "Event %d", event->type);
		break;
	}
	(void) fprintf(tracef, " -> %s(", action_name((XtActionProc)action));
	for (i = 0; i < *num_params; i++) {
		Boolean quoted;

		quoted = (strchr(params[i], ' ')  != CN ||
			  strchr(params[i], '\t') != CN);
		(void) fprintf(tracef, "%s%s%s%s",
		    i ? ", " : "",
		    quoted ? "\"" : "",
		    params[i],
		    quoted ? "\"" : "");
	}
	(void) fprintf(tracef, ")\n");
}

/*
 * Wrapper for calling an action internally.
 */
void
action_internal(fn, cause, parm1, parm2)
void (*fn)();
enum iaction cause;
char *parm1;
char *parm2;
{
	Cardinal count = 0;
	String parms[2];

	parms[0] = parm1;
	parms[1] = parm2;
	if (parm1 != CN) {
		if (parm2 != CN)
			count = 2;
		else
			count = 1;
	} else
		count = 0;

	ia_cause = cause;
	(*fn)((Widget) NULL, (XEvent *) NULL, count ? parms : (String *) NULL,
	    &count);
}
