/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	macrosc.h
 *		Global declarations for macros.c.
 */

/* macro definition */
struct macro_def {
	char			*name;
	char			*action;
	struct macro_def	*next;
};
extern struct macro_def *macro_defs;

extern void abort_script();
extern void AnsiText_action();
extern void AsciiField_action();
extern void Ascii_action();
extern void CloseScript_action();
extern void ContinueScript_action();
extern void EbcdicField_action();
extern void Ebcdic_action();
extern void Execute_action();
extern void execute_action_option();
extern void Expect_action();
extern void login_macro();
extern void macros_init();
extern void Macro_action();
extern void macro_command();
extern void PauseScript_action();
extern void ps_set();
extern void Script_action();
extern void peer_script_init();
extern Boolean sms_active();
extern void sms_cancel_login();
extern void sms_connect_wait();
extern void sms_continue();
extern void sms_error();
extern Boolean sms_redirect();
extern void sms_store();
extern void Wait_action();
