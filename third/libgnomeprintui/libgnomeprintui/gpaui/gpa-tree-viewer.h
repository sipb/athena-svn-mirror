/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-option-menu.h:
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
 *   Chema Celorio <chema@ximian.com>
 *
 * Copyright (C) 2002 Ximian, Inc. 
 *
 */

#ifndef __GPA_TREE_VIEWER_H__
#define __GPA_TREE_VIEWER_H__

#include <glib.h>

G_BEGIN_DECLS

#include <libgnomeprint/private/gpa-node.h>

GtkWidget * gpa_tree_viewer_new (GPANode *node);

G_END_DECLS

#endif /* __GPA_TREE_VIEWER_H__ */

