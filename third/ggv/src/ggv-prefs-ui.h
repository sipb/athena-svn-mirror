/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-prefs-ui.h: GGV preferences ui
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#ifndef __GGV_PREFS_UI_H__
#define __GGV_PREFS_UI_H__

#include <config.h>

#include <glib.h>
#include <gconf/gconf-client.h>

#include "ggv-prefs.h"

G_BEGIN_DECLS 

#define GGV_TYPE_PREFS_DIALOG            (ggv_prefs_dialog_get_type ())
#define GGV_PREFS_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGV_TYPE_PREFS_DIALOG, GgvPrefsDialog))
#define GGV_PREFS_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GGV_TYPE_PREFS_DIALOG, GgvPrefsDialogClass))
#define GGV_IS_PREFS_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGV_TYPE_PREFS_DIALOG))
#define GGV_IS_PREFS_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGV_TYPE_PREFS_DIALOG))
#define GGV_PREFS_DIALOG_GET_CLASS(o)    (G_TYPE_INSTANCE_GET_CLASS ((o), GGV_TYPE_PREFS_DIALOG, GgvPrefsDialogClass))

typedef struct _GgvPrefsDialog      GgvPrefsDialog;
typedef struct _GgvPrefsDialogClass GgvPrefsDialogClass;

struct _GgvPrefsDialog
{
        GtkWindow dlg;

        GtkWidget *notebook;

        GtkWidget *gs, *alpha_params, *convert_pdf, *unzip, *unbzip2, *print;   /* entries */
        GtkWidget *size, *zoom, *orientation, *units, *auto_fit;      /* option menus */
        GtkWidget *watch, *aa, *override_size, *respect_eof;
        GtkWidget *show_scroll_rect, *page_flip; /* checkbuttons */
        GtkWidget *override_orientation;
        GtkWidget *sidebar, *mbar, *sbar, *toolbar, *tbstyle[GGV_TOOLBAR_STYLE_LAST], *savegeo,*auto_jump;/* checkbuttons */
        GtkWidget *right_panel;
        GtkWidget **size_choice;               /* paper items */
        GtkWidget **zoom_choice;               /* zoom items */
        GtkWidget **orientation_choice;        /* orientation items */
        GtkWidget **unit_choice;               /* unit items */
        GtkWidget **auto_fit_choice;           /* auto-fit mode */
        GtkWidget *scroll_step;

        GtkWidget *ok, *cancel, *apply;
};

struct _GgvPrefsDialogClass
{
        GtkWindowClass klass;
};

GType ggv_prefs_dialog_get_type(void);
GtkWidget *ggv_prefs_dialog_new(void);
void ggv_prefs_dialog_show(GgvPrefsDialog *dlg);

G_END_DECLS

#endif /* __GGV_PREFS_UI_H__ */
