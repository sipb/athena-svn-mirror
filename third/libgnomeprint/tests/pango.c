/*
 * This test program tests drawing a PangoLayout to a Postscript or PDF file
 *
 * Assembled from other tests by Owen Taylor <otaylor@redhat.com>
 */
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprint/gnome-print-pango.h>

#include <popt.h>

#define TOP_MARGIN 36.
#define LEFT_MARGIN 72.
#define RIGHT_MARGIN 72.

static char *prog_name;
static const char *input_file;
static char *text;
static int len;
static gchar*   options_font = NULL;
static gboolean options_markup = FALSE;
static gchar*   options_output = NULL;
static gboolean options_pdf = FALSE;
static int      options_rotate = 0;

double page_width, page_height;
double margin_left, margin_right, margin_top, margin_bottom;

static poptContext popt;

static const struct poptOption options[] = {
	{ "markup",    'm', POPT_ARG_NONE,   &options_markup,   0,
	  "Input uses Pango markup",  NULL},
	{ "font",      'f', POPT_ARG_STRING, &options_font,   0,
	  "Specify the font",  NULL},
	{ "output",    'o', POPT_ARG_STRING, &options_output,   0,
	  "Specify the output file",  NULL},
	{ "pdf",       'p', POPT_ARG_NONE,    &options_pdf,   0,
	  "Generate a pdf file instead of a Postscript one",  NULL},
	{ "rotate",    'r', POPT_ARG_INT,    &options_rotate, 0,
	  "Angle at which to rotate results" },
	POPT_AUTOHELP
	{ NULL }
};

static void
fail (const char *format, ...)
{
  const char *msg;
  
  va_list vap;
  va_start (vap, format);
  msg = g_strdup_vprintf (format, vap);
  g_printerr ("%s: %s\n", prog_name, msg);

  exit (1);
}

static void
my_draw (GnomePrintContext *gpc)
{
	PangoLayout *layout;
	int width, height;
	gnome_print_beginpage (gpc, "1");

	layout = gnome_print_pango_create_layout (gpc);
	if (options_font) {
		PangoFontDescription *desc = pango_font_description_from_string (options_font);
		pango_layout_set_font_description (layout, desc);
		pango_font_description_free (desc);
	}

	pango_layout_set_width (layout, (page_width - LEFT_MARGIN - RIGHT_MARGIN) * PANGO_SCALE);
	if (options_markup)
		pango_layout_set_markup (layout, text, len);
	else
		pango_layout_set_text (layout, text, len);
	pango_layout_get_size (layout, &width, &height);

	gnome_print_translate (gpc, page_width / 2, page_height / 2);
	gnome_print_rotate (gpc, options_rotate);
	gnome_print_translate (gpc,
			       - (double) width / (2 * PANGO_SCALE), (double) height / (2 * PANGO_SCALE));
	
	gnome_print_moveto (gpc, 0, 0);
	gnome_print_pango_layout (gpc, layout);
	g_object_unref (layout);

	gnome_print_showpage (gpc);
}

static void
my_print (void)
{
	GnomePrintJob *job;
	GnomePrintContext *gpc;
	GnomePrintConfig *config;
	gchar *out_file;
	
	job = gnome_print_job_new (NULL);
	gpc = gnome_print_job_get_context (job);
	config = gnome_print_job_get_config (job);

	if (options_pdf) {
		if (!gnome_print_config_set (config, "Printer", "PDF"))
			fail ("Could not set the printer to PDF\n");
	} else {
		gnome_print_config_set (config, "Printer", "GENERIC");
	}
	
	out_file = options_output ? g_strdup (options_output) : g_strdup_printf ("o.%s", options_pdf ? "pdf" : "ps");
	gnome_print_job_print_to_file (job, out_file);
	g_free (out_file);
	
	gnome_print_config_set (config, GNOME_PRINT_KEY_PAPER_SIZE, "USLetter");
	
	/* Layout size */
	gnome_print_job_get_page_size (job, &page_width, &page_height);
 
	my_draw (gpc);

	gnome_print_job_close (job);
	gnome_print_job_print (job);

	g_object_unref (G_OBJECT (config));
	g_object_unref (G_OBJECT (gpc));
	g_object_unref (G_OBJECT (job));
}

static void
usage (gchar *error)
{
	if (error)
		g_printerr ("Error: %s\n\n", error);

	if (error)
		poptPrintHelp (popt, stderr, 0);
	else
		poptPrintHelp (popt, stdout, 0);
	
	exit (-1);
}

int
main (int argc, char * argv[])
{
	GError *error = NULL;
	int ret;

	g_type_init ();

	prog_name = g_path_get_basename (argv[0]);
	
	popt = poptGetContext ("pango", argc, (const char **)argv, options, 0);

	ret = poptGetNextOpt (popt);
	if (ret != -1) {
		usage (NULL);
	}
	
	input_file = poptGetArg (popt);
	if (!input_file)
		usage ("Input file not specified");

	if (!g_file_get_contents (input_file, &text, &len, &error))
		fail ("%s\n", error->message);
	if (!g_utf8_validate (text, len, NULL))
		fail ("Text is not valid UTF-8");

	/* Make sure we have valid markup
	 */
	if (options_markup &&
	    !pango_parse_markup (text, -1, 0, NULL, NULL, NULL, &error))
		fail ("Cannot parse input as markup: %s", error->message);

	my_print ();

	return 0;
}
