/**
 * bonobo-config-subdir.h: config database subdirectory 
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_SUBDIR_H__
#define __BONOBO_CONFIG_SUBDIR_H__

#include <bonobo/bonobo-xobject.h>
#include <bonobo-conf/Bonobo_Config.h>

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_SUBDIR_TYPE        (bonobo_config_subdir_get_type ())
#define BONOBO_CONFIG_SUBDIR(o)	         (GTK_CHECK_CAST ((o), BONOBO_CONFIG_SUBDIR_TYPE, BonoboConfigSubdir))
#define BONOBO_CONFIG_SUBDIR_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_SUBDIR_TYPE, BonoboConfigSubdirClass))
#define BONOBO_IS_CONFIG_SUBDIR(o)	 (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_SUBDIR_TYPE))
#define BONOBO_IS_CONFIG_SUBDIR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_SUBDIR_TYPE))

typedef struct _BonoboConfigSubdirPrivate BonoboConfigSubdirPrivate;

typedef struct {
	BonoboXObject base;

	BonoboConfigSubdirPrivate *priv;
} BonoboConfigSubdir;

typedef struct {
	BonoboXObjectClass  parent_class;

	POA_Bonobo_ConfigDatabase__epv epv;
} BonoboConfigSubdirClass;


GtkType		      
bonobo_config_subdir_get_type  (void);

Bonobo_ConfigDatabase 
bonobo_config_subdir_new       (Bonobo_ConfigDatabase  db, 
				const char            *subdir);

Bonobo_ConfigDatabase 
bonobo_config_proxy_new        (Bonobo_ConfigDatabase  db, 
				const char            *moniker,
				const char            *subdir);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_SUBDIR_H__ */
