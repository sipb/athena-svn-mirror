/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * ggv-postscript-view.h.
 *
 * Author:  Jaka Mocnik  <jaka@gnu.org>
 *
 * Copyright (c) 2001, 2002 Free Software Foundation
 */

#ifndef _GGV_POSTSCRIPT_VIEW_H_
#define _GGV_POSTSCRIPT_VIEW_H_

#include <gtkgs.h>

#include <gnome.h>

#include <bonobo.h>

#include <Ggv.h>

G_BEGIN_DECLS

#define GGV_POSTSCRIPT_VIEW_TYPE          (ggv_postscript_view_get_type ())
#define GGV_POSTSCRIPT_VIEW(o)            (GTK_CHECK_CAST ((o), GGV_POSTSCRIPT_VIEW_TYPE, GgvPostScriptView))
#define GGV_POSTSCRIPT_VIEW_CLASS(k)      (GTK_CHECK_CLASS_CAST((k), GGV_POSTSCRIPT_VIEW_TYPE, GgvPostScriptViewClass))
#define GGV_IS_POSTSCRIPT_VIEW(o)         (GTK_CHECK_TYPE ((o), GGV_POSTSCRIPT_VIEW_TYPE))
#define GGV_IS_POSTSCRIPT_VIEW_CLASS(k)   (GTK_CHECK_CLASS_TYPE ((k), GGV_POSTSCRIPT_VIEW_TYPE))

typedef struct _GgvPostScriptView             GgvPostScriptView;
typedef struct _GgvPostScriptViewClass        GgvPostScriptViewClass;
typedef struct _GgvPostScriptViewPrivate      GgvPostScriptViewPrivate;
typedef struct _GgvPostScriptViewClassPrivate GgvPostScriptViewClassPrivate;

struct _GgvPostScriptView {
	BonoboObject base;

	GgvPostScriptViewPrivate *priv;
};

struct _GgvPostScriptViewClass {
	BonoboObjectClass parent_class;

	POA_GNOME_GGV_PostScriptView__epv epv;

	GgvPostScriptViewClassPrivate *priv;
};

GtkType               ggv_postscript_view_get_type              (void);
GgvPostScriptView     *ggv_postscript_view_new                  (GtkGS *gs,
																 gboolean zoom_fit);
GgvPostScriptView     *ggv_postscript_view_construct            (GgvPostScriptView *ps_view,
																 GtkGS *gs,
																 gboolean zoom_fit);
BonoboPropertyBag     *ggv_postscript_view_get_property_bag     (GgvPostScriptView *image_view);
BonoboPropertyControl *ggv_postscript_view_get_property_control (GgvPostScriptView *image_view);
void                  ggv_postscript_view_set_ui_container      (GgvPostScriptView *image_view,
																 Bonobo_UIContainer ui_container);
void                  ggv_postscript_view_unset_ui_container    (GgvPostScriptView *image_view);
GtkWidget             *ggv_postscript_view_get_widget           (GgvPostScriptView *image_view);
void                  ggv_postscript_view_set_popup_ui_component(GgvPostScriptView *ps_view,
																 BonoboUIComponent *uic);

/* Zooming */
float ggv_postscript_view_get_zoom_factor (GgvPostScriptView *image_view);
void  ggv_postscript_view_set_zoom_factor (GgvPostScriptView *image_view,
										   float zoom_factor);
gfloat ggv_postscript_view_zoom_to_fit     (GgvPostScriptView *image_view,
											gboolean fit_width);
void  ggv_postscript_view_set_zoom        (GgvPostScriptView *image_view,
										   double zoomx,
										   double zoomy);

BonoboObject *ggv_postscript_view_add_interfaces (GgvPostScriptView *ps_view,
												  BonoboObject *to_aggregate);

void ggv_postscript_view_goto_page(GgvPostScriptView *ps_view, gint page);
gint ggv_postscript_view_get_current_page(GgvPostScriptView *ps_view);
gint ggv_postscript_view_get_page_count(GgvPostScriptView *ps_view);
gchar **ggv_postscript_view_get_page_names(GgvPostScriptView *ps_view);

/* Properties */
void
ggv_postscript_view_set_orientation (GgvPostScriptView *ps_view,
									 GNOME_GGV_Orientation orientation);
GNOME_GGV_Orientation
ggv_postscript_view_get_orientation (GgvPostScriptView *ps_view);

void
ggv_postscript_view_set_size        (GgvPostScriptView *ps_view,
									 GNOME_GGV_Size size);
GNOME_GGV_Size
ggv_postscript_view_get_size        (GgvPostScriptView *ps_view);

GtkAdjustment *ggv_postscript_view_get_hadj(GgvPostScriptView *ps_view);
GtkAdjustment *ggv_postscript_view_get_vadj(GgvPostScriptView *ps_view);

gboolean ggv_postscript_view_get_auto_jump(GgvPostScriptView *ps_view);
gboolean ggv_postscript_view_get_page_flip(GgvPostScriptView *ps_view);

G_END_DECLS

#endif /* _GGV_POSTSCRIPT_VIEW_H_ */
