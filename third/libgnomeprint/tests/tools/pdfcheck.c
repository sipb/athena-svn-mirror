/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Code to verify pdf files. I am sick of doing this by hand. I am SOOoo sick
 * that i am parsing it in C, that is how sick of doing this I am
 *
 * It is GPL but who is going to care anyways ...
 *
 * This is ugly code i know. That is a design desicion.
 *
 * Authors: Chema Celoiro <chema@ximian.com>
 */

#include <glib.h>
#include <glib-object.h>
#include <popt.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif

static gint debug;

static struct poptOption options[] = {
	{ "debug",   '\0', POPT_ARG_INT, &debug, 0,
	  "debug level",          "0,1,2,3"},
	{ NULL }
};

static void
my_error (const gchar *format, ...)
{
	va_list args;
	gchar buffer [2048];
	
	va_start (args, format);
	g_print ("Fatal error:\n    ");
	vsnprintf (buffer, 2048, format, args);
	g_print (buffer);
	va_end (args);
	g_print ("\nAborting ...\n");
	exit (-1);
}

static gboolean inline
is_valid_char (guchar x)
{
	if ((x >= '0' && x <= '9') ||
	    (x >= 'A' && x <= 'Z') ||
	    (x >= 'a' && x <= 'z') ||
	    (x == '<') || (x == '>') ||
	    (x == '[') || (x == ']') ||
	    (x == '(') || (x == ')') ||
	    (x == '/') || (x == '\\') ||
	    (x == '.') || (x == '-') ||
	    (x == '.') || (x == '+') ||
	    (x == ']') || (x == ':') ||
	    (x == '_'))
		return TRUE;
	return FALSE;
}
	

#define move_whitespace_backwards(x) {while (*x == ' ' || *x == '\n' || \
						*x == '\r' || *x == '\t' || \
						*x == 13) {x--;}}
#define move_whitespace_forward(x) {while (*x == ' ' || *x == '\n' || \
						*x == '\r' || *x == '\t' || \
						*x == 13) {x++;}}
#define move_alphnum_forward(x) {while (is_valid_char (*x)) {x++;}} 
#define move_digits_backwards(x) {while (*x >= '0' && *x <= '9') {x--;}}
#define move_digits_forward(x) {while (*x >= '0' && *x <= '9') {x++;}}

static guchar *
my_get_next_token (guchar **in_)
{
	guchar *in = *in_;
	guchar *helper;
	guchar *token;

	move_whitespace_forward (in);
	*in_ = in;
	helper = in;
	move_alphnum_forward (in);
	token = g_strndup (helper, in - helper);
	*in_ = *in_ + (in - helper);
	
	return token;
}

static guchar *
my_get_next_token_backwards (guchar *in)
{
	guchar *helper;
	guchar *token;

	helper = in;
	move_digits_backwards (in);
	token = g_strndup (in + 1, helper - in);
	
	return token;
}

static gint
my_find_xref_pos (guchar *in, gint size)
{
	guchar *end;
	gchar *eof_str;
	gint eof_len;
	gchar *xref_pos_str;
	gint xref_pos;

	eof_str = g_strdup ("%%EOF");
	eof_len = strlen (eof_str);
	
	g_return_val_if_fail (size > 0, FALSE);
	g_return_val_if_fail (in != NULL, FALSE);

	end = in + size - 1;
	move_whitespace_backwards (end);
	if (strncmp (end - eof_len + 1, eof_str, eof_len)) {
		my_error ("Could not find %s", eof_str);
	}
	if (debug)
		g_print ("Found EOF with %d extra chars\n", size + in - end);
	end -= eof_len;
	move_whitespace_backwards (end);
	
	xref_pos_str = my_get_next_token_backwards (end);
	xref_pos = atoi (xref_pos_str);
	g_free (xref_pos_str);

	if (debug)
		   g_print ("xref pos is at: %d\n", xref_pos);

	return xref_pos;
}

static gint
my_xref_guess_location (guchar *in, gint size)
{
	guchar *i;

	i = in;
	
	while (TRUE) {
		if (*in == 'x') {
			if (strncmp (in, "xref", strlen ("xref")) == 0) {
				break;
			}
		}
#if 0	
		if (i + size <= in)
			return -1;
#endif	
		in++;
	}
	
	return in - i;
}

#define my_check_token(x,y) if ((strlen (x) != strlen (y)) || strncmp (x, y, strlen(x)) != 0) \
						my_error ("Token error. Found \"%s\" where \"%s\" was "\
						"expected.", y, x);

static GList *
my_get_offsets (guchar *in, gint size)
{
	guchar *token, *token2, *token3;
	GList *list = NULL;
	gint xref_pos;

	xref_pos = my_find_xref_pos (in, size);
	if (xref_pos >= size)
		my_error ("Xref address is larger then the file size [%d:%d]", xref_pos, size);
	in = in + xref_pos;
	if (strncmp (in, "xref", 4) != 0) {
		gint location;
		in = in - xref_pos;
		location = my_xref_guess_location (in, size);
		my_error ("Xref is not at the location it was expected [%d] I think it is at %d", xref_pos, location);
	}
	in += 4;
	move_whitespace_forward (in);
	move_digits_forward (in);
	move_whitespace_forward (in);
	move_digits_forward (in);
	move_whitespace_forward (in);

	if (debug)
		   g_print ("Checking xref ..\n");
	token = my_get_next_token (&in);
	my_check_token ("0000000000", token);
	token = my_get_next_token (&in);
	my_check_token ("65535", token);
	token = my_get_next_token (&in);
	my_check_token ("f", token);

	token = my_get_next_token (&in);
	while (strcmp (token, "trailer") != 0) {
		gint pos;
		token2 = my_get_next_token (&in);
		token3 = my_get_next_token (&in);
		my_check_token ("n", token3);
		pos = atoi (token);
		list = g_list_prepend (list, GINT_TO_POINTER (pos));
		g_free (token);
		g_free (token2);		
		g_free (token3);
		token = my_get_next_token (&in);
	}
	
	return g_list_reverse (list);
}

static gint
my_get_size_from_object (guchar *in, gint object, GList *offsets)
{
	gint offset;
	gchar *token1, *token2, *token3, *token4;
	gchar *num_str;

	offset = GPOINTER_TO_INT (g_list_nth_data (offsets, object - 1));
	in += offset;
	
	token1 = my_get_next_token (&in);
	token2 = my_get_next_token (&in);
	token3 = my_get_next_token (&in);
	token4 = my_get_next_token (&in);

	num_str = g_strdup_printf ("%d", object);
	my_check_token (num_str, token1);
	my_check_token ("0", token2);
	my_check_token ("obj", token3);

	return atoi (token4);
}

static gint
my_guess_stream_length (guchar *in, gint stream_len)
{
	guchar *i;

	i = in;
	
	while (TRUE) {
		if (*in == 'e') {
			if (strncmp (in, "endstream", strlen ("endstream")) == 0) {
				break;
			}
		}
		in++;
	}
	
	return in - i;
}

static void
my_parse_array (guchar *token, guchar **in_)
{
	guchar *in = *in_;
	gint i = 0;

	if (debug > 1)
		g_print ("Inside parse array\n");

	while (TRUE) {
		gint len;
		if (debug > 1)
			g_print ("Parse array. Token %d.->%s<-\n", i, token);
		len = strlen (token);
		if (token [len-1] == ']') {
			if (len > 1)
			    i++;
			if (debug)
				g_print ("Found end of array. Array size %d\n", i);
			break;
		}
		i++;
		g_free (token);
		token = my_get_next_token (&in);
	}
	
	if (debug > 1)
		g_print ("Leaving parse array\n");

	*in_ = in;
}

static gint
my_guess_object_pos (guchar *in, gint num, gint in_size)
{
	gchar *s;
	guchar *i;

	if (debug)
		   g_print ("Start guess_object_pos\n");
	s = g_strdup_printf ("%d 0 obj", num);

	i = in;
	
	while (TRUE) {
		if (debug > 2) {
			g_print ("%d\n", in - i);
		}
		if (in - i > in_size) {
			g_print ("Object %d missing\n", num);
			return -3;
		}
		if (*in == s[0]) {
			if (strncmp (in, s, strlen (s)) == 0) {
				break;
			}
		}
		in++;
	}
	g_free (s);
	
	return in - i;
}

static void
my_check_object (gint num, gint offset, guchar *real_in, GList *offsets, gint in_size)
{
	guchar *in = real_in;
	guchar *end = in + in_size;
	gchar *string;
	gint len;
	gchar *token;
	gint stream_len = -1;
	gint next_object = GPOINTER_TO_INT (g_list_nth_data (offsets, num));

	if (debug)
		g_print ("Checking object %d at offset %d, next object starts at %d\n",
			    num, offset, next_object);
	
	/* Check that the object is at the location it was expected */
	if (debug > 1)
		g_print ("Check if the object is at the location it was expected\n");
	string = g_strdup_printf ("%d 0 obj", num);
	len = strlen (string);
	if (strncmp (in + offset, string, len) != 0) {
		gint guess = -1;
		guess = my_guess_object_pos (in, num, in_size);
		if (guess == -3)
			my_error ("Object %d is not located at offset %d, "
				  "and could not be found\n", num, offset);
		else
			my_error ("Object %d is not located at offset %d, "
				  "I think it is at %d\n",
				  num, offset, guess);
	}
	in = in + offset + len;
	if (debug > 1)
		g_print ("Object is as the offset expected\n");
	

	token = my_get_next_token (&in);
	if (strncmp (token, "<<", 2) != 0) {
		gchar *t1;

		if (debug > 1)
			g_print ("This object does not contain a dictionary\n");
		/* check for an object with just an integer or an array */
		if (*token == '[') {
			my_parse_array (token, &in);
 		}  else {
			t1 = my_get_next_token (&in);
			my_check_token ("endobj", t1);
			g_free (t1);
		}
		return;
	}

	if (debug > 1)
		g_print ("Starting while loop\n");
	
	while (strcmp (token, ">>") != 0) {
		if (strncmp (in, "0 obj", 5) == 0) {
			my_error ("We are starting to parse the next object");
		}
		g_free (token);
		token = my_get_next_token (&in);
		if (debug) {
			if (debug > 1)
				g_print ("[%s]\n", token);
			if (strlen (token) == 0) {
				my_error ("Len 0. Aborting!\n");
			}
		}
		if (strcmp (token, "/Length") == 0) {
			g_free (token);
			token = my_get_next_token (&in);
/*			g_print ("Size %s\n", token); */
			stream_len = atoi (token);
			g_free (token);
			token = my_get_next_token (&in);
			if (strncmp (token, "0", 1) == 0) {
				/* Length is provided as an external object */
				g_free (token);
				token = my_get_next_token (&in);
				my_check_token ("R", token);
				/* steram_len contains the object num */
				stream_len = my_get_size_from_object (real_in, stream_len, offsets);
			}
/*			g_print ("Stream len is %d\n", stream_len); */
		}
		/* We should recurse but i am lazy. Bah */
		if (strncmp (token, "<<", 2) == 0) {
			while (strcmp (token, ">>") != 0) {
				g_free (token);
				token = my_get_next_token (&in);
			}
			g_free (token);
			token = my_get_next_token (&in);
		}
		if (token [0] == '[') {
/*			g_print ("This is an array! -->%s<--\n", token); */
			my_parse_array (token, &in);
			token = g_strdup ("Dont' crash cause token was freed in parse array");
		}
	}
	g_free (token);
	token = my_get_next_token (&in);

	if (strncmp (token, "stream", 6) == 0) {
		gint a = 0;
		if (stream_len < 0)
			my_error ("We have a stream but we don't have a lenght entry [%d]", num);
		if (*in == '\n' || *in == '\r' || *in == 13) {
			in++; a++;
		}
		if (*in == '\n' || *in == '\r' || *in == 13) {
			in++; a++;
		}
		if (((in + stream_len + strlen ("endstream") > end))||
		     (strncmp (in + stream_len, "endstream", strlen ("endstream")) != 0)) {
			gint real_length;
			if (debug) {
				g_print ("We where checking in:\n");
#if 0
				for (x = 0; x < 20; x++)
					g_print (".%c", *(in + stream_len + x));
#endif	
			}
			real_length = my_guess_stream_length (in, stream_len);
			my_error ("Stream has a bad length in object %d. My guess is that lenght "
					"should be: %d but the PDF says %d",
					num, real_length, stream_len);
		}
		in += stream_len;
	} else if (strncmp (token, "endobj", 6) == 0) {
		/* This is what it was expected */
		;
	} else {
		my_error ("Object didn't ended on \"endobj\"");
	}

	if (debug)
		g_print ("Object %d looks good\n", num);
}

static gboolean
my_check_xref (guchar *in, gint size)
{
	GList *offsets;
	GList *list;
	gint object = 1;

	offsets = my_get_offsets (in, size);
	list = offsets;

	if (!offsets) {
		my_error ("Does not contain any objects");
	}

	while (list) {
		my_check_object (object, GPOINTER_TO_INT (list->data), in, offsets, size);
		object++;
		list = list->next;
	}

	if (debug)
		g_print ("The pdf looks fine!\n");
	return TRUE;
}

static gchar *
my_read_file (gchar *filename, gint *size)
{
	struct stat s;
	guchar *buf;
	gint fh;
	
	fh = open (filename, O_RDONLY);
	if (fh < 0) {
		my_error ("Could not read %s", filename);
	}
	if (fstat (fh, &s) != 0) {
		my_error ("Could not stat %s", filename);
	}
	*size = s.st_size;
#ifndef HAVE_MMAP
	my_error ("mmap unsupported on this platform");
	buf = NULL;
#else
	buf = mmap (NULL, s.st_size, PROT_READ, MAP_SHARED, fh, 0);
#endif
	close (fh);
	if ((buf == NULL) || (buf == (void *) -1)) {
		my_error ("Buff is NULL after mmaping");
	}

	return buf;
}

int
main (int argc, const char * argv[])
{
	poptContext popt;
	const gchar **args;
	gchar *in_filename;
	guchar *in;
	gint size;

	/* Args */
	popt = poptGetContext ("checkxref", argc, argv, options, 0);
	poptGetNextOpt (popt);
	args = poptGetArgs (popt);
	if (!args || !args[0])
		my_error ("Input file not specified");
	if (debug)
		   g_print ("Running pdfcheck with debug %d\n", debug);
	
	/* Read the filename */
	in_filename = g_strdup (args [0]);
	in = my_read_file (in_filename, &size);
	if (!in || in[0] == '\0')
		my_error ("Error while reading %s\n", in_filename);
	g_free (in_filename);

	/* Do your thing */
	g_type_init ();
	my_check_xref (in, size);

	return 0;
}
