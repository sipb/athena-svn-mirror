/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-categories-master-list-option-menu.h
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _E_CATEGORIES_MASTER_LIST_OPTION_MENU_H_
#define _E_CATEGORIES_MASTER_LIST_OPTION_MENU_H_

#include <libgnome/gnome-defs.h>
#include <gal/widgets/e-categories-master-list.h>
#include <gal/widgets/e-option-menu.h>

BEGIN_GNOME_DECLS

#define E_CATEGORIES_MASTER_LIST_OPTION_MENU_TYPE        (e_categories_master_list_option_menu_get_type ())
#define E_CATEGORIES_MASTER_LIST_OPTION_MENU(o)          (GTK_CHECK_CAST ((o), E_CATEGORIES_MASTER_LIST_OPTION_MENU_TYPE, ECategoriesMasterListOptionMenu))
#define E_CATEGORIES_MASTER_LIST_OPTION_MENU_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), E_CATEGORIES_MASTER_LIST_OPTION_MENU_TYPE, ECategoriesMasterListOptionMenuClass))
#define E_IS_CATEGORIES_MASTER_LIST_OPTION_MENU(o)       (GTK_CHECK_TYPE ((o), E_CATEGORIES_MASTER_LIST_OPTION_MENU_TYPE))
#define E_IS_CATEGORIES_MASTER_LIST_OPTION_MENU_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), E_CATEGORIES_MASTER_LIST_OPTION_MENU_TYPE))

typedef struct ECategoriesMasterListOptionMenuPriv ECategoriesMasterListOptionMenuPriv;

typedef struct {
	EOptionMenu parent;

	ECategoriesMasterListOptionMenuPriv *priv;
} ECategoriesMasterListOptionMenu;

typedef struct {
	EOptionMenuClass parent_class;
} ECategoriesMasterListOptionMenuClass;

GtkType    e_categories_master_list_option_menu_get_type      (void);
GtkWidget *e_categories_master_list_option_menu_new           (ECategoriesMasterList           *ecml);
GtkWidget *e_categories_master_list_option_menu_construct     (ECategoriesMasterListOptionMenu *ecmlom,
							       ECategoriesMasterList           *ecml);
char      *e_categories_master_list_option_menu_get_category  (ECategoriesMasterListOptionMenu *ecmlom);

END_GNOME_DECLS

#endif /* _E_CATEGORIES_MASTER_LIST_OPTION_MENU_H */
