/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-settings-selector.h: Simple OptionMenu for selecting settings
 *
 * Libgnomeprint is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Libgnomeprint is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the libgnomeprint; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors :
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#ifndef __GPA_SETTINGS_SELECTOR_H__
#define __GPA_SETTINGS_SELECTOR_H__

#include <glib.h>

G_BEGIN_DECLS

#define GPA_TYPE_SETTINGS_SELECTOR        (gpa_settings_selector_get_type ())
#define GPA_SETTINGS_SELECTOR(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_SETTINGS_SELECTOR, GPASettingsSelector))
#define GPA_SETTINGS_SELECTOR_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST ((k),    GPA_TYPE_SETTINGS_SELECTOR, GPASettingsSelectorClass))
#define GPA_IS_SETTINGS_SELECTOR(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_SETTINGS_SELECTOR))
#define GPA_IS_SETTINGS_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k),    GPA_TYPE_SETTINGS_SELECTOR))

typedef struct _GPASettingsSelector      GPASettingsSelector;
typedef struct _GPASettingsSelectorClass GPASettingsSelectorClass;

#include <libgnomeprint/private/gpa-node.h>
#include "gpa-widget.h"

struct _GPASettingsSelector {
	GPAWidget widget;
	GtkWidget *menu;

	GPANode *printer;
	GPANode *settings;

	GSList *settingslist;

	guint handler; /* The handler that listens for changes in @printer */
};

struct _GPASettingsSelectorClass {
	GPAWidgetClass widget_class;
};

GType gpa_settings_selector_get_type (void);

G_END_DECLS

#endif /* __GPA_SETTINGS_SELECTOR_H__ */
