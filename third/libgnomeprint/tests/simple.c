/*
 * This file is used to create simple test code for testing and developing
 * gnome-print. It is just a template for code samples to replicate a bug
 * or test a particular feature of gnome-print (Chema)
 */
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>

#define _TEXT "A"
#define TEXT "The quick brown fox jumps over the lazy dog."

static void
my_draw (GnomePrintContext *gpc)
{
	GnomeFont *font;

	gnome_print_beginpage (gpc, "1");

#if 0
	font = gnome_font_find_closest ("Sans Regular", 12);
	gnome_print_setfont (gpc, font);
	gnome_print_moveto (gpc, 100, 200);
	gnome_print_show (gpc, TEXT);
	font = gnome_font_find_closest ("Luxi Sans Regular", 12);
	gnome_print_setfont (gpc, font);
	gnome_print_moveto (gpc, 100, 180);
	gnome_print_show (gpc, TEXT);
#endif

#if 0
	font = gnome_font_find_closest ("Luxi Sans Regular", 12);
	gnome_print_setfont (gpc, font);
	gnome_print_moveto (gpc, 100, 200);
	gnome_print_show (gpc, TEXT);

	font = gnome_font_find_closest ("Sans Regular", 12);
	gnome_print_setfont (gpc, font);
	gnome_print_moveto (gpc, 100, 180);
	gnome_print_show (gpc, TEXT);
#endif

#if 1
	font = gnome_font_find_closest ("Luxi Sans Bold", 12);
	gnome_print_setfont (gpc, font);
	gnome_print_moveto (gpc, 100, 200);
	gnome_print_show (gpc, TEXT);

	font = gnome_font_find_closest ("Sans Bold", 12);
	gnome_print_setfont (gpc, font);
	gnome_print_moveto (gpc, 100, 180);
	gnome_print_show (gpc, TEXT);
#endif

	gnome_print_showpage (gpc);

	g_object_unref (G_OBJECT (font));
}

static void
my_print (void)
{
	GnomePrintJob *job;
	GnomePrintContext *gpc;
	GnomePrintConfig *config;
	
	job = gnome_print_job_new (NULL);
	gpc = gnome_print_job_get_context (job);
	config = gnome_print_job_get_config (job);

	gnome_print_config_set (config, "Printer", "GENERIC");
	gnome_print_job_print_to_file (job, "o.ps");
	gnome_print_config_set (config, GNOME_PRINT_KEY_PAPER_SIZE, "USLetter");

	my_draw (gpc);

	gnome_print_job_close (job);
	gnome_print_job_print (job);

	g_object_unref (G_OBJECT (config));
	g_object_unref (G_OBJECT (gpc));
	g_object_unref (G_OBJECT (job));
}

int
main (int argc, char * argv[])
{
	g_type_init ();
	
	my_print ();

	g_print ("Done...\n");

	return 0;
}
