/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * gnome-print-bonobo.h: Remote printing support, client side.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _GNOME_PRINT_BONOBO_H_
#define _GNOME_PRINT_BONOBO_H_

#include <bonobo/bonobo-object.h>
#include <libgnomeprint/gnome-print-meta.h>

G_BEGIN_DECLS

#define GNOME_PRINT_BONOBO_TYPE        (gnome_print_bonobo_get_type ())
#define GNOME_PRINT_BONOBO(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_PRINT_BONOBO_TYPE, GnomePrintBonobo))
#define GNOME_PRINT_BONOBO_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), GNOME_PRINT_BONOBO_TYPE, GnomePrintBonoboClass))
#define GNOME_IS_PRINT_BONOBO(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_PRINT_BONOBO_TYPE))
#define GNOME_IS_PRINT_BONOBO_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GNOME_PRINT_BONOBO_TYPE))

typedef struct _GnomePrintBonobo        GnomePrintBonobo;
typedef struct _GnomePrintBonoboPrivate GnomePrintBonoboPrivate;

typedef void     (GnomePrintBonoboRenderFn) (GnomePrintBonobo          *print,
					     GnomePrintContext         *ctx,
					     double                     width,
					     double                     height,
					     const Bonobo_PrintScissor *opt_scissor,
					     gpointer                   user_data);

struct _GnomePrintBonobo {
        BonoboObject             object;

	GClosure                *render;
	GnomePrintBonoboPrivate *priv;
};

typedef struct {
	BonoboObjectClass       parent;

	POA_Bonobo_Print__epv   epv;

	void         (*render) (GnomePrintBonobo          *print,
				GnomePrintContext         *ctx,
				double                     width,
				double                     height,
				const Bonobo_PrintScissor *opt_scissor);
} GnomePrintBonoboClass;

GType             gnome_print_bonobo_get_type    (void);
GnomePrintBonobo *gnome_print_bonobo_construct   (GnomePrintBonobo         *p,
						  GClosure            *render);
GnomePrintBonobo *gnome_print_bonobo_new         (GnomePrintBonoboRenderFn *render,
						  gpointer             user_data);
GnomePrintBonobo *gnome_print_bonobo_new_closure (GClosure            *render);

G_END_DECLS

#endif /* _GNOME_PRINT_BONOBO_H_ */

