/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_CONFIG_H__
#define __GPA_CONFIG_H__

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
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GPA_TYPE_CONFIG (gpa_config_get_type ())
#define GPA_CONFIG(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_CONFIG, GPAConfig))
#define GPA_CONFIG_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_CONFIG, GPAConfigClass))
#define GPA_IS_CONFIG(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_CONFIG))
#define GPA_IS_CONFIG_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_CONFIG))
#define GPA_CONFIG_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_CONFIG, GPAConfigClass))

typedef struct _GPAConfig GPAConfig;
typedef struct _GPAConfigClass GPAConfigClass;

#include "gpa-list.h"

struct _GPAConfig {
	GPANode node;
	GPANode *globals;
	GPANode *printer;
	GPANode *settings;
};

struct _GPAConfigClass {
	GPANodeClass node_class;
};

GType gpa_config_get_type (void);

GPANode *gpa_config_new (void);

G_END_DECLS

#endif
