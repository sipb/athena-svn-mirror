/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000 CodeFactory AB
   Copyright (C) 2000 Jonas Borgstr\366m <jonas@codefactory.se>
   Copyright (C) 2000 Anders Carlsson <andersca@codefactory.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __DOM_TEST_TREE_MODEL_H__
#define __DOM_TEST_TREE_MODEL_H__

#include <glib-object.h>
#include <dom/dom.h>

typedef struct _DomTestTreeModel DomTestTreeModel;
typedef struct _DomTestTreeModelClass DomTestTreeModelClass;

#define DOM_TYPE_TEST_TREE_MODEL             (dom_test_tree_model_get_type ())
#define DOM_TEST_TREE_MODEL(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_TEST_TREE_MODEL, DomTestTreeModel))
#define DOM_TEST_TREE_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_TEST_TREE_MODEL, DomTestTreeModelClass))
#define DOM_IS_TEST_TREE_MODEL(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_TEST_TREE_MODEL))
#define DOM_IS_TEST_TREE_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_TEST_TREE_MODEL))
#define DOM_TEST_TREE_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_TEST_TREE_MODEL, DomTestTreeModelClass))

enum _DomTestTreeModelType {
	DOM_TEST_TREE_MODEL_TREE,
	DOM_TEST_TREE_MODEL_LIST
};

typedef enum _DomTestTreeModelType DomTestTreeModelType;

struct _DomTestTreeModel {
	GObject parent;

	DomNode *root;

	DomTestTreeModelType type;
};

struct _DomTestTreeModelClass {
	GObjectClass parent_class;
};


GType dom_test_tree_model_get_type (void);

DomTestTreeModel *dom_test_tree_model_new (DomNode *root, DomTestTreeModelType type);

#endif /* __DOM_TEST_TREE_MODEL_H__ */

