/*
 * bonobo-config-utils.h: Utility functions
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#ifndef __BONOBO_CONFIG_UTILS_H__
#define __BONOBO_CONFIG_UTILS_H__

#include <bonobo/bonobo-ui-node.h>

BEGIN_GNOME_DECLS

BonoboUINode *
bonobo_config_xml_encode_any (const CORBA_any   *any,
			      const char        *name,
			      CORBA_Environment *ev);

CORBA_any *
bonobo_config_xml_decode_any (BonoboUINode      *node,
			      const char        *locale, 
			      CORBA_Environment *ev);

char *
bonobo_config_any_to_string  (CORBA_any *any);

char *
bonobo_config_dir_name       (const char *key);

char *
bonobo_config_leaf_name      (const char *key);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_UTILS_H__ */
