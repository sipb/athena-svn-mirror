/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-prefs-ui.c: GGV preferences ui
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#include <config.h>

#include <math.h>

#include "ggv-prefs.h"
#include "ggvutils.h"
#include "gsdefaults.h"
#include "ggv-prefs-ui.h"

static GtkWindowClass *parent_class;

static void
ggv_prefs_dialog_apply(GgvPrefsDialog *dlg)
{
        GtkWidget *active;
        gint n, i;

        gtk_widget_set_sensitive(dlg->apply, FALSE);

        gtk_gs_defaults_set_interpreter_cmd
                (g_strdup(gtk_entry_get_text(GTK_ENTRY(dlg->gs))));
        gtk_gs_defaults_set_convert_pdf_cmd
                (g_strdup(gtk_entry_get_text(GTK_ENTRY(dlg->convert_pdf))));
        gtk_gs_defaults_set_ungzip_cmd
                (g_strdup(gtk_entry_get_text (GTK_ENTRY (dlg->unzip))));
        gtk_gs_defaults_set_unbzip2_cmd
                (g_strdup(gtk_entry_get_text (GTK_ENTRY (dlg->unbzip2))));
        gtk_gs_defaults_set_alpha_parameters
                (g_strdup(gtk_entry_get_text (GTK_ENTRY (dlg->alpha_params))));
        gtk_gs_defaults_set_scroll_step
                (gtk_spin_button_get_value(GTK_SPIN_BUTTON(dlg->scroll_step)));
        if(ggv_print_cmd)
                g_free(ggv_print_cmd);
        ggv_print_cmd = g_strdup(gtk_entry_get_text(GTK_ENTRY(dlg->print)));

        active = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(dlg->size))));
        n = gtk_gs_defaults_get_paper_count();
        for(i = 0; i < n; i++)
                if(active == dlg->size_choice[i]) {
                        gtk_gs_defaults_set_size(i);
                        break;
                }

        active = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(dlg->zoom))));
        for(i = 0; i <= ggv_max_zoom_levels; i++){
                if(active == dlg->zoom_choice[i]) {
                        gtk_gs_defaults_set_zoom_factor(ggv_zoom_levels[i]);
                        break;
                }
        }

        active = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(dlg->orientation))));
        for(i = 0; i <= ggv_max_orientation_labels; i++){
                if(active == dlg->orientation_choice[i]) {
                        gtk_gs_defaults_set_orientation(i);
                        break;
                }
        }

        active = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(dlg->units))));
        for(i = 0; i <= ggv_max_unit_labels; i++){
                if(active == dlg->unit_choice[i]) {
                        ggv_unit_index = i;
                        break;
                }
        }

        active = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(dlg->auto_fit))));
        for(i = 0; i <= ggv_max_auto_fit_modes; i++){
                if(active == dlg->auto_fit_choice[i]) {
                        gtk_gs_defaults_set_zoom_mode(i);
                        break;
                }
        }

        gtk_gs_defaults_set_respect_eof(GTK_TOGGLE_BUTTON(dlg->respect_eof)->active);
        gtk_gs_defaults_set_watch_doc(GTK_TOGGLE_BUTTON(dlg->watch)->active);
        gtk_gs_defaults_set_antialiased(GTK_TOGGLE_BUTTON(dlg->aa)->active);
        gtk_gs_defaults_set_override_size(GTK_TOGGLE_BUTTON(dlg->override_size)->active);
        gtk_gs_defaults_set_override_orientation(GTK_TOGGLE_BUTTON(dlg->override_orientation)->active);
        gtk_gs_defaults_set_show_scroll_rect(GTK_TOGGLE_BUTTON(dlg->show_scroll_rect)->active);

        ggv_panel = GTK_TOGGLE_BUTTON(dlg->sidebar)->active;
        ggv_save_geometry = GTK_TOGGLE_BUTTON(dlg->savegeo)->active;
	ggv_toolbar = GTK_TOGGLE_BUTTON(dlg->toolbar)->active;
        for(i = 0; i < GGV_TOOLBAR_STYLE_LAST; i++) {
                if(GTK_TOGGLE_BUTTON(dlg->tbstyle[i])->active) {
                        ggv_toolbar_style = (GgvToolbarStyle)i;
                        break;
                }
        }
        if(i == GGV_TOOLBAR_STYLE_LAST)
                ggv_toolbar_style = GGV_TOOLBAR_STYLE_DEFAULT;
        ggv_menubar = GTK_TOGGLE_BUTTON(dlg->mbar)->active;
        ggv_statusbar = GTK_TOGGLE_BUTTON(dlg->sbar)->active;
        ggv_autojump = GTK_TOGGLE_BUTTON(dlg->auto_jump)->active;
        ggv_pageflip = GTK_TOGGLE_BUTTON(dlg->page_flip)->active;
        ggv_right_panel = GTK_TOGGLE_BUTTON(dlg->right_panel)->active;

        gtk_gs_defaults_save();

        ggv_prefs_save();        
}

static void
ggv_prefs_dialog_ok_clicked(GtkWidget *widget, gpointer *data)
{
        GgvPrefsDialog *dlg = GGV_PREFS_DIALOG(data);

        gtk_widget_hide(GTK_WIDGET(dlg));
        ggv_prefs_dialog_apply(GGV_PREFS_DIALOG(dlg));
}


static void
ggv_prefs_dialog_apply_clicked(GtkWidget *widget, gpointer *data)
{
        GgvPrefsDialog *dlg = GGV_PREFS_DIALOG(data);

        ggv_prefs_dialog_apply(GGV_PREFS_DIALOG(dlg));
}


static void
ggv_prefs_dialog_cancel_clicked(GtkWidget *widget, gpointer *data)
{
        GgvPrefsDialog *dlg = GGV_PREFS_DIALOG(data);

        gtk_widget_hide(GTK_WIDGET(dlg));
}

static gboolean
ggv_prefs_dialog_delete_event(GtkWidget *widget, GdkEventAny *event)
{
        gtk_widget_hide(widget);

        return TRUE;
}

static void
ggv_prefs_dialog_class_init(GgvPrefsDialogClass *klass)
{
	parent_class = g_type_class_peek_parent (klass);

        GTK_WIDGET_CLASS(klass)->delete_event = ggv_prefs_dialog_delete_event;
}

static void
ggv_prefs_dialog_init (GgvPrefsDialog *dlg)
{
}

static void
ggv_prefs_dialog_changed(GgvPrefsDialog *dlg)
{
        gtk_widget_set_sensitive(dlg->ok, TRUE);
        gtk_widget_set_sensitive(dlg->apply, TRUE);
}

static void
prefs_dlg_change_handler(GtkWidget *widget, gpointer data)
{
        GgvPrefsDialog *dlg = GGV_PREFS_DIALOG(data);
        ggv_prefs_dialog_changed(dlg);
}

static void
ggv_prefs_dialog_setup(GgvPrefsDialog *dlg)
{
        int i;

        /* We have to find which zoom option to activate. */
        {
                gfloat zoom = gtk_gs_defaults_get_zoom_factor();
                gfloat mindist = 1000.0, dist;
                gint opt = 0;
                
                for (i = 0; i <= ggv_max_zoom_levels; i++) {
                        dist = fabs((double)(ggv_zoom_levels[i] - zoom));
                        if(dist < mindist) {
                                opt = i;
                                mindist = dist;
                        }
                        gtk_option_menu_set_history(GTK_OPTION_MENU(dlg->zoom), opt);
                }
        }

        /* set unit */
        gtk_option_menu_set_history(GTK_OPTION_MENU(dlg->units),
                                    ggv_unit_index);

        /* set auto-fit mode */
        gtk_option_menu_set_history(GTK_OPTION_MENU(dlg->auto_fit),
                                    gtk_gs_defaults_get_zoom_mode());

        /* set default size */
        i = gtk_gs_defaults_get_size();
        gtk_option_menu_set_history(GTK_OPTION_MENU(dlg->size), i);
        
        /* set override size */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->override_size), 
                                     gtk_gs_defaults_get_override_size());

        /* set fallback orientation */
        i = gtk_gs_defaults_get_orientation();
        gtk_option_menu_set_history(GTK_OPTION_MENU(dlg->orientation), i);

        /* set override orientation */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->override_orientation), 
                                     gtk_gs_defaults_get_override_orientation());

        /* set antialiasing */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->aa), 
                                     gtk_gs_defaults_get_antialiased());

        /* set respect EOF */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->respect_eof), 
                                     gtk_gs_defaults_get_respect_eof());

        /* set watch document */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->watch),
                                     gtk_gs_defaults_get_watch_doc());

        /* set show scroll rect */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->show_scroll_rect), 
                                     gtk_gs_defaults_get_show_scroll_rect());

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->sidebar),
                                     ggv_panel);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->mbar),
                                     ggv_menubar);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->toolbar),
                                     ggv_toolbar);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->tbstyle[ggv_toolbar_style]),
                                     TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->sbar),
                                     ggv_statusbar);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->savegeo),
                                     ggv_save_geometry);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->auto_jump),
                                     ggv_autojump);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->page_flip),
                                     ggv_pageflip);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->right_panel),
                                     ggv_right_panel);

        gtk_entry_set_text(GTK_ENTRY(dlg->gs),
                           gtk_gs_defaults_get_interpreter_cmd());
	gtk_entry_set_position(GTK_ENTRY(dlg->gs), 0);
        gtk_entry_set_text(GTK_ENTRY(dlg->alpha_params),
                           gtk_gs_defaults_get_alpha_parameters());
	gtk_entry_set_position(GTK_ENTRY(dlg->alpha_params), 0);
        gtk_entry_set_text(GTK_ENTRY(dlg->convert_pdf),
                           gtk_gs_defaults_get_convert_pdf_cmd());
	gtk_entry_set_position(GTK_ENTRY(dlg->convert_pdf), 0);
        gtk_entry_set_text(GTK_ENTRY(dlg->unzip),
                           gtk_gs_defaults_get_ungzip_cmd());
	gtk_entry_set_position(GTK_ENTRY(dlg->unzip), 0);
        gtk_entry_set_text(GTK_ENTRY(dlg->unbzip2),
                           gtk_gs_defaults_get_unbzip2_cmd());
	gtk_entry_set_position(GTK_ENTRY(dlg->unbzip2), 0);
        gtk_entry_set_text(GTK_ENTRY(dlg->print),
                           ggv_print_cmd);
	gtk_entry_set_position(GTK_ENTRY(dlg->print), 0);

        gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->scroll_step),
                                  gtk_gs_defaults_get_scroll_step());
	gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_MOUSE);

        gtk_widget_grab_default(dlg->cancel);

        gtk_widget_set_sensitive(dlg->ok, FALSE);
        gtk_widget_set_sensitive(dlg->apply, FALSE);
}

void
ggv_prefs_dialog_show(GgvPrefsDialog *dlg)
{
        if(GTK_WIDGET_VISIBLE(dlg))
                return;

        ggv_prefs_dialog_setup(dlg);
        gtk_widget_show(GTK_WIDGET(dlg));
}

static gboolean
ggv_prefs_escape_pressed(GtkAccelGroup *accel_group,
                         GObject *acceleratable,
                         guint keyval,
                         GdkModifierType modifier,
                         gpointer data)
{
        GgvPrefsDialog *dlg = GGV_PREFS_DIALOG(data);
        gtk_widget_hide(GTK_WIDGET(dlg));
        return TRUE;
}

GtkWidget *
ggv_prefs_dialog_new()
{
        static const gchar *tbstyle_labels[] = {
                N_("_Use GNOME defaults"),
                N_("Show _both icons and text"),
                N_("Show only _icons"),
                N_("Show only t_ext")
        };

        GgvPrefsDialog *dlg;
        GtkWidget *widget, *vbox, *sep, *bbox;
        GtkWidget *table, *label, *menu, *frame;
        gint i;
        GtkGSPaperSize *papersizes = gtk_gs_defaults_get_paper_sizes();
        GSList *tbstyle_group = NULL;
        GtkAccelGroup *dlg_accel_group;
        GClosure *closure;
        GtkObject *adj;

        dlg = GGV_PREFS_DIALOG(g_object_new(GGV_TYPE_PREFS_DIALOG, NULL));
        widget = GTK_WIDGET(dlg);

        vbox = gtk_vbox_new(FALSE, 2);
        gtk_widget_show(vbox);
        gtk_container_add(GTK_CONTAINER(dlg), vbox);
        gtk_container_set_border_width(GTK_CONTAINER(dlg), 2);

        dlg->notebook = gtk_notebook_new();
        gtk_widget_show(dlg->notebook);
        gtk_box_pack_start(GTK_BOX(vbox), dlg->notebook,
                           2, TRUE, TRUE);

        sep = gtk_hseparator_new();
        gtk_widget_show(sep);
        gtk_box_pack_start(GTK_BOX(vbox), sep, 0, FALSE, TRUE);

        bbox = gtk_hbutton_box_new();
        gtk_box_set_spacing(GTK_BOX(bbox), 4);
        gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
        gtk_widget_show(bbox);
        gtk_box_pack_end(GTK_BOX(vbox), bbox, 0, FALSE, FALSE);
        
        dlg->cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

        dlg_accel_group = gtk_accel_group_new();
        closure = g_cclosure_new(G_CALLBACK(ggv_prefs_escape_pressed),
                                 dlg, NULL);
        gtk_accel_group_connect(dlg_accel_group, GDK_Escape, 0, 0, closure);
        gtk_window_add_accel_group(GTK_WINDOW(dlg), dlg_accel_group);

        dlg->apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
        GTK_WIDGET_SET_FLAGS(dlg->apply, GTK_CAN_DEFAULT);
        gtk_widget_show(dlg->apply);
        g_signal_connect(G_OBJECT(dlg->apply), "clicked",
                         G_CALLBACK(ggv_prefs_dialog_apply_clicked), dlg);
        gtk_box_pack_start(GTK_BOX(bbox), dlg->apply, 0, FALSE, FALSE);

        GTK_WIDGET_SET_FLAGS(dlg->cancel, GTK_CAN_DEFAULT);
        gtk_widget_show(dlg->cancel);
        g_signal_connect(G_OBJECT(dlg->cancel), "clicked",
                         G_CALLBACK(ggv_prefs_dialog_cancel_clicked), dlg);
        gtk_box_pack_start(GTK_BOX(bbox), dlg->cancel, 0, FALSE, FALSE);

        dlg->ok = gtk_button_new_from_stock(GTK_STOCK_OK);
        GTK_WIDGET_SET_FLAGS(dlg->ok, GTK_CAN_DEFAULT);
        gtk_widget_show(dlg->ok);
        g_signal_connect(G_OBJECT(dlg->ok), "clicked",
                         G_CALLBACK(ggv_prefs_dialog_ok_clicked), dlg);
        gtk_box_pack_start(GTK_BOX(bbox), dlg->ok, 0, FALSE, FALSE);

        gtk_window_set_title(GTK_WINDOW(dlg), _("GGV Preferences"));

        dlg->zoom_choice = g_new0(GtkWidget *, ggv_max_zoom_levels + 1);
        dlg->orientation_choice = g_new0(GtkWidget *, ggv_max_orientation_labels + 1);
        dlg->unit_choice = g_new0(GtkWidget *, ggv_max_unit_labels + 1);
        dlg->auto_fit_choice = g_new0(GtkWidget *, ggv_max_auto_fit_modes + 1);

        /* Document page */
        table = gtk_table_new(10, 2, FALSE);

        /* zoom choice menu */
        label = gtk_label_new_with_mnemonic(_("_Default Zoom:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);

        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 0, 1,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);

        gtk_widget_show(label);
        dlg->zoom = gtk_option_menu_new();
        gtk_widget_show(dlg->zoom);
        menu = gtk_menu_new();

        /* We only go as far a as MENU_ZOOM_SIZE-1 because the last option
           is "other" */
        for(i = 0; i <= ggv_max_zoom_levels; i++) {
                dlg->zoom_choice[i] = gtk_menu_item_new_with_label(ggv_zoom_level_names[i]);
                gtk_widget_show(dlg->zoom_choice[i]);
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), dlg->zoom_choice[i]);
                g_signal_connect(G_OBJECT(dlg->zoom_choice[i]),
                                 "activate",
                                 G_CALLBACK(prefs_dlg_change_handler), dlg);
        }
        gtk_option_menu_set_menu(GTK_OPTION_MENU(dlg->zoom), menu);

        gtk_table_attach(GTK_TABLE(table), dlg->zoom,
                         1, 2, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->zoom);

        /* auto-fit choice menu */
        label = gtk_label_new_with_mnemonic(_("A_uto-fit mode:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);

        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 1, 2,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);

        gtk_widget_show(label);
        dlg->auto_fit = gtk_option_menu_new();
        gtk_widget_show(dlg->auto_fit);
        menu = gtk_menu_new();
        for(i = 0; i <= ggv_max_auto_fit_modes; i++) {
                dlg->auto_fit_choice[i] = gtk_menu_item_new_with_label(_(ggv_auto_fit_modes[i]));
                gtk_widget_show(dlg->auto_fit_choice[i]);
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), dlg->auto_fit_choice[i]);
                g_signal_connect(G_OBJECT(dlg->auto_fit_choice[i]),
                                 "activate",
                                 G_CALLBACK(prefs_dlg_change_handler), dlg);
        }
        gtk_option_menu_set_menu(GTK_OPTION_MENU(dlg->auto_fit), menu);

        gtk_table_attach(GTK_TABLE(table), dlg->auto_fit,
                         1, 2, 1, 2,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->auto_fit);

        /* units choice menu */
       label = gtk_label_new_with_mnemonic(_("_Coordinate units:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);

        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 2, 3,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);

        gtk_widget_show(label);
        dlg->units = gtk_option_menu_new();
        gtk_widget_show(dlg->units);
        menu = gtk_menu_new();
        for(i = 0; i <= ggv_max_unit_labels; i++) {
                dlg->unit_choice[i] = gtk_menu_item_new_with_label(_(ggv_unit_labels[i]));
                gtk_widget_show(dlg->unit_choice[i]);
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), dlg->unit_choice[i]);
                g_signal_connect(G_OBJECT(dlg->unit_choice[i]),
                                 "activate",
                                 G_CALLBACK(prefs_dlg_change_handler), dlg);
        }
        gtk_option_menu_set_menu(GTK_OPTION_MENU(dlg->units), menu);

        gtk_table_attach(GTK_TABLE(table), dlg->units,
                         1, 2, 2, 3,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->units);

        /* size choice menu */
        label = gtk_label_new_with_mnemonic(_("_Fallback page size:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 3, 4,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        dlg->size = gtk_option_menu_new();
        gtk_widget_show(dlg->size);
        menu = gtk_menu_new();
        dlg->size_choice = g_new0(GtkWidget *, gtk_gs_defaults_get_paper_count());
        for(i = 0; papersizes[i].name != NULL; i++) {
                dlg->size_choice[i] = gtk_menu_item_new_with_label(papersizes[i].name);
                gtk_widget_show(dlg->size_choice[i]);
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), dlg->size_choice[i]);
                g_signal_connect(G_OBJECT(dlg->size_choice[i]),
                                 "activate",
                                 G_CALLBACK(prefs_dlg_change_handler), dlg);
        }
        gtk_option_menu_set_menu(GTK_OPTION_MENU(dlg->size), menu);

        gtk_table_attach(GTK_TABLE(table), dlg->size,
                         1, 2, 3, 4,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->size);

        /* override media size */
        dlg->override_size = gtk_check_button_new_with_mnemonic (_("Override _document size"));
        gtk_table_attach(GTK_TABLE(table), dlg->override_size,
                         0, 2, 4, 5,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->override_size), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->override_size);

        /* orientation choice menu */
        label = gtk_label_new_with_mnemonic(_("Fallback _media orientation:"));
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 5, 6,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);

        dlg->orientation = gtk_option_menu_new();
        gtk_widget_show(dlg->orientation);
        menu = gtk_menu_new();
        for(i = 0; i <= ggv_max_orientation_labels; i++) {
                dlg->orientation_choice[i] = gtk_menu_item_new_with_label(_(ggv_orientation_labels[i]));
                gtk_widget_show(dlg->orientation_choice[i]);
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), dlg->orientation_choice[i]);
                g_signal_connect(G_OBJECT(dlg->orientation_choice[i]),
                                 "activate",
                                 G_CALLBACK(prefs_dlg_change_handler), dlg);
        }
        gtk_option_menu_set_menu(GTK_OPTION_MENU(dlg->orientation), menu);

        gtk_table_attach(GTK_TABLE(table), dlg->orientation,
                         1, 2, 5, 6,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_label_set_mnemonic_widget(GTK_LABEL (label), dlg->orientation);

        /* override orientation media */
        dlg->override_orientation = gtk_check_button_new_with_mnemonic (_("O_verride document orientation"));
        gtk_table_attach(GTK_TABLE(table), dlg->override_orientation,
                         0, 2, 6, 7,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->override_orientation), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->override_orientation);

        /* antialiasing */
        dlg->aa = gtk_check_button_new_with_mnemonic (_("A_ntialiasing"));
        gtk_table_attach(GTK_TABLE(table), dlg->aa,
                         0, 2, 7, 8,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->aa), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->aa);

        /* respect EOF */
        dlg->respect_eof = gtk_check_button_new_with_mnemonic (_("_Respect EOF comments"));
        gtk_table_attach(GTK_TABLE(table), dlg->respect_eof,
                         0, 2, 8, 9,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->respect_eof), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->respect_eof);

        /* watch file */
        dlg->watch = gtk_check_button_new_with_mnemonic (_("_Watch file"));
        gtk_table_attach(GTK_TABLE(table), dlg->watch,
                         0, 2, 9, 10,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->watch), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->watch);

        label = gtk_label_new(_("Document"));
        gtk_widget_show(label);
        gtk_widget_show(table);
        gtk_notebook_append_page(GTK_NOTEBOOK(dlg->notebook), table, label);

        /* Layout page */
        table = gtk_table_new(7, 1, FALSE);

        /* show side panel */
        dlg->sidebar = gtk_check_button_new_with_mnemonic(_("_Show side panel"));
        gtk_table_attach(GTK_TABLE(table), dlg->sidebar,
                         0, 1, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->sidebar), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->sidebar);

        dlg->right_panel = gtk_check_button_new_with_mnemonic(_("_Put side panel on the right-hand side"));
        gtk_table_attach(GTK_TABLE(table), dlg->right_panel,
                         0, 1, 1, 2,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->right_panel), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->right_panel);

        /* show menubar */
        dlg->mbar = gtk_check_button_new_with_mnemonic(_("Show _menubar"));
        gtk_table_attach(GTK_TABLE(table), dlg->mbar,
                         0, 1, 2, 3,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->mbar), "clicked",
                           G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->mbar);
	/* show toolbar */
        dlg->toolbar = gtk_check_button_new_with_mnemonic(_("Show _toolbar"));
        gtk_table_attach(GTK_TABLE(table), dlg->toolbar,
                         0, 1, 3, 4,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->toolbar), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->toolbar);

        /* toolbar style */
        frame = gtk_frame_new(_("Toolbar style"));
        gtk_widget_show(frame);
        vbox = gtk_vbox_new(TRUE, 0);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_widget_show(vbox);
        gtk_table_attach(GTK_TABLE(table), frame,
                         0, 1, 4, 5,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        for(i = 0; i < GGV_TOOLBAR_STYLE_LAST; i++) {
                dlg->tbstyle[i] = gtk_radio_button_new_with_mnemonic(tbstyle_group,
                                                                  _(tbstyle_labels[i]));
                tbstyle_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(dlg->tbstyle[i]));
                g_signal_connect(G_OBJECT(dlg->tbstyle[i]), "toggled",
                                 G_CALLBACK(prefs_dlg_change_handler), dlg);
                gtk_widget_show(dlg->tbstyle[i]);
                gtk_box_pack_start(GTK_BOX(vbox), dlg->tbstyle[i], TRUE, TRUE,
                                   GNOME_PAD_SMALL);
        }
                                   
        /* show statusbar */
        dlg->sbar = gtk_check_button_new_with_mnemonic(_("Show statusba_r"));
        gtk_table_attach(GTK_TABLE(table), dlg->sbar,
                         0, 1, 5, 6,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->sbar), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->sbar);

        /* save geometry */
        dlg->savegeo = gtk_check_button_new_with_mnemonic(_("Save _geometry"));
        gtk_table_attach(GTK_TABLE(table), dlg->savegeo,
                         0, 1, 6, 7,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->savegeo), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->savegeo);

        label = gtk_label_new(_("Layout"));
        gtk_widget_show(label);
        gtk_widget_show(table);
        gtk_notebook_append_page(GTK_NOTEBOOK(dlg->notebook), table, label);


        /* Navigation page */
        table = gtk_table_new(4, 2, FALSE);

        /* auto jump to beginning of page */
        dlg->auto_jump = gtk_check_button_new_with_mnemonic(_("_Jump to beginning of page"));
        gtk_table_attach(GTK_TABLE(table), dlg->auto_jump,
                         0, 2, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->auto_jump), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->auto_jump);
        /* automatically flip pages */
        dlg->page_flip = gtk_check_button_new_with_mnemonic(_("Automatically _flip pages"));
        gtk_table_attach(GTK_TABLE(table), dlg->page_flip,
                         0, 2, 1, 2,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->page_flip), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->page_flip);

        /* show previously visible part */
        dlg->show_scroll_rect = gtk_check_button_new_with_mnemonic(_("Outline _last visible part when scrolling"));
        gtk_table_attach(GTK_TABLE(table), dlg->show_scroll_rect,
                         0, 2, 9, 10,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->show_scroll_rect), "clicked",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->show_scroll_rect);

        /* scroll step */
        adj = gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.1, 0.1);
        dlg->scroll_step = gtk_spin_button_new(GTK_ADJUSTMENT(adj),
                                               0.01, 2);
        label = gtk_label_new_with_mnemonic(_("Amount of _visible area to scroll"));  
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 2, 3,
                         GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_table_attach(GTK_TABLE(table), dlg->scroll_step,
                         1, 2, 2, 3,
                         GTK_SHRINK | GTK_FILL | GTK_EXPAND,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        g_signal_connect(G_OBJECT(dlg->scroll_step), "value-changed",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->scroll_step);
        gtk_widget_show(label);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->scroll_step);

        label = gtk_label_new(_("Navigation"));
        gtk_widget_show(label);
        gtk_widget_show(table);
        gtk_notebook_append_page(GTK_NOTEBOOK(dlg->notebook), table, label);

        /* GhostScript page */
        table = gtk_table_new(4, 2, FALSE);

        /* interpreter */
        label = gtk_label_new_with_mnemonic(_("_Interpreter:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 0, 1,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        dlg->gs = gtk_entry_new();
        gtk_table_attach(GTK_TABLE(table), dlg->gs,
                         1, 2, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	g_signal_connect(G_OBJECT(dlg->gs), "activate",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
	g_signal_connect(G_OBJECT(dlg->gs), "changed",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->gs);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->gs);

        /* antialiasing */
        label = gtk_label_new_with_mnemonic(_("A_ntialiasing:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 1, 2,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        dlg->alpha_params = gtk_entry_new();
        gtk_table_attach(GTK_TABLE(table), dlg->alpha_params,
                         1, 2, 1, 2,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	g_signal_connect(G_OBJECT(dlg->alpha_params), "activate",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
	g_signal_connect(G_OBJECT(dlg->alpha_params), "changed",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->alpha_params);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->alpha_params);

        /* scan PDF command */
        label = gtk_label_new_with_mnemonic(_("Convert _PDF to PS:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 2, 3,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        dlg->convert_pdf = gtk_entry_new();
        gtk_table_attach(GTK_TABLE(table), dlg->convert_pdf,
                         1, 2, 2, 3,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	g_signal_connect(G_OBJECT(dlg->convert_pdf), "activate",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
	g_signal_connect(G_OBJECT(dlg->convert_pdf), "changed",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->convert_pdf);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->convert_pdf);

	/* unzip command: gzip */
        label = gtk_label_new_with_mnemonic(_("_Gzip:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 3, 4,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        dlg->unzip = gtk_entry_new();
        gtk_table_attach(GTK_TABLE(table), dlg->unzip,
                         1, 2, 3, 4,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	g_signal_connect(G_OBJECT(dlg->unzip), "activate",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
	g_signal_connect(G_OBJECT(dlg->unzip), "changed",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
	gtk_widget_show(dlg->unzip);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->unzip);

	/* unzip command: bzip2 */
        label = gtk_label_new_with_mnemonic(_("_Bzip2:"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 4, 5,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        dlg->unbzip2 = gtk_entry_new();
        gtk_table_attach(GTK_TABLE(table), dlg->unbzip2,
                         1, 2, 4, 5,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	g_signal_connect(G_OBJECT(dlg->unbzip2), "activate",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
	g_signal_connect(G_OBJECT(dlg->unbzip2), "changed",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
	gtk_widget_show(dlg->unbzip2);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->unbzip2);

        label = gtk_label_new(_("Ghostscript"));
        gtk_widget_show(label);
        gtk_widget_show(table);
        gtk_notebook_append_page(GTK_NOTEBOOK(dlg->notebook), table, label);

        /* Printing page */
        table = gtk_table_new(1, 2, FALSE);

        /* print command */
        label = gtk_label_new_with_mnemonic(_("_Print command:"));
        gtk_table_attach(GTK_TABLE(table), label,
                         0, 1, 0, 1,
                         GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
        gtk_widget_show(label);
        dlg->print = gtk_entry_new();
        gtk_table_attach(GTK_TABLE(table), dlg->print,
                         1, 2, 0, 1,
                         GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                         GTK_SHRINK | GTK_FILL,
                         GNOME_PAD_SMALL, GNOME_PAD_SMALL);
	g_signal_connect(G_OBJECT(dlg->print), "activate",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
	g_signal_connect(G_OBJECT(dlg->print), "changed",
                         G_CALLBACK(prefs_dlg_change_handler), dlg);
        gtk_widget_show(dlg->print);
        gtk_label_set_mnemonic_widget(GTK_LABEL(label), dlg->print);

        label = gtk_label_new(_("Printing"));
        gtk_widget_show(label);
        gtk_widget_show(table);
                
        gtk_notebook_append_page(GTK_NOTEBOOK(dlg->notebook), table, label);

        return widget;
}

GType
ggv_prefs_dialog_get_type (void) 
{
	static GType ggv_prefs_dialog_type = 0;
	
	if(!ggv_prefs_dialog_type) {
		static const GTypeInfo ggv_prefs_dialog_info =
		{
			sizeof(GgvPrefsDialogClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) ggv_prefs_dialog_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof(GgvPrefsDialog),
			0,		/* n_preallocs */
			(GInstanceInitFunc) ggv_prefs_dialog_init,
		};
		
		ggv_prefs_dialog_type = g_type_register_static(GTK_TYPE_WINDOW, 
                                                               "GgvPrefsDialog", 
                                                               &ggv_prefs_dialog_info, 0);
	}

	return ggv_prefs_dialog_type;
}
