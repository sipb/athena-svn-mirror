/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-ui-preferences.h: private wrappers for global UI preferences.
 *
 * Authors:
 *     Michael Meeks (michael@helixcode.com)
 *     Martin Baulig (martin@home-of-linux.org)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_UI_PREFERENCES_H_
#define _BONOBO_UI_PREFERENCES_H_

#include <glib.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-toolbar.h>

G_BEGIN_DECLS

#define BONOBO_UI_PAD          8
#define BONOBO_UI_PAD_SMALL    4
#define BONOBO_UI_PAD_BIG      12

/* Add a UI engine to the configuration list */
void bonobo_ui_preferences_add_engine    (BonoboUIEngine *engine);
void bonobo_ui_preferences_remove_engine (BonoboUIEngine *engine);

/* Default toolbar style */
BonoboUIToolbarStyle bonobo_ui_preferences_get_toolbar_style (void);
/* Whether menus have icons */
gboolean bonobo_ui_preferences_get_menus_have_icons   (void);
/* Whether menus can be torn off */
gboolean bonobo_ui_preferences_get_menus_have_tearoff (void);
/* Whether menubars can be detached */
gboolean bonobo_ui_preferences_get_menubar_detachable (void);
/* Whether a per-app toolbar style has been specified */
gboolean bonobo_ui_preferences_get_toolbar_detachable (void);

G_END_DECLS

#endif /* _BONOBO_UI_PREFERENCES_H_ */

