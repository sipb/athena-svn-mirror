/*
 * bonobo-preferences.h:
 *
 * Authors:
 *   based on eog-preferences.h from Martin Baulig (baulig@suse.de)
 *   modified by Dietmar Maurer (dietmar@ximian.com)
 */
#ifndef _BONOBO_PREFERENCES_H_
#define _BONOBO_PREFERENCES_H_

#include <libgnomeui/gnome-propertybox.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_PREFERENCES_TYPE        (bonobo_preferences_get_type ())
#define BONOBO_PREFERENCES(o)          (GTK_CHECK_CAST ((o), BONOBO_PREFERENCES_TYPE, BonoboPreferences))
#define BONOBO_PREFERENCES_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PREFERENCES_TYPE, BonoboPreferencesClass))

#define BONOBO_IS_PREFERENCES(o)       (GTK_CHECK_TYPE ((o), BONOBO_PREFERENCES_TYPE))
#define BONOBO_IS_PREFERENCES_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PREFERENCES_TYPE))

typedef struct _BonoboPreferences         BonoboPreferences;
typedef struct _BonoboPreferencesClass    BonoboPreferencesClass;
typedef struct _BonoboPreferencesPrivate  BonoboPreferencesPrivate;

struct _BonoboPreferences {
	GnomePropertyBox dialog;

	BonoboPreferencesPrivate *priv;
};

struct _BonoboPreferencesClass {
	GnomePropertyBoxClass parent_class;
};

GtkType           
bonobo_preferences_get_type     (void);

GtkWidget *
bonobo_preferences_new          (Bonobo_PropertyControl prop_control);

END_GNOME_DECLS

#endif /* _BONOBO_PREFERENCES_H */
