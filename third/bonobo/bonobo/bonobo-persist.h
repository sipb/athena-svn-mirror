/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-persist.h: a persistance interface
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_PERSIST_H_
#define _BONOBO_PERSIST_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define BONOBO_PERSIST_TYPE        (bonobo_persist_get_type ())
#define BONOBO_PERSIST(o)          (GTK_CHECK_CAST ((o), BONOBO_PERSIST_TYPE, BonoboPersist))
#define BONOBO_PERSIST_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PERSIST_TYPE, BonoboPersistClass))
#define BONOBO_IS_PERSIST(o)       (GTK_CHECK_TYPE ((o), BONOBO_PERSIST_TYPE))
#define BONOBO_IS_PERSIST_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PERSIST_TYPE))

typedef struct _BonoboPersistPrivate BonoboPersistPrivate;

typedef struct {
	BonoboObject object;

	BonoboPersistPrivate *priv;
} BonoboPersist;

typedef struct {
	BonoboObjectClass parent_class;

	Bonobo_Persist_ContentTypeList * (*get_content_types) (BonoboPersist *persist,
							       CORBA_Environment *ev);
} BonoboPersistClass;

GtkType       bonobo_persist_get_type  (void);
BonoboPersist *bonobo_persist_construct (BonoboPersist *persist,
					 Bonobo_Persist corba_persist);

Bonobo_Persist_ContentTypeList *bonobo_persist_generate_content_types (int num,
								       ...);

POA_Bonobo_Persist__epv *bonobo_persist_get_epv (void);

END_GNOME_DECLS

#endif /* _BONOBO_PERSIST_H_ */
