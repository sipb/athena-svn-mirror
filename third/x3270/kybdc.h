/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	kybdc.h
 *		Global declarations for kybd.c.
 */

/* keyboard lock states */
extern unsigned int kybdlock;
#define KL_OERR_MASK		0x000f
#define  KL_OERR_PROTECTED	1
#define  KL_OERR_NUMERIC	2
#define  KL_OERR_OVERFLOW	3
#define	KL_NOT_CONNECTED	0x0010
#define	KL_AWAITING_FIRST	0x0020
#define	KL_OIA_TWAIT		0x0040
#define	KL_OIA_LOCKED		0x0080
#define	KL_DEFERRED_UNLOCK	0x0100
#define KL_ENTER_INHIBIT	0x0200
#define KL_SCROLLED		0x0400

/* actions */
extern void AltCursor_action();
extern void Attn_action();
extern void BackSpace_action();
extern void BackTab_action();
extern void CircumNot_action();
extern void Clear_action();
extern void Compose_action();
extern void CursorSelect_action();
extern void Default_action();
extern void DeleteField_action();
extern void DeleteWord_action();
extern void Delete_action();
extern void Down_action();
extern void Dup_action();
extern void Enter_action();
extern void EraseEOF_action();
extern void EraseInput_action();
extern void Erase_action();
extern void FieldEnd_action();
extern void FieldMark_action();
extern void Flip_action();
extern void Home_action();
extern void ignore_action();
extern void Insert_action();
extern void insert_selection_action();
extern void Keymap_action();
extern void Key_action();
extern void Left2_action();
extern void Left_action();
extern void MonoCase_action();
extern void Newline_action();
extern void NextWord_action();
extern void PA_action();
extern void PF_action();
extern void PreviousWord_action();
extern void Reset_action();
extern void Right2_action();
extern void Right_action();
extern void Shift_action();
extern void String_action();
extern void SysReq_action();
extern void Tab_action();
extern void ToggleInsert_action();
extern void ToggleReverse_action();
extern void Up_action();

/* other functions */
extern void add_xk();
extern void clear_xks();
extern void do_reset();
extern int emulate_input();
extern void enq_ta();
extern void kybdlock_clr();
extern void kybdlock_set();
extern void kybd_connect();
extern void kybd_inhibit();
extern int kybd_prime();
extern void kybd_scroll_lock();
extern XtTranslations lookup_tt();
extern Boolean run_ta();
extern int state_from_keymap();
