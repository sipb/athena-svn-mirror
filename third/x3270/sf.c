/*
 * Copyright 1994, 1995, 1996 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	sf.c
 *		This module handles 3270 structured fields.
 *
 */

#include "globals.h"
#include <errno.h>
#include "3270ds.h"
#include "appres.h"
#include "screen.h"
#include "cg.h"

#include "ctlrc.h"
#include "ft_dftc.h"
#include "kybdc.h"
#include "sfc.h"
#include "telnetc.h"
#include "trace_dsc.h"

/* Externals: ctlr.c */
extern Boolean  screen_alt;
extern unsigned char reply_mode;
extern int      crm_nattr;
extern unsigned char crm_attr[];

/* Globals */
void            write_structured_field();

/* Statics */
static Boolean  qr_in_progress = False;
static void     sf_read_part();
static void     sf_erase_reset();
static void     sf_set_reply_mode();
static void     sf_outbound_ds();
static void     query_reply_start();
static void     do_query_reply();
static void     query_reply_end();

static unsigned char supported_replies[] = {
	QR_SUMMARY,		/* 0x80 */
	QR_USABLE_AREA,		/* 0x81 */
	QR_ALPHA_PART,		/* 0x84 */
	QR_CHARSETS,		/* 0x85 */
	QR_COLOR,		/* 0x86 */
	QR_HIGHLIGHTING,	/* 0x87 */
	QR_REPLY_MODES,		/* 0x88 */
	QR_IMP_PART,		/* 0xa6 */
	QR_DDM,			/* 0x95 */
};
#define NSR	(sizeof(supported_replies)/sizeof(unsigned char))



/*
 * Process a 3270 Write Structured Field command
 */
void
write_structured_field(buf, buflen)
unsigned char buf[];
int buflen;
{
	unsigned short fieldlen;
	unsigned char *cp = buf;
	Boolean first = True;

	/* Skip the WSF command itself. */
	cp++;
	buflen--;

	/* Interpret fields. */
	while (buflen > 0) {

		if (first)
			trace_ds(" ");
		else
			trace_ds("< WriteStructuredField ");
		first = False;

		/* Pick out the field length. */
		if (buflen < 2) {
			trace_ds("error: single byte at end of message\n");
			return;
		}
		fieldlen = (cp[0] << 8) + cp[1];
		if (fieldlen == 0)
			fieldlen = buflen;
		if (fieldlen < 3) {
			trace_ds("error: field length %d too small\n",
			    fieldlen);
			return;
		}
		if ((int)fieldlen > buflen) {
			trace_ds("error: field length %d exceeds remaining message length %d\n",
			    fieldlen, buflen);
			return;
		}

		/* Dispatch on the ID. */
		switch (cp[2]) {
		    case SF_READ_PART:
			trace_ds("ReadPartition");
			sf_read_part(cp, (int)fieldlen);
			break;
		    case SF_ERASE_RESET:
			trace_ds("EraseReset");
			sf_erase_reset(cp, (int)fieldlen);
			break;
		    case SF_SET_REPLY_MODE:
			trace_ds("SetReplyMode");
			sf_set_reply_mode(cp, (int)fieldlen);
			break;
		    case SF_OUTBOUND_DS:
			trace_ds("OutboundDS");
			sf_outbound_ds(cp, (int)fieldlen);
			break;
		    case SF_TRANSFER_DATA:   /* File transfer data         */
			trace_ds("FileTransferData");
			ft_dft_data(cp, (int)fieldlen);
			break;
		    default:
			trace_ds("unsupported ID 0x%02x\n", cp[2]);
			break;
		}

		/* Skip to the next field. */
		cp += fieldlen;
		buflen -= fieldlen;
	}
	if (first)
		trace_ds(" (null)\n");
}

static void
sf_read_part(buf, buflen)
unsigned char buf[];
int buflen;
{
	unsigned char partition;
	int i;
	int any = 0;
	char *comma = "";

	if (buflen < 5) {
		trace_ds(" error: field length %d too small\n", buflen);
		return;
	}

	partition = buf[3];
	trace_ds("(0x%02x)", partition);

	switch (buf[4]) {
	    case SF_RP_QUERY:
		trace_ds(" Query");
		if (partition != 0xff) {
			trace_ds(" error: illegal partition\n");
			return;
		}
		trace_ds("\n");
		query_reply_start();
		for (i = 0; i < NSR; i++)
		      do_query_reply(supported_replies[i]);
 		query_reply_end();
		break;
	    case SF_RP_QLIST:
		trace_ds(" QueryList ");
		if (partition != 0xff) {
			trace_ds("error: illegal partition\n");
			return;
		}
		if (buflen < 6) {
			trace_ds("error: missing request type\n");
			return;		/* error */
		}
		query_reply_start();
		switch (buf[5]) {
		    case SF_RPQ_LIST:
			trace_ds("List(");
			if (buflen < 7) {
				trace_ds(")\n");
				do_query_reply(QR_NULL);
			} else {
				for (i = 6; i < buflen; i++) {
					trace_ds("%s%s", comma,
					    see_qcode(buf[i]));
					comma = ",";
				}
				trace_ds(")\n");
				for (i = 0; i < NSR; i++) {
					if (memchr((char *)&buf[6],
						   (char)supported_replies[i],
						   buflen-6)) {
						do_query_reply(supported_replies[i]);
						any++;
					}
				}
				if (!any) {
					do_query_reply(QR_NULL);
				}
			}
			break;
		    case SF_RPQ_EQUIV:
			trace_ds("Equivlent+List(");
			for (i = 6; i < buflen; i++) {
				trace_ds("%s%s", comma, see_qcode(buf[i]));
				comma = ",";
			}
			trace_ds(")\n");
			for (i = 0; i < NSR; i++)
				do_query_reply(supported_replies[i]);
			break;
		    case SF_RPQ_ALL:
			trace_ds("All\n");
			for (i = 0; i < NSR; i++)
				do_query_reply(supported_replies[i]);
			break;
		    default:
			trace_ds("unknown request type 0x%02x\n", buf[5]);
			return;		/* error */
		}
		query_reply_end();
		break;
	    case SNA_CMD_RMA:
		trace_ds(" ReadModifiedAll");
		if (partition != 0x00) {
			trace_ds(" error: illegal partition\n");
			return;
		}
		trace_ds("\n");
		ctlr_read_modified(AID_QREPLY, True);
		break;
	    case SNA_CMD_RB:
		trace_ds(" ReadBuffer");
		if (partition != 0x00) {
			trace_ds(" error: illegal partition\n");
			return;
		}
		trace_ds("\n");
		ctlr_read_buffer(AID_QREPLY);
		break;
	    case SNA_CMD_RM:
		trace_ds(" ReadModified");
		if (partition != 0x00) {
			trace_ds(" error: illegal partition\n");
			return;
		}
		trace_ds("\n");
		ctlr_read_modified(AID_QREPLY, False);
		break;
	    default:
		trace_ds(" unknown type 0x%02x\n", buf[4]);
		return;
	}
}

static void
sf_erase_reset(buf, buflen)
unsigned char buf[];
int buflen;
{
	if (buflen != 4) {
		trace_ds(" error: wrong field length %d\n", buflen);
		return;
	}

	switch (buf[3]) {
	    case SF_ER_DEFAULT:
		trace_ds(" Default\n");
		ctlr_erase(False);
		break;
	    case SF_ER_ALT:
		trace_ds(" Alternate\n");
		ctlr_erase(True);
		break;
	    default:
		trace_ds(" unknown type 0x%02x\n", buf[3]);
		return;
	}
}

static void
sf_set_reply_mode(buf, buflen)
unsigned char buf[];
int buflen;
{
	unsigned char partition;
	int i;
	char *comma = "(";

	if (buflen < 5) {
		trace_ds(" error: wrong field length %d\n", buflen);
		return;
	}

	partition = buf[3];
	trace_ds("(0x%02x)", partition);
	if (partition != 0x00) {
		trace_ds(" error: illegal partition\n");
		return;
	}

	switch (buf[4]) {
	    case SF_SRM_FIELD:
		trace_ds(" Field\n");
		break;
	    case SF_SRM_XFIELD:
		trace_ds(" ExtendedField\n");
		break;
	    case SF_SRM_CHAR:
		trace_ds(" Character");
		break;
	    default:
		trace_ds(" unknown mode 0x%02x\n", buf[4]);
		return;
	}
	reply_mode = buf[4];
	if (buf[4] == SF_SRM_CHAR) {
		crm_nattr = buflen - 5;
		for (i = 5; i < buflen; i++) {
			crm_attr[i - 5] = buf[i];
			trace_ds("%s%s", comma, see_efa_only(buf[i]));
			comma = ",";
		}
		trace_ds("%s\n", crm_nattr ? ")" : "");
	}
}

static void
sf_outbound_ds(buf, buflen)
unsigned char buf[];
int buflen;
{
	if (buflen < 5) {
		trace_ds(" error: field length %d too short\n", buflen);
		return;
	}

	trace_ds("(0x%02x)", buf[3]);
	if (buf[3] != 0x00) {
		trace_ds(" error: illegal partition 0x%0x\n", buf[3]);
		return;
	}

	switch (buf[4]) {
	    case SNA_CMD_W:
		trace_ds(" Write");
		if (buflen > 5)
			ctlr_write(&buf[4], buflen-4, False);
		else
			trace_ds("\n");
		break;
	    case SNA_CMD_EW:
		trace_ds(" EraseWrite");
		ctlr_erase(screen_alt);
		if (buflen > 5)
			ctlr_write(&buf[4], buflen-4, True);
		else
			trace_ds("\n");
		break;
	    case SNA_CMD_EWA:
		trace_ds(" EraseWriteAlternate");
		ctlr_erase(screen_alt);
		if (buflen > 5)
			ctlr_write(&buf[4], buflen-4, True);
		else
			trace_ds("\n");
		break;
	    case SNA_CMD_EAU:
		trace_ds(" EraseAllUnprotected\n");
		ctlr_erase_all_unprotected();
		break;
	    default:
		trace_ds(" unknown type 0x%02x\n", buf[4]);
		return;		/* error */
	}
}

static void
query_reply_start()
{
	obptr = obuf;
	space3270out(1);
	*obptr++ = AID_SF;
	qr_in_progress = True;
}

static void
do_query_reply(code)
unsigned char code;
{
	int len;
	int i;
	char *comma = "";
	int obptr0 = obptr - obuf;
	unsigned char *obptr_len;
	unsigned short num, denom;
	extern int default_screen;

	if (qr_in_progress) {
		trace_ds("> StructuredField\n");
		qr_in_progress = False;
	}

	space3270out(4);
	obptr += 2;	/* skip length for now */
	*obptr++ = SFID_QREPLY;
	*obptr++ = code;
	switch (code) {

	    case QR_CHARSETS:
		trace_ds("> QueryReply(CharacterSets)\n");
		space3270out(23);
		*obptr++ = 0x82;	/* flags: GE, CGCSGID present */
		*obptr++ = 0x00;	/* more flags */
		*obptr++ = *char_width;	/* SDW */
		*obptr++ = *char_height;/* SDH */
		*obptr++ = 0x00;	/* Load PS format types */
		*obptr++ = 0x00;
		*obptr++ = 0x00;
		*obptr++ = 0x00;
		*obptr++ = 0x07;	/* DL */
		*obptr++ = 0x00;	/* SET 0: */
		*obptr++ = 0x10;	/*  FLAGS: non-loadable, single-plane,
					     single-byte, no compare */
		*obptr++ = 0x00;	/*  LCID */
		*obptr++ = 0x00;	/*  CGCSGID: international */
		*obptr++ = 0x67;
		*obptr++ = 0x00;
		*obptr++ = 0x26;
		*obptr++ = 0x01;	/* SET 1: */
		*obptr++ = 0x00;	/*  FLAGS: non-loadable, single-plane,
					     single-byte, no compare */
		*obptr++ = 0xf1;	/*  LCID */
		*obptr++ = 0x03;	/*  CGCSGID: 3179-style APL2 */
		*obptr++ = 0xc3;
		*obptr++ = 0x01;
		*obptr++ = 0x36;
		break;

	    case QR_IMP_PART:
		trace_ds("> QueryReply(ImplicitPartition)\n");
		space3270out(13);
		*obptr++ = 0x0;		/* reserved */
		*obptr++ = 0x0;
		*obptr++ = 0x0b;	/* length of display size */
		*obptr++ = 0x01;	/* "implicit partition size" */
		*obptr++ = 0x00;	/* reserved */
		SET16(obptr, 80);	/* implicit partition width */
		SET16(obptr, 24);	/* implicit partition height */
		SET16(obptr, maxCOLS);	/* alternate height */
		SET16(obptr, maxROWS);	/* alternate width */
		break;

	    case QR_NULL:
		trace_ds("> QueryReply(Null)\n");
		break;

	    case QR_SUMMARY:
		trace_ds("> QueryReply(Summary(");
		space3270out(NSR);
		for (i = 0; i < NSR; i++) {
			trace_ds("%s%s", comma,
			    see_qcode(supported_replies[i]));
			comma = ",";
			*obptr++ = supported_replies[i];
		}
		trace_ds("))\n");
		break;

	    case QR_USABLE_AREA:
		trace_ds("> QueryReply(UsableArea)\n");
		space3270out(19);
		*obptr++ = 0x01;	/* 12/14-bit addressing */
		*obptr++ = 0x00;	/* no special character features */
		SET16(obptr, maxCOLS);	/* usable width */
		SET16(obptr, maxROWS);	/* usable height */
		*obptr++ = 0x01;	/* units (mm) */
		num = XDisplayWidthMM(display, default_screen);
		denom = XDisplayWidth(display, default_screen);
		while (!(num %2) && !(denom % 2)) {
			num /= 2;
			denom /= 2;
		}
		SET16(obptr, (int)num);	/* Xr numerator */
		SET16(obptr, (int)denom); /* Xr denominator */
		num = XDisplayHeightMM(display, default_screen);
		denom = XDisplayHeight(display, default_screen);
		while (!(num %2) && !(denom % 2)) {
			num /= 2;
			denom /= 2;
		}
		SET16(obptr, (int)num);	/* Yr numerator */
		SET16(obptr, (int)denom); /* Yr denominator */
		*obptr++ = *char_width;	/* AW */
		*obptr++ = *char_height;/* AH */
		SET16(obptr, 0);	/* buffer */
		break;

	    case QR_COLOR:
		trace_ds("> QueryReply(Color)\n");
		space3270out(4 + 2*15);
		*obptr++ = 0x00;	/* no options */
		*obptr++ = 16;		/* report on 16 colors */
		*obptr++ = 0x00;	/* default color: */
		*obptr++ = 0xf0 + COLOR_GREEN;	/*  green */
		for (i = 0xf1; i <= 0xff; i++) {
			*obptr++ = i;
			if (appres.m3279)
				*obptr++ = i;
			else
				*obptr++ = 0x00;
		}
		break;

	    case QR_HIGHLIGHTING:
		trace_ds("> QueryReply(Highlighting)\n");
		space3270out(11);
		*obptr++ = 5;		/* report on 5 pairs */
		*obptr++ = XAH_DEFAULT;	/* default: */
		*obptr++ = XAH_NORMAL;	/*  normal */
		*obptr++ = XAH_BLINK;	/* blink: */
		*obptr++ = XAH_BLINK;	/*  blink */
		*obptr++ = XAH_REVERSE;	/* reverse: */
		*obptr++ = XAH_REVERSE;	/*  reverse */
		*obptr++ = XAH_UNDERSCORE; /* underscore: */
		*obptr++ = XAH_UNDERSCORE; /*  underscore */
		*obptr++ = XAH_INTENSIFY; /* intensify: */
		*obptr++ = XAH_INTENSIFY; /*  intensify */
		break;

	    case QR_REPLY_MODES:
		trace_ds("> QueryReply(ReplyModes)\n");
		space3270out(3);
		*obptr++ = SF_SRM_FIELD;
		*obptr++ = SF_SRM_XFIELD;
		*obptr++ = SF_SRM_CHAR;
		break;

	    case QR_ALPHA_PART:
		trace_ds("> QueryReply(AlphanumericPartitions)\n");
		space3270out(4);
		*obptr++ = 0;		/* 1 partition */
		SET16(obptr, maxROWS*maxCOLS);	/* buffer space */
		*obptr++ = 0;		/* no special features */
		break;

	    case QR_DDM:
		trace_ds("> QueryReply(DistributedDataManagement)\n");
		space3270out(8);
		SET16(obptr,0);		/* set reserved field to 0 */
		SET16(obptr,2048);	/* set inbound length limit */
		SET16(obptr,2048);	/* set outbound length limit */
		SET16(obptr,0x0101);	/* NSS=01, DDMSS=01 */
		break;

	    default:
		return;	/* internal error */
	}
	obptr_len = obuf + obptr0;
	len = (obptr - obuf) - obptr0;
	SET16(obptr_len, len);
}

static void
query_reply_end()
{
	net_output();
	kybd_inhibit(True);
}
