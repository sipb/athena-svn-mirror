/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* GPdf Bonobo PersistFile implementation
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GPDF_PERSIST_FILE_H
#define GPDF_PERSIST_FILE_H

#include "gpdf-g-switch.h"
#  include <bonobo/bonobo-persist.h>
#include "gpdf-g-switch.h"
#include "PDFDoc.h"

G_BEGIN_DECLS

typedef struct _BonoboControl BonoboControl;

#define GPDF_TYPE_PERSIST_FILE            (gpdf_persist_file_get_type ())
#define GPDF_PERSIST_FILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_PERSIST_FILE, GPdfPersistFile))
#define GPDF_PERSIST_FILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_PERSIST_FILE, GPdfPersistFileClass))
#define GPDF_IS_PERSIST_FILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_PERSIST_FILE))
#define GPDF_IS_PERSIST_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_PERSIST_FILE))

typedef struct _GPdfPersistFile        GPdfPersistFile;
typedef struct _GPdfPersistFileClass   GPdfPersistFileClass;
typedef struct _GPdfPersistFilePrivate GPdfPersistFilePrivate;

struct _GPdfPersistFile {
	BonoboPersist parent;

	GPdfPersistFilePrivate *priv;
};

struct _GPdfPersistFileClass {
	BonoboPersistClass parent_class;

	POA_Bonobo_PersistFile__epv epv;

	/* Signals */
	void (*loading_finished) (GPdfPersistFile *persist_file);
	void (*loading_failed)   (GPdfPersistFile *persist_file, const gchar *message);
};

GType            gpdf_persist_file_get_type    (void);
GPdfPersistFile *gpdf_persist_file_new         (const gchar *iid);
GPdfPersistFile *gpdf_persist_file_construct   (GPdfPersistFile *gpdf_persist_file, const gchar *iid);

PDFDoc          *gpdf_persist_file_get_pdf_doc (GPdfPersistFile *gpdf_persist_file);
const char      *gpdf_persist_file_get_current_uri (GPdfPersistFile *pfile);
void		 gpdf_persist_file_set_control (GPdfPersistFile *gpdf_persist_file,
						BonoboControl *control);

G_END_DECLS

#endif /* GPDF_PERSISTSTREAM_H */
