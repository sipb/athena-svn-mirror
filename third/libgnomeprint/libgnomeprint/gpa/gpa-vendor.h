/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_VENDOR_H__
#define __GPA_VENDOR_H__

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

#define GPA_TYPE_VENDOR (gpa_vendor_get_type ())
#define GPA_VENDOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_VENDOR, GPAVendor))
#define GPA_VENDOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_VENDOR, GPAVendorClass))
#define GPA_IS_VENDOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_VENDOR))
#define GPA_IS_VENDOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_VENDOR))
#define GPA_VENDOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_VENDOR, GPAVendorClass))

typedef struct _GPAVendor GPAVendor;
typedef struct _GPAVendorClass GPAVendorClass;

#include <libxml/tree.h>
#include "gpa-list.h"

/* GPAVendor */

struct _GPAVendor {
	GPANode node;
	GPANode *name;
	GPANode *url;
	GPAList *models;
};

struct _GPAVendorClass {
	GPANodeClass node_class;
};

GType gpa_vendor_get_type (void);

GPANode *gpa_vendor_get_by_id (const guchar *id);

/* GPAVendorList */

GPAList *gpa_vendor_list_load (void);

G_END_DECLS

#endif /* __GPA_VENDOR_H__ */

