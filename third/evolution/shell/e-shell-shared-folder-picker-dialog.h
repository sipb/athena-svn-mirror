/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* e-shell-shared-folder-picker-dialog.h - Implementation for the shared folder
 * picker dialog.
 *
 * Copyright (C) 2002 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Ettore Perazzoli <ettore@ximian.com>
 */

#ifndef E_SHELL_SHARED_FOLDER_PICKER_DIALOG_H
#define E_SHELL_SHARED_FOLDER_PICKER_DIALOG_H

#include "e-shell.h"
#include "e-shell-view.h"

void e_shell_show_shared_folder_picker_dialog (EShell *shell,
					       EShellView *parent);

#endif /* E_SHELL_SHARED_FOLDER_PICKER_DIALOG_H */