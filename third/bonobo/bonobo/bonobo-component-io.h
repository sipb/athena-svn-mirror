/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_EMBEDDABLE_IO_H_
#define _BONOBO_EMBEDDABLE_IO_H_

#include <bonobo/bonobo-embeddable.h>
#include <bonobo/bonobo-storage.h>

BEGIN_GNOME_DECLS

BonoboEmbeddable *bonobo_embeddable_load             (BonoboStorage *storage,
							 const char *interface,
							 BonoboClientSite *client_site);

int                bonobo_embeddable_save             (BonoboEmbeddable *bonobo_object,
							 BonoboStorage   *storage,
							 gboolean       same_as_loaded);

BonoboEmbeddable *bonobo_embeddable_load_from_stream (BonoboStream *stream,
							 const char *interface);

int                bonobo_embeddable_save_to_stream   (BonoboEmbeddable *bonobo_object,
							 BonoboStream    *stream);

char              *gnome_get_class_id_from_file         (const char *filename);

END_GNOME_DECLS

#endif /* _BONOBO_EMBEDDABLE_IO_H_ */
