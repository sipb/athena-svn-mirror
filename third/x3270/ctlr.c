/*
 * Modifications Copyright 1993, 1994, 1995, 1996 by Paul Mattes.
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
 *	ctlr.c
 *		This module handles interpretation of the 3270 data stream and
 *		maintenance of the 3270 device state.  It was split out from
 *		screen.c, which handles X operations.
 *
 */

#include "globals.h"
#include <errno.h>
#include "3270ds.h"
#include "appres.h"
#include "ctlr.h"
#include "screen.h"
#include "cg.h"
#include "resources.h"

#include "ctlrc.h"
#include "ft_cutc.h"
#include "ftc.h"
#include "kybdc.h"
#include "macrosc.h"
#include "popupsc.h"
#include "screenc.h"
#include "scrollc.h"
#include "selectc.h"
#include "sfc.h"
#include "statusc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"

/* Externals: kybd.c */
extern unsigned char aid;

/* Globals */
int             ROWS, COLS;
int             maxROWS, maxCOLS;
int             cursor_addr, buffer_addr;
Boolean         screen_alt = True;	/* alternate screen? */
Boolean         is_altbuffer = False;
unsigned char  *screen_buf;	/* 3270 display buffer */
struct ea      *ea_buf;		/* 3270 extended attribute buffer */
Boolean         formatted = False;	/* set in screen_disp */
Boolean         screen_changed = False;
int             first_changed = -1;
int             last_changed = -1;
unsigned char   reply_mode = SF_SRM_FIELD;
int             crm_nattr = 0;
unsigned char   crm_attr[16];

/* Statics */
static unsigned char *ascreen_buf;	/* alternate 3270 display buffer */
static struct ea *aea_buf;	/* alternate 3270 extended attribute buffer */
static unsigned char *zero_buf;	/* empty buffer, for area clears */
static void     set_formatted();
static void     ctlr_blanks();
static unsigned char fake_fa;
static struct ea fake_ea;
static Boolean  trace_primed = False;
static unsigned char default_fg;
static unsigned char default_gr;
static unsigned char default_cs;

/*
 * code_table is used to translate buffer addresses and attributes to the 3270
 * datastream representation
 */
static unsigned char	code_table[64] = {
	0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
	0xC8, 0xC9, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
	0xD8, 0xD9, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
	0xE8, 0xE9, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
	0xF8, 0xF9, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
};

#define IsBlank(c)	((c == CG_null) || (c == CG_space))

#define ALL_CHANGED	{ \
	screen_changed = True; \
	if (IN_ANSI) { first_changed = 0; last_changed = ROWS*COLS; } }
#define REGION_CHANGED(f, l)	{ \
	screen_changed = True; \
	if (IN_ANSI) { \
	    if (first_changed == -1 || f < first_changed) first_changed = f; \
	    if (last_changed == -1 || l > last_changed) last_changed = l; } }
#define ONE_CHANGED(n)	REGION_CHANGED(n, n+1)

#define DECODE_BADDR(c1, c2) \
	((((c1) & 0xC0) == 0x00) ? \
	(((c1) & 0x3F) << 8) | (c2) : \
	(((c1) & 0x3F) << 6) | ((c2) & 0x3F))

#define ENCODE_BADDR(ptr, addr) { \
	if ((addr) > 0xfff) { \
		*(ptr)++ = ((addr) >> 8) & 0x3F; \
		*(ptr)++ = (addr) & 0xFF; \
	} else { \
		*(ptr)++ = code_table[((addr) >> 6) & 0x3F]; \
		*(ptr)++ = code_table[(addr) & 0x3F]; \
	} \
    }

/*
 * Initialize the emulated 3270 hardware.
 */
void
ctlr_init(keep_contents, model_changed)
Boolean keep_contents;
Boolean model_changed;
{
	/* Allocate buffers */

	if (model_changed) {
		if (screen_buf)
			XtFree((char *)screen_buf);
		screen_buf = (unsigned char *)XtCalloc(sizeof(unsigned char),
		    maxROWS * maxCOLS);
		if (ea_buf)
			XtFree((char *)ea_buf);
		ea_buf = (struct ea *)XtCalloc(sizeof(struct ea),
		    maxROWS * maxCOLS);
		if (ascreen_buf)
			XtFree((char *)ascreen_buf);
		ascreen_buf = (unsigned char *)XtCalloc(sizeof(unsigned char),
		    maxROWS * maxCOLS);
		if (aea_buf)
			XtFree((char *)aea_buf);
		aea_buf = (struct ea *)XtCalloc(sizeof(struct ea),
		    maxROWS * maxCOLS);
		if (zero_buf)
			XtFree((char *)zero_buf);
		zero_buf = (unsigned char *)XtCalloc(sizeof(unsigned char),
		    maxROWS * maxCOLS);
	}

	if (!keep_contents) {
		cursor_addr = 0;
		buffer_addr = 0;
	}
}

/*
 * Deal with the relationships between model numbers and rows/cols.
 */
void
set_rows_cols(mn, ovc, ovr)
int mn;
int ovc, ovr;
{
	int defmod;

	switch (mn) {
	case 2:
		maxCOLS = COLS = 80;
		maxROWS = ROWS = 24; 
		model_num = 2;
		break;
	case 3:
		maxCOLS = COLS = 80;
		maxROWS = ROWS = 32; 
		model_num = 3;
		break;
	case 4:
#if defined(RESTRICT_3279) /*[*/
		if (appres.m3279) {
			XtWarning("No 3279 Model 4, defaulting to Model 3");
			set_rows_cols("3", ovc, ovr);
			return;
		}
#endif /*]*/
		maxCOLS = COLS = 80;
		maxROWS = ROWS = 43; 
		model_num = 4;
		break;
	case 5:
#if defined(RESTRICT_3279) /*[*/
		if (appres.m3279) {
			XtWarning("No 3279 Model 5, defaulting to Model 3");
			set_rows_cols(3, ovc, ovr);
			return;
		}
#endif /*]*/
		maxCOLS = COLS = 132;
		maxROWS = ROWS = 27; 
		model_num = 5;
		break;
	default:
#if defined(RESTRICT_3279) /*[*/
		defmod = appres.m3279 ? 3 : 4;
#else /*][*/
		defmod = 4;
#endif
		{
			char mnb[2];

			mnb[0] = defmod + '0';
			mnb[1] = '\0';
			xs_warning("Unknown model, defaulting to %s", mnb);
		}
		set_rows_cols(defmod, ovc, ovr);
		return;
	}

	/* Apply oversize. */
	ov_cols = 0;
	ov_rows = 0;
	if (ovc != 0 || ovr != 0) {
		if (ovc <= 0 || ovr <= 0)
			popup_an_error("Invalid %s %dx%d:\nNegative or zero",
			    ResOversize, ovc, ovr);
		else if (ovc * ovr >= 0x4000)
			popup_an_error("Invalid %s %dx%d:\nToo big",
			    ResOversize, ovc, ovr);
		else if (ovc > 0 && ovc < maxCOLS)
			popup_an_error("Invalid %s cols (%d):\nLess than model %d cols (%d)",
			    ResOversize, ovc, model_num, maxCOLS);
		else if (ovr > 0 && ovr < maxROWS)
			popup_an_error("Invalid %s rows (%d):\nLess than model %d rows (%d)",
			    ResOversize, ovr, model_num, maxROWS);
		else {
			ov_cols = maxCOLS = COLS = ovc;
			ov_rows = maxROWS = ROWS = ovr;
		}
	}

	/* Update the model name. */
	(void) sprintf(model_name, "327%c-%d%s",
	    appres.m3279 ? '9' : '8',
	    model_num,
	    appres.extended ? "-E" : "");
}


/*
 * Set the formatted screen flag.  A formatted screen is a screen that
 * has at least one field somewhere on it.
 */
static void
set_formatted()
{
	register int	baddr;

	formatted = False;
	baddr = 0;
	do {
		if (IS_FA(screen_buf[baddr])) {
			formatted = True;
			break;
		}
		INC_BA(baddr);
	} while (baddr != 0);
}

/*
 * Called when a host connects, disconnects, or changes ANSI/3270 modes.
 */
void
ctlr_connect()
{
	if (ever_3270)
		fake_fa = 0xE0;
	else
		fake_fa = 0xC4;

	default_fg = 0x00;
	default_gr = 0x00;
	default_cs = 0x00;
	reply_mode = SF_SRM_FIELD;
	crm_nattr = 0;
}


/*
 * Find the field attribute for the given buffer address.  Return its address
 * rather than its value.
 */
unsigned char *
get_field_attribute(baddr)
register int	baddr;
{
	int	sbaddr;

	if (!formatted)
		return &fake_fa;

	sbaddr = baddr;
	do {
		if (IS_FA(screen_buf[baddr]))
			return &(screen_buf[baddr]);
		DEC_BA(baddr);
	} while (baddr != sbaddr);
	return &fake_fa;
}

/*
 * Find the field attribute for the given buffer address, bounded by another
 * buffer address.  Return the attribute in a parameter.
 *
 * Returns True if an attribute is found, False if boundary hit.
 */
Boolean
get_bounded_field_attribute(baddr, bound, fa_out)
register int	baddr;
register int	bound;
unsigned char	*fa_out;
{
	int	sbaddr;

	if (!formatted) {
		*fa_out = fake_fa;
		return True;
	}

	sbaddr = baddr;
	do {
		if (IS_FA(screen_buf[baddr])) {
			*fa_out = screen_buf[baddr];
			return True;
		}
		DEC_BA(baddr);
	} while (baddr != sbaddr && baddr != bound);

	/* Screen is unformatted (and 'formatted' is inaccurate). */
	if (baddr == sbaddr) {
		*fa_out = fake_fa;
		return True;
	}

	/* Wrapped to boundary. */
	return False;
}

/*
 * Given the address of a field attribute, return the address of the
 * extended attribute structure.
 */
struct ea *
fa2ea(fa)
unsigned char *fa;
{
	if (fa == &fake_fa)
		return &fake_ea;
	else
		return &ea_buf[fa - screen_buf];
}

/*
 * Find the next unprotected field.  Returns the address following the
 * unprotected attribute byte, or 0 if no nonzero-width unprotected field
 * can be found.
 */
int
next_unprotected(baddr0)
int baddr0;
{
	register int baddr, nbaddr;

	nbaddr = baddr0;
	do {
		baddr = nbaddr;
		INC_BA(nbaddr);
		if (IS_FA(screen_buf[baddr]) &&
		    !FA_IS_PROTECTED(screen_buf[baddr]) &&
		    !IS_FA(screen_buf[nbaddr]))
			return nbaddr;
	} while (nbaddr != baddr0);
	return 0;
}

/*
 * Perform an erase command, which may include changing the (virtual) screen
 * size.
 */
void
ctlr_erase(alt)
Boolean alt;
{
	kybd_inhibit(False);

	ctlr_clear(True);

	if (alt == screen_alt)
		return;

	screen_disp();

	if (alt) {
		/* Going from 24x80 to maximum. */
		screen_disp();
		ROWS = maxROWS;
		COLS = maxCOLS;
	} else {
		/* Going from maximum to 24x80. */
		if (maxROWS > 24 || maxCOLS > 80) {
			if (*debugging_font) {
				ctlr_blanks();
				screen_disp();
			}
			ROWS = 24;
			COLS = 80;
		}
	}

	screen_alt = alt;
}


/*
 * Interpret an incoming 3270 command.
 */
int
process_ds(buf, buflen)
unsigned char	*buf;
int	buflen;
{
	if (!buflen)
		return 0;

	scroll_to_bottom();

	trace_ds("< ");

	switch (buf[0]) {	/* 3270 command */
	case CMD_EAU:	/* erase all unprotected */
	case SNA_CMD_EAU:
		trace_ds("EraseAllUnprotected\n");
		ctlr_erase_all_unprotected();
		break;
	case CMD_EWA:	/* erase/write alternate */
	case SNA_CMD_EWA:
		trace_ds("EraseWriteAlternate");
		ctlr_erase(True);
		ctlr_write(buf, buflen, True);
		break;
	case CMD_EW:	/* erase/write */
	case SNA_CMD_EW:
		trace_ds("EraseWrite");
		ctlr_erase(False);
		ctlr_write(buf, buflen, True);
		break;
	case CMD_W:	/* write */
	case SNA_CMD_W:
		trace_ds("Write");
		ctlr_write(buf, buflen, False);
		break;
	case CMD_RB:	/* read buffer */
	case SNA_CMD_RB:
		trace_ds("ReadBuffer\n");
		ctlr_read_buffer(aid);
		break;
	case CMD_RM:	/* read modifed */
	case SNA_CMD_RM:
		trace_ds("ReadModified\n");
		ctlr_read_modified(aid, False);
		break;
	case CMD_RMA:	/* read modifed all */
	case SNA_CMD_RMA:
		trace_ds("ReadModifiedAll\n");
		ctlr_read_modified(aid, True);
		break;
	case CMD_WSF:	/* write structured field */
	case SNA_CMD_WSF:
		trace_ds("WriteStructuredField");
		write_structured_field(buf, buflen);
		break;
	case CMD_NOP:	/* no-op */
		trace_ds("NoOp\n");
		break;
	default:
		/* unknown 3270 command */
		popup_an_error("Unknown 3270 Data Stream command: 0x%X\n",
		    buf[0]);
		return -1;
	}

	return 0;
}

/*
 * Functions to insert SA attributes into the inbound data stream.
 */
static void
insert_sa1(attr, value, currentp, anyp)
unsigned char attr;
unsigned char value;
unsigned char *currentp;
Boolean *anyp;
{
	if (value == *currentp)
		return;
	*currentp = value;
	space3270out(3);
	*obptr++ = ORDER_SA;
	*obptr++ = attr;
	*obptr++ = value;
	if (*anyp)
		trace_ds("'");
	trace_ds(" SetAttribute(%s)", see_efa(attr, value));
	*anyp = False;
}

static void
insert_sa(baddr, current_fgp, current_grp, current_csp, anyp)
int baddr;
unsigned char *current_fgp;
unsigned char *current_grp;
unsigned char *current_csp;
Boolean *anyp;
{
	if (reply_mode != SF_SRM_CHAR)
		return;

	if (memchr((char *)crm_attr, XA_FOREGROUND, crm_nattr))
		insert_sa1(XA_FOREGROUND, ea_buf[baddr].fg, current_fgp, anyp);
	if (memchr((char *)crm_attr, XA_HIGHLIGHTING, crm_nattr)) {
		unsigned char gr;

		gr = ea_buf[baddr].gr;
		if (gr)
			gr |= 0xf0;
		insert_sa1(XA_HIGHLIGHTING, gr, current_grp, anyp);
	}
	if (memchr((char *)crm_attr, XA_CHARSET, crm_nattr)) {
		unsigned char cs;

		cs = ea_buf[baddr].cs & CS_MASK;
		if (cs)
			cs |= 0xf0;
		insert_sa1(XA_CHARSET, cs, current_csp, anyp);
	}

}


/*
 * Process a 3270 Read-Modified command and transmit the data back to the
 * host.
 */
void
ctlr_read_modified(aid_byte, all)
unsigned char aid_byte;
Boolean all;
{
	register int	baddr, sbaddr;
	Boolean		send_data = True;
	Boolean		short_read = False;
	unsigned char	current_fg = 0x00;
	unsigned char	current_gr = 0x00;
	unsigned char	current_cs = 0x00;

	trace_ds("> ");
	obptr = obuf;

	switch (aid_byte) {

	    case AID_SYSREQ:			/* test request */
		space3270out(4);
		*obptr++ = 0x01;	/* soh */
		*obptr++ = 0x5b;	/*  %  */
		*obptr++ = 0x61;	/*  /  */
		*obptr++ = 0x02;	/* stx */
		trace_ds("SYSREQ");
		break;

	    case AID_PA1:			/* short-read AIDs */
	    case AID_PA2:
	    case AID_PA3:
	    case AID_CLEAR:
		short_read = True;
		/* fall through... */

	    case AID_SELECT:			/* No data on READ MODIFIED */
		if (!all)
			send_data = False;
		/* fall through... */

	    default:				/* ordinary AID */
		space3270out(3);
		*obptr++ = aid_byte;
		trace_ds(see_aid(aid_byte));
		if (short_read)
		    goto rm_done;
		ENCODE_BADDR(obptr, cursor_addr);
		trace_ds(rcba(cursor_addr));
		break;
	}

	baddr = 0;
	if (formatted) {
		/* find first field attribute */
		do {
			if (IS_FA(screen_buf[baddr]))
				break;
			INC_BA(baddr);
		} while (baddr != 0);
		sbaddr = baddr;
		do {
			if (FA_IS_MODIFIED(screen_buf[baddr])) {
				Boolean	any = False;

				INC_BA(baddr);
				space3270out(3);
				*obptr++ = ORDER_SBA;
				ENCODE_BADDR(obptr, baddr);
				trace_ds(" SetBufferAddress%s", rcba(baddr));
				while (!IS_FA(screen_buf[baddr])) {
					if (send_data &&
					    screen_buf[baddr]) {
						insert_sa(baddr,
						    &current_fg,
						    &current_gr,
						    &current_cs,
						    &any);
						if (ea_buf[baddr].cs & CS_GE) {
							space3270out(1);
							*obptr++ = ORDER_GE;
							if (any)
								trace_ds("'");
							trace_ds(" GraphicEscape");
							any = False;
						}
						space3270out(1);
						*obptr++ = cg2ebc[screen_buf[baddr]];
						if (!any)
							trace_ds(" '");
						trace_ds(see_ebc(cg2ebc[screen_buf[baddr]]));
						any = True;
					}
					INC_BA(baddr);
				}
				if (any)
					trace_ds("'");
			}
			else {	/* not modified - skip */
				do {
					INC_BA(baddr);
				} while (!IS_FA(screen_buf[baddr]));
			}
		} while (baddr != sbaddr);
	} else {
		Boolean	any = False;

		do {
			if (screen_buf[baddr]) {
				insert_sa(baddr,
				    &current_fg,
				    &current_gr,
				    &current_cs,
				    &any);
				if (ea_buf[baddr].cs & CS_GE) {
					space3270out(1);
					*obptr++ = ORDER_GE;
					if (any)
						trace_ds("' ");
					trace_ds(" GraphicEscape ");
					any = False;
				}
				space3270out(1);
				*obptr++ = cg2ebc[screen_buf[baddr]];
				if (!any)
					trace_ds("'");
				trace_ds(see_ebc(cg2ebc[screen_buf[baddr]]));
				any = True;
			}
			INC_BA(baddr);
		} while (baddr != 0);
		if (any)
			trace_ds("'");
	}

    rm_done:
	trace_ds("\n");
	net_output();
}

/*
 * Calculate the proper 3270 DS value for an internal field attribute.
 */
static unsigned char
calc_fa(fa)
unsigned char fa;
{
	register unsigned char r = 0x00;

	if (FA_IS_PROTECTED(fa))
		r |= 0x20;
	if (FA_IS_NUMERIC(fa))
		r |= 0x10;
	if (FA_IS_MODIFIED(fa))
		r |= 0x01;
	r |= ((fa & FA_INTENSITY) << 2);
	return r;
}

/*
 * Process a 3270 Read-Buffer command and transmit the data back to the
 * host.
 */
void
ctlr_read_buffer(aid_byte)
unsigned char aid_byte;
{
	register int	baddr;
	unsigned char	fa;
	Boolean		any = False;
	int		attr_count;
	unsigned char	current_fg = 0x00;
	unsigned char	current_gr = 0x00;
	unsigned char	current_cs = 0x00;

	trace_ds("> ");
	obptr = obuf;

	space3270out(3);
	*obptr++ = aid_byte;
	ENCODE_BADDR(obptr, cursor_addr);
	trace_ds("%s%s", see_aid(aid_byte), rcba(cursor_addr));

	baddr = 0;
	do {
		if (IS_FA(screen_buf[baddr])) {
			if (reply_mode == SF_SRM_FIELD) {
				space3270out(2);
				*obptr++ = ORDER_SF;
			} else {
				space3270out(4);
				*obptr++ = ORDER_SFE;
				attr_count = obptr - obuf;
				*obptr++ = 1; /* for now */
				*obptr++ = XA_3270;
			}
			fa = calc_fa(screen_buf[baddr]);
			*obptr++ = code_table[fa];
			if (any)
				trace_ds("'");
			trace_ds(" StartField%s%s%s",
			    (reply_mode == SF_SRM_FIELD) ? "" : "Extended",
			    rcba(baddr), see_attr(fa));
			if (reply_mode != SF_SRM_FIELD) {
				if (ea_buf[baddr].fg) {
					space3270out(2);
					*obptr++ = XA_FOREGROUND;
					*obptr++ = ea_buf[baddr].fg;
					trace_ds("%s", see_efa(XA_FOREGROUND,
					    ea_buf[baddr].fg));
					(*(obuf + attr_count))++;
				}
				if (ea_buf[baddr].gr) {
					space3270out(2);
					*obptr++ = XA_HIGHLIGHTING;
					*obptr++ = ea_buf[baddr].gr | 0xf0;
					trace_ds("%s", see_efa(XA_HIGHLIGHTING,
					    ea_buf[baddr].gr | 0xf0));
					(*(obuf + attr_count))++;
				}
				if (ea_buf[baddr].cs & CS_MASK) {
					space3270out(2);
					*obptr++ = XA_CHARSET;
					*obptr++ =
					    (ea_buf[baddr].cs & CS_MASK) | 0xf0;
					trace_ds("%s", see_efa(XA_CHARSET,
					    (ea_buf[baddr].cs & CS_MASK) | 0xf0));
					(*(obuf + attr_count))++;
				}
			}
			any = False;
		} else {
			insert_sa(baddr,
			    &current_fg,
			    &current_gr,
			    &current_cs,
			    &any);
			if (ea_buf[baddr].cs & CS_GE) {
				space3270out(1);
				*obptr++ = ORDER_GE;
				if (any)
					trace_ds("'");
				trace_ds(" GraphicEscape");
				any = False;
			}
			space3270out(1);
			*obptr++ = cg2ebc[screen_buf[baddr]];
			if (cg2ebc[screen_buf[baddr]] <= 0x3f ||
			    cg2ebc[screen_buf[baddr]] == 0xff) {
				if (any)
					trace_ds("'");

				trace_ds(" %s", see_ebc(cg2ebc[screen_buf[baddr]]));
				any = False;
			} else {
				if (!any)
					trace_ds(" '");
				trace_ds(see_ebc(cg2ebc[screen_buf[baddr]]));
				any = True;
			}
		}
		INC_BA(baddr);
	} while (baddr != 0);
	if (any)
		trace_ds("'");

	trace_ds("\n");
	net_output();
}

/*
 * Construct a 3270 command to reproduce the current state of the display.
 */
void
ctlr_snap_buffer()
{
	register int	baddr = 0;
	int		attr_count;
	unsigned char	current_fg = 0x00;
	unsigned char	current_gr = 0x00;
	unsigned char	current_cs = 0x00;
	unsigned char   av;

	obptr = obuf;
	space3270out(2);
	*obptr++ = screen_alt ? CMD_EWA : CMD_EW;
	*obptr++ = code_table[0];

	do {
		if (IS_FA(screen_buf[baddr])) {
			space3270out(4);
			*obptr++ = ORDER_SFE;
			attr_count = obptr - obuf;
			*obptr++ = 1; /* for now */
			*obptr++ = XA_3270;
			*obptr++ = code_table[calc_fa(screen_buf[baddr])];
			if (ea_buf[baddr].fg) {
				space3270out(2);
				*obptr++ = XA_FOREGROUND;
				*obptr++ = ea_buf[baddr].fg;
				(*(obuf + attr_count))++;
			}
			if (ea_buf[baddr].gr) {
				space3270out(2);
				*obptr++ = XA_HIGHLIGHTING;
				*obptr++ = ea_buf[baddr].gr | 0xf0;
				(*(obuf + attr_count))++;
			}
			if (ea_buf[baddr].cs & CS_MASK) {
				space3270out(2);
				*obptr++ = XA_CHARSET;
				*obptr++ = (ea_buf[baddr].cs & CS_MASK) | 0xf0;
				(*(obuf + attr_count))++;
			}
		} else {
			av = ea_buf[baddr].fg;
			if (current_fg != av) {
				current_fg = av;
				space3270out(3);
				*obptr++ = ORDER_SA;
				*obptr++ = XA_FOREGROUND;
				*obptr++ = av;
			}
			av = ea_buf[baddr].gr;
			if (av)
				av |= 0xf0;
			if (current_gr != av) {
				current_gr = av;
				space3270out(3);
				*obptr++ = ORDER_SA;
				*obptr++ = XA_HIGHLIGHTING;
				*obptr++ = av;
			}
			av = ea_buf[baddr].cs & CS_MASK;
			if (av)
				av |= 0xf0;
			if (current_cs != av) {
				current_cs = av;
				space3270out(3);
				*obptr++ = ORDER_SA;
				*obptr++ = XA_CHARSET;
				*obptr++ = av;
			}
			if (ea_buf[baddr].cs & CS_GE) {
				space3270out(1);
				*obptr++ = ORDER_GE;
			}
			space3270out(1);
			*obptr++ = cg2ebc[screen_buf[baddr]];
		}
		INC_BA(baddr);
	} while (baddr != 0);

	space3270out(4);
	*obptr++ = ORDER_SBA;
	ENCODE_BADDR(obptr, cursor_addr);
	*obptr++ = ORDER_IC;
}

/*
 * Construct a 3270 command to reproduce the reply mode.
 * Returns a Boolean indicating if one is necessary.
 */
Boolean
ctlr_snap_modes()
{
	int i;

	if (!IN_3270 || reply_mode == SF_SRM_FIELD)
		return False;

	obptr = obuf;
	space3270out(6 + crm_nattr);
	*obptr++ = CMD_WSF;
	*obptr++ = 0x00;	/* implicit length */
	*obptr++ = 0x00;
	*obptr++ = SF_SET_REPLY_MODE;
	*obptr++ = 0x00;	/* partition 0 */
	*obptr++ = reply_mode;
	if (reply_mode == SF_SRM_CHAR)
		for (i = 0; i < crm_nattr; i++)
			*obptr++ = crm_attr[i];
	return True;
}


/*
 * Process a 3270 Erase All Unprotected command.
 */
void
ctlr_erase_all_unprotected()
{
	register int	baddr, sbaddr;
	unsigned char	fa;
	Boolean		f;

	kybd_inhibit(False);

	ALL_CHANGED;
	if (formatted) {
		/* find first field attribute */
		baddr = 0;
		do {
			if (IS_FA(screen_buf[baddr]))
				break;
			INC_BA(baddr);
		} while (baddr != 0);
		sbaddr = baddr;
		f = False;
		do {
			fa = screen_buf[baddr];
			if (!FA_IS_PROTECTED(fa)) {
				mdt_clear(&screen_buf[baddr]);
				do {
					INC_BA(baddr);
					if (!f) {
						cursor_move(baddr);
						f = True;
					}
					if (!IS_FA(screen_buf[baddr])) {
						ctlr_add(baddr, CG_null, 0);
					}
				} while (!IS_FA(screen_buf[baddr]));
			}
			else {
				do {
					INC_BA(baddr);
				} while (!IS_FA(screen_buf[baddr]));
			}
		} while (baddr != sbaddr);
		if (!f)
			cursor_move(0);
	} else {
		ctlr_clear(True);
	}
	aid = AID_NO;
	do_reset(False);
}



/*
 * Process a 3270 Write command.
 */
void
ctlr_write(buf, buflen, erase)
unsigned char	buf[];
int	buflen;
Boolean erase;
{
	register unsigned char	*cp;
	register int	baddr;
	unsigned char	*current_fa;
	unsigned char	new_attr;
	Boolean		last_cmd;
	Boolean		last_zpt;
	Boolean		wcc_keyboard_restore, wcc_sound_alarm;
	Boolean		ra_ge;
	int		i;
	unsigned char	na;
	int		any_fa;
	unsigned char	efa_fg;
	unsigned char	efa_gr;
	unsigned char	efa_cs;
	char		*paren = "(";
	enum { NONE, ORDER, SBA, TEXT, NULLCH } previous = NONE;

#define END_TEXT0	{ if (previous == TEXT) trace_ds("'"); }
#define END_TEXT(cmd)	{ END_TEXT0; trace_ds(" %s", cmd); }

#define ATTR2FA(attr) \
	(FA_BASE | \
	 (((attr) & 0x20) ? FA_PROTECT : 0) | \
	 (((attr) & 0x10) ? FA_NUMERIC : 0) | \
	 (((attr) & 0x01) ? FA_MODIFY : 0) | \
	 (((attr) >> 2) & FA_INTENSITY))
#define START_FIELDx(fa) { \
			current_fa = &(screen_buf[buffer_addr]); \
			ctlr_add(buffer_addr, fa, 0); \
			ctlr_add_fg(buffer_addr, 0); \
			ctlr_add_gr(buffer_addr, 0); \
			trace_ds(see_attr(fa)); \
			formatted = True; \
		}
#define START_FIELD0	{ START_FIELDx(FA_BASE); }
#define START_FIELD(attr) { \
			new_attr = ATTR2FA(attr); \
			START_FIELDx(new_attr); \
		}

	kybd_inhibit(False);

	if (buflen < 2)
		return;

	default_fg = 0;
	default_gr = 0;
	default_cs = 0;
	trace_primed = True;
	buffer_addr = cursor_addr;
	if (WCC_RESET(buf[1])) {
		if (erase)
			reply_mode = SF_SRM_FIELD;
		trace_ds("%sreset", paren);
		paren = ",";
	}
	wcc_sound_alarm = WCC_SOUND_ALARM(buf[1]);
	if (wcc_sound_alarm) {
		trace_ds("%salarm", paren);
		paren = ",";
	}
	wcc_keyboard_restore = WCC_KEYBOARD_RESTORE(buf[1]);
	if (wcc_keyboard_restore)
		ticking_stop();
	if (wcc_keyboard_restore) {
		trace_ds("%srestore", paren);
		paren = ",";
	}

	if (WCC_RESET_MDT(buf[1])) {
		trace_ds("%sresetMDT", paren);
		paren = ",";
		baddr = 0;
		if (appres.modified_sel)
			ALL_CHANGED;
		do {
			if (IS_FA(screen_buf[baddr])) {
				mdt_clear(&screen_buf[baddr]);
			}
			INC_BA(baddr);
		} while (baddr != 0);
	}
	if (strcmp(paren, "("))
		trace_ds(")");

	last_cmd = True;
	last_zpt = False;
	current_fa = get_field_attribute(buffer_addr);
	for (cp = &buf[2]; cp < (buf + buflen); cp++) {
		switch (*cp) {
		case ORDER_SF:	/* start field */
			END_TEXT("StartField");
			if (previous != SBA)
				trace_ds(rcba(buffer_addr));
			previous = ORDER;
			cp++;		/* skip field attribute */
			START_FIELD(*cp);
			ctlr_add_fg(buffer_addr, 0);
			INC_BA(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_SBA:	/* set buffer address */
			cp += 2;	/* skip buffer address */
			buffer_addr = DECODE_BADDR(*(cp-1), *cp);
			END_TEXT("SetBufferAddress");
			previous = SBA;
			trace_ds(rcba(buffer_addr));
			if (buffer_addr >= COLS * ROWS) {
				trace_ds(" [invalid address, write command terminated]\n");
				return;
			}
			current_fa = get_field_attribute(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_IC:	/* insert cursor */
			END_TEXT("InsertCursor");
			if (previous != SBA)
				trace_ds(rcba(buffer_addr));
			previous = ORDER;
			cursor_move(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_PT:	/* program tab */
			END_TEXT("ProgramTab");
			previous = ORDER;
			/*
			 * If the buffer address is the field attribute of
			 * of an unprotected field, simply advance one
			 * position.
			 */
			if (IS_FA(screen_buf[buffer_addr]) &&
			    !FA_IS_PROTECTED(screen_buf[buffer_addr])) {
				INC_BA(buffer_addr);
				last_zpt = False;
				last_cmd = True;
				break;
			}
			/*
			 * Otherwise, advance to the first position of the
			 * next unprotected field.
			 */
			baddr = next_unprotected(buffer_addr);
			if (baddr < buffer_addr)
				baddr = 0;
			/*
			 * Null out the remainder of the current field -- even
			 * if protected -- if the PT doesn't follow a command
			 * or order, or (honestly) if the last order we saw was
			 * a null-filling PT that left the buffer address at 0.
			 */
			if (!last_cmd || last_zpt) {
				trace_ds("(nulling)");
				while ((buffer_addr != baddr) &&
				       (!IS_FA(screen_buf[buffer_addr]))) {
					ctlr_add(buffer_addr, CG_null, 0);
					INC_BA(buffer_addr);
				}
				if (baddr == 0)
					last_zpt = True;

			} else
				last_zpt = False;
			buffer_addr = baddr;
			last_cmd = True;
			break;
		case ORDER_RA:	/* repeat to address */
			END_TEXT("RepeatToAddress");
			cp += 2;	/* skip buffer address */
			baddr = DECODE_BADDR(*(cp-1), *cp);
			trace_ds(rcba(baddr));
			cp++;		/* skip char to repeat */
			if (*cp == ORDER_GE){
				ra_ge = True;
				trace_ds("GraphicEscape");
				cp++;
			} else
				ra_ge = False;
			previous = ORDER;
			if (*cp)
				trace_ds("'");
			trace_ds(see_ebc(*cp));
			if (*cp)
				trace_ds("'");
			if (baddr >= COLS * ROWS) {
				trace_ds(" [invalid address, write command terminated]\n");
				return;
			}
			do {
				if (ra_ge)
					ctlr_add(buffer_addr, ebc2cg0[*cp],
					    CS_GE);
				else if (default_cs)
					ctlr_add(buffer_addr, ebc2cg0[*cp], 1);
				else
					ctlr_add(buffer_addr, ebc2cg[*cp], 0);
				ctlr_add_fg(buffer_addr, default_fg);
				ctlr_add_gr(buffer_addr, default_gr);
				INC_BA(buffer_addr);
			} while (buffer_addr != baddr);
			current_fa = get_field_attribute(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_EUA:	/* erase unprotected to address */
			cp += 2;	/* skip buffer address */
			baddr = DECODE_BADDR(*(cp-1), *cp);
			END_TEXT("EraseUnprotectedAll");
			if (previous != SBA)
				trace_ds(rcba(baddr));
			previous = ORDER;
			if (baddr >= COLS * ROWS) {
				trace_ds(" [invalid address, write command terminated]\n");
				return;
			}
			do {
				if (IS_FA(screen_buf[buffer_addr]))
					current_fa = &(screen_buf[buffer_addr]);
				else if (!FA_IS_PROTECTED(*current_fa)) {
					ctlr_add(buffer_addr, CG_null, 0);
				}
				INC_BA(buffer_addr);
			} while (buffer_addr != baddr);
			current_fa = get_field_attribute(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_GE:	/* graphic escape */
			END_TEXT("GraphicEscape ");
			cp++;		/* skip char */
			previous = ORDER;
			if (*cp)
				trace_ds("'");
			trace_ds(see_ebc(*cp));
			if (*cp)
				trace_ds("'");
			ctlr_add(buffer_addr, ebc2cg0[*cp], CS_GE);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_gr(buffer_addr, default_gr);
			INC_BA(buffer_addr);
			current_fa = get_field_attribute(buffer_addr);
			last_cmd = False;
			last_zpt = False;
			break;
		case ORDER_MF:	/* modify field */
			END_TEXT("ModifyField");
			if (previous != SBA)
				trace_ds(rcba(buffer_addr));
			previous = ORDER;
			cp++;
			na = *cp;
			if (IS_FA(screen_buf[buffer_addr])) {
				if (na == 0) {
					INC_BA(buffer_addr);
				} else {
					for (i = 0; i < (int)na; i++) {
						cp++;
						if (*cp == XA_3270) {
							trace_ds(" 3270");
							cp++;
							new_attr = ATTR2FA(*cp);
							ctlr_add(buffer_addr,
							    new_attr,
							    0);
							trace_ds(see_attr(new_attr));
						} else if (*cp == XA_FOREGROUND) {
							trace_ds("%s",
							    see_efa(*cp,
								*(cp + 1)));
							cp++;
							if (appres.m3279)
								ctlr_add_fg(buffer_addr, *cp);
						} else if (*cp == XA_HIGHLIGHTING) {
							trace_ds("%s",
							    see_efa(*cp,
								*(cp + 1)));
							cp++;
							ctlr_add_gr(buffer_addr, *cp & 0x07);
						} else if (*cp == XA_CHARSET) {
							int cs = 0;

							trace_ds("%s",
							    see_efa(*cp,
								*(cp + 1)));
							cp++;
							if (*cp == 0xf1)
								cs = 1;
							ctlr_add(buffer_addr,
							    screen_buf[buffer_addr], cs);
						} else if (*cp == XA_ALL) {
							trace_ds("%s",
							    see_efa(*cp,
								*(cp + 1)));
							cp++;
						} else {
							trace_ds("%s[unsupported]", see_efa(*cp, *(cp + 1)));
							cp++;
						}
					}
				}
				INC_BA(buffer_addr);
			} else
				cp += na * 2;
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_SFE:	/* start field extended */
			END_TEXT("StartFieldExtended");
			if (previous != SBA)
				trace_ds(rcba(buffer_addr));
			previous = ORDER;
			cp++;	/* skip order */
			na = *cp;
			any_fa = 0;
			efa_fg = 0;
			efa_gr = 0;
			efa_cs = 0;
			for (i = 0; i < (int)na; i++) {
				cp++;
				if (*cp == XA_3270) {
					trace_ds(" 3270");
					cp++;
					START_FIELD(*cp);
					any_fa++;
				} else if (*cp == XA_FOREGROUND) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
					if (appres.m3279)
						efa_fg = *cp;
				} else if (*cp == XA_HIGHLIGHTING) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
					efa_gr = *cp & 0x07;
				} else if (*cp == XA_CHARSET) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
					if (*cp == 0xf1)
						efa_cs = 1;
				} else if (*cp == XA_ALL) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
				} else {
					trace_ds("%s[unsupported]", see_efa(*cp, *(cp + 1)));
					cp++;
				}
			}
			if (!any_fa)
				START_FIELD0;
			ctlr_add(buffer_addr, screen_buf[buffer_addr], efa_cs);
			ctlr_add_fg(buffer_addr, efa_fg);
			ctlr_add_gr(buffer_addr, efa_gr);
			INC_BA(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_SA:	/* set attribute */
			END_TEXT("SetAttribtue");
			previous = ORDER;
			cp++;
			if (*cp == XA_FOREGROUND)  {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				if (appres.m3279)
					default_fg = *(cp + 1);
			} else if (*cp == XA_HIGHLIGHTING)  {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				default_gr = *(cp + 1) & 0x07;
			} else if (*cp == XA_ALL)  {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				default_fg = 0;
				default_gr = 0;
				default_cs = 0;
			} else if (*cp == XA_CHARSET) {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				default_cs = (*(cp + 1) == 0xf1) ? 1 : 0;
			} else
				trace_ds("%s[unsupported]",
				    see_efa(*cp, *(cp + 1)));
			cp++;
			last_cmd = True;
			last_zpt = False;
			break;
		case FCORDER_SUB:	/* format control orders */
		case FCORDER_DUP:
		case FCORDER_FM:
		case FCORDER_FF:
		case FCORDER_CR:
		case FCORDER_NL:
		case FCORDER_EM:
		case FCORDER_EO:
			END_TEXT(see_ebc(*cp));
			previous = ORDER;
			ctlr_add(buffer_addr, ebc2cg[*cp], default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_gr(buffer_addr, default_gr);
			INC_BA(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case FCORDER_NULL:
			END_TEXT("NULL");
			previous = NULLCH;
			ctlr_add(buffer_addr, ebc2cg[*cp], default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_gr(buffer_addr, default_gr);
			INC_BA(buffer_addr);
			last_cmd = False;
			last_zpt = False;
			break;
		default:	/* enter character */
			if (*cp <= 0x3F) {
				END_TEXT("ILLEGAL_ORDER");
				trace_ds(see_ebc(*cp));
				last_cmd = True;
				last_zpt = False;
				break;
			}
			if (previous != TEXT)
				trace_ds(" '");
			previous = TEXT;
			trace_ds(see_ebc(*cp));
			ctlr_add(buffer_addr, ebc2cg[*cp], default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_gr(buffer_addr, default_gr);
			INC_BA(buffer_addr);
			last_cmd = False;
			last_zpt = False;
			break;
		}
	}
	set_formatted();
	END_TEXT0;
	trace_ds("\n");
	if (wcc_keyboard_restore) {
		aid = AID_NO;
		do_reset(False);
	} else if (kybdlock & KL_OIA_TWAIT) {
		kybdlock_clr(KL_OIA_TWAIT, "ctlr_write");
		status_syswait();
	}
	if (wcc_sound_alarm)
		ring_bell();

	trace_primed = False;

	ps_process();
}

#undef START_FIELDx
#undef START_FIELD0
#undef START_FIELD
#undef END_TEXT0
#undef END_TEXT


/*
 * Process pending input.
 */
void
ps_process()
{
	/* Process typeahead. */
	while (run_ta())
		;

	/* Process pending scripts and macros. */
	sms_continue();

	/* Process file transfers. */
	if (ft_state != FT_NONE &&	/* transfer in progress */
	    formatted &&		/* screen is formatted */
	    !screen_alt &&		/* 24x80 screen */
	    !kybdlock &&		/* keyboard not locked */
	    				/* magic field */
	    IS_FA(screen_buf[1919]) && FA_IS_SKIP(screen_buf[1919]))
		ft_cut_data();
}

/*
 * Tell me if there is any data on the screen.
 */
Boolean
ctlr_any_data()
{
	register unsigned char *c = screen_buf;
	register int i;
	register unsigned char oc;

	for (i = 0; i < ROWS*COLS; i++) {
		oc = *c++;
		if (!IS_FA(oc) && !IsBlank(oc))
			return True;
	}
	return False;
}

/*
 * Clear the text (non-status) portion of the display.  Also resets the cursor
 * and buffer addresses and extended attributes.
 */
void
ctlr_clear(can_snap)
Boolean can_snap;
{
	extern Boolean trace_skipping;

	/* Snap any data that is about to be lost into the trace file. */
	if (ctlr_any_data()) {
		if (can_snap && !trace_skipping && toggled(SCREEN_TRACE))
			trace_screen();
		scroll_save(maxROWS, ever_3270 ? False : True);
	}
	trace_skipping = False;

	/* Clear the screen. */
	(void) memset((char *)screen_buf, 0, ROWS*COLS);
	(void) memset((char *)ea_buf, 0, ROWS*COLS*sizeof(struct ea));
	ALL_CHANGED;
	cursor_move(0);
	buffer_addr = 0;
	(void) unselect(0, ROWS*COLS);
	formatted = False;
	default_fg = 0;
	default_gr = 0;
}

/*
 * Fill the screen buffer with blanks.
 */
static void
ctlr_blanks()
{
	(void) memset((char *)screen_buf, CG_space, ROWS*COLS);
	ALL_CHANGED;
	cursor_move(0);
	buffer_addr = 0;
	(void) unselect(0, ROWS*COLS);
	formatted = False;
}


/*
 * Change a character in the 3270 buffer.
 */
void
ctlr_add(baddr, c, cs)
int	baddr;		/* buffer address */
unsigned char	c;	/* character */
unsigned char	cs;	/* character set */
{
	unsigned char oc;

	if ((oc = screen_buf[baddr]) != c || ea_buf[baddr].cs != cs) {
		if (trace_primed && !IsBlank(oc)) {
			if (toggled(SCREEN_TRACE))
				trace_screen();
			scroll_save(maxROWS, False);
			trace_primed = False;
		}
		if (SELECTED(baddr))
			(void) unselect(baddr, 1);
		ONE_CHANGED(baddr);
		screen_buf[baddr] = c;
		ea_buf[baddr].cs = cs;
	}
}

/*
 * Change the graphic rendition of a character in the 3270 buffer.
 */
void
ctlr_add_gr(baddr, gr)
int	baddr;
unsigned char	gr;
{
	if (ea_buf[baddr].gr != gr) {
		if (SELECTED(baddr))
			(void) unselect(baddr, 1);
		ONE_CHANGED(baddr);
		ea_buf[baddr].gr = gr;
		if (gr & GR_BLINK)
			blink_start();
	}
}

/*
 * Change the foreground color for a character in the 3270 buffer.
 */
void
ctlr_add_fg(baddr, color)
int	baddr;
unsigned char	color;
{
	if (!appres.m3279)
		return;
	if ((color & 0xf0) != 0xf0)
		color = 0;
	if (ea_buf[baddr].fg != color) {
		if (SELECTED(baddr))
			(void) unselect(baddr, 1);
		ONE_CHANGED(baddr);
		ea_buf[baddr].fg = color;
	}
}

/*
 * Change the background color for a character in the 3270 buffer.
 */
void
ctlr_add_bg(baddr, color)
int	baddr;
unsigned char	color;
{
	if (!appres.m3279)
		return;
	if ((color & 0xf0) != 0xf0)
		color = 0;
	if (ea_buf[baddr].bg != color) {
		if (SELECTED(baddr))
			(void) unselect(baddr, 1);
		ONE_CHANGED(baddr);
		ea_buf[baddr].bg = color;
	}
}

/*
 * Copy a block of characters in the 3270 buffer, optionally including all of
 * the extended attributes.  (The character set, which is actually kept in the
 * extended attributes, is considered part of the characters here.)
 */
void
ctlr_bcopy(baddr_from, baddr_to, count, move_ea)
int	baddr_from;
int	baddr_to;
int	count;
int	move_ea;
{
	/* Move the characters. */
	if (memcmp((char *) &screen_buf[baddr_from],
	           (char *) &screen_buf[baddr_to],
		   count)) {
		(void) MEMORY_MOVE((char *) &screen_buf[baddr_to],
			           (char *) &screen_buf[baddr_from],
			           count);
		REGION_CHANGED(baddr_to, baddr_to + count);
		/*
		 * For the time being, if any selected text shifts around on
		 * the screen, unhighlight it.  Eventually there should be
		 * logic for preserving the highlight if the *all* of the
		 * selected text moves.
		 */
		if (area_is_selected(baddr_to, count))
			(void) unselect(baddr_to, count);
	}

	/*
	 * If we aren't supposed to move all the extended attributes, move
	 * the character sets separately.
	 */
	if (!move_ea) {
		int i;
		int any = 0;
		int start, end, inc;

		if (baddr_to < baddr_from || baddr_from + count < baddr_to) {
			/* Scan forward. */
			start = 0;
			end = count + 1;
			inc = 1;
		} else {
			/* Scan backward. */
			start = count - 1;
			end = -1;
			inc = -1;
		}

		for (i = start; i != end; i += inc) {
			if (ea_buf[baddr_to+i].cs != ea_buf[baddr_from+i].cs) {
				ea_buf[baddr_to+i].cs = ea_buf[baddr_from+i].cs;
				REGION_CHANGED(baddr_to + i, baddr_to + i + 1);
				any++;
			}
		}
		if (any && area_is_selected(baddr_to, count))
			(void) unselect(baddr_to, count);
	}

	/* Move extended attributes. */
	if (move_ea && memcmp((char *) &ea_buf[baddr_from],
			      (char *) &ea_buf[baddr_to],
			      count*sizeof(struct ea))) {
		(void) MEMORY_MOVE((char *) &ea_buf[baddr_to],
		                   (char *) &ea_buf[baddr_from],
			           count*sizeof(struct ea));
		REGION_CHANGED(baddr_to, baddr_to + count);
	}
}

/*
 * Erase a region of the 3270 buffer, optionally clearing extended attributes
 * as well.
 */
void
ctlr_aclear(baddr, count, clear_ea)
int	baddr;
int	count;
int	clear_ea;
{
	if (memcmp((char *) &screen_buf[baddr], (char *) zero_buf, count)) {
		(void) memset((char *) &screen_buf[baddr], 0, count);
		REGION_CHANGED(baddr, baddr + count);
		if (area_is_selected(baddr, count))
			(void) unselect(baddr, count);
	}
	if (clear_ea && memcmp((char *) &ea_buf[baddr], (char *) zero_buf, count*sizeof(struct ea))) {
		(void) memset((char *) &ea_buf[baddr], 0, count*sizeof(struct ea));
		REGION_CHANGED(baddr, baddr + count);
	}
}

/*
 * Scroll the screen 1 row.
 *
 * This could be accomplished with ctlr_bcopy() and ctlr_aclear(), but this
 * operation is common enough to warrant a separate path.
 */
void
ctlr_scroll()
{
	int qty = (ROWS - 1) * COLS;
	Boolean obscured;

	/* Make sure nothing is selected. (later this can be fixed) */
	(void) unselect(0, ROWS*COLS);

	/* Synchronize pending changes prior to this. */
	obscured = screen_obscured();
	if (!obscured && screen_changed)
		screen_disp();

	/* Move screen_buf and ea_buf. */
	(void) MEMORY_MOVE((char *) &screen_buf[0],
	    (char *) &screen_buf[COLS],
	    qty);
	(void) MEMORY_MOVE((char *) &ea_buf[0],
	    (char *) &ea_buf[COLS],
	    qty * sizeof(struct ea));

	/* Clear the last line. */
	(void) memset((char *) &screen_buf[qty], 0, COLS);
	(void) memset((char *) &ea_buf[qty], 0, COLS * sizeof(struct ea));

	/* Update the screen. */
	if (obscured) {
		ALL_CHANGED;
	} else
		screen_scroll();
}

/*
 * Note that a particular region of the screen has changed.
 */
void
ctlr_changed(bstart, bend)
int bstart;	/* first changed location */
int bend;	/* last changed location, plus 1 */
{
	REGION_CHANGED(bstart, bend);
}

/*
 * Swap the regular and alternate screen buffers
 */
void
ctlr_altbuffer(alt)
Boolean	alt;
{
	unsigned char *stmp;
	struct ea *etmp;

	if (alt != is_altbuffer) {

		stmp = screen_buf;
		screen_buf = ascreen_buf;
		ascreen_buf = stmp;

		etmp = ea_buf;
		ea_buf = aea_buf;
		aea_buf = etmp;

		is_altbuffer = alt;
		ALL_CHANGED;
		(void) unselect(0, ROWS*COLS);

		/*
		 * There may be blinkers on the alternate screen; schedule one
		 * iteration just in case.
		 */
		blink_start();
	}
}
#undef SWAP


/*
 * Set or clear the MDT on an attribute
 */
void
mdt_set(fa)
unsigned char *fa;
{
	if (*fa & FA_MODIFY)
		return;
	*fa |= FA_MODIFY;
	if (appres.modified_sel)
		ALL_CHANGED;
}

void
mdt_clear(fa)
unsigned char *fa;
{
	if (!(*fa & FA_MODIFY))
		return;
	*fa &= ~FA_MODIFY;
	if (appres.modified_sel)
		ALL_CHANGED;
}


/*
 * Support for screen-size swapping for scrolling
 */
void
ctlr_shrink()
{
	(void) memset((char *)screen_buf,
	    *debugging_font ? CG_space : CG_null,
	    ROWS*COLS);
	ALL_CHANGED;
	screen_disp();
}


/*
 * Transaction timing.  The time between sending an interrupt (PF, PA, Enter,
 * Clear) and the host unlocking the keyboard is indicated on the status line
 * to an accuracy of 0.1 seconds.  If we don't repaint the screen before we see
 * the unlock, the time should be fairly accurate.
 */
static struct timeval t_start;
static Boolean ticking = False;
static XtIntervalId tick_id;
static struct timeval t_want;

/* Return the difference in milliseconds between two timevals. */
static long
delta_msec(t1, t0)
struct timeval *t1, *t0;
{
	return (t1->tv_sec - t0->tv_sec) * 1000 +
	       (t1->tv_usec - t0->tv_usec + 500) / 1000;
}

/*ARGSUSED*/
static void
keep_ticking(closure, id)
XtPointer closure;
XtIntervalId *id;
{
	struct timeval t1;
	long msec;

	do {
		(void) gettimeofday(&t1, (struct timezone *) 0);
		t_want.tv_sec++;
		msec = delta_msec(&t_want, &t1);
	} while (msec <= 0);
	tick_id = XtAppAddTimeOut(appcontext, msec, keep_ticking, 0);
	status_timing(&t_start, &t1);
}

void
ticking_start(anyway)
Boolean anyway;
{
	if (!toggled(SHOW_TIMING) && !anyway)
		return;
	status_untiming();
	if (ticking)
		XtRemoveTimeOut(tick_id);
	ticking = True;
	(void) gettimeofday(&t_start, (struct timezone *) 0);
	tick_id = XtAppAddTimeOut(appcontext, 1000, keep_ticking, 0);
	t_want = t_start;
}

void
ticking_stop()
{
	struct timeval t1;

	if (!ticking)
		return;
	XtRemoveTimeOut(tick_id);
	(void) gettimeofday(&t1, (struct timezone *) 0);
	ticking = False;
	status_timing(&t_start, &t1);
}

/*ARGSUSED*/
void
toggle_showTiming(t, tt)
struct toggle *t;
enum toggle_type tt;
{
	if (!toggled(SHOW_TIMING))
		status_untiming();
}


/*
 * No-op toggle.
 */
/*ARGSUSED*/
void
toggle_nop(t, tt)
struct toggle *t;
enum toggle_type tt;
{
}
