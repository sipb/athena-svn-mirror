/* $Header: /afs/dev.mit.edu/source/repository/third/mh/miscellany/patch-2.0.12u8/pch.h,v 1.1.1.1 1996-10-07 07:13:17 ghudson Exp $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0.1.1  87/01/30  22:47:16  lwall
 * Added do_ed_script().
 * 
 * Revision 2.0  86/09/17  15:39:57  lwall
 * Baseline for netwide release.
 * 
 */

EXT FILE *pfp INIT(Nullfp);		/* patch file pointer */

void re_patch();
void open_patch_file();
void set_hunkmax();
void grow_hunkmax();
bool there_is_another_patch();
int intuit_diff_type();
void next_intuit_at();
void skip_to();
bool another_hunk();
bool pch_swap();
char *pfetch();
short pch_line_len();
LINENUM pch_first();
LINENUM pch_ptrn_lines();
LINENUM pch_newfirst();
LINENUM pch_repl_lines();
LINENUM pch_end();
LINENUM pch_context();
LINENUM pch_hunk_beg();
char pch_char();
char *pfetch();
char *pgets();
void do_ed_script();
