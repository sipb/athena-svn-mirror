/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Authors: 
 *   Christopher James Lahey <clahey@ximian.com>
 *
 * Copyright (C) 2002 Ximian, Inc.
 */

#include <config.h>

#include "e-table/gal-a11y-e-table-factory.h"
#include "e-table/gal-a11y-e-cell-text.h"
#include "e-table/gal-a11y-e-cell-registry.h"

#include "e-text/gal-a11y-e-text-factory.h"
#include <gal/e-text/e-text.h> 

#include <gal/e-table/e-table.h>
#include <gal/e-table/e-cell-text.h>
#include <atk/atkregistry.h>

/* Static functions */

static gboolean initialized = FALSE;

extern void gnome_accessibility_module_init     (void);
extern void gnome_accessibility_module_shutdown (void);

void
gal_a11y_init ()
{
	if (initialized)
		return;
	atk_registry_set_factory_type (atk_get_default_registry (),
				       E_TABLE_TYPE,
				       gal_a11y_e_table_factory_get_type ());
	atk_registry_set_factory_type (atk_get_default_registry (),
				       E_TYPE_TEXT,
				       gal_a11y_e_text_factory_get_type ());

	gal_a11y_e_cell_registry_add_cell_type (NULL,
						E_CELL_TEXT_TYPE,
						gal_a11y_e_cell_text_new);
	initialized = TRUE;
}

void
gnome_accessibility_module_init ()
{
	gal_a11y_init();
}

void
gnome_accessibility_module_shutdown ()
{
	if (!initialized)
		return;
	atk_registry_set_factory_type (atk_get_default_registry (),
				       E_TABLE_TYPE,
				       0);

	initialized = FALSE;
}

int
gtk_module_init (gint *argc, char** argv[])
{
  gal_a11y_init ();

  return 0;
}
