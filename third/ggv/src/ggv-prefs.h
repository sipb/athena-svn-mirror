/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-prefs.h: GGV preferences
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#ifndef __GGV_PREFS_H__
#define __GGV_PREFS_H__

#include <config.h>

#include <glib.h>
#include <gconf/gconf-client.h>

G_BEGIN_DECLS 

#define DEFAULT_WINDOW_WIDTH  640
#define DEFAULT_WINDOW_HEIGHT 480

#define DEFAULT_FILE_SEL_WIDTH   320
#define DEFAULT_FILE_SEL_HEIGHT  256

typedef enum {
        GGV_TOOLBAR_STYLE_DEFAULT = 0,
        GGV_TOOLBAR_STYLE_BOTH = 1,
        GGV_TOOLBAR_STYLE_ICONS = 2,
        GGV_TOOLBAR_STYLE_TEXT = 3,
        GGV_TOOLBAR_STYLE_LAST = 4
} GgvToolbarStyle;

extern gchar    *ggv_print_cmd;       /* print command: must print its stdin */
extern gboolean  ggv_panel;           /* panel visible */
extern gboolean  ggv_toolbar;         /* toolbar visible */
extern GgvToolbarStyle  ggv_toolbar_style;  /* toolbar style */
extern gboolean  ggv_menubar;         /* menubar visible */
extern gboolean  ggv_statusbar;       /* statusbar visible */
extern gboolean  ggv_save_geometry;   /* Save the current geometry for next session */
extern gint      ggv_unit_index;      /* the unit we want to use for coordinates */
extern gboolean  ggv_autojump;        /* auto jump to beginning of the page */
extern gboolean  ggv_pageflip;        /* automatically flip pages */
extern gboolean  ggv_right_panel;     /* panel appears on the left side by default */

/* default window dimensions */
extern gint ggv_default_width;
extern gint ggv_default_height;

/* this is used _only_ by the ggv-postscript-viewer */
void         ggv_prefs_load (void);
void         ggv_prefs_save (void);

/* this is also used by ggv shell */
GConfClient *ggv_prefs_gconf_client (void);

G_END_DECLS

#endif /* __GGV_PREFS_H__ */
