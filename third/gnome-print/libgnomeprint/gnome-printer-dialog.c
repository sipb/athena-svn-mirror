/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * A simple printer widget and dialog
 *
 *
 * Authors:
 *   Raph Levien (raph@acm.org)
 *   Miguel de Icaza (miguel@gnu.org)
 *   Chema Celorio (chema@celorio.com)
 * 
 * TODO:
 *   Load the printer definition from some system profile
 *   
 */

#include <config.h>
#include <libgnomeprint/gnome-print-i18n.h>

#include <string.h>
#include <gtk/gtkwidget.h>
#include <libgnomeui/gnome-file-entry.h>
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeui/gnome-dialog-util.h>
#include <libgnomeui/gnome-uidefs.h>
#include <libgnomeui/gnome-stock.h>

#include <libgnomeprint/gnome-printer.h>
#ifdef ENABLE_LIBGPA
#include <libgnomeprint/gnome-printer-private.h>
#else
#include <libgnomeprint/gnome-printer-profile.h>
#endif

#include <libgnomeprint/gnome-printer-dialog.h>

#ifdef ENABLE_LIBGPA
#include <libgpa/gpa-printer.h>
#include <libgpa/gpa-generic-ps.h>
#include <libgpa/gpa-tags.h>
#include <gmodule.h>
#endif

struct _GnomePrinterWidget {
	GtkFrame frame;

#ifdef ENABLE_LIBGPA
	GList *printer_list;
	GpaPrinter *gpa_printer;
#else
	GnomePrinterProfileList *profiles;
	GnomePrinterProfile *profile;
#endif	
	GtkWidget *profile_selector;
	
	GtkWidget *label_state;
	GtkWidget *label_type;
	GtkWidget *label_location;
	GtkWidget *label_comment;

#ifdef ENABLE_LIBGPA	
	GtkWidget *print_to_file_button;
#else	
	GtkWidget *r1;
	GtkWidget *r2;
	GtkWidget *entry_command;
	GtkWidget *entry_filename;
#endif
	GtkAccelGroup *accel_group;
};

struct _GnomePrinterWidgetClass {
	GtkFrameClass parent_class;
};

struct _GnomePrinterDialog {
	GnomeDialog dialog;
	GnomePrinterWidget *gnome_printer_widget;
};

struct _GnomePrinterDialogClass {
	GnomeDialogClass parent_class;
};

static GnomeDialogClass *dialog_parent_class = NULL;
static GtkFrameClass    *widget_parent_class = NULL;

static gchar *defaultpath = NULL;

#ifdef ENABLE_LIBGPA
gboolean print_to_file = FALSE;
#endif

#ifdef ENABLE_LIBGPA
#else
static void
gnome_printer_widget_b_cb (GtkWidget *b, GnomePrinterWidget *pd)
{
	gtk_widget_set_sensitive (pd->entry_command, b == pd->r1);
	gtk_widget_set_sensitive (pd->entry_filename, b == pd->r2);
	gtk_widget_grab_focus
		(b == pd->r1 ? pd->entry_command : pd->entry_filename);
}
#endif

static void
set_text (GtkWidget *label, const char *msg)
{
	gtk_label_set_text (GTK_LABEL (label), msg ? msg : "");
}

#ifdef ENABLE_LIBGPA
static void
set_printer (GnomePrinterWidget *gpw, GpaPrinter *printer)
{
	const gchar *force_print_to_file;
	static gchar *printer_id = NULL;
	gboolean print_to_file_flag;
	gboolean temp;
	GList *list;

	g_return_if_fail (GNOME_IS_PRINTER_WIDGET (gpw));

	/* Set the printer that was used the last time the dialog
	 * was poped up */
	if (printer == NULL && printer_id != NULL) {
		gint idx = 0;
		list = gpw->printer_list;
		for (; list != NULL; list = list->next) {
			printer = (GpaPrinter *)list->data;
			g_return_if_fail (GPA_IS_PRINTER (printer));
			if (strcmp (printer_id, gpa_printer_get_id (printer)) == 0)
				break;
			idx++;
		}
		if (list == NULL)
			printer = NULL;
		else
			gtk_option_menu_set_history (GTK_OPTION_MENU (gpw->profile_selector), idx);
	}

	/* No printer ? get the default one */
	if (!printer)
		printer = gpa_printer_get_default (gpw->printer_list);
	g_return_if_fail (GPA_IS_PRINTER (printer));

	/* Save the printer_id for the next time the dialog is created */
	if (printer_id)
		g_free (printer_id);
	printer_id = gpa_printer_dup_id (printer);

	/* Set the printer */
	gpw->gpa_printer = printer;
	
	/* Set the labels */
	set_text (gpw->label_state, "");
	set_text (gpw->label_type, "");
	set_text (gpw->label_location, "");
	set_text (gpw->label_comment, "");

	/* Set the print to file toggle button */
	force_print_to_file =  gpa_printer_backend_info_get (printer,
							     "GNOME",
							     "ForcePrintToFile");

	print_to_file_flag = (force_print_to_file && strcmp (force_print_to_file, GPA_TAG_TRUE) == 0);
	gtk_widget_set_sensitive (gpw->print_to_file_button, !print_to_file_flag);
	temp = print_to_file;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gpw->print_to_file_button),
				      print_to_file_flag ? TRUE : print_to_file);
	/* This will trigger a print_to_file_toggled, so set print_to_file as it was
	 * was before we toggled it and thus saving what the user has set (or not set) */
	print_to_file = temp;
}

static void
printer_activate (GtkObject *item, GpaPrinter *printer)
{
	GnomePrinterWidget *gpw = gtk_object_get_user_data (item);

	set_printer (gpw, printer);
}

#else

static void
set_profile (GnomePrinterWidget *gpw, GnomePrinterProfile *pp)
{
	GnomePrinter *printer;
	const char *msg, *output;
	int command;
	
	gpw->profile = pp;

	/*
	 * Load the printer status
	 */
	printer = gnome_printer_widget_get_printer (gpw);
	msg = gnome_printer_str_status (gnome_printer_get_status (printer));
	gtk_label_set_text (GTK_LABEL (gpw->label_state), msg);
	gtk_object_unref (GTK_OBJECT (printer));

	/*
	 * Load the rest of the information fields
	 */
	set_text (gpw->label_type, gnome_printer_profile_get_mime_type (pp));
	set_text (gpw->label_location, gnome_printer_profile_get_location (pp));
	set_text (gpw->label_comment, gnome_printer_profile_get_comment (pp));

	/*
	 * Set the entry values
	 */
	output = gnome_printer_profile_get_output (pp);
	if ((output != NULL) && (strncmp (output, "command", 7) == 0)) {
		gtk_entry_set_text (GTK_ENTRY (gpw->entry_command), output+8);
		if (!GTK_TOGGLE_BUTTON (gpw->r1)->active)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gpw->r1), 1);
	} else {
		const char *msg;
		
		if ((output != NULL) && (strncmp (output, "file", 4) == 0))
			msg = output + 5;
		else
			msg = "output.ps";
		
		gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (gpw->entry_filename))), msg);

		if (!GTK_TOGGLE_BUTTON (gpw->r2)->active)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gpw->r2), 1);
	}

	/*
	 * Set the sensitivity of the widgets correctly
	 */
	if (GTK_TOGGLE_BUTTON (gpw->r1)->active)
		command = 1;
	else
		command = 0;
	
	gtk_widget_set_sensitive (gpw->entry_command, command);
	gtk_widget_set_sensitive (gpw->entry_filename, !command);
}

static void
profile_activate (GtkObject *item, GnomePrinterProfile *pp)
{
	GnomePrinterWidget *gpw = gtk_object_get_user_data (item);

	set_profile (gpw, pp);
}
#endif

static guint
label_at (GtkTable *t, const char *string, int col, int row)
{
	GtkWidget *l = gtk_label_new ("");
	guint key = gtk_label_parse_uline (GTK_LABEL (l), string);

	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach (
		t, l,
		col, col+1, row, row+1,
		GTK_FILL|GTK_EXPAND, 0, GNOME_PAD_SMALL, GNOME_PAD_SMALL);

	return key;
}

static GtkWidget *
empty_label_at (GtkTable *t, int col, int row)
{
	GtkWidget *l = gtk_label_new ("");

	gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
	gtk_table_attach (t, l,
			  col, col+1, row, row+1,
			  GTK_FILL|GTK_EXPAND, 0, GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	return l;
}

#ifdef ENABLE_LIBGPA
/* IMPORTANT : This struct is a mirror of the one in gnome-print-admin/libgpaui/gpa-config.c
 * keep them in sync */
typedef struct _GpaConfigDlopenData GpaConfigDlopenData;
struct _GpaConfigDlopenData
{
	gint (*gpa_config_printer) (GpaPrinter *printer, gboolean opened_by_app);
	gint (*init_gpa_config_printer) (GpaConfigDlopenData *pd);
};

static void
gnome_printer_dialog_gpa_not_installed (const gchar *error)
{
	GnomeDialog *dialog;
	gchar *errstr = g_strdup (_("gnome-print-admin was not found.\n\n"
				    "Please install gnome-print-admin to configure this printer"));
	dialog = GNOME_DIALOG (gnome_error_dialog (errstr));
	gnome_dialog_run_and_close (dialog);
	g_free (errstr);
}

static void
gnome_printer_dialog_properties_clicked (GtkWidget *widget, GnomePrinterWidget *gpw)
{
	GpaConfigDlopenData *gpa_config;
	GModule *handle = NULL;
	gchar *error = NULL;

	gpa_config = g_new0 (GpaConfigDlopenData, 1);

	/* 1, is gmodule suported int his plataform ? */
	if (!g_module_supported ()) {
		error = g_strdup ("g_module not supported on this plataform");
		goto gnome_printer_dialog_properties_clicked_error;
	}

	/* 2. Can we open the library ? */
	handle = g_module_open (GNOMEPRINT_LIBDIR "/libgpaui.so", 0);
	if (handle == NULL) {
		error = g_strdup (g_module_error ());
		goto gnome_printer_dialog_properties_clicked_error;
	}

	/* 3. Can we find the init function ? */
	if (!g_module_symbol (handle, "init_gpa_config_printer", 
			      (gpointer*)&gpa_config->init_gpa_config_printer)) {
		error = g_strdup ("Could not find the init_gpa_config_printer function in libgpaui");
		goto gnome_printer_dialog_properties_clicked_error;
	}

	gpa_config->init_gpa_config_printer (gpa_config);

	/* 4. Did the init function load the config function ? */
	if (!gpa_config->gpa_config_printer) {
		error = g_strdup ("Init function did not set ->gpa_config_printer");
		goto gnome_printer_dialog_properties_clicked_error;
	}

	/* Go, go, go  ... */
	gpa_config->gpa_config_printer (gpw->gpa_printer, TRUE);

	/* gpa_config printer enters in it's own gtk_main loop so we can free
	 * the struct and close the g_module after it is done */
	g_free (gpa_config);
	if (handle)
		g_module_close (handle);
	
	return;
	
gnome_printer_dialog_properties_clicked_error:
	gnome_printer_dialog_gpa_not_installed (error);
	if (error)
		g_free (error);
	g_free (gpa_config);
	if (handle)
		g_module_close (handle);
	
	return;
}

static void
gnome_printer_dialog_print_to_file_toggled (GtkWidget *button, gpointer no_used)
{
	g_return_if_fail (GTK_IS_BUTTON (button));
	
	print_to_file = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
}

static GtkWidget *
gnome_printer_dialog_create_print_to_file (GnomePrinterWidget *gpw)
{
	/* If this printer is Print to file only, set sensitivity
	 * to off
	 */
	GtkWidget *check_button;
	
	check_button = gtk_check_button_new_with_label (_("Print to file"));

	gpw->print_to_file_button = check_button;

	gtk_signal_connect (GTK_OBJECT (check_button), "toggled",
			    gnome_printer_dialog_print_to_file_toggled, NULL);

	return check_button;
}

static GtkWidget *
gnome_printer_dialog_create_properties_button (GnomePrinterWidget *gpw)
{
	GtkWidget *button;
	
	g_return_val_if_fail (GTK_IS_WIDGET (gpw), NULL);

	button = gtk_button_new_with_label (_("Properties"));

	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (gnome_printer_dialog_properties_clicked),
			    gpw);

	return button;
}
#endif


static GtkWidget *
gnome_printer_dialog_create_printers_option_menu (GnomePrinterWidget *gpw)
{
	GtkWidget *option_menu;
	GtkWidget *menu;
	GList *list;
	gint index = 0;
#ifdef ENABLE_LIBGPA
	gint index_counter = 0;
#endif	
	
	option_menu = gtk_option_menu_new ();
	menu = gtk_menu_new ();

#ifdef ENABLE_LIBGPA
	for (list = gpw->printer_list; list; list = list->next){
		GpaPrinter *printer;
		GtkWidget *item;
		const char *name;

		printer = list->data;

		g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);
		
		if (gpa_printer_is_default (printer))
			index = index_counter;
		index_counter++;
		
		name = gpa_printer_get_name (printer);
		item = gtk_menu_item_new_with_label (name);
		gtk_widget_show (item);
		gtk_menu_append (GTK_MENU (menu), item);
		gtk_signal_connect (
			GTK_OBJECT (item), "activate",
			GTK_SIGNAL_FUNC (printer_activate), printer);
		gtk_object_set_user_data (GTK_OBJECT (item), gpw);
	}
#else
	for (list = gpw->profiles; list; list = list->next){
		GnomePrinterProfile *pp = list->data;
		GtkWidget *item;
		const char *name;

		name = gnome_printer_profile_get_printer_name (pp);
		item = gtk_menu_item_new_with_label (name);
		gtk_widget_show (item);
		gtk_menu_append (GTK_MENU (menu), item);
		gtk_signal_connect (
			GTK_OBJECT (item), "activate",
			GTK_SIGNAL_FUNC (profile_activate), pp);
		gtk_object_set_user_data (GTK_OBJECT (item), gpw);
	}
#endif
	gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), index);

	gpw->profile_selector = option_menu;

	return option_menu;
}
	
	
static void
gnome_printer_widget_init (GtkObject *object)
{
	GnomePrinterWidget *gpw = GNOME_PRINTER_WIDGET (object);
	GtkFrame *frame = GTK_FRAME (object);
#ifdef ENABLE_LIBGPA
	static GList * printers_list = NULL;
	GtkWidget *properties_button;
	GtkWidget *print_to_file_button;
#else	
	GtkWidget *r1, *r2;
	guint command_key, filename_key;
#endif	
	GtkWidget *option_menu;
	GtkTable *t;
	guint profile_key;

	gpw->accel_group = gtk_accel_group_new ();

#ifdef ENABLE_LIBGPA
	if (!printers_list) {
		if (!gpa_printers_list_load (&printers_list) ||
		    (g_list_length (printers_list) == 0)) {
			GpaPrinter *ps_printer;
			ps_printer = gpa_generic_ps_printer ();
			if (ps_printer != NULL)
				printers_list = g_list_prepend (printers_list,
								ps_printer);
		}
	}
	gpw->printer_list = printers_list;
       
#else	
	gpw->profiles = gnome_printer_get_profiles ();
#endif	
	gtk_frame_set_label (frame, _("Select printer"));
	
	t = (GtkTable *) gtk_table_new (0, 0, FALSE);
	gtk_container_add (GTK_CONTAINER (object), GTK_WIDGET (t));

	/*
	 * Create the profile information display
	 */
	(void) label_at (t, _("State:"), 0, 7);
	(void) label_at (t, _("Type:"), 0, 8);
	(void) label_at (t, _("Location:"), 0, 9);
	(void) label_at (t, _("Comment:"), 0, 10);
	
	gpw->label_state    = empty_label_at (t, 1, 7);
	gpw->label_type     = empty_label_at (t, 1, 8);
	gpw->label_location = empty_label_at (t, 1, 9);
	gpw->label_comment  = empty_label_at (t, 1, 10);

#ifdef ENABLE_LIBGPA	
	print_to_file_button = gnome_printer_dialog_create_print_to_file (gpw);
	gtk_table_attach (t, print_to_file_button, 2, 3, 10, 11,
			  0, 0,
			  /*
			  GTK_EXPAND | GTK_FILL,
			  GTK_EXPAND | GTK_FILL,
			  */
			  2, 2);
#endif	
	/*
	 * Create the menu with the printer profiles
	 */
	option_menu = gnome_printer_dialog_create_printers_option_menu (gpw);
	gtk_table_attach (t, option_menu, 1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL,
			  GTK_EXPAND | GTK_FILL,
			  2, 2);

	profile_key = label_at (t, _("_Name:"), 0, 0);
	if (profile_key != GDK_VoidSymbol)
		gtk_widget_add_accelerator (gpw->profile_selector,
					    "grab_focus", gpw->accel_group,
					    profile_key, GDK_MOD1_MASK, 0);

#ifdef ENABLE_LIBGPA
	properties_button = gnome_printer_dialog_create_properties_button (gpw);
	gtk_table_attach (t, properties_button, 2, 3, 0, 1,
			  GTK_EXPAND | GTK_FILL,
			  GTK_EXPAND | GTK_FILL,
			  2, 2);
#else	
	r1 = gtk_radio_button_new_with_label (NULL, "");
	gpw->r1 = r1;
	command_key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (r1)->child),
								  _("_Printer"));
	if (command_key != GDK_VoidSymbol)
		gtk_widget_add_accelerator (gpw->r1, "clicked",
							   gpw->accel_group, command_key,
							   GDK_MOD1_MASK, 0);
	gtk_signal_connect (GTK_OBJECT (r1), "clicked",
			    (GtkSignalFunc) gnome_printer_widget_b_cb,
			    (gpointer) gpw);
	gtk_table_attach (GTK_TABLE (t), r1, 0, 1, 2, 3,
			  GTK_EXPAND | GTK_FILL,
			  GTK_EXPAND | GTK_FILL,
			  2, 2);
	
	r2 = gtk_radio_button_new_with_label (GTK_RADIO_BUTTON (r1)->group,
					      "");
	gpw->r2 = r2;
	filename_key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (r2)->child),
					      _("_File"));
	if (filename_key != GDK_VoidSymbol)
		gtk_widget_add_accelerator (gpw->r2, "clicked",
							   gpw->accel_group, filename_key,
							   GDK_MOD1_MASK, 0);
	gtk_table_attach (GTK_TABLE (t), r2, 0, 1, 3, 4,
			  GTK_EXPAND | GTK_FILL,
			  GTK_EXPAND | GTK_FILL,
			  2, 2);
	gtk_signal_connect (GTK_OBJECT (r2), "clicked",
			    (GtkSignalFunc) gnome_printer_widget_b_cb,
			    (gpointer) gpw);
	
	gpw->entry_command = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (gpw->entry_command), "lpr");
	gtk_table_attach (GTK_TABLE (t), gpw->entry_command, 1, 2, 2, 3,
			  GTK_EXPAND | GTK_FILL,
			  GTK_EXPAND | GTK_FILL,
			  2, 2);
	
	gpw->entry_filename = gnome_file_entry_new ("PRINTTOFILE", "Output file");
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (gpw->entry_filename), defaultpath ? defaultpath : g_get_home_dir ());
	gnome_file_entry_set_modal (GNOME_FILE_ENTRY (gpw->entry_filename), TRUE);
	gtk_table_attach (GTK_TABLE (t), gpw->entry_filename, 1, 2, 3, 4,
			  GTK_EXPAND | GTK_FILL,
			  GTK_EXPAND | GTK_FILL,
			  2, 2);
#endif	

#ifdef ENABLE_LIBGPA
	if (gpw->printer_list)
		set_printer (gpw, NULL);
#else	
	set_profile (gpw, gpw->profiles->data);
#endif	
	gtk_widget_show_all (GTK_WIDGET (gpw));
}

/**
 * gnome_printer_widget_is_print_to_file:
 * @gpd: The dialog that needs to be queried
 * 
 * Determines if the user selected print to file or not
 * 
 * Return Value: TRUE if print to file selected, FALSE otherwise
 **/
#ifdef ENABLE_LIBGPA
static gboolean
gnome_printer_widget_is_print_to_file (GnomePrinterWidget *gpw)
{
	g_return_val_if_fail (GNOME_IS_PRINTER_WIDGET (gpw), FALSE);
	
	return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gpw->print_to_file_button));
}
#endif	


static void
gnome_printer_widget_destroy (GtkObject *object)
{
	GnomePrinterWidget *gpw;
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_PRINTER_WIDGET (object));
	
	gpw = GNOME_PRINTER_WIDGET (object);
	if (gpw->accel_group)
		gtk_accel_group_unref (gpw->accel_group);

	if (GTK_OBJECT_CLASS (widget_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (widget_parent_class)->destroy) (object);
}

static void
gnome_printer_widget_finalize (GtkObject *object)
{
	GnomePrinterWidget *gpw;

	gpw = GNOME_PRINTER_WIDGET (object);
	
#ifdef	ENABLE_LIBGPA
#else	
	gnome_printer_profile_free_profiles (gpw->profiles);
#endif	
	
	(* GTK_OBJECT_CLASS (widget_parent_class)->finalize) (object);
}

static void
gnome_printer_widget_class_init (GnomePrinterDialogClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass*) class;
	
	widget_parent_class = gtk_type_class (gtk_frame_get_type ());
	
	object_class->destroy  = gnome_printer_widget_destroy;
	object_class->finalize = gnome_printer_widget_finalize;
}

GtkType
gnome_printer_widget_get_type (void)
{
	static GtkType printer_widget_type = 0;
	
	if (!printer_widget_type)
	{
		GtkTypeInfo printer_widget_info =
		{
			"GnomePrinterWidget",
			sizeof (GnomePrinterWidget),
			sizeof (GnomePrinterWidgetClass),
			(GtkClassInitFunc) gnome_printer_widget_class_init,
			(GtkObjectInitFunc) gnome_printer_widget_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		printer_widget_type = gtk_type_unique (gtk_frame_get_type (), &printer_widget_info);
	}
	
	return printer_widget_type;
}


#ifdef ENABLE_LIBGPA	
static GnomePrinter *
gnome_printer_create_from_widget (GnomePrinterWidget *gpw)
{
	GnomePrinter *printer;
	GpaPrinter *gpa_printer;
	gchar *command;
	gchar *file = NULL;

	g_return_val_if_fail (GNOME_IS_PRINTER_WIDGET (gpw), NULL);

	g_print ("Refing printer ..\n");
	gpa_printer_ref (gpw->gpa_printer);
	gpa_printer = gpw->gpa_printer;
	
	printer = gtk_type_new (gnome_printer_get_type ());

	printer->gpa_printer = gpa_printer;
	printer->gpa_settings = gpa_printer_settings_get_first (gpa_printer);
	printer->print_to_file = gnome_printer_widget_is_print_to_file (gpw);
	
	command = gnome_printer_dup_command (printer);
	if (printer->filename)
		g_free (printer->filename);
	if (file != NULL)
		printer->filename = g_strdup (file);
	else
		printer->filename = g_strdup_printf ("|%s", command);
	g_free (command);

	return printer;
}
#endif


/**
 * gnome_printer_widget_get_printer:
 * @widget: a GnomePrinterWidget
 *
 * Returns a GnomePrinter object which corresponds to the user
 * selected printer on the widget.
 *
 * This function might return a NULL value on errors.
 */
GnomePrinter *
gnome_printer_widget_get_printer (GnomePrinterWidget *widget)
{
#ifdef ENABLE_LIBGPA	
	GnomePrinter *printer;
	GnomePrinterWidget *gpw;

	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	gpw = GNOME_PRINTER_WIDGET (widget);
	
	printer = gnome_printer_create_from_widget (gpw);

	g_print ("Getting printer from gnome printer widget..\n");
	g_return_val_if_fail (GPA_IS_PRINTER (printer->gpa_printer), printer);
	return printer;
#else	
	GnomePrinter *printer = NULL;
	GnomePrinterWidget *gpw;
	
	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER_WIDGET (widget), NULL);

	gpw = GNOME_PRINTER_WIDGET (widget);

	if (GTK_TOGGLE_BUTTON (gpw->r1)->active){
		char *com = gtk_entry_get_text (GTK_ENTRY (gpw->entry_command));

		printer = gnome_printer_profile_get_printer (gpw->profile, NULL, com);
	} else if (GTK_TOGGLE_BUTTON (gpw->r2)->active){
		gchar * dir;
		gchar *fn;

		fn = gnome_file_entry_get_full_path (GNOME_FILE_ENTRY (gpw->entry_filename), FALSE);
		dir = g_dirname (fn);
		gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (gpw->entry_filename), dir);
		g_free (dir);
		printer = gnome_printer_profile_get_printer (gpw->profile, fn, NULL);
	}

	return printer;
#endif	
}

/**
 * gnome_printer_widget_new:
 *
 * Creates a widget that can be used to select a printer.
 * This widget can be inserted into a dialog box to provide
 * a more elaborate printer setup.
 *
 * Returns: the widget.
 */
GtkWidget *
gnome_printer_widget_new (void)
{
	GtkWidget *gpw;

	gpw = gtk_type_new (gnome_printer_widget_get_type ());

#ifdef ENABLE_LIBGPA	
	if (GNOME_PRINTER_WIDGET (gpw)->printer_list == NULL)
		return NULL;
#endif	

	return gpw;
}

void
gnome_printer_widget_bind_editable_enters (GnomePrinterWidget * gpw, GnomeDialog * dialog)
{
	g_return_if_fail (gpw != NULL);
	g_return_if_fail (GNOME_IS_PRINTER_WIDGET (gpw));
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOME_IS_DIALOG (dialog));

#ifdef ENABLE_LIBGPA
#else	
	gnome_dialog_editable_enters (dialog, GTK_EDITABLE (gpw->entry_command));
#if 0
	gnome_dialog_editable_enters (dialog, GTK_EDITABLE (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (gpw->entry_filename))));
#endif
#endif	
}

void
gnome_printer_widget_bind_accel_group (GnomePrinterWidget * gpw, GtkWindow * window)
{
	g_return_if_fail (gpw != NULL);
	g_return_if_fail (GNOME_IS_PRINTER_WIDGET (gpw));
	g_return_if_fail (window != NULL);
	g_return_if_fail (GTK_IS_WINDOW (window));

	gtk_window_add_accel_group (window, gpw->accel_group);
}


static void
gnome_printer_dialog_finalize (GtkObject *object)
{
	GnomePrinterDialog *printer_dialog;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_PRINTER_DIALOG (object));
	
	printer_dialog = GNOME_PRINTER_DIALOG (object);
	
	(* GTK_OBJECT_CLASS (dialog_parent_class)->finalize) (object);
}

static void
gnome_printer_dialog_class_init (GnomePrinterDialogClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass*) class;
	
	dialog_parent_class = gtk_type_class (gnome_dialog_get_type ());
	
	object_class->finalize = gnome_printer_dialog_finalize;
}

GtkType
gnome_printer_dialog_get_type (void)
{
	static GtkType printer_dialog_type = 0;
	
	if (!printer_dialog_type)
	{
		GtkTypeInfo printer_dialog_info =
		{
			"GnomePrinterDialog",
			sizeof (GnomePrinterDialog),
			sizeof (GnomePrinterDialogClass),
			(GtkClassInitFunc) gnome_printer_dialog_class_init,
			(GtkObjectInitFunc) NULL,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		printer_dialog_type = gtk_type_unique (gnome_dialog_get_type (), &printer_dialog_info);
	}
	
	return printer_dialog_type;
}

/**
 * gnome_printer_dialog_new:
 *
 * Creates a dialog box for selecting a printer.
 * This returns a GnomePrinterDialog object, the programmer
 * is resposible for querying the gnome_printer_dialog
 * to fetch the selected GnomePrinter object
 *
 * Returns: the GnomeDialog, ready to be ran.
 */
GtkWidget *
gnome_printer_dialog_new (void)
{
	GtkWidget *printer_dialog;
	GnomePrinterDialog *pd;
	GnomePrinterWidget *gpw;
	
	pd = gtk_type_new (gnome_printer_dialog_get_type ());
	printer_dialog = GTK_WIDGET (pd);
	
	gtk_window_set_title (GTK_WINDOW (printer_dialog), _("Select Printer"));
	
	gnome_dialog_append_button (GNOME_DIALOG(printer_dialog),
				    GNOME_STOCK_BUTTON_OK);
	
	gnome_dialog_append_button (GNOME_DIALOG(printer_dialog),
				    GNOME_STOCK_BUTTON_CANCEL);
	
	gnome_dialog_set_default (GNOME_DIALOG (printer_dialog), 0);
	
	pd->gnome_printer_widget = GNOME_PRINTER_WIDGET (gnome_printer_widget_new ());
	if (pd->gnome_printer_widget == NULL)
		return NULL;

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (printer_dialog)->vbox),
			    GTK_WIDGET (pd->gnome_printer_widget), TRUE, TRUE, 0);
	gpw = pd->gnome_printer_widget;

#ifdef	ENABLE_LIBGPA
#else	
	gnome_dialog_editable_enters (GNOME_DIALOG(pd),
				      GTK_EDITABLE(gpw->entry_command));
#if 0
	gnome_dialog_editable_enters (GNOME_DIALOG(pd),
				      GTK_EDITABLE(gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (gpw->entry_filename))));
#endif
#endif
	
	gtk_widget_grab_focus (gpw->profile_selector);
	gtk_window_add_accel_group (GTK_WINDOW (pd), gpw->accel_group);
	
	return printer_dialog;
}


/**
 * gnome_printer_dialog_new_modal:
 *
 * Runs a dialog that allows the user to select a target
 * printer and returns a GnomePrinter object.  It returns NULL
 * on failure or user cancel.
 *
 * This function runs the dialog and returns the user selection.
 *
 * Returns: A GnomePrinter object that represents the printer
 * selected by the user or NULL if the user cancels the operation.
 */
GnomePrinter *
gnome_printer_dialog_new_modal (void)
{
	GtkWidget *printer_dialog;
	GnomePrinterDialog *pd;
	GnomePrinter *printer;
	int button;

	printer_dialog = gnome_printer_dialog_new();
	pd = GNOME_PRINTER_DIALOG (printer_dialog);
	
	gtk_window_set_modal (GTK_WINDOW (printer_dialog),TRUE);
	button = gnome_dialog_run (GNOME_DIALOG (printer_dialog));
	
	if (button < 0)
		return NULL;
	
	printer = NULL;
	if (button == 0)
		printer = gnome_printer_dialog_get_printer (pd);
	
	gtk_widget_destroy (printer_dialog);

	return printer;
}

/**
 * gnome_printer_dialog_get_printer:
 * @dialog: a GnomePrinterDialog
 *
 * Returns: the GnomePrinter associated with the @dialog GnomePrinterDialog
 */
GnomePrinter *
gnome_printer_dialog_get_printer (GnomePrinterDialog *dialog)
{
	g_return_val_if_fail (dialog != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER_DIALOG (dialog), NULL);

	return gnome_printer_widget_get_printer (
		GNOME_PRINTER_WIDGET (dialog->gnome_printer_widget));
}
