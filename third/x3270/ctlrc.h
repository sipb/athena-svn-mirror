/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	ctlrc.h
 *		Global declarations for ctlr.c.
 */

extern void ctlr_aclear();
extern void ctlr_add();
extern void ctlr_add_bg();
extern void ctlr_add_fg();
extern void ctlr_add_gr();
extern void ctlr_altbuffer();
extern Boolean ctlr_any_data();
extern void ctlr_bcopy();
extern void ctlr_changed();
extern void ctlr_clear();
extern void ctlr_connect();
extern void ctlr_erase();
extern void ctlr_erase_all_unprotected();
extern void ctlr_init();
extern void ctlr_read_buffer();
extern void ctlr_read_modified();
extern void ctlr_scroll();
extern void ctlr_shrink();
extern void ctlr_snap_buffer();
extern Boolean ctlr_snap_modes();
extern void ctlr_write();
extern struct ea *fa2ea();
extern Boolean get_bounded_field_attribute();
extern unsigned char *get_field_attribute();
extern void mdt_clear();
extern void mdt_set();
extern int next_unprotected();
extern int process_ds();
extern void ps_process();
extern void set_rows_cols();
extern void ticking_start();
extern void ticking_stop();
extern void toggle_nop();
extern void toggle_showTiming();
