/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-print.h: Remote printing support, client side.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_PRINT_H_
#define _BONOBO_PRINT_H_

#include <bonobo/bonobo-object.h>
#include <libgnomeprint/gnome-print-meta.h>

BEGIN_GNOME_DECLS

#define BONOBO_PRINT_TYPE        (bonobo_print_get_type ())
#define BONOBO_PRINT(o)          (GTK_CHECK_CAST ((o), BONOBO_PRINT_TYPE, BonoboPrint))
#define BONOBO_PRINT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PRINT_TYPE, BonoboPrintClass))
#define BONOBO_IS_PRINT(o)       (GTK_CHECK_TYPE ((o), BONOBO_PRINT_TYPE))
#define BONOBO_IS_PRINT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PRINT_TYPE))

typedef void     (BonoboPrintRenderFn) (GnomePrintContext         *ctx,
					double                     width,
					double                     height,
					const Bonobo_PrintScissor *opt_scissor,
					gpointer                   user_data);

typedef struct {
        BonoboObject         object;

	BonoboPrintRenderFn *render;
	gpointer             user_data;
} BonoboPrint;

typedef struct {
	BonoboObjectClass    parent;
	BonoboPrintRenderFn *render;
} BonoboPrintClass;

POA_Bonobo_Print__epv *bonobo_print_get_epv (void);

GtkType         bonobo_print_get_type            (void);
Bonobo_Print    bonobo_print_corba_object_create (BonoboObject *object);
BonoboPrint    *bonobo_print_construct           (BonoboPrint         *p,
						  Bonobo_Print         corba_p,
						  BonoboPrintRenderFn *render,
						  gpointer             user_data);
BonoboPrint    *bonobo_print_new                 (BonoboPrintRenderFn *render,
						  gpointer             user_data);


END_GNOME_DECLS

#endif /* _BONOBO_PRINT_H_ */

