/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-print-to-file.h: A print to file selector
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
 *  Authors :
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2003 Ximian, Inc. 
 */

#ifndef __GPA_PRINT_TO_FILE_H__
#define __GPA_PRINT_TO_FILE_H__

#include <glib.h>

G_BEGIN_DECLS

#include "gpa-widget.h"

#define GPA_TYPE_P2F      (gpa_p2f_get_type ())
#define GPA_P2F(o)        (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_P2F, GPAPrintToFile))
#define GPA_IS_P2F(o)     (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_P2F))

typedef struct _GPAPrintToFile      GPAPrintToFile;
typedef struct _GPAPrintToFileClass GPAPrintToFileClass;

#include <libgnomeprint/private/gpa-node.h>
#include "gpa-widget.h"

struct _GPAPrintToFile {
	GPAWidget widget;
	
	GtkWidget *checkbox;   /* The widget */
	GtkWidget *entry;      /* output file entry */
	
	GPANode *node;         /* node we are listening to the Output.Job.PrintToFile */
	GPANode *node_output;  /* node we are listening to the Output.Job.Filename */
	GPANode *config;       /* the GPAConfig node */

	gulong handler;        /* signal handler of ->node "modified" signal */
	gulong handler_output; /* signal handler of ->node_output "modified" signal */
	gulong handler_config; /* signal handler of ->config "modified" signal */

	gboolean updating;     /* A flag used to ignore emmissions create by us */
};

struct _GPAPrintToFileClass {
	GPAWidgetClass widget_class;
};

GType   gpa_p2f_get_type (void);
void    gpa_p2f_enable_filename_entry (GPAPrintToFile *c, gboolean enable);

G_END_DECLS

#endif /* __GPA_PRINT_TO_FILE_H__ */
