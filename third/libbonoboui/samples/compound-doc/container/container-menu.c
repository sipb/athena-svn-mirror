#include "config.h"
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-selector.h>
#include <bonobo/bonobo-window.h>
#include <gtk/gtkfilesel.h>

#include "container-menu.h"
#include "container.h"
#include "container-filesel.h"
#include "document.h"

static void
verb_AddComponent_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *inst = user_data;
	char *required_interfaces [2] =
	    { "IDL:Bonobo/CanvasComponentFactory:1.0", NULL };
	char *obj_id;

	/* Ask the user to select a component. */
	obj_id = bonobo_selector_select_id (
		"Select an embeddable Bonobo component to add",
		(const gchar **) required_interfaces);

	if (!obj_id)
		return;

	sample_doc_add_component (inst->doc, obj_id);

	g_free (obj_id);
}

static void
load_ok_cb (GtkWidget *caller, SampleApp *app)
{
	GtkWidget *fs = app->fileselection;
	const gchar *filename = gtk_file_selection_get_filename
		(GTK_FILE_SELECTION (fs));

	if (filename)
		sample_doc_load (app->doc, filename);

	gtk_widget_destroy (fs);
}

static void
save_ok_cb (GtkWidget *caller, SampleApp *app)
{
	GtkWidget *fs = app->fileselection;
	const gchar *filename = gtk_file_selection_get_filename
	    (GTK_FILE_SELECTION (fs));

	if (filename)
		sample_doc_save (app->doc, filename);

	gtk_widget_destroy (fs);
}

static void
verb_FileSaveAs_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *app = user_data;

	container_request_file (app, TRUE, (GtkSignalFunc) save_ok_cb, app);
}

static void
verb_FileLoad_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *app = user_data;

	container_request_file (app, FALSE, (GtkSignalFunc) load_ok_cb, app);
}

static void
verb_PrintPreview_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
#if 0
	SampleApp *app = user_data;

	sample_app_print_preview (app);
#else
	g_warning ("Print Preview not implemented yet.");
#endif
}

static void
verb_HelpAbout_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
#if 0
	/* gnome_about would require a libgnomeui dependency */

	static const gchar *authors[] = {
		"ÉRDI Gergõ <cactus@cactus.rulez.org>",
		"Mike Kestner <mkestner@speakeasy.net",
		"Michael Meeks <michael@ximian.com>",
		NULL
	};

	GtkWidget *about = gnome_about_new ("sample-container-2", VERSION,
					    "(C) 2000-2001 ÉRDI Gergõ, Mike Kestner, and Ximian, Inc",
					    authors,
					    "Bonobo sample container", NULL);
	gtk_widget_show (about);
#endif
}

static void
verb_FileExit_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *app = user_data;

	sample_app_exit (app);
}

/*
 * The menus.
 */
static char ui_commands [] =
"<commands>\n"
"	<cmd name=\"AddEmbeddable\" _label=\"A_dd Embeddable\"/>\n"
"	<cmd name=\"FileOpen\" _label=\"_Open\"\n"
"		pixtype=\"stock\" pixname=\"Open\" _tip=\"Open a file\"/>\n"
"	<cmd name=\"FileSaveAs\" _label=\"Save _As...\"\n"
"		pixtype=\"stock\" pixname=\"Save\"\n"
"		_tip=\"Save the current file with a different name\"/>\n"
"	<cmd name=\"PrintPreview\" _label=\"Print Pre_view\"/>\n"
"	<cmd name=\"FileExit\" _label=\"E_xit\" _tip=\"Exit the program\"\n"
"		pixtype=\"stock\" pixname=\"Quit\" accel=\"*Control*q\"/>\n"
"	<cmd name=\"HelpAbout\" _label=\"_About...\" _tip=\"About this application\"\n"
"		pixtype=\"stock\" pixname=\"About\"/>\n"
"</commands>";

static char ui_data [] =
"<menu>\n"
"	<submenu name=\"File\" _label=\"_File\">\n"
"		<menuitem name=\"AddEmbeddable\" verb=\"\"/>\n"
"		<separator/>"
"		<menuitem name=\"FileOpen\" verb=\"\"/>\n"
"\n"
"		<menuitem name=\"FileSaveAs\" verb=\"\"/>\n"
"\n"
"		<placeholder name=\"Placeholder\"/>\n"
"\n"
"		<separator/>\n"
"		<menuitem name=\"PrintPreview\" verb=\"\"/>\n"
"		<separator/>\n"
"		<menuitem name=\"FileExit\" verb=\"\"/>\n"
"	</submenu>\n"
"\n"
"	<submenu name=\"Help\" _label=\"_Help\">\n"
"		<menuitem name=\"HelpAbout\" verb=\"\"/>\n"
"	</submenu>\n"
"</menu>";

static BonoboUIVerb sample_app_verbs[] = {
	BONOBO_UI_VERB ("AddEmbeddable", verb_AddComponent_cb),
	BONOBO_UI_VERB ("FileOpen", verb_FileLoad_cb),
	BONOBO_UI_VERB ("FileSaveAs", verb_FileSaveAs_cb),
	BONOBO_UI_VERB ("PrintPreview", verb_PrintPreview_cb),
	BONOBO_UI_VERB ("FileExit", verb_FileExit_cb),
	BONOBO_UI_VERB ("HelpAbout", verb_HelpAbout_cb),
	BONOBO_UI_VERB_END
};

void
sample_app_fill_menu (SampleApp *app)
{
	Bonobo_UIContainer corba_uic;
	BonoboUIComponent *uic;

	uic = bonobo_ui_component_new ("sample");
	corba_uic = BONOBO_OBJREF (bonobo_window_get_ui_container (
						BONOBO_WINDOW (app->win)));
	bonobo_ui_component_set_container (uic, corba_uic, NULL);

	bonobo_ui_component_set_translate (uic, "/", ui_commands, NULL);
	bonobo_ui_component_set_translate (uic, "/", ui_data, NULL);

	bonobo_ui_component_add_verb_list_with_data (uic, sample_app_verbs, app);
}
