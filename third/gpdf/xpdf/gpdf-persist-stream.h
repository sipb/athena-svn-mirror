/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * gpdf bonobo pdf persistor
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * Copyright 2002 Martin Kretzschmar
 */

#ifndef GPDF_PERSIST_STREAM_H
#define GPDF_PERSIST_STREAM_H

#include "gpdf-g-switch.h"
#  include <bonobo/bonobo-persist.h>
#include "gpdf-g-switch.h"
#include "PDFDoc.h"

G_BEGIN_DECLS

typedef struct _BonoboControl BonoboControl;

#define GPDF_TYPE_PERSIST_STREAM            (gpdf_persist_stream_get_type ())
#define GPDF_PERSIST_STREAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_PERSIST_STREAM, GPdfPersistStream))
#define GPDF_PERSIST_STREAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_PERSIST_STREAM, GPdfPersistStreamClass))
#define GPDF_IS_PERSIST_STREAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_PERSIST_STREAM))
#define GPDF_IS_PERSIST_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_PERSIST_STREAM))

typedef struct _GPdfPersistStream        GPdfPersistStream;
typedef struct _GPdfPersistStreamClass   GPdfPersistStreamClass;
typedef struct _GPdfPersistStreamPrivate GPdfPersistStreamPrivate;

struct _GPdfPersistStream {
	BonoboPersist parent;

	GPdfPersistStreamPrivate *priv;
};

struct _GPdfPersistStreamClass {
	BonoboPersistClass parent_class;

	POA_Bonobo_PersistStream__epv epv;

	/* Signals */
	void (*set_pdf) (GPdfPersistStream *persist_stream);
};

GType              gpdf_persist_stream_get_type    (void);
GPdfPersistStream *gpdf_persist_stream_new         (const gchar *iid);
GPdfPersistStream *gpdf_persist_stream_construct   (GPdfPersistStream *gpdf_persist_stream, const gchar *iid);

PDFDoc            *gpdf_persist_stream_get_pdf_doc (GPdfPersistStream *gpdf_persist_stream);

void 		   gpdf_persist_stream_set_control (GPdfPersistStream *gpdf_persist_stream,  BonoboControl *control);

G_END_DECLS

#endif /* GPDF_PERSISTSTREAM_H */
