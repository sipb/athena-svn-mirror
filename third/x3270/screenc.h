/*
 * Copyright 1995, 1996 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	screenc.h
 *		Global declarations for screen.c.
 */

extern void aicon_font_init();
extern void aicon_size();
extern void blink_start();
extern void Configure_action();
extern void cursor_move();
extern void do_toggle();
extern void enable_cursor();
extern void EnterLeave_action();
extern void FocusEvent_action();
extern void GraphicsExpose_action();
extern void initialize_toggles();
extern void KeymapEvent_action();
extern char *load_fixed_font();
extern void mcursor_locked();
extern void mcursor_normal();
extern void mcursor_waiting();
extern void Quit_action();
extern void Redraw_action();
extern void ring_bell();
extern void save_00translations();
extern void screen_change_model();
extern void screen_connect();
extern void screen_disp();
extern void screen_flip();
extern void screen_init();
extern void screen_newcharset();
extern void screen_newfont();
extern void screen_newscheme();
extern Boolean screen_obscured();
extern void screen_scroll();
extern void screen_set_keymap();
extern void screen_set_temp_keymap();
extern void screen_set_thumb();
extern void screen_showikeypad();
extern void SetFont_action();
extern void set_aicon_label();
extern void set_font_globals();
extern void set_translations();
extern void shift_event();
extern void shutdown_toggles();
extern void StateChanged_action();
extern void Visible_action();
extern void WMProtocols_action();
