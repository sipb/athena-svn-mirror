/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_PRINTER_H__
#define __GPA_PRINTER_H__

/*
 * This file is part of libgnomeprint 2
 *
 * Libgnomeprint is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Libgnomeprint is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the libgnomeprint; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors :
 *   Jose M. Celorio <chema@ximian.com>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GPA_TYPE_PRINTER (gpa_printer_get_type ())
#define GPA_PRINTER(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_PRINTER, GPAPrinter))
#define GPA_PRINTER_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_PRINTER, GPAPrinterClass))
#define GPA_IS_PRINTER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_PRINTER))
#define GPA_IS_PRINTER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_PRINTER))
#define GPA_PRINTER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_PRINTER, GPAPrinterClass))

typedef struct _GPAPrinter GPAPrinter;
typedef struct _GPAPrinterClass GPAPrinterClass;

#include <libxml/tree.h>
#include "gpa-list.h"
#include "gpa-model.h"

/* GPAPrinter */

struct _GPAPrinter {
	GPANode node;

	GPANode *name;
	GPANode *model;
	GPAList *settings;
};

struct _GPAPrinterClass {
	GPANodeClass node_class;
};

GType gpa_printer_get_type (void);

GPANode *gpa_printer_new_from_tree (xmlNodePtr tree);

GPANode *gpa_printer_get_default (void);
GPANode *gpa_printer_get_by_id (const guchar *id);

GPANode *gpa_printer_get_default_settings (GPAPrinter *printer);

/* Manipulation */

GPANode *gpa_printer_new_from_model (GPAModel *model, const guchar *name);

gboolean gpa_printer_save (GPAPrinter *printer);

/* GPAPrinterList */

GPAList *gpa_printer_list_load (void);

G_END_DECLS

#endif /* __GPA_PRINTER_H__ */
