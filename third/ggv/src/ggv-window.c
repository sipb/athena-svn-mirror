/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-window.c: the ggv shell
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#include <config.h>

#include <gnome.h>
#include <bonobo-activation/bonobo-activation.h>
#include <libgnomeui/gnome-window-icon.h>
#include <bonobo.h>
#include <bonobo/bonobo-ui-main.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h> 
#include <gdk/gdkx.h>

#include <math.h>
#include <ctype.h>

#include "gtkgs.h"
#include "gtkchecklist.h"
#include "ggv-prefs.h"
#include "ggv-window.h"
#include "ggv-file-sel.h"
#include "ggvutils.h"

#define GGV_POSTSCRIPT_VIEWER_CONTROL_IID "OAFIID:GNOME_GGV_Control"

static BonoboWindowClass *parent_class;

static GList *window_list = NULL;

/* what can be dragged in us... */
enum {
        TARGET_URI_LIST,
};

static void
sync_settings_menu_items(GgvWindow *win)
{
        if(win->uic == NULL)
                return;

	bonobo_ui_component_freeze(win->uic, NULL);
        bonobo_ui_component_set_prop(win->uic, "/commands/SettingsShowMenus", "state",
                                     win->show_menus?"1":"0", NULL);
        bonobo_ui_component_set_prop(win->uic, "/commands/SettingsShowToolbar", "state",
                                     win->show_toolbar?"1":"0", NULL);
        bonobo_ui_component_set_prop(win->uic, "/commands/SettingsShowSidebar", "state",
                                     win->show_sidebar?"1":"0", NULL);
        bonobo_ui_component_set_prop(win->uic, "/commands/SettingsShowStatusbar", "state",
                                     win->show_statusbar?"1":"0", NULL);
	bonobo_ui_component_thaw(win->uic, NULL);
}

static void
sync_settings_popup_items(GgvWindow *win)
{
        if(win->popup_uic == NULL)
                return;

	bonobo_ui_component_freeze(win->popup_uic, NULL);
        bonobo_ui_component_set_prop(win->popup_uic, "/commands/SettingsShowMenus", "state",
                                     win->show_menus?"1":"0", NULL);
        bonobo_ui_component_set_prop(win->popup_uic, "/commands/SettingsShowToolbar", "state",
                                     win->show_toolbar?"1":"0", NULL);
        bonobo_ui_component_set_prop(win->popup_uic, "/commands/SettingsShowSidebar", "state",
                                     win->show_sidebar?"1":"0", NULL);
        bonobo_ui_component_set_prop(win->popup_uic, "/commands/SettingsShowStatusbar", "state",
                                     win->show_statusbar?"1":"0", NULL);
	bonobo_ui_component_thaw(win->popup_uic, NULL);
}

void
sync_toolbar_style(GgvWindow *win)
{
        const gchar *tbstyle;
        gchar *gnome_tbstyle;

        switch(ggv_toolbar_style) {
        case GGV_TOOLBAR_STYLE_DEFAULT:
                gnome_tbstyle = gconf_client_get_string(ggv_prefs_gconf_client(),
                                                        "/desktop/gnome/interface/toolbar_style",
                                                        NULL);
                if(gnome_tbstyle) {
                        if(!strcmp(gnome_tbstyle, "icons"))
                                tbstyle = "icon";
                        else if(!strcmp(gnome_tbstyle, "both"))
                                tbstyle = "both";
                        else if(!strcmp(gnome_tbstyle, "text"))
                                tbstyle = "text";
                        else if(!strcmp(gnome_tbstyle, "both_horiz"))
                                tbstyle = "both_horiz";
                        else
                                tbstyle = "both";
                        g_free(gnome_tbstyle);
                }
                else
                        tbstyle = "both";
                break;
        case GGV_TOOLBAR_STYLE_BOTH:
                tbstyle = "both";
                break;
        case GGV_TOOLBAR_STYLE_ICONS:
                tbstyle = "icon";
                break;
        case GGV_TOOLBAR_STYLE_TEXT:
                tbstyle = "text";
                break;
        default:
                tbstyle = NULL;
                break;
        }
        if(tbstyle)
                bonobo_ui_component_set_prop(win->uic, "/Toolbar", "look",
                                             tbstyle, NULL);
}

static void
sync_fullscreen_items(GgvWindow *win)
{
        if(win->uic != NULL) {
                bonobo_ui_component_set_prop(win->uic, "/commands/ViewFullscreen", "state",
                                             win->fullscreen?"1":"0", NULL);
        }
        if(win->popup_uic != NULL) {
                bonobo_ui_component_set_prop(win->popup_uic, "/commands/ViewFullscreen", "state",
                                             win->fullscreen?"1":"0", NULL);
        }
}

/* utility functions to raise window above all others! */
#define XA_WIN_LAYER         "_WIN_LAYER"
typedef enum {
        WIN_LAYER_NORMAL = 4,
        WIN_LAYER_ABOVE_DOCK = 10
} GgvWindowLayer;
static Atom A_XA_WIN_LAYER;
static gboolean atom_initialized = FALSE;

static void
ggv_init_atoms()
{
        if(!GDK_DISPLAY() || atom_initialized) return;
        A_XA_WIN_LAYER = XInternAtom(GDK_DISPLAY(), XA_WIN_LAYER, False);
        atom_initialized = TRUE;
}

static void
ggv_window_set_layer(GgvWindow *win, GgvWindowLayer layer)
{
        if(!atom_initialized)
                ggv_init_atoms();
        if(GTK_WIDGET_MAPPED(win)) {
                XEvent e;
                e.type = ClientMessage;
                e.xclient.type = ClientMessage;
                e.xclient.window = GDK_WINDOW_XID(GTK_WIDGET(win)->window);
                e.xclient.message_type = A_XA_WIN_LAYER;
                e.xclient.format = 32;
                e.xclient.data.l[0] = (long)layer;
                e.xclient.data.l[1] = GDK_CURRENT_TIME;

                XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
                           SubstructureNotifyMask, (XEvent *)&e);
        }
        else {
                long data[1];

                data[0] = layer;
                XChangeProperty(GDK_DISPLAY(),
                                GDK_WINDOW_XID(GTK_WIDGET(win)->window),
                                A_XA_WIN_LAYER, XA_CARDINAL, 32,
                                PropModeReplace, (unsigned char *)data, 1);
        }
}

static void
ggv_window_set_fullscreen(GgvWindow *win, gboolean fs)
{
        if(win->fullscreen == fs)
                return;

        win->fullscreen = fs;
        if(win->fullscreen) {
                gint clx, cly, rx, ry, w, h;
                gdk_window_get_origin(GTK_WIDGET(win)->window, &rx, &ry);
                gdk_window_get_geometry(GTK_WIDGET(win)->window,
                                        &clx, &cly, &w, &h, NULL);
                win->orig_x = rx - clx;
                win->orig_y = ry - cly;
                win->orig_width = w;
                win->orig_height = h;
                if(win->show_menus) {
                        bonobo_ui_component_set_prop(win->uic, "/menu",
                                                     "hidden", "1", NULL);
                }
                if(win->show_toolbar) {
                        bonobo_ui_component_set_prop(win->uic, "/Toolbar",
                                                     "hidden", "1", NULL);
                }
                if(win->show_statusbar) {
                        gtk_widget_hide(win->statusbar);
                }
                if(win->show_sidebar) {
                        bonobo_ui_component_set_prop(win->uic, "/Sidebar",
                                                     "hidden", "1", NULL);
                }
                win->orig_sm = win->show_menus;
                win->orig_st = win->show_toolbar;
                win->orig_ss = win->show_sidebar;
                win->orig_sss = win->show_statusbar;
                win->show_menus = win->show_toolbar = win->show_sidebar = win->show_statusbar = FALSE;
                gtk_window_move(GTK_WINDOW(win), -clx, -cly);
                gtk_window_resize(GTK_WINDOW(win),
                                  gdk_screen_width(),
                                  gdk_screen_height());
        }
        else {
                win->show_menus = win->orig_sm;
                win->show_toolbar = win->orig_st;
                win->show_sidebar = win->orig_ss;
                win->show_statusbar = win->orig_sss;
                if(win->show_menus) {
                        bonobo_ui_component_set_prop(win->uic, "/menu",
                                                     "hidden", "0", NULL);
                }
                if(win->show_toolbar) {
                        bonobo_ui_component_set_prop(win->uic, "/Toolbar",
                                                     "hidden", "0", NULL);
                }
                if(win->show_statusbar) {
                        gtk_widget_show(win->statusbar);
                }
                if(win->show_sidebar) {
                        bonobo_ui_component_set_prop(win->uic, "/Sidebar",
                                                     "hidden", "0", NULL);
                }
                gtk_window_move(GTK_WINDOW(win), win->orig_x, win->orig_y);
                gtk_window_resize(GTK_WINDOW(win), win->orig_width, win->orig_height);
        }
        ggv_window_set_layer(win, fs?WIN_LAYER_ABOVE_DOCK:WIN_LAYER_NORMAL);
        sync_settings_menu_items(win);
        sync_settings_popup_items(win);
        sync_fullscreen_items(win);
}

static void
ggv_window_drag_data_received(GtkWidget *widget,
                              GdkDragContext *context,
                              gint x, gint y,
                              GtkSelectionData *selection_data,
                              guint info, guint time)
{
        GgvWindow *win = GGV_WINDOW(widget);

	if (info != TARGET_URI_LIST)
		return;

        win->uris_to_open = g_strsplit(selection_data->data, "\r\n", 0);
        if (context->suggested_action == GDK_ACTION_ASK) {
                GtkWidget *menu = gtk_menu_new ();
		
                bonobo_window_add_popup (BONOBO_WINDOW (win), 
                                         GTK_MENU (menu), 
                                         "/popups/DnD");
                gtk_menu_popup (GTK_MENU (menu),
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                0,
                                GDK_CURRENT_TIME);
        }
        else {
                GtkWidget *newwin;
                gchar **uri = win->uris_to_open;

                if(win->current_page < 0)
                        newwin = GTK_WIDGET(win);
                else
                        newwin = NULL;
                while(*uri && **uri != '\0') {
#ifdef _DEBUG
                        g_message("URI %s\n", *uri);
#endif /* _DEBUG */
                        if(newwin == NULL) {
                                newwin = ggv_window_new();
                                gtk_widget_show(newwin);
                        }
                        ggv_window_load(GGV_WINDOW(newwin), (*uri));
                        newwin = NULL;
                        uri++;
                }
                g_strfreev(win->uris_to_open);
                win->uris_to_open = NULL;
        }
}

static void
control_property_changed_handler(BonoboListener    *listener,
                                 char              *event_name, 
                                 CORBA_any         *any,
                                 CORBA_Environment *ev,
                                 gpointer           data)
{
        GgvWindow *win = GGV_WINDOW(data);

        if(!g_ascii_strcasecmp(event_name, "Bonobo/Property:change:title")) {
        }
        else if(!g_ascii_strcasecmp(event_name, "Bonobo/Property:change:status")) {
                gnome_appbar_set_status(GNOME_APPBAR(win->statusbar),
                                        BONOBO_ARG_GET_STRING(any));
        }
        else if(!g_ascii_strcasecmp(event_name, "Bonobo/Property:change:page")) {
                if(win->current_page == BONOBO_ARG_GET_LONG(any) &&
                   win->current_page < 0)
                        return;

                win->current_page = BONOBO_ARG_GET_LONG(any);
                if(win->current_page < 0) {
                        GtkWidget *dlg;
                        BonoboArg *arg;
                        gchar *msg;

                        gtk_window_set_title(GTK_WINDOW(win),
                                             _("GGV: no document loaded"));

                        arg = bonobo_pbclient_get_value(win->pb, "status",
                                                        NULL, NULL);
                        if(arg == NULL)
                                return;
                        msg = BONOBO_ARG_GET_STRING(arg);
                        if(*msg != '\0') {
                                dlg = gtk_message_dialog_new(GTK_WINDOW(win),
                                                             GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_OK,
                                                             msg);
                                gtk_widget_show(dlg);
                                gtk_dialog_run(GTK_DIALOG(dlg));
                                gtk_widget_destroy(dlg);
                        }
                        bonobo_arg_release(arg);
                        return;
                }
        }
        else if(!g_ascii_strcasecmp(event_name, "Bonobo/Property:change:page_count")) {
                bonobo_ui_component_set_prop(win->uic,
                                             "/commands/FileReload",
                                             "sensitive",
                                             (BONOBO_ARG_GET_LONG(any) > 0)?"1":"0",
                                             NULL);
                bonobo_ui_component_set_prop(win->uic,
                                             "/commands/FileClose",
                                             "sensitive",
                                             (BONOBO_ARG_GET_LONG(any) > 0)?"1":"0",
                                             NULL);
        }
}

static void
verb_FileOpen(BonoboUIComponent *uic, gpointer data, const char *cname)
{
        GgvWindow *win = GGV_WINDOW(data);
        gchar *fname, *escaped_fname;

        fname = ggv_file_sel_request_uri(_("Select a PostScript document"),
                                         win->filename);
        if(fname) {
                if(!ggv_window_load(win, fname)) {
                        GtkWidget *dlg;
                        dlg = gtk_message_dialog_new(GTK_WINDOW(win),
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK,
                                                     _("Unable to load file:\n%s"),
                                                     fname);
                        gtk_widget_show(dlg);
                        gtk_dialog_run(GTK_DIALOG(dlg));
                        gtk_widget_destroy(dlg);
                }
                g_free(fname);
        }
}

static void
verb_FileReload(BonoboUIComponent *uic, gpointer data, const char *cname)
{
        GgvWindow *win = GGV_WINDOW(data);
        GNOME_GGV_PostScriptView ps_view;
        CORBA_Environment ev;

        CORBA_exception_init(&ev);
        ps_view = (GNOME_GGV_PostScriptView)
                Bonobo_Unknown_queryInterface(win->control,
                                              "IDL:GNOME/GGV/PostScriptView:1.0",
                                              &ev);
        if(!BONOBO_EX(&ev)) {
                GNOME_GGV_PostScriptView_reload(ps_view, &ev);
                if(BONOBO_EX(&ev))
                        g_warning("Could not invoke reload method.");
                bonobo_object_release_unref(ps_view, &ev);
        }
        CORBA_exception_free(&ev);
}

static void
verb_FileClose(BonoboUIComponent *uic, gpointer data, const char *cname)
{
        GgvWindow *win = GGV_WINDOW(data);

        if(window_list->next != NULL)
                ggv_window_close(win);
        else {
                GNOME_GGV_PostScriptView ps_view;
                CORBA_Environment ev;

                CORBA_exception_init(&ev);
                ps_view = (GNOME_GGV_PostScriptView)
                        Bonobo_Unknown_queryInterface(win->control,
                                                      "IDL:GNOME/GGV/PostScriptView:1.0",
                                                      &ev);
                if(!BONOBO_EX(&ev)) {
                        GNOME_GGV_PostScriptView_close(ps_view, &ev);
                        if(BONOBO_EX(&ev))
                                g_warning("Could not invoke reload method.");
                        bonobo_object_release_unref(ps_view, &ev);
                }
                CORBA_exception_free(&ev);                
        }
}

static void
verb_FileNew(BonoboUIComponent *uic, gpointer data, const char *cname)
{
        GtkWidget *win = ggv_window_new();
        gtk_widget_show(win);
}

void
ggv_window_destroy_all()
{
        while(window_list)
                gtk_widget_destroy(GTK_WIDGET(window_list->data));
}

static void
verb_FileExit(BonoboUIComponent *uic, gpointer data, const char *cname)
{
        GList *l;

        ggv_window_destroy_all();
        bonobo_main_quit();
}

static void
listener_ViewFullscreen (BonoboUIComponent *uic, const char *path,
                         Bonobo_UIComponent_EventType type, const char *state,
                         gpointer user_data)
{
	GgvWindow *window;
        gboolean state_f;

	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GGV_IS_WINDOW (user_data));

	if(type != Bonobo_UIComponent_STATE_CHANGED)
		return;

	if(!state)
		return;

        window = GGV_WINDOW(user_data);
        state_f = atoi(state);

        if(!strcmp(path, "ViewFullscreen")) {
                if(window->fullscreen != state_f)
                        ggv_window_set_fullscreen(window, state_f);
        }
}

static void
listener_SettingsShow (BonoboUIComponent *uic, const char *path,
                       Bonobo_UIComponent_EventType type, const char *state,
                       gpointer user_data)
{
	GgvWindow *window;
        gboolean state_f;

	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GGV_IS_WINDOW (user_data));

	if(type != Bonobo_UIComponent_STATE_CHANGED)
		return;

	if(!state)
		return;

        window = GGV_WINDOW(user_data);
        state_f = atoi(state);

        if(!strcmp(path, "SettingsShowMenus")) {
                if(window->show_menus != state_f) {
                        window->show_menus = state_f;
                        bonobo_ui_component_set_prop(window->uic, "/menu", "hidden",
                                                     state_f?"0":"1", NULL);
                }
        }
	else if(!strcmp(path, "SettingsShowSidebar")) {
                if(window->show_sidebar != state_f) {
                        window->show_sidebar = state_f;
                        bonobo_ui_component_set_prop(window->uic, "/Sidebar", "hidden",
                                                     state_f?"0":"1", NULL);
                }
        }
	else if(!strcmp(path, "SettingsShowToolbar")) {
                if(window->show_toolbar != state_f) {
                        window->show_toolbar = state_f;
                        bonobo_ui_component_set_prop(window->uic, "/Toolbar", "hidden",
                                                     state_f?"0":"1", NULL);
                }
        }
	else if(!strcmp(path, "SettingsShowStatusbar")) {
                if(window->show_statusbar != state_f) {
                        window->show_statusbar = state_f;
                        if(state_f)
                                gtk_widget_show(window->statusbar);
                        else
                                gtk_widget_hide(window->statusbar);
                }
        }
        if(uic == window->uic)
                sync_settings_popup_items(window);
        else
                sync_settings_menu_items(window);
}

static void
verb_HelpAbout(BonoboUIComponent *uic, gpointer data, const char *cname)
{
	static GtkWidget *about = NULL;
	static const char *authors[] = {
                "Jaka Mocnik (current maintainer)",
                "Jonathan Blandford",
                "Daniel M. German",
                "Satyajit Kanungo",
                "Dan E. Kelley",
                "Werner Koerner",
                "Tuomas J. Lukka",
                "Johannes Plass",
		"Istvan Szekeres",
                "Tim Theisen",
                "And many more...",
 		NULL
	};

	static const char *documenters[] = {
		NULL
	};

	/* Translator Credits */
	char *translator_credits = _("translator_credits");

        static GdkPixbuf *logo = NULL;

        if(!logo)
                logo = gdk_pixbuf_new_from_file(GNOMEICONDIR
                                                "/ggv/ggv-splash.png",
                                                NULL);
	if (!about) {
		about = gnome_about_new (
			_("Gnome Ghostview"),
			VERSION,
			"Copyright \xc2\xa9 1998-2002 Free Software Foundation, Inc.",
			_("The GNOME PostScript document previewer"),
			authors,
			documenters,
			strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			logo);
                gtk_window_set_transient_for(GTK_WINDOW(about),
                                             GTK_WINDOW(data));
		g_signal_connect (G_OBJECT (about), "destroy",
                                  G_CALLBACK (gtk_widget_destroyed),
                                  &about);
	}

	gtk_widget_show_now (about);
	ggv_raise_and_focus_widget (about);
}

static void
verb_HelpContents(BonoboUIComponent *uic, gpointer data, const char *cname)
{
	GError *error = NULL;
	gnome_help_display ("ggv",NULL,&error);
}

static void
verb_DnDNewWindow(BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
        GgvWindow *win = GGV_WINDOW(user_data);
        gchar **uri;
        GtkWidget *newwin;

        uri = win->uris_to_open;
        while(*uri && **uri != '\0') {
#ifdef _DEBUG
                g_message("URI %s", *uri);
#endif /* _DEBUG */
                newwin = ggv_window_new();
                gtk_widget_show(newwin);
                ggv_window_load(GGV_WINDOW(newwin), (*uri));
                uri++;
        }
        g_strfreev(win->uris_to_open);
        win->uris_to_open = NULL;
}

static void
verb_DnDThisWindow(BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
        GgvWindow *win = GGV_WINDOW(user_data);
        GtkWidget *newwin;
        gchar **uri;

        uri = win->uris_to_open;
        newwin = GTK_WIDGET(win);
        while(*uri && **uri != '\0') {
#ifdef _DEBUG
                g_message("URI %s", *uri);
#endif /* _DEBUG */
                if(newwin == NULL) {
                        newwin = ggv_window_new();
                        gtk_widget_show(newwin);
                }
                ggv_window_load(GGV_WINDOW(newwin), (*uri));
                newwin = NULL;
                uri++;
        }
        g_strfreev(win->uris_to_open);
        win->uris_to_open = NULL;	
}

static void
verb_DnDCancel(BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
        GgvWindow *win = GGV_WINDOW(user_data);
        g_strfreev(win->uris_to_open);
        win->uris_to_open = NULL;	
}

/* our verb list */
static BonoboUIVerb ggv_app_verbs[] = {
        BONOBO_UI_VERB("FileNew", verb_FileNew),
        BONOBO_UI_VERB("FileClose", verb_FileClose),
        BONOBO_UI_VERB("FileOpen", verb_FileOpen),
        BONOBO_UI_VERB("FileReload", verb_FileReload),
        BONOBO_UI_VERB("FileExit", verb_FileExit),
        BONOBO_UI_VERB("HelpAbout", verb_HelpAbout),
        BONOBO_UI_VERB("Help", verb_HelpContents),
        BONOBO_UI_VERB("DnDNewWindow", verb_DnDNewWindow),
        BONOBO_UI_VERB("DnDThisWindow", verb_DnDThisWindow),
        BONOBO_UI_VERB("DnDCancel", verb_DnDCancel),
        BONOBO_UI_VERB_END
};

static void
control_frame_activate_uri(BonoboControlFrame *control_frame, const char *uri,
                           gboolean relative, gpointer data)
{
	GgvWindow *win;

	g_return_if_fail(uri != NULL);

        win = GGV_WINDOW(ggv_window_new());
        ggv_window_load(win, uri);
        gtk_widget_show(GTK_WIDGET(win));
}

static void
ggv_window_destroy(GtkObject *object)
{
        GgvWindow *win;

        g_return_if_fail(object != NULL);
        g_return_if_fail(GGV_IS_WINDOW(object));

        win = GGV_WINDOW(object);

        if(win->ctlframe != NULL) {
                bonobo_object_unref(BONOBO_OBJECT(win->ctlframe));
                win->ctlframe = NULL;
        }
        if(win->control != CORBA_OBJECT_NIL) {
                bonobo_object_release_unref(win->control, NULL);
                win->control = CORBA_OBJECT_NIL;
        }
        if(win->uic) {
                bonobo_object_unref(win->uic);
                win->uic = NULL;
        }
        if(win->popup_uic) {
                bonobo_object_unref(win->popup_uic);
                win->popup_uic = NULL;
        }
        if(win->pb) {
                bonobo_object_release_unref(win->pb, NULL);
                win->pb = CORBA_OBJECT_NIL;
        }
        if(win->filename) {
                g_free(win->filename);
                win->filename = NULL;
        }

        window_list = g_list_remove(window_list, win);

        ggv_get_window_size(GTK_WIDGET(object),
                            &ggv_default_width, &ggv_default_height);

        if(GTK_OBJECT_CLASS(parent_class)->destroy)
                GTK_OBJECT_CLASS(parent_class)->destroy(object);
}

static gboolean
ggv_window_delete_event(GtkWidget *widget, GdkEventAny *e)
{
        ggv_window_close(GGV_WINDOW(widget));
        return TRUE;
}

static void
prefs_changed(GConfClient* client, guint cnxn_id,
              GConfEntry *entry, gpointer user_data)
{
        if(!strcmp(entry->key, "/desktop/gnome/interface/toolbar_style") &&
           ggv_toolbar_style == GGV_TOOLBAR_STYLE_DEFAULT) {
                sync_toolbar_style(GGV_WINDOW(user_data));
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/toolbarstyle")) {
                ggv_toolbar_style = gconf_value_get_int(entry->value);
                sync_toolbar_style(GGV_WINDOW(user_data));
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/showtoolbar")) {
                GgvWindow *window = GGV_WINDOW(user_data);
                gboolean state_f = gconf_value_get_bool(entry->value);
                if(window->show_toolbar != state_f) {
                        window->show_toolbar = state_f;
                        sync_settings_menu_items(window);
                        sync_settings_popup_items(window);
                        bonobo_ui_component_set_prop(window->uic, "/Toolbar", "hidden",
                                                     state_f?"0":"1", NULL);
                }
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/showmenubar")) {
                GgvWindow *window = GGV_WINDOW(user_data);
                gboolean state_f = gconf_value_get_bool(entry->value);
                if(window->show_menus != state_f) {
                        window->show_menus = state_f;
                        sync_settings_menu_items(window);
                        sync_settings_popup_items(window);
                        bonobo_ui_component_set_prop(window->uic, "/menu", "hidden",
                                                     state_f?"0":"1", NULL);
                }
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/showpanel")) {
                GgvWindow *window = GGV_WINDOW(user_data);
                gboolean state_f = gconf_value_get_bool(entry->value);
                if(window->show_sidebar != state_f) {
                        window->show_sidebar = state_f;
                        sync_settings_menu_items(window);
                        sync_settings_popup_items(window);
                        bonobo_ui_component_set_prop(window->uic, "/Sidebar", "hidden",
                                                     state_f?"0":"1", NULL);
                }
        }
        else if(!strcmp(entry->key, "/apps/ggv/layout/showstatusbar")) {
                GgvWindow *window = GGV_WINDOW(user_data);
                gboolean state_f = gconf_value_get_bool(entry->value);
                if(window->show_statusbar != state_f) {
                        window->show_statusbar = state_f;
                        sync_settings_menu_items(window);
                        sync_settings_popup_items(window);
                        if(state_f)
                                gtk_widget_show(window->statusbar);
                        else
                                gtk_widget_hide(window->statusbar);
                }
        }
}

static void
ggv_window_class_init(GgvWindowClass *class)
{
	GObjectClass   *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	gobject_class = (GObjectClass *) class;
	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = g_type_class_peek_parent (class);

	object_class->destroy = ggv_window_destroy;

	widget_class->delete_event = ggv_window_delete_event;
	widget_class->drag_data_received = ggv_window_drag_data_received;
}

static void
ggv_window_init (GgvWindow *window)
{
        window_list = g_list_prepend(window_list, window);
}

GType
ggv_window_get_type (void) 
{
	static GType ggv_window_type = 0;
	
	if(!ggv_window_type) {
		static const GTypeInfo ggv_window_info =
		{
			sizeof(GgvWindowClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) ggv_window_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof(GgvWindow),
			0,		/* n_preallocs */
			(GInstanceInitFunc) ggv_window_init,
		};
		
		ggv_window_type = g_type_register_static(BONOBO_TYPE_WINDOW, 
                                                         "GgvWindow", 
                                                         &ggv_window_info, 0);
	}

	return ggv_window_type;
}

GtkWidget *
ggv_window_new(void)
{
        GgvWindow *win;
	BonoboUIContainer *ui_container;
	BonoboUIComponent *uic, *popup_uic;
        CORBA_Environment ev;
        Bonobo_Control control;
        GtkWidget *widget;
        Bonobo_PropertyBag pb;
        gchar *mask;

	static const GtkTargetEntry drag_types[] = {
		{ "text/uri-list", 0, TARGET_URI_LIST }
	};

        /* get the control */
        CORBA_exception_init(&ev);
        control = bonobo_get_object(GGV_POSTSCRIPT_VIEWER_CONTROL_IID,
                                    "Bonobo/Control", &ev);
        if(BONOBO_EX(&ev) || control == CORBA_OBJECT_NIL) {
                g_warning("Could not get GGV control: '%s'",
                          bonobo_exception_get_text(&ev));
                CORBA_exception_free(&ev);
                return NULL;
        }
        CORBA_exception_free(&ev);

	win = GGV_WINDOW(g_object_new(GGV_TYPE_WINDOW, "win_name", "ggv", "title", _("GGV: no document loaded"), NULL));
        win->control = control;

        win->current_page = -2;
        win->show_toolbar = ggv_toolbar;
        win->show_menus = ggv_menubar;
        win->show_statusbar = ggv_statusbar;
        win->show_sidebar = ggv_panel;

        /* a vbox */
        win->vbox = gtk_vbox_new(FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(win->vbox), 2);
        gtk_widget_show(win->vbox);

        /* a hbox at its top*/
        win->hbox = gtk_hbox_new(FALSE, 2);
        gtk_widget_show(win->hbox);
        gtk_box_pack_start(GTK_BOX(win->vbox), win->hbox,
                           TRUE, TRUE, 2);

	/* add statusbar */
	win->statusbar = gnome_appbar_new(FALSE, TRUE,
                                          GNOME_PREFERENCES_NEVER);
        if(ggv_statusbar)
                gtk_widget_show(GTK_WIDGET(win->statusbar));
	gtk_box_pack_end(GTK_BOX(win->vbox), GTK_WIDGET(win->statusbar),
                         FALSE, FALSE, 0);

        bonobo_window_set_contents(BONOBO_WINDOW(win), win->vbox);

	gtk_drag_dest_set (GTK_WIDGET (win),
			   GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
			   drag_types,
			   sizeof (drag_types) / sizeof (drag_types[0]),
			   GDK_ACTION_COPY | GDK_ACTION_ASK);

	/* add menu and toolbar */
	ui_container = bonobo_window_get_ui_container(BONOBO_WINDOW(win));
	uic = bonobo_ui_component_new("ggv");
        win->uic = uic;
	bonobo_ui_component_set_container(uic, BONOBO_OBJREF(ui_container),
                                          NULL);
	bonobo_ui_util_set_ui(uic, DATADIR, "ggv-ui.xml", "GGV", NULL);
        sync_toolbar_style(win);
	bonobo_ui_component_add_verb_list_with_data(uic, ggv_app_verbs, win);
	bonobo_ui_component_add_listener(uic, "SettingsShowMenus",
                                         listener_SettingsShow, win);
	bonobo_ui_component_add_listener(uic, "SettingsShowSidebar",
                                         listener_SettingsShow, win);
	bonobo_ui_component_add_listener(uic, "SettingsShowToolbar",
                                         listener_SettingsShow, win);
	bonobo_ui_component_add_listener(uic, "SettingsShowStatusbar",
                                         listener_SettingsShow, win);
	bonobo_ui_component_add_listener(uic, "ViewFullscreen",
                                         listener_ViewFullscreen, win);
        sync_settings_menu_items(win);

        if(!win->show_toolbar)
                bonobo_ui_component_set_prop(uic, "/Toolbar", "hidden", "1", NULL);
        if(!win->show_menus)
                bonobo_ui_component_set_prop(uic, "/menu", "hidden", "1", NULL);

	/* add control frame interface and bind it to ggv control */
	win->ctlframe = bonobo_control_frame_new(BONOBO_OBJREF(ui_container));
	bonobo_control_frame_set_autoactivate(win->ctlframe, FALSE);
	g_signal_connect(G_OBJECT(win->ctlframe), "activate_uri",
                         (GtkSignalFunc)control_frame_activate_uri, NULL);
	bonobo_control_frame_bind_to_control(win->ctlframe, control, NULL);
	widget = bonobo_control_frame_get_widget(win->ctlframe);
	gtk_widget_show(widget);
        gtk_box_pack_start(GTK_BOX(win->hbox), widget,
                           TRUE, TRUE, 0);
	bonobo_control_frame_control_activate(win->ctlframe);

        /* now get the control's property bag */
        pb = bonobo_control_frame_get_control_property_bag(win->ctlframe, NULL);
        if(pb == CORBA_OBJECT_NIL) {
                g_warning("Control does not have any properties.");
        }
        else {
                /* TODO: set initial status & title */
                mask =  "Bonobo/Property:change:page,"
                        "Bonobo/Property:change:page_count,"
                        "Bonobo/Property:change:width,"
                        "Bonobo/Property:change:height,"
                        "Bonobo/Property:change:title,"
                        "Bonobo/Property:change:status";
                bonobo_ui_component_set_prop(win->uic,
                                             "/commands/FileReload",
                                             "sensitive",
                                             (bonobo_pbclient_get_long(pb, "page", NULL) >= 0)?"1":"0",
                                             NULL);
                bonobo_ui_component_set_prop(win->uic,
                                             "/commands/FileClose",
                                             "sensitive",
                                             (bonobo_pbclient_get_long(pb, "page", NULL) >= 0)?"1":"0",
                                             NULL);
                bonobo_event_source_client_add_listener(pb,
                                                        (BonoboListenerCallbackFn)control_property_changed_handler,
                                                        mask, NULL, win);
                win->pb = pb;
        }

        /* merge our items in the control's popup menu */
        popup_uic = bonobo_control_frame_get_popup_component(win->ctlframe, NULL);
        if(popup_uic == NULL) {
                g_warning("Control does not have a popup component.");
        }
        else {
                bonobo_ui_util_set_ui(popup_uic, DATADIR, "ggv-ui.xml",
                                      "GGV", NULL);
                bonobo_ui_component_add_listener(popup_uic, "SettingsShowMenus",
                                                 listener_SettingsShow, win);
                bonobo_ui_component_add_listener(popup_uic, "SettingsShowSidebar",
                                                 listener_SettingsShow, win);
                bonobo_ui_component_add_listener(popup_uic, "SettingsShowToolbar",
                                                 listener_SettingsShow, win);
                bonobo_ui_component_add_listener(popup_uic, "SettingsShowStatusbar",
                                                 listener_SettingsShow, win);
                bonobo_ui_component_add_listener(popup_uic, "ViewFullscreen",
                                                 listener_ViewFullscreen, win);
                win->popup_uic = popup_uic;
                sync_settings_popup_items(win);
        }

	/* set default geometry */
        gtk_widget_set_usize(GTK_WIDGET(win),
                             ggv_default_width, ggv_default_height);
        gtk_window_set_policy(GTK_WINDOW(win), TRUE, TRUE, FALSE);

        gconf_client_notify_add(ggv_prefs_gconf_client(),
                                "/desktop/gnome/interface/toolbar_style",
                                (GConfClientNotifyFunc)prefs_changed,
                                win, NULL, NULL);
        gconf_client_notify_add(ggv_prefs_gconf_client(),
                                "/apps/ggv/layout",
                                (GConfClientNotifyFunc)prefs_changed,
                                win, NULL, NULL);

	return GTK_WIDGET(win);
}

gboolean
ggv_window_load(GgvWindow *win, const gchar *filename)
{
        CORBA_Environment ev;
        Bonobo_PersistFile pf;
        gchar *title, *unescaped, *utf8_title;

        g_return_val_if_fail(win != NULL, FALSE);
        g_return_val_if_fail(GGV_IS_WINDOW(win), FALSE);
        g_return_val_if_fail(filename != NULL, FALSE);

        CORBA_exception_init(&ev);
        pf = Bonobo_Unknown_queryInterface(win->control,
                                           "IDL:Bonobo/PersistFile:1.0", &ev);
        if(BONOBO_EX(&ev) || pf == CORBA_OBJECT_NIL) {
                CORBA_exception_free(&ev);
                return FALSE;
        }

        Bonobo_PersistFile_load(pf, filename, &ev);
        bonobo_object_release_unref(pf, NULL);

        if(win->filename)
                g_free(win->filename);
        win->filename = g_strdup(filename);

        unescaped = gnome_vfs_unescape_string_for_display(win->filename);
        title = g_strconcat(_("GGV: "), unescaped, NULL);
        g_free(unescaped);
        utf8_title = g_locale_to_utf8(title, -1, NULL, NULL, NULL);
        g_free(title);
        gtk_window_set_title(GTK_WINDOW(win), utf8_title);
        g_free(utf8_title);

        if(BONOBO_EX(&ev)) {
                CORBA_exception_free(&ev);
                return FALSE;
        }
        CORBA_exception_free(&ev);

        return TRUE;
}

void
ggv_window_close(GgvWindow *win)
{
        g_return_if_fail(win != NULL);
        g_return_if_fail(GGV_IS_WINDOW(win));

        gtk_widget_destroy(GTK_WIDGET(win));

        if(!window_list)
                bonobo_main_quit();
}

void
ggv_window_apply_prefs(GgvWindow *win)
{
        if(win) {
                sync_toolbar_style(win);
        }
        else {
                GList *node = window_list;
                while(node) {
                        ggv_window_apply_prefs(GGV_WINDOW(node->data));
                        node = node->next;
                }
        }
}

const GList *
ggv_get_window_list()
{
        return window_list;
}
