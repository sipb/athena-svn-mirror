/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-engine-private.h: Private Bonobo UI/XML Sync engine bits
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2001 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_ENGINE_PRIVATE_H_
#define _BONOBO_UI_ENGINE_PRIVATE_H_

G_BEGIN_DECLS

#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-engine-config.h>


struct _BonoboUIEnginePrivate {
	GObject      *view;

	BonoboUIXml  *tree;
	int           frozen;
	GSList       *syncs;
	GSList       *state_updates;
	GSList       *components;

	BonoboUIContainer    *container;
	BonoboUIEngineConfig *config;

	GHashTable   *cmd_to_node;
};

BonoboUIXml          *bonobo_ui_engine_get_xml    (BonoboUIEngine *engine);
BonoboUIEngineConfig *bonobo_ui_engine_get_config (BonoboUIEngine *engine);

G_END_DECLS

#endif /* _BONOBO_UI_ENGINE_H_ */
