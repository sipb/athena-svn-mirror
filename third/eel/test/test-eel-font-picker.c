#include "test.h"

#include <eel/eel-font-picker.h>

static void
update_font (GtkWidget *widget,
	     const char *font_name)
{
	PangoFontDescription *font;
	
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (font_name != NULL);

	font = pango_font_description_from_string (font_name);
	gtk_widget_modify_font (GTK_WIDGET (widget), font);
	pango_font_description_free (font);
}

static void
font_changed_update_label_callback (EelFontPicker *font_picker,
				    gpointer callback_data)
{
	char *font_file_name;

	g_return_if_fail (EEL_IS_FONT_PICKER (font_picker));
	g_return_if_fail (GTK_IS_LABEL (callback_data));

	font_file_name = eel_font_picker_get_selected_font (font_picker);
	update_font (GTK_WIDGET (callback_data), font_file_name);
	g_free (font_file_name);
}

static void
font_changed_update_file_name_callback (EelFontPicker *font_picker,
					gpointer callback_data)
{
	char *font_file_name;

	g_return_if_fail (EEL_IS_FONT_PICKER (font_picker));
	g_return_if_fail (EEL_IS_TEXT_CAPTION (callback_data));

	font_file_name = eel_font_picker_get_selected_font (font_picker);
	eel_text_caption_set_text (EEL_TEXT_CAPTION (callback_data), font_file_name);
	g_free (font_file_name);
}

int
main (int argc, char * argv[])
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *font_picker;
	GtkWidget *label;
	GtkWidget *file_name_caption;
	char *current_font;

	test_init (&argc, &argv);

	//eel_global_preferences_init ();

	window = test_window_new ("Font Picker Test", 10);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	font_picker = eel_font_picker_new (NULL);

	current_font = eel_font_picker_get_selected_font (EEL_FONT_PICKER (font_picker));
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<big><big><big>Something Big</big></big></big>");

	g_signal_connect (font_picker,
			  "changed",
			  G_CALLBACK (font_changed_update_label_callback),
			  label);

	file_name_caption = eel_text_caption_new ();
	eel_caption_set_title_label (EEL_CAPTION (file_name_caption),
				     "Current Font");
	
	g_signal_connect (font_picker,
			  "changed",
			  G_CALLBACK (font_changed_update_file_name_callback),
			  file_name_caption);
	
	gtk_box_pack_start (GTK_BOX (vbox), font_picker, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 10);
	gtk_box_pack_start (GTK_BOX (vbox), file_name_caption, TRUE, TRUE, 10);

	g_free (current_font);

	gtk_widget_show (font_picker);
	gtk_widget_show (label);
	gtk_widget_show (file_name_caption);
	gtk_widget_show (vbox);
	gtk_widget_show (window);

	gtk_main ();
	return test_quit (EXIT_SUCCESS);
}
