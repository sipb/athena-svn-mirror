/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/**
 * ggv-control.c
 *
 * Author:  Jaka Mocnik  <jaka@gnu.org>
 *
 * Copyright (c) 2001, 2002 Free Software Foundation
 */

#include <config.h>

#include <stdio.h>
#include <math.h>
#include <string.h>

#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>

#include <gnome.h>

#include <ggv-control.h>
#include <gsdefaults.h>
#include <ggvutils.h>

struct _GgvControlPrivate {
	GgvPostScriptView *ps_view;

	BonoboZoomable *zoomable;
	float zoom_level;
	gboolean has_zoomable_frame;

	GtkWidget *root;

	GtkObject *zoom_adj;
	BonoboControl *zoom_control;

	BonoboUIComponent *uic, *popup_uic;

	/* GConfClient notification ID */
	guint gconf_notify_id;
};

struct _GgvControlClassPrivate {
	int dummy;
};

static struct {
	gfloat level;
	gchar *path;
} zoom_level_items[] = {
	{ 1.0/4.0, "/commands/Zoom0104" },
	{ 1.0/2.0, "/commands/Zoom0102" },
	{ 3.0/4.0, "/commands/Zoom0304" },
	{ 1.0/1.0, "/commands/Zoom0101" },
	{ 3.0/2.0, "/commands/Zoom0302" },
	{ 2.0/1.0, "/commands/Zoom0201" },
	{ 4.0/1.0, "/commands/Zoom0401" },
	{ 0.0, NULL }
};

static BonoboControlClass *ggv_control_parent_class;

static void
ggv_control_destroy (BonoboObject *object)
{
	GgvControl *control;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GGV_IS_CONTROL (object));

	control = GGV_CONTROL (object);

	if (control->priv->gconf_notify_id) {
		gconf_client_notify_remove (gtk_gs_defaults_gconf_client (),
									control->priv->gconf_notify_id);
		control->priv->gconf_notify_id = 0;
	}
	if (control->priv->zoom_control) {
		bonobo_object_unref(BONOBO_OBJECT(control->priv->zoom_control));
		control->priv->zoom_control = NULL;
	}

	if(BONOBO_OBJECT_CLASS (ggv_control_parent_class)->destroy)
		BONOBO_OBJECT_CLASS (ggv_control_parent_class)->destroy (object);
}

static void
ggv_control_finalize (GObject *object)
{
	GgvControl *control;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GGV_IS_CONTROL (object));

	control = GGV_CONTROL (object);

	g_free (control->priv);

	G_OBJECT_CLASS (ggv_control_parent_class)->finalize (object);
}

static void
sync_zoom_level_items(GgvControl *control)
{
	int i;
	Bonobo_UIContainer container;

	container = bonobo_ui_component_get_container (control->priv->uic);
	if (container == CORBA_OBJECT_NIL)
		return;

	for(i = 0; zoom_level_items[i].path; i++) {
		if(fabs(ggv_postscript_view_get_zoom_factor(control->priv->ps_view) - zoom_level_items[i].level) < 0.001) {
			bonobo_ui_component_set_prop(control->priv->uic,
										 zoom_level_items[i].path,
										 "state", "1", NULL);
			return;
		}
	}
	bonobo_ui_component_set_prop(control->priv->uic,
								 "/commands/ZoomOther",
								 "state", "1", NULL);
}

static void
sync_auto_fit_items(GgvControl *control)
{
	int i;
	Bonobo_UIContainer container;
	GgvPostScriptView *ps_view = control->priv->ps_view;
	GtkWidget *gs;

	container = bonobo_ui_component_get_container (control->priv->uic);
	if (container == CORBA_OBJECT_NIL)
		return;

	gs = ggv_postscript_view_get_widget(ps_view);

	switch(gtk_gs_get_zoom_mode(GTK_GS(gs))) {
	case GTK_GS_ZOOM_ABSOLUTE:
		bonobo_ui_component_set_prop(control->priv->uic,
									 "/commands/AutoFitNone",
									 "state", "1", NULL);
		break;
	case GTK_GS_ZOOM_FIT_WIDTH:
		bonobo_ui_component_set_prop(control->priv->uic,
									 "/commands/AutoFitWidth",
									 "state", "1", NULL);
		break;
	case GTK_GS_ZOOM_FIT_PAGE:
		bonobo_ui_component_set_prop(control->priv->uic,
									 "/commands/AutoFitPage",
									 "state", "1", NULL);
		break;
	default:
		break;
	}
}

static void
sync_command_state(GgvControl *control)
{
	sync_auto_fit_items(control);
	sync_zoom_level_items(control);
}

static void
zoomable_set_frame_cb (BonoboZoomable *zoomable, GgvControl *control)
{
	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));

	control->priv->has_zoomable_frame = TRUE;
}

static void
zoomable_set_zoom_level_cb (BonoboZoomable *zoomable, float new_zoom_level,
							GgvControl *control)
{
	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));

	if(fabs(ggv_postscript_view_get_zoom_factor(control->priv->ps_view) - new_zoom_level) < 0.001)
		return;

	ggv_postscript_view_set_zoom_factor
		(control->priv->ps_view, new_zoom_level);
	control->priv->zoom_level = ggv_postscript_view_get_zoom_factor
		(control->priv->ps_view);

	if(control->priv->zoom_adj) {
		if(fabs(GTK_ADJUSTMENT(control->priv->zoom_adj)->value - new_zoom_level*100.0) > 0.01)
			gtk_adjustment_set_value(GTK_ADJUSTMENT(control->priv->zoom_adj),
									 new_zoom_level*100.0);
	}

	bonobo_zoomable_report_zoom_level_changed
		(zoomable, control->priv->zoom_level, NULL);

	sync_zoom_level_items(control);
}

static void
zoomable_zoom_in_cb (BonoboZoomable *zoomable, GgvControl *control)
{
	float new_zoom_level;
	int index;

	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));

	index = ggv_zoom_index_from_float (control->priv->zoom_level);
	if (index == ggv_max_zoom_levels)
		return;

	index++;
	new_zoom_level = ggv_zoom_level_from_index (index);

	g_signal_emit_by_name (G_OBJECT (zoomable), "set_zoom_level",
						   new_zoom_level);
}

static void
zoomable_zoom_out_cb (BonoboZoomable *zoomable, GgvControl *control)
{
	float new_zoom_level;
	int index;

	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));

	index = ggv_zoom_index_from_float (control->priv->zoom_level);
	if (index == 0)
		return;

	index--;
	new_zoom_level = ggv_zoom_level_from_index (index);

	g_signal_emit_by_name (G_OBJECT (zoomable), "set_zoom_level",
						   new_zoom_level);
}

static void
zoomable_zoom_to_fit_cb (BonoboZoomable *zoomable, GgvControl *control)
{
	float new_zoom_level;

	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));

	new_zoom_level =
		ggv_postscript_view_zoom_to_fit (control->priv->ps_view, FALSE);

	g_signal_emit_by_name (G_OBJECT (zoomable), "set_zoom_level",
						   new_zoom_level);
}

static void
zoomable_zoom_to_default_cb (BonoboZoomable *zoomable, GgvControl *control)
{
	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));

	g_signal_emit_by_name (G_OBJECT (zoomable), "set_zoom_level", 1.0);
}

static gboolean
scrollbar_button_press_event(GtkWidget *widget, GdkEventButton *event,
							 gpointer data)
{
	GtkGS *gs = GTK_GS(data);

	if(event->button == 1)
		gtk_gs_start_scroll(gs);

	return FALSE;
}

static gboolean
scrollbar_button_release_event(GtkWidget *widget, GdkEventButton *event,
							   gpointer data)
{
	GtkGS *gs = GTK_GS(data);

	if(event->button == 1)
		gtk_gs_end_scroll(gs);

	return FALSE;
}

static void
listener_ZoomLevel_cb(BonoboUIComponent *uic, const char *path,
					  Bonobo_UIComponent_EventType type, const char *state,
					  gpointer user_data)
{
	BonoboZoomable *zoomable;
	GgvControl *control;
	const char *zl;
	gint i;

	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GGV_IS_CONTROL(user_data));

	if(type != Bonobo_UIComponent_STATE_CHANGED)
		return;

	if(!state || !atoi(state))
		return;

	control = GGV_CONTROL(user_data);
	zoomable = control->priv->zoomable;

	zl = path + strlen("Zoom");

	for(i = 0; zoom_level_items[i].path != NULL; i++) {
		if(strstr(zoom_level_items[i].path, zl) != NULL) {
			g_signal_emit_by_name (G_OBJECT (zoomable), "set_zoom_level",
								   (gfloat)zoom_level_items[i].level);
			break;
		}
	}
}

static void
listener_AutoFitMode_cb(BonoboUIComponent *uic, const char *path,
						Bonobo_UIComponent_EventType type, const char *state,
						gpointer user_data)
{
	GgvControl *control;
	GtkGS *gs;
	gint i;

	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GGV_IS_CONTROL(user_data));

	if(type != Bonobo_UIComponent_STATE_CHANGED)
		return;

	if(!state || !atoi(state))
		return;

	control = GGV_CONTROL(user_data);
	gs = GTK_GS(ggv_postscript_view_get_widget(control->priv->ps_view));

	if(!strcmp(path, "AutoFitNone"))
		gtk_gs_set_zoom_mode(gs, GTK_GS_ZOOM_ABSOLUTE);
	if(!strcmp(path, "AutoFitWidth"))
		gtk_gs_set_zoom_mode(gs, GTK_GS_ZOOM_FIT_WIDTH);
	if(!strcmp(path, "AutoFitPage"))
		gtk_gs_set_zoom_mode(gs, GTK_GS_ZOOM_FIT_PAGE);

	gtk_widget_unref(GTK_WIDGET(gs));
}

static gboolean
ggv_control_button_press_event(GtkWidget *widget, GdkEventButton *event,
							   gpointer data)
{
	GgvControl *control = GGV_CONTROL(data);

	if(event->button == 3) {
		return bonobo_control_do_popup(BONOBO_CONTROL(control), 3, event->time);
	}
	return FALSE;
}

static gboolean
ggv_control_key_press_event(GtkWidget *widget, GdkEventKey *event,
							gpointer data)
{
	GtkGS *gs = GTK_GS(widget);
	gint key = event->keyval;
	GgvControl *control = GGV_CONTROL(data);
	GtkGSOrientation orientation = gtk_gs_get_orientation(gs);
	GgvPostScriptView *ps_view = control->priv->ps_view;

	/* ugh. the possibilities! */
	switch (key) {
	case GDK_space:
		switch (orientation) {
		case GTK_GS_ORIENTATION_PORTRAIT:
			if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_DOWN, TRUE)) {
				ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
			}
			break;
		case GTK_GS_ORIENTATION_LANDSCAPE:
			if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_LEFT, TRUE)) {
				ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
			}
			break;
		case GTK_GS_ORIENTATION_SEASCAPE:
			if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_RIGHT, TRUE)) {
				ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
			}
			break;
		case GTK_GS_ORIENTATION_UPSIDEDOWN:
			if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_UP, TRUE)) {
				ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
			}
			break;
		default:
			break;
		}
		break;
	case GDK_BackSpace:
	case GDK_Delete:
		switch (orientation) {
		case GTK_GS_ORIENTATION_PORTRAIT:
			if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_UP, TRUE)) {
				ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
			}
			break;
		case GTK_GS_ORIENTATION_LANDSCAPE:
			if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_RIGHT, TRUE)) {
				ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
			}
			break;
		case GTK_GS_ORIENTATION_SEASCAPE:
			if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_LEFT, TRUE)) {
				ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
			}
			break;
		case GTK_GS_ORIENTATION_UPSIDEDOWN:
			if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_DOWN, TRUE)) {
				ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
			}
			break;
		default:
			break;
		}
		break;
	case GDK_Left:
		if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_LEFT, FALSE) &&
		   ggv_postscript_view_get_page_flip(ps_view))
			ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
		break;
	case GDK_Right:
		if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_RIGHT, FALSE) &&
		   ggv_postscript_view_get_page_flip(ps_view))
			ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
		break;
	case GDK_Up:
		if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_UP, FALSE) &&
		   ggv_postscript_view_get_page_flip(ps_view))
			ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
		break;
	case GDK_Down:
		if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_DOWN, FALSE) &&
		   ggv_postscript_view_get_page_flip(ps_view))
			ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
		break;
	case GDK_Page_Up:
		ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
		break;
	case GDK_Page_Down:
		ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
		break;
	case GDK_plus:
		g_signal_emit_by_name(G_OBJECT (control->priv->zoomable),
							  "zoom_in");
		break;
	case GDK_minus:
		g_signal_emit_by_name(G_OBJECT(control->priv->zoomable),
							  "zoom_out");
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static void
verb_ZoomIn_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	GgvControl *control;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_CONTROL (user_data));

	control = GGV_CONTROL (user_data);

	g_signal_emit_by_name (G_OBJECT (control->priv->zoomable),
						   "zoom_in");
}

static void
verb_ZoomOut_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	GgvControl *control;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_CONTROL (user_data));

	control = GGV_CONTROL (user_data);

	g_signal_emit_by_name (G_OBJECT (control->priv->zoomable),
						   "zoom_out");
}

static void
verb_ZoomToDefault_cb (BonoboUIComponent *uic, gpointer user_data,
					   const char *cname)
{
	GgvControl *control;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_CONTROL (user_data));

	control = GGV_CONTROL (user_data);

	g_signal_emit_by_name (G_OBJECT (control->priv->zoomable),
						   "zoom_to_default");
}

static void
verb_ZoomToFit_cb (BonoboUIComponent *uic, gpointer user_data,
				   const char *cname)
{
	GgvControl *control;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_CONTROL (user_data));

	control = GGV_CONTROL (user_data);

	g_signal_emit_by_name (G_OBJECT (control->priv->zoomable),
						   "zoom_to_fit");
}

static void
verb_ZoomToFitWidth_cb (BonoboUIComponent *uic, gpointer user_data,
						const char *cname)
{
	GgvControl *control;
	gfloat zoom_level;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_CONTROL (user_data));

	control = GGV_CONTROL (user_data);

	zoom_level = ggv_postscript_view_zoom_to_fit(control->priv->ps_view, TRUE);

	g_signal_emit_by_name(G_OBJECT(control->priv->zoomable), "set_zoom_level",
						  zoom_level);

}

static void
_set_zoom_items_sensitivity(GgvControl *control, BonoboUIComponent *uic, gboolean sens)
{
	gchar *prop_val = sens?"1":"0";
	Bonobo_UIContainer container;
	gint i;

	container = bonobo_ui_component_get_container (uic);
	if (container == CORBA_OBJECT_NIL)
		return;

	bonobo_ui_component_freeze(uic, NULL);
	bonobo_ui_component_set_prop(uic, "/commands/ZoomIn",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(uic, "/commands/ZoomOut",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(uic, "/commands/ZoomToDefault",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(uic, "/commands/ZoomToFit",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(uic, "/commands/ZoomToFitWidth",
								 "sensitive", prop_val, NULL);
	for(i = 0; zoom_level_items[i].path != NULL; i++) {
		bonobo_ui_component_set_prop(uic, zoom_level_items[i].path,
									 "sensitive", prop_val, NULL);
	}
	bonobo_ui_component_thaw(uic, NULL);
}

static void
set_zoom_items_sensitivity(GgvControl *control, gboolean sens)
{
	_set_zoom_items_sensitivity(control, control->priv->uic, sens);
	if(control->priv->popup_uic != NULL)
		_set_zoom_items_sensitivity(control, control->priv->popup_uic, sens);
}

static void
set_auto_fit_items_sensitivity(GgvControl *control, gboolean sens)
{
	gchar *prop_val = sens?"1":"0";
	Bonobo_UIContainer container;

	container = bonobo_ui_component_get_container (control->priv->uic);
	if (container == CORBA_OBJECT_NIL)
		return;

	bonobo_ui_component_freeze(control->priv->uic, NULL);
	bonobo_ui_component_set_prop(control->priv->uic, "/commands/AutoFitNone",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(control->priv->uic, "/commands/AutoFitWidth",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(control->priv->uic, "/commands/AutoFitPage",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_thaw(control->priv->uic, NULL);	
}

static void
set_command_items_sensitivity(GgvControl *control, gboolean sens)
{
	set_auto_fit_items_sensitivity(control, sens);
	set_zoom_items_sensitivity(control, sens);
}

static BonoboUIVerb ggv_control_verbs[] = {
		BONOBO_UI_VERB ("ZoomIn",         verb_ZoomIn_cb),
		BONOBO_UI_VERB ("ZoomOut",        verb_ZoomOut_cb),
		BONOBO_UI_VERB ("ZoomToDefault",  verb_ZoomToDefault_cb),
		BONOBO_UI_VERB ("ZoomToFit",      verb_ZoomToFit_cb),
		BONOBO_UI_VERB ("ZoomToFitWidth", verb_ZoomToFitWidth_cb),
		BONOBO_UI_VERB_END
};

extern BonoboUIVerb ggv_postscript_view_verbs[];

static void 
zoom_adj_value_changed_cb(GtkAdjustment *adj, gpointer data)
{
	GgvControl *control = GGV_CONTROL(data);

	if(fabs(adj->value - ggv_postscript_view_get_zoom_factor(control->priv->ps_view)) > 0.001) {
		g_signal_emit_by_name(G_OBJECT (control->priv->zoomable),
							  "set_zoom_level", ((gfloat)adj->value)/100.0);
	}
}

static void
ggv_control_create_ui (GgvControl *control)
{
	GgvControlClass *klass;
	GdkPixbuf *pixbuf, *pixbuf_small;
	gchar *pixbuf_xml;
	int i;

	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));

	klass = GGV_CONTROL_CLASS(G_OBJECT_GET_CLASS(control));

	bonobo_ui_component_freeze(control->priv->uic, NULL);
	bonobo_ui_util_set_ui(control->priv->uic, DATADIR, "ggv-control-ui.xml",
						  "GGV", NULL);
								 
	control->priv->popup_uic = bonobo_control_get_popup_ui_component(BONOBO_CONTROL(control));
	bonobo_ui_component_freeze(control->priv->popup_uic, NULL);
	bonobo_ui_util_set_ui(control->priv->popup_uic, NULL, "ggv-postscript-view-ui.xml", "GGV", NULL);
	bonobo_ui_util_set_ui(control->priv->popup_uic, DATADIR, "ggv-control-ui.xml", "GGV", NULL);
	
	pixbuf = gdk_pixbuf_new_from_file(GNOMEICONDIR "/ggv/fitwidth.png", NULL);
	if(pixbuf != NULL) {
		pixbuf_xml = bonobo_ui_util_pixbuf_to_xml(pixbuf);
		bonobo_ui_component_set_prop(control->priv->uic,
									 "/Toolbar/GgvItems/GgvZoomItems/ZoomToFitWidth",
									 "pixname", pixbuf_xml,
									 NULL);
		g_free(pixbuf_xml);
		pixbuf_small = gdk_pixbuf_scale_simple(pixbuf, 18, 18, GDK_INTERP_TILES);
		pixbuf_xml = bonobo_ui_util_pixbuf_to_xml(pixbuf_small);
		bonobo_ui_component_set_prop(control->priv->uic,
									 "/menu/View/Zoom Items Placeholder/Zoom/ZoomToFitWidth",
									 "pixname", pixbuf_xml,
									 NULL);
		bonobo_ui_component_set_prop(control->priv->popup_uic,
									 "/popups/button3/ZoomMenu/ZoomToFitWidth",
									 "pixname", pixbuf_xml,
									 NULL);
		g_free(pixbuf_xml);
		gdk_pixbuf_unref(pixbuf);
		gdk_pixbuf_unref(pixbuf_small);
	}

	for(i = 0; zoom_level_items[i].path; i++) {
		bonobo_ui_component_add_listener(control->priv->uic,
										 zoom_level_items[i].path + strlen("/commands/"),
										 listener_ZoomLevel_cb, control);
	}
	bonobo_ui_component_add_listener(control->priv->uic,
									 "AutoFitNone",
									 listener_AutoFitMode_cb, control);
	bonobo_ui_component_add_listener(control->priv->uic,
									 "AutoFitWidth",
									 listener_AutoFitMode_cb, control);
	bonobo_ui_component_add_listener(control->priv->uic,
									 "AutoFitPage",
									 listener_AutoFitMode_cb, control);

	set_command_items_sensitivity(control, ggv_postscript_view_get_page_count(control->priv->ps_view) > 0);

	sync_command_state(control);

	bonobo_ui_component_add_verb_list_with_data(control->priv->uic,
												ggv_control_verbs,
												control);
	bonobo_ui_component_add_verb_list_with_data(control->priv->popup_uic,
												ggv_control_verbs,
												control);
	bonobo_ui_component_add_verb_list_with_data(control->priv->popup_uic,
												ggv_postscript_view_verbs,
												control->priv->ps_view);
	ggv_postscript_view_set_popup_ui_component(control->priv->ps_view,
											   control->priv->popup_uic);

	if(!control->priv->has_zoomable_frame) {
		BonoboControl *zoom_control;
		GtkWidget *zoom_spin, *zoom_image, *perc_label, *hbox;
		GtkObject *zoom_adj;

		hbox = gtk_hbox_new(FALSE, 2);
		gtk_widget_show(hbox);
		zoom_image = gtk_image_new_from_file(GNOMEICONDIR "/ggv/zoom.xpm");
		gtk_widget_show(zoom_image);
		gtk_box_pack_start(GTK_BOX(hbox), zoom_image, FALSE, TRUE, 0);
		zoom_adj = gtk_adjustment_new(ggv_postscript_view_get_zoom_factor(control->priv->ps_view)*100.0,
									  16.67, 600.0, 10.0, 100.0, 100.0);
		g_signal_connect(G_OBJECT(zoom_adj), "value-changed",
						 G_CALLBACK(zoom_adj_value_changed_cb), control);
		zoom_spin = gtk_spin_button_new(GTK_ADJUSTMENT(zoom_adj), 1.0, 0);
		gtk_widget_show(zoom_spin);
		gtk_box_pack_start(GTK_BOX(hbox), zoom_spin, TRUE, TRUE, 0);
		perc_label = gtk_label_new("%");
		gtk_widget_show(perc_label);
		gtk_box_pack_start(GTK_BOX(hbox), perc_label, FALSE, TRUE, 0);

		zoom_control = bonobo_control_new(hbox);
		bonobo_ui_component_object_set(control->priv->uic, "/Sidebar/Zoom Control Placeholder/ZoomControl",
									   BONOBO_OBJREF(zoom_control), NULL);
		control->priv->zoom_control = zoom_control;
		control->priv->zoom_adj = zoom_adj;
	}
	bonobo_ui_component_thaw(control->priv->uic, NULL);
	bonobo_ui_component_thaw(control->priv->popup_uic, NULL);
}

static void
ggv_control_set_ui_container (GgvControl *control,
							  Bonobo_UIContainer ui_container)
{
	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));
	g_return_if_fail (ui_container != CORBA_OBJECT_NIL);

	ggv_postscript_view_set_ui_container (control->priv->ps_view,
										  ui_container);

	bonobo_ui_component_set_container (control->priv->uic, ui_container, NULL);

	/* NOTE: we always merge our UI, as we have more than merely zoom items
	   to offer. */
	if(TRUE || !control->priv->has_zoomable_frame) {
		ggv_control_create_ui(control);
	}
}

static void
ggv_control_unset_ui_container (GgvControl *control)
{
	g_return_if_fail (control != NULL);
	g_return_if_fail (GGV_IS_CONTROL (control));

	ggv_postscript_view_unset_ui_container (control->priv->ps_view);

	bonobo_ui_component_unset_container (control->priv->uic, NULL);
}

static void
ggv_control_activate (BonoboControl *object, gboolean state)
{
	GgvControl *control;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GGV_IS_CONTROL (object));

	control = GGV_CONTROL (object);

	if (state) {
		Bonobo_UIContainer ui_container;
			
		ui_container = bonobo_control_get_remote_ui_container (BONOBO_CONTROL (control), NULL);
		if (ui_container != CORBA_OBJECT_NIL) {
			ggv_control_set_ui_container (control, ui_container);
			bonobo_object_release_unref (ui_container, NULL);
		}
	} else
		ggv_control_unset_ui_container (control);

	if (BONOBO_CONTROL_CLASS (ggv_control_parent_class)->activate)
		BONOBO_CONTROL_CLASS (ggv_control_parent_class)->activate (object, state);
}

static void
ggv_control_class_init (GgvControlClass *klass)
{
	GObjectClass *object_class = (GObjectClass *)klass;
	BonoboObjectClass *bonobo_object_class = (BonoboObjectClass *)klass;
	BonoboControlClass *control_class = (BonoboControlClass *)klass;

	ggv_control_parent_class = gtk_type_class (bonobo_control_get_type ());

	bonobo_object_class->destroy = ggv_control_destroy;
	object_class->finalize = ggv_control_finalize;

	control_class->activate = ggv_control_activate;

	klass->priv = g_new0(GgvControlClassPrivate, 1);
}

static void
ggv_control_init (GgvControl *control)
{
	control->priv = g_new0 (GgvControlPrivate, 1);
}

BONOBO_TYPE_FUNC (GgvControl, BONOBO_TYPE_CONTROL, ggv_control);

static void
ps_view_property_changed_handler(BonoboListener    *listener,
                                 char              *event_name, 
                                 CORBA_any         *any,
                                 CORBA_Environment *ev,
                                 gpointer           data)
{
        GgvControl *control = GGV_CONTROL(data);

        if(!g_ascii_strcasecmp(event_name, "Bonobo/Property:change:page")) {
			set_command_items_sensitivity(control, BONOBO_ARG_GET_INT(any) != -1);
        }
}

GgvControl *
ggv_control_construct (GgvControl *control, GgvPostScriptView *ps_view)
{
	BonoboPropertyBag     *property_bag;
	BonoboPropertyControl *property_control;
	GtkWidget *view;
	gchar *mask;

	g_return_val_if_fail (ps_view != NULL, NULL);
	g_return_val_if_fail (control != NULL, NULL);
	g_return_val_if_fail (GGV_IS_POSTSCRIPT_VIEW (ps_view), NULL);
	g_return_val_if_fail (GGV_IS_CONTROL (control), NULL);

	control->priv->ps_view = ps_view;
	bonobo_object_ref (BONOBO_OBJECT (ps_view));

	if (!ggv_postscript_view_add_interfaces (ps_view, BONOBO_OBJECT (control))) {
		g_message("control: can't add interfaces");
		return NULL;
	}

	view = ggv_postscript_view_get_widget (control->priv->ps_view);
	g_signal_connect(G_OBJECT(view), "key_press_event",
					 G_CALLBACK(ggv_control_key_press_event), control);
	g_signal_connect(G_OBJECT(view), "button_press_event",
					 G_CALLBACK(ggv_control_button_press_event), control);

	control->priv->root = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(control->priv->root);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(control->priv->root),
										GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(control->priv->root),
								   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	g_signal_connect(G_OBJECT(GTK_SCROLLED_WINDOW(control->priv->root)->hscrollbar),
					 "button_press_event",
					 G_CALLBACK(scrollbar_button_press_event),
					 view);
	g_signal_connect(G_OBJECT(GTK_SCROLLED_WINDOW(control->priv->root)->vscrollbar),
					 "button_press_event",
					 G_CALLBACK(scrollbar_button_press_event),
					 view);
	g_signal_connect(G_OBJECT(GTK_SCROLLED_WINDOW(control->priv->root)->hscrollbar),
					 "button_release_event",
					 G_CALLBACK(scrollbar_button_release_event),
					 view);
	g_signal_connect(G_OBJECT(GTK_SCROLLED_WINDOW(control->priv->root)->vscrollbar),
					 "button_release_event",
					 G_CALLBACK(scrollbar_button_release_event),
					 view);
	gtk_container_add(GTK_CONTAINER(control->priv->root), view);
	/* unref the GtkGS acquired from GgvPostScriptView */
	gtk_widget_unref(view);
	
	bonobo_control_construct (BONOBO_CONTROL (control), control->priv->root);

	bonobo_object_add_interface (BONOBO_OBJECT (control),
								 BONOBO_OBJECT (control->priv->ps_view));

	/* Interface Bonobo::Zoomable */
	control->priv->zoomable = bonobo_zoomable_new ();

	g_signal_connect (G_OBJECT (control->priv->zoomable),
					  "set_frame",
					  G_CALLBACK (zoomable_set_frame_cb),
					  control);
	g_signal_connect (G_OBJECT (control->priv->zoomable),
					  "set_zoom_level",
					  G_CALLBACK (zoomable_set_zoom_level_cb),
					  control);
	g_signal_connect (G_OBJECT (control->priv->zoomable),
					  "zoom_in",
					  G_CALLBACK (zoomable_zoom_in_cb),
					  control);
	g_signal_connect (G_OBJECT (control->priv->zoomable),
					  "zoom_out",
					  G_CALLBACK (zoomable_zoom_out_cb),
					  control);
	g_signal_connect (G_OBJECT (control->priv->zoomable),
					  "zoom_to_fit",
					  G_CALLBACK (zoomable_zoom_to_fit_cb),
					  control);
	g_signal_connect (G_OBJECT (control->priv->zoomable),
					  "zoom_to_default",
					  G_CALLBACK (zoomable_zoom_to_default_cb),
					  control);

	control->priv->zoom_level = ggv_postscript_view_get_zoom_factor(ps_view);
	bonobo_zoomable_set_parameters_full (control->priv->zoomable,
										 control->priv->zoom_level,
										 ggv_zoom_levels [0],
										 ggv_zoom_levels [ggv_max_zoom_levels],
										 TRUE, TRUE, TRUE,
										 ggv_zoom_levels,
										 ggv_zoom_level_names,
										 ggv_max_zoom_levels + 1);

	bonobo_object_add_interface (BONOBO_OBJECT (control),
								 BONOBO_OBJECT (control->priv->zoomable));

	property_bag =
		ggv_postscript_view_get_property_bag (control->priv->ps_view);

	bonobo_control_set_properties (BONOBO_CONTROL(control),
								   BONOBO_OBJREF(property_bag), NULL);
	mask = "Bonobo/Property:change:page";
	bonobo_event_source_client_add_listener(BONOBO_OBJREF(property_bag),
											(BonoboListenerCallbackFn)ps_view_property_changed_handler,
											mask, NULL, control);
	bonobo_object_unref (BONOBO_OBJECT (property_bag));

	property_control =
		ggv_postscript_view_get_property_control (control->priv->ps_view);
	bonobo_object_add_interface (BONOBO_OBJECT (control),
								 BONOBO_OBJECT (property_control));
	bonobo_object_unref(BONOBO_OBJECT(property_control));

	control->priv->uic =
		bonobo_control_get_ui_component (BONOBO_CONTROL (control));

	return control;
}

GgvControl *
ggv_control_new (GgvPostScriptView *ps_view)
{
	GgvControl *control;
	
	g_return_val_if_fail (ps_view != NULL, NULL);
	g_return_val_if_fail (GGV_IS_POSTSCRIPT_VIEW (ps_view), NULL);

	control = g_object_new(GGV_CONTROL_TYPE, NULL);

	return ggv_control_construct (control, ps_view);
}
