#ifndef __GPA_SETTINGS_SELECTOR_H__
#define __GPA_SETTINGS_SELECTOR_H__

/*
 * GPASettingsSelector
 *
 * Simple OptonMenu for selecting settingss
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib/gmacros.h>

G_BEGIN_DECLS

#define GPA_TYPE_SETTINGS_SELECTOR (gpa_settings_selector_get_type ())
#define GPA_SETTINGS_SELECTOR(obj) (GTK_CHECK_CAST ((obj), GPA_TYPE_SETTINGS_SELECTOR, GPASettingsSelector))
#define GPA_SETTINGS_SELECTOR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GPA_TYPE_SETTINGS_SELECTOR, GPASettingsSelectorClass))
#define GPA_IS_SETTINGS_SELECTOR(obj) (GTK_CHECK_TYPE ((obj), GPA_TYPE_SETTINGS_SELECTOR))
#define GPA_IS_SETTINGS_SELECTOR_CLASS (GTK_CHECK_CLASS ((obj), GPA_TYPE_SETTINGS_SELECTOR))

typedef struct _GPASettingsSelector GPASettingsSelector;
typedef struct _GPASettingsSelectorClass GPASettingsSelectorClass;

#include <libgnomeprint/private/gpa-private.h>
#include "gpa-widget.h"

struct _GPASettingsSelector {
	GPAWidget widget;
	GtkWidget *menu;

	GPANode *printer;
	GPANode *settings;

	GSList *settingslist;
};

struct _GPASettingsSelectorClass {
	GPAWidgetClass widget_class;
};

GtkType gpa_settings_selector_get_type (void);

G_END_DECLS

#endif
