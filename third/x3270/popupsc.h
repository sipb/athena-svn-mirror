/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	popupsc.h
 *		Global declarations for popups.c.
 */

/* window placement enumeration */
enum placement { Center, Bottom, Left, Right };
extern enum kp_placement {
	kp_right, kp_left, kp_bottom, kp_integral
} kp_placement;
extern enum placement *CenterP;
extern enum placement *BottomP;
extern enum placement *LeftP;
extern enum placement *RightP;

extern void confirm_action();
extern Widget create_form_popup();
extern void error_popup_init();
extern void Info_action();
extern void info_popup_init();
extern void place_popup();
#if defined(__STDC__)
extern void popup_an_info(char *fmt, ...);
extern void popup_an_errno(int err, char *fmt, ...);
extern void popup_an_error(char *fmt, ...);
#else
extern void popup_an_info();
extern void popup_an_errno();
extern void popup_an_error();
#endif
extern void popup_popup();
extern void toplevel_geometry();
