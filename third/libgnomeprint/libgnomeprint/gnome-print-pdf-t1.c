/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-pdf-t1.c: Tyep1 Font embeding for gnome-print-pdf
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useoful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Chema Celorio <chema@celorio.com>
 *
 *  Copyright 2000-2003 Ximian, Inc.
 *
 *  References:
 *    [1] Portable Document Format Referece Manual, Version 1.3 (March 11, 1999)
 *    [2] Adobe Type 1 Font Format, (Sec 7.11.11 and others)
 *    [3] Adobe technical note 5040, "Supporting Downloadable PostScript Language Fonts" (~page 9)
 */

#include <config.h>

#include <string.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/gnome-print-pdf-private.h>

#define T1_BLOCK_SIZE 1024
#define T1_SEGMENT_HEADER_SIZE 6
#define T1_SEGMENT_1_END "currentfile eexec"
#define T1_SEGMENT_3_END "cleartomark"

typedef struct _GnomePrintPdfT1Font GnomePrintPdfT1Font;
struct _GnomePrintPdfT1Font {
	gboolean is_binary;
	gint length[3];

	GnomePrintBuffer b;
};

/* g_strstr_len contains checks for embedded NULs that
 * might confuse us and are slow, so here's a simplified
 * version.
 */
static char *
my_strrstr_len (const gchar *haystack,
		gssize       haystack_len,
		const gchar *needle)
{
	gsize needle_len = strlen (needle);
	const gchar *p;
	gsize i;
	
	if (haystack_len < needle_len)
		return NULL;

	p = haystack + haystack_len - needle_len;
	while (p >= haystack) {
		for (i = 0; i < needle_len; i++)
			if (p[i] != needle[i])
				goto next;

		return (gchar *)p;

	next:
		p--;
	}

	return NULL;
}

static gint
gnome_print_pdf_t1_determine_lengths_pfa (GnomePrintPdfT1Font *font)
{
	guchar *pos;
	guchar *buf;
	gint len;
	gint i;
	gint error;
	gint zeros;

	/* get the length of segment 1 */
	buf = font->b.buf;
	pos = strstr (buf, T1_SEGMENT_1_END) + strlen (T1_SEGMENT_1_END);
	i = 0;
	while ((*pos == '\n' || *pos == '\r') && (i < 2)) {
		pos++; i++;
	}
	len = pos - buf;
	error = 1;
	if (len < 1)
		goto determine_lengths_pfa_error;
	font->length[0] = len;
	
	/* Get the length of segment 2, scanning from the end of the
	 * font. First look for the cleartomark, then rewind 512
	 * zeros, which will most likely have newlines between them (Chema)
	 */

	error = 2;
	pos = my_strrstr_len (pos, font->b.buf_size - len, T1_SEGMENT_3_END);
	if (!pos)
		goto determine_lengths_pfa_error;
	
	pos --;

	/* Rewind 512 zeros */
	zeros = 512;
	while ((zeros > 0) && (pos > buf) && ((*pos == '0' || *pos == '\r' || *pos == '\n'))) {
		if (*pos == '0')
			zeros--;
		pos --;
	}
	error = 5;
	if (zeros > 0)
		goto determine_lengths_pfa_error;

	/* Rewind any new lines, but no more than 10 */
	i = 0;
	while ((*pos == '\n' || *pos == '\r') && (i < 10)) {
		pos--; i++;
	}
	pos++;

	font->length[1] = pos - font->b.buf - font->length[0];

	return GNOME_PRINT_OK;
	
determine_lengths_pfa_error:
	g_warning ("While parsing font. Error num=%d.%02d\n", __LINE__, error);
	
	return GNOME_PRINT_ERROR_UNKNOWN;
}

#define	get_length(x) (((x)[3] << 24) + \
		       ((x)[2] << 16) + \
		       ((x)[1] << 8)  + \
		       ((x)[0] << 0))

static gint
gnome_print_pdf_t1_determine_lengths_pfb (GnomePrintPdfT1Font *font)
{
	const guchar *buf;
	gint error = 0;

	buf = font->b.buf;

	/* Check the header and type of segment 1 */
	error = 1;
	if (buf[0] != 0x80)
		goto get_lengths_error;
	error = 2;
	if (buf[1] != 0x01) /* 01 = ASCII */
		goto get_lengths_error;

	/* Get the length of segment 1 */
	font->length[0] = get_length (buf + 2);
	buf += T1_SEGMENT_HEADER_SIZE + font->length[0];
	
	/* Check the header and type of segment 2 */
	error = 4;
	if (buf[0] != 0x80)
		goto get_lengths_error;
	error = 5;
	if (buf[1] != 0x02) /* 02 = binary */
		goto get_lengths_error;

	/* Get the length of segment 2 */
	font->length[1] = get_length (buf + 2);
	buf += T1_SEGMENT_HEADER_SIZE + font->length[1];

	/* Check the header and type of segment 3 */
	error = 6;
	if (buf[0] != 0x80)
		goto get_lengths_error;
	error = 7;
	if (buf[1] != 0x01) /* 01 = ASCII */
		goto get_lengths_error;

	/* Get the length of segment 3 */
	font->length[2] = get_length (buf + 2);
	buf += T1_SEGMENT_HEADER_SIZE + font->length[2];

	/* Check the header and type of end-of-file */
	error = 8;
	if (buf[0] != 0x80)
		goto get_lengths_error;
	error = 9;
	if (buf[1] != 0x03) /* 03 = end of file */
		goto get_lengths_error;
	
	return GNOME_PRINT_OK;

get_lengths_error:
	g_warning ("There was an error while parsing a Type 1 font, error num: %d.%02d", __LINE__, error);

	return GNOME_PRINT_ERROR_UNKNOWN;
}

static gint
gnome_print_pdf_t1_determine_lengths (GnomePrintPdfT1Font *font)
{
	if (font->is_binary)
		return gnome_print_pdf_t1_determine_lengths_pfb (font);
	else
		return gnome_print_pdf_t1_determine_lengths_pfa (font);
}
static gint
gnome_print_pdf_t1_determine_type (GnomePrintPdfT1Font *font)
{
	GnomePrintReturnCode retval = GNOME_PRINT_ERROR_UNKNOWN;
	guchar *buf;
	
	buf = font->b.buf;
	if (buf[0] == 0x80 &&
	    buf[1] == 0x01) {
		font->is_binary = TRUE;
	} else if (buf[0] == '%' &&
		   buf[1] == '!' &&
		   buf[2] == 'P' &&
		   buf[3] == 'S') {
		font->is_binary = FALSE;
	} else {
		g_warning ("Could not parse font, should start with 0x80.0x01 or %%!PS'"
			   " starts with: 0x%2x.0x%2x", buf[0], buf[1]);
		goto pdf_determin_type_error;
	}

	retval = GNOME_PRINT_OK;
pdf_determin_type_error:
	return retval;
}

/**
 * gnome_print_pdf_font_type1_embed:
 * @pc: Print Context to embed in
 * @font: Font to embed
 * 
 * Embeds in the print context a the type1 font
 *
 * Return Value: 0 on success, -1 on error
 **/
gint
gnome_print_pdf_t1_embed (GnomePrintPdf *pdf,
			  const guchar* file_name,
			  gint *object_number_ret)
{
	GnomePrintReturnCode retval = GNOME_PRINT_ERROR_UNKNOWN;
	GnomePrintPdfT1Font *font;
	gint object_number;

	g_return_val_if_fail (file_name != NULL, retval);

	font = g_new0 (GnomePrintPdfT1Font, 1);

	if (gnome_print_buffer_mmap (&font->b, file_name))
		goto pdf_type1_error;

	if (font->b.buf_size < 8) /* FIXME: why 8? (Chema) */
		goto pdf_type1_error;

	if (gnome_print_pdf_t1_determine_type (font))
		goto pdf_type1_error;

	if (gnome_print_pdf_t1_determine_lengths (font))
		goto pdf_type1_error;

	object_number = gnome_print_pdf_object_new (pdf);
	*object_number_ret = object_number;

	if (font->is_binary) {
		gint stream_size;

		stream_size = font->length[0] + font->length[1];

		gnome_print_pdf_object_start (pdf, object_number, FALSE);
		gnome_print_pdf_fprintf (pdf,
					 "/Length %d"  EOL "/Length1 %d" EOL
					 "/Length2 %d" EOL "/Length3 0"  EOL
					 ">>" EOL "stream" EOL,
					 stream_size + strlen (EOL),
					 font->length[0],
					 font->length[1]);

		gnome_print_pdf_print_sized (pdf,
					     font->b.buf + T1_SEGMENT_HEADER_SIZE,
					     font->length[0]);
		gnome_print_pdf_print_sized (pdf,
					     font->b.buf + T1_SEGMENT_HEADER_SIZE + font->length[0] + T1_SEGMENT_HEADER_SIZE,
					     font->length[1]);
		
		gnome_print_pdf_fprintf (pdf, EOL "endstream" EOL "endobj" EOL);
		gnome_print_pdf_object_end  (pdf, object_number, TRUE);
	} else {
		guchar *buf, *end;
		gint out_size_total;
		gint len_obj_num, len2_obj_num;

		len_obj_num  = gnome_print_pdf_object_new (pdf);
		len2_obj_num = gnome_print_pdf_object_new (pdf);

		/* Object header, we don't know what Length & Lenth2 are so we
		 * write the lenght in an object by itself later, when we know it
		 */
		gnome_print_pdf_object_start (pdf, object_number, FALSE);
		gnome_print_pdf_fprintf (pdf,
					 "/Length %d 0 R"  EOL "/Length1 %d" EOL
					 "/Length2 %d 0 R" EOL "/Length3 0"  EOL
					 ">>" EOL
					 "stream" EOL,
					 len_obj_num,
					 font->length[0],
					 len2_obj_num);
		gnome_print_pdf_print_sized (pdf, font->b.buf, font->length[0]);

		out_size_total = 0;
		/* Segment 2, convert it from ASCII to binary */
		buf = font->b.buf + font->length[0];
		end = buf + font->length[1];

		while (buf < end) {
			guchar out [T1_BLOCK_SIZE*2];
			gint in_size, out_size;

			in_size = ((end - buf) > T1_BLOCK_SIZE) ? T1_BLOCK_SIZE : (end - buf);
			out_size = gnome_print_decode_hex (buf, out, &in_size);
			buf += in_size;

			gnome_print_pdf_print_sized (pdf, out, out_size);
			out_size_total += out_size;
		}

		gnome_print_pdf_fprintf (pdf, EOL "endstream" EOL "endobj" EOL);
		gnome_print_pdf_object_end  (pdf, object_number, TRUE);

		/* Write the object for the length of segment 2, "/Length2" */
		gnome_print_pdf_object_start (pdf, len2_obj_num, TRUE);
		gnome_print_pdf_fprintf (pdf,
					 "%d 0 obj" EOL
					 "%d" EOL
					 "endobj" EOL,
					 len2_obj_num, out_size_total + strlen (EOL));
		gnome_print_pdf_object_end (pdf, len2_obj_num, TRUE);

		/* Write the object for the length of the object, "/Length" */
		out_size_total += font->length[0] + strlen (EOL);
		gnome_print_pdf_object_start (pdf, len_obj_num, TRUE);
		gnome_print_pdf_fprintf (pdf,
					 "%d 0 obj" EOL
					 "%d" EOL
					 "endobj" EOL,
					 len_obj_num, out_size_total);
		gnome_print_pdf_object_end (pdf, len_obj_num, TRUE);
	}

	retval = GNOME_PRINT_OK;

pdf_type1_error:
	if (font->b.buf)
		gnome_print_buffer_munmap (&font->b);
	if (retval != GNOME_PRINT_OK)
		g_warning ("Could not parse Type1 font from %s\n", file_name);
	g_free (font);
		
	return retval;
}

