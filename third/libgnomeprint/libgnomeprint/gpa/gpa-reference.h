/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_REFERENCE_H__
#define __GPA_REFERENCE_H__

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

#define GPA_TYPE_REFERENCE (gpa_reference_get_type ())
#define GPA_REFERENCE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_REFERENCE, GPAReference))
#define GPA_REFERENCE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_REFERENCE, GPAReferenceClass))
#define GPA_IS_REFERENCE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_REFERENCE))
#define GPA_IS_REFERENCE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_REFERENCE))
#define GPA_REFERENCE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_REFERENCE, GPAReferenceClass))

typedef struct _GPAReference GPAReference;
typedef struct _GPAReferenceClass GPAReferenceClass;

#include "gpa-node-private.h"

/* GPAReference */

struct _GPAReference {
	GPANode node;
	GPANode *ref;
};

struct _GPAReferenceClass {
	GPANodeClass node_class;
	/* fixme: Either find better name or move signalling to GPANodeClass */
	gboolean (* set_value) (GPAReference *reference, const guchar *value);
};

#define GPA_REFERENCE_REFERENCE(r) ((r) ? GPA_REFERENCE (r)->ref : NULL)

GType gpa_reference_get_type (void);

GPANode *gpa_reference_new (GPANode *ref);
GPANode *gpa_reference_new_empty (void);

gboolean gpa_reference_set_reference (GPAReference *reference, GPANode *ref);

G_END_DECLS

#endif /* __GPA_REFERENCE_H__ */

