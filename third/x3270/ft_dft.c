/*
 * Modifications Copyright 1996 by Paul Mattes.
 * Copyright Octover 1995 by Dick Altenbern.
 * Based in part on code Copyright 1993, 1994, 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	tc_dft.c
 *		File transfer: DFT-style data processing functions
 */

#include "globals.h"

#include "3270ds.h"
#include "ft_dft_ds.h"
#include "actionsc.h"
#include "kybdc.h"
#include "ft_dftc.h"
#include "ftc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"

#include <errno.h>

/* Macros. */
#define OPEN_DATA	"FT:    "	/* Open request for data */
#define OPEN_MSG	"FT:MSG "	/* Open request for message */
#define END_TRANSFER	"TRANS03"	/* Message for xfer complete */

/* Typedefs. */
struct data_buffer {
	char sf_length[2];		/* SF length = 0x0023 */
	char sf_d0;			/* 0xD0 */
	char sf_request_type[2];	/* request type */
	char compress_indic[2];		/* 0xc080 */
	char begin_data;		/* 0x61 */
	char data_length[2];		/* Data Length in 3270 byte order+5 */
	char data[256];			/* The actual data */
};

struct open_buffer {			/* Buffer passed in open request */
	char sf_request_type[2];	/* 0x0012 for open request */
	char fixed_parms[6];		/* Doc says 010601010403 */
	char func_required[10];		/* Doc says 0a0a0000000011010100 */
	char data_not_compr[5];		/* Doc says 50055203f0 */
	char hdr_length[2];		/* 0x0309 */
	char name[7];			/* ft: or ft:msg */
};
struct error_response {			/* Buffer to build for an error resp. */
	char sf_length[2];		/* 0009 */
	char sf_d0;			/* 0xd0 */
	char sf_tt;			/* tt = 00,47,45,46 */
	char sf_08;			/* 0x08 */
	char sf_err_hdr[2];		/* 0x6904 */
	char sf_err_code[2];		/* The error code */
};
#define UPLOAD_LENGTH 2048-2
struct upload_buffer_hdr {
	char sf_length[2];		/* SF length */
	char sf_d0;			/* 0xd0 */
	char sf_request_type[2];	/* 0x4605 */
	char sf_recnum_hdr[2];		/* 0x6306 */
	char sf_recnum[4];		/* record number in host byte order */
	char sf_compr_indic[2];		/* 0xc080 */
	char sf_begin_data;		/* 0x61 */
	char sf_data_length[2];		/* Data length */
};
struct upload_buffer {
	struct upload_buffer_hdr header;	/* The header */
	/* The actual data */
	char sf_data[UPLOAD_LENGTH - sizeof(struct upload_buffer_hdr)];
};

/* Statics. */
static Boolean message_flag = False;	/* Open Request for msg received */
static int at_eof;
static unsigned long recnum;
static char *abort_string = CN;

static void dft_abort();
static void dft_close_request();
static void dft_data_insert();
static void dft_get_request();
static void dft_insert_request();
static void dft_open_request();
static void dft_set_cur_req();
static int filter_len();

/* Process a Transfer Data structured field from the host. */
void
ft_dft_data(data_bufr, length)
struct data_buffer *data_bufr;
int length;
{
	unsigned short data_type;
	unsigned char *cp;

	if (ft_state == FT_NONE) {
		trace_ds(" (no transfer in progress)\n");
		return;
	}

	/* Position to character after the d0. */
	cp = (unsigned char *)(data_bufr->sf_request_type);

	/* Get the function type */
	GET16(data_type, cp);

	/* Handle the requests */
	switch (data_type) {
	    case TR_OPEN_REQ:
		dft_open_request((struct open_buffer *)cp);
		break;
	    case TR_INSERT_REQ:	/* Insert Request */
		dft_insert_request();
		break;
	    case TR_DATA_INSERT:
		dft_data_insert(data_bufr);
		break;
	    case TR_SET_CUR_REQ:
		dft_set_cur_req();
		break;
	    case TR_GET_REQ:
		dft_get_request();
		break;
	    case TR_CLOSE_REQ:
		dft_close_request();
		break;
	    default:
		trace_ds(" Unsupported(0x%04x)\n", data_type);
		break;
	}
}

/* Process an Open request. */
static void
dft_open_request(open_buf)
struct open_buffer *open_buf;
{
	trace_ds(" Open %*.*s\n", 7, 7, open_buf->name);

	if (memcmp(open_buf->name, OPEN_MSG, 7) == 0 )
		message_flag = True;
	else {
		message_flag = False;
		ft_running(False);
	}
	at_eof = False;
	recnum = 1;

	trace_ds("> WriteStructuredField FileTransferData OpenAck\n");
	obptr = obuf;
	space3270out(6);
	*obptr++ = AID_SF;
	SET16(obptr, 5);
	*obptr++ = SF_TRANSFER_DATA;
	SET16(obptr, 9);
	net_output();
}

/* Process an Insert request. */
static void
dft_insert_request()
{
	trace_ds(" Insert\n");
	/* Doesn't currently do anything. */
}

/* Process a Data Insert request. */
static void
dft_data_insert(data_bufr)
struct data_buffer *data_bufr;
{
	/* Received a data buffer, get the length and process it */
	int my_length;
	unsigned char *cp;

	trace_ds(" Data\n");

	if (!message_flag && ft_state == FT_ABORT_WAIT) {
		dft_abort(get_message("ftUserCancel"), TR_DATA_INSERT);
		return;
	}

	cp = (unsigned char *) (data_bufr->data_length);

	/* Get the data length in native format. */
	GET16(my_length, cp);

	/* Adjust for 5 extra count */
	my_length -= 5;

	/*
	 * First, check to see if we have message data or file data.
	 * Message data will result in a popup.
	 */
	if (message_flag) {
		/* Data is from a message */
		unsigned char *msgp;
		unsigned char *dollarp;

		/* Get storage to copy the message. */
		msgp = (unsigned char *)XtMalloc(my_length + 1);

		/* Copy the message. */
		memcpy(msgp, data_bufr->data, my_length);

		/* Null terminate the string. */
		dollarp = (unsigned char *)memchr(msgp, '$', my_length);
		if (dollarp != NULL)
			*dollarp = '\0';
		else
			*(msgp + my_length) = '\0';

		/* If transfer completed ok, use our msg. */
		if (memcmp(msgp, END_TRANSFER, strlen(END_TRANSFER)) == 0) {
			XtFree(msgp);
			ft_complete((String)NULL);
		} else if (ft_state == FT_ABORT_SENT && abort_string != CN) {
			XtFree(msgp);
			ft_complete(abort_string);
			abort_string = CN;
		} else
			ft_complete(msgp);
	} else if (my_length > 0) {
		/* Write the data out to the file. */
		int rv = 1;

		if (ascii_flag && cr_flag) {
			char *s = (char *)data_bufr->data;
			unsigned len = my_length;

			/* Delete CRs and ^Zs. */
			while (len) {
				int l = filter_len(s, len);

				if (l) {
					rv = fwrite(s, l, (size_t)1,
					    ft_local_file);
					if (rv == 0)
						break;
					ft_length += l;
				}
				if (l < len)
					l++;
				s += l;
				len -= l;
			}
		} else {
			rv = fwrite((char *)data_bufr->data, my_length,
				(size_t)1, ft_local_file);
			ft_length += my_length;
		}

		if (!rv) {
			/* write failed */
			char *buf;

			buf = xs_buffer("write(%s): %s", ft_local_filename,
			    local_strerror(errno));

			dft_abort(buf, TR_DATA_INSERT);
			XtFree(buf);
		}

		/* Add up amount transferred. */
		ft_update_length();
	}

	/* Send an acknowledgement frame back. */
	trace_ds("> WriteStructuredField FileTransferData DataAck(%lu)\n",
	    recnum);
	obptr = obuf;
	space3270out(12);
	*obptr++ = AID_SF;
	SET16(obptr, 11);
	*obptr++ = SF_TRANSFER_DATA;
	SET16(obptr, TR_NORMAL_REPLY);
	SET16(obptr, TR_RECNUM_HDR);
	SET32(obptr, recnum);
	recnum++;
	net_output();
}

/* Process a Set Cursor request. */
static void
dft_set_cur_req()
{
	trace_ds(" SetCursor\n");
	/* Currently doesn't do anything. */
}

/* Process a Get request. */
static void
dft_get_request()
{
	int numbytes;
	size_t numread;
	unsigned char *bufptr;
	struct upload_buffer *upbufp;

	trace_ds(" Get\n");

	if (!message_flag && ft_state == FT_ABORT_WAIT) {
		dft_abort(get_message("ftUserCancel"), TR_GET_REQ);
		return;
	}

	/*
	 * This is a request to send an upload buffer.
	 * First check to see if we are finished (at_eof = True).
	 */
	if (at_eof) {
		/* We are done, send back the eof error. */
		trace_ds("> WriteStructuredField FileTransferData EOF\n");
		space3270out(sizeof(struct error_response) + 1);
		obptr = obuf;
		*obptr++ = AID_SF;
		SET16(obptr, sizeof(struct error_response));
		*obptr++ = SF_TRANSFER_DATA;
		*obptr++ = HIGH8(TR_GET_REQ);
		*obptr++ = TR_ERROR_REPLY;
		SET16(obptr, TR_ERROR_HDR);
		SET16(obptr, TR_ERR_EOF);
	} else {
		trace_ds("> WriteStructuredField FileTransferData Data(%lu)\n",
		    recnum);
		space3270out(sizeof(struct upload_buffer) + 1);
		obptr = obuf;
		*obptr++ = AID_SF;
		/* Set buffer pointer */
		upbufp = (struct upload_buffer *) obptr;
		/* Skip length for now */
		obptr += 2;
		*obptr++ = SF_TRANSFER_DATA;
		SET16(obptr, TR_GET_REPLY);
		SET16(obptr, TR_RECNUM_HDR);
		SET32(obptr, recnum);
		recnum++;
		SET16(obptr, TR_NOT_COMPRESSED);
		*obptr++ = TR_BEGIN_DATA;

		/* Size of the data buffer */
		numbytes = sizeof(upbufp->sf_data);
		SET16(obptr, numbytes+5);
		bufptr = (unsigned char *)upbufp->sf_data;
		obptr = (unsigned char *)upbufp->header.sf_data_length;
		while (numbytes > 1) {
			/* Continue until we run out of buffer */
			if (cr_flag) {
				/* Insert CR after LF. */
				if (fgets((char *)bufptr, numbytes,
					    ft_local_file) != CN) {
					/* We got a line. */

					/* Decrement left. */
					numbytes -= (strlen((char *)bufptr)+1);

					/* Point to \r at end of str. */
					bufptr += (strlen((char *)bufptr)-1);

					if (*bufptr == '\n') {
						/* Stick in the \r\n. */
						memcpy(bufptr, "\r\n", 2);
					}

					/* Point to next space. */
					bufptr += 2;

					if (numbytes == 0) {
						/* At end of buffer */
						if (*(bufptr - 1) != '\n') {
							/*
							 * If not a LF, Back up
							 * the buff pointer.
							 */
							bufptr--;
						}
					}
				}
			} else {
			    	/* Not crlf, do binary read. */
				numread = fread(bufptr, 1, numbytes,
				    ft_local_file);
				bufptr += numread;
				numbytes -= numread;
			}
			if (feof(ft_local_file)) {
				/* End of file. */

				/* Set that we are out of data. */
				at_eof = True;
				break;
			} else if (ferror(ft_local_file)) {
				char *buf;

				buf = xs_buffer("read(%s): %s",
					ft_local_filename,
					local_strerror(errno));
				dft_abort(buf, TR_GET_REQ);
			}
		}

		/* Set data length. */
		SET16(obptr, bufptr-obptr+4);

		/* Accumulate length written. */
		ft_length += bufptr-obptr;

		/* Send last byte to net_output. */
		obptr = bufptr;
	}

	/* We built a buffer, let's write it back to the mainframe. */

	/* Position to beg. of buffer. */
	bufptr = obuf;
	bufptr++;

	/* Set the sf length. */
	SET16(bufptr, (obptr-1)-obuf);

	net_output();
	ft_update_length();
}

/* Process a Close request. */
static void
dft_close_request()
{
	/*
	 * Recieved a close request from the system.
	 * Return a close acknowledgement.
	 */
	trace_ds(" Close\n");
	trace_ds("> WriteStructuredField FileTransferData CloseAck\n");
	obptr = obuf;
	space3270out(6);
	*obptr++ = AID_SF;
	SET16(obptr, 5);	/* length */
	*obptr++ = SF_TRANSFER_DATA;
	SET16(obptr, TR_CLOSE_REPLY);
	net_output();
}

/* Abort a transfer. */
static void
dft_abort(s, code)
char *s;
unsigned short code;
{
	if (abort_string != CN)
		XtFree(abort_string);
	abort_string = XtNewString(s);

	trace_ds("> WriteStructuredField FileTransferData Error\n");

	obptr = obuf;
	space3270out(10);
	*obptr++ = AID_SF;
	SET16(obptr, 9);	/* length */
	*obptr++ = SF_TRANSFER_DATA;
	*obptr++ = HIGH8(code);
	*obptr++ = TR_ERROR_REPLY;
	SET16(obptr, TR_ERROR_HDR);
	SET16(obptr, TR_ERR_CMDFAIL);
	net_output();

	/* Update the pop-up and state. */
	ft_aborting();
}

/* Returns the number of bytes in s, limited by len, that aren't CRs or ^Zs. */
static int
filter_len(s, len)
char *s;
register int len;
{
	register char *t = s;

	while (len && *t != '\r' && *t != 0x1a) {
		len--;
		t++;
	}
	return t - s;
}
