/* -*- Mode: C; tab-width: 4; indent-tabs-mode: 4; c-basic-offset: 4 -*- */

/* cdemenu-desktop--method.c

	Author: Stephen Browne <stephen.browne@sun.com>

   Copyright (C) 2002 Sun Microsystems, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <libgnomevfs/gnome-vfs-mime.h>

#include <libgnomevfs/gnome-vfs-module.h>
#include <libgnomevfs/gnome-vfs-method.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-module-shared.h>

#define LINESIZE 1024
#define CDE_ICON_NAME_CACHE "/.gnome/g-cdeiconnames"

static GSList *cdemenufiles;

typedef struct _DtFile DtFile;

struct _DtFile {
	gchar *contents;
	gchar *current;
};

static GnomeVFSMethod *parent_method = NULL;

static gchar *
find_title_for_menu (gchar *name)
{
	GSList *filename;
	char line[LINESIZE];
	gboolean found=FALSE;
	char *title="\0";
    FILE *file = NULL;
    gchar *tmp=NULL;

    for (filename = cdemenufiles; filename !=NULL; filename=filename->next){
		file = fopen((gchar*)filename->data,"r");
		while (fgets(line,LINESIZE,file) != NULL){
            if (strstr(line,"f.menu")  && strstr(line, name)) {
				tmp = strstr (line, "f.menu");
				*tmp = '\0';
				title = strdup (line);
				found = TRUE;
				break;
			}
		}
		fclose (file);
		file = NULL;
		if (found) break;
	}
	return title;
}

static FILE *
open_and_find_pointer_to_menu(gchar *name)
{
	GSList *filename;
	gchar line[LINESIZE];
	FILE *filesave = NULL, *file = NULL;
	gboolean found = FALSE;
	gchar *tmp=NULL;
	
	for (filename = cdemenufiles; filename !=NULL; filename=filename->next){
		found = FALSE;
		file = fopen((gchar*)filename->data,"r");
		while (fgets(line,LINESIZE,file) != NULL){
			if (strstr(line,"Menu") == line) {
				if ((tmp = strstr(line,name)) != NULL){
					tmp = g_strstrip(tmp);
					if (strcmp(tmp,name) == 0) {
						found = TRUE;
						break;
					}
				}
			}
		}
		if (found) {
			if (filesave !=NULL) fclose(filesave);
			filesave = file;
			if (!strcmp(name, "DtRootMenu")) break;
		} else {
			fclose(file);
			file=NULL;
		}
	}
	return filesave;
}

static char *
expand_env_vars(char *s)
{
	char **tokens, **token;
	char *tmp, *tmp2;
	const char *tmp3;
	char *expanded;
	tokens = g_strsplit(s,"/",30);
	for (token = tokens; *token !=NULL; token++) {
		if (**token=='$'){
			tmp = *token+1;
			if (*tmp == '{'){
				tmp++;
				tmp2 = tmp+(strlen(tmp))-1; *tmp2='\0';
			}
			tmp3 = g_getenv(tmp);
			if (!tmp3 && strstr(tmp, "LC_CTYPE"))
				/* It's not always the case $LC_CTYPE or $LANG is set */
				tmp3 = (char*)setlocale(LC_CTYPE, NULL);
			g_free(*token);
			/* don't do g_strdup (s?s1:s2) as that doesn't work with
			   certain gcc 2.96 versions */
			*token = tmp3 ? g_strdup(tmp3) : g_strdup("");
		}
	}
	expanded = g_strjoinv("/", tokens);
	g_strfreev(tokens);
	return expanded;
}

/* This function is only used to make the default CDE menu look nice */
static char*
get_icon_for_menu (char *name) 
{
	if (!strcmp(name,"Applications")) 
		return ("/usr/dt/appconfig/icons/C/Dtapps.m.pm");
	if (!strcmp(name,"Cards")) 
		return ("/usr/dt/appconfig/icons/C/SDtCard.m.pm");
	if (!strcmp(name,"Files")) 
		return ("/usr/dt/appconfig/icons/C/Dtdata.m.pm");
	if (!strcmp(name,"Folders")) 
		return ("/usr/dt/appconfig/icons/C/DtdirB.m.pm");
	if (!strcmp(name,"Help")) 
		return ("/usr/dt/appconfig/icons/C/Dthelp.m.pm");
	if (!strcmp(name,"Hosts")) 
		return ("/usr/dt/appconfig/icons/C/Dtterm.m.pm");
	if (!strcmp(name,"Links")) 
		return ("/usr/dt/appconfig/icons/C/SDturlweb.m.pm");
	if (!strcmp(name,"Mail")) 
		return ("/usr/dt/appconfig/icons/C/Dtmail.m.pm");
	if (!strcmp(name,"Tools")) 
		return ("/usr/dt/appconfig/icons/C/SDtGears.m.pm");
	if (!strcmp(name,"Windows")) 
		return ("/usr/dt/appconfig/icons/C/DtDtwm.m.pm");
	return ("");
}

static char*
get_icon_for_action(char *action)
{
	gchar line[LINESIZE];
	FILE *file;
	char **dir, *expandeddir, *fullpath=NULL;
	char *tmp=NULL;
	char *icon_name_cache_path;

	char *iconsearchpath[] = {"/usr/dt/appconfig/icons/$LC_CTYPE",
                       "/etc/dt/appconfig/icons/$LC_CTYPE",
                       "$HOME/.dt/icons",
                       "/usr/dt/appconfig/icons/C", NULL};

	/*if action has arg strip the arg*/
	if ((tmp=strchr(action,'\"'))) {*tmp='\0'; tmp=NULL;}
	
	/*Return NULL icon if icon DB does not exist*/
	icon_name_cache_path = g_strconcat (g_get_home_dir (), CDE_ICON_NAME_CACHE, NULL);
	file = fopen(icon_name_cache_path,"r");
	g_free(icon_name_cache_path);
	if (!file) return NULL;

	while (fgets(line,LINESIZE,file) != NULL){
		if (strstr(line,action)){
			if (fgets(line,LINESIZE,file) != NULL) {
				if (strstr(line,"ICON")){
					tmp = strchr(line,':');
					if (tmp == NULL) return NULL;
					tmp++;
					tmp = g_strstrip(tmp);
					for (dir = iconsearchpath; *dir != NULL; dir++) {
						expandeddir = expand_env_vars(*dir);
						fullpath = g_strdup_printf("%s/%s.m.pm",
													expandeddir, tmp);
						if (g_file_test(fullpath,G_FILE_TEST_EXISTS)){ 
							g_free(expandeddir);
							break;
						} else {
							g_free(fullpath);
							fullpath = g_strdup_printf("%s/%s.s.pm",
														expandeddir, tmp);
							if (g_file_test(fullpath,G_FILE_TEST_EXISTS)){ 
											g_free(expandeddir);
								break;
							} else g_free (fullpath);
						}
					}
   				}
			}
			break;
		}
	}
	fclose(file);
	return fullpath;
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	FILE *file;
	gchar *tmp, *tmp2, *end;
	gchar line[LINESIZE];
	gchar *name, *title, *exec=NULL, *icon=NULL;
	gchar *menuname = g_path_get_basename(g_path_get_dirname(uri->text));
	DtFile *dtfile = g_new0(DtFile, 1);

	*method_handle = (GnomeVFSMethodHandle *)dtfile;

	if (strcmp(menuname,"/") == 0) menuname = "DtRootMenu";

	name = g_path_get_basename(uri->text);
	/* Strip off the .desktop if this is a .desktop file */
	tmp = strstr(name,".desktop"); if (tmp) *tmp='\0'; tmp=NULL;

	/* disabling separators for now, panel is messing things up 
	if (strstr(name,"separator")) {
		dtfile->contents= g_strdup("[Desktop Entry]\nName=Separator\n"
				"Comment=Separator\nIcon=\nType=Separator\n");
		dtfile->current=dtfile->contents;
		return GNOME_VFS_OK;
	}*/
	file = open_and_find_pointer_to_menu(menuname);
	if(!file) return GNOME_VFS_ERROR_NOT_FOUND;

	if (strcmp(name,".directory") == 0) {
				char *utf8_name = NULL;
				if (!strcmp (menuname, "DtRootMenu"))
					title = g_strdup_printf ("\"%s\"",menuname);
				else title = find_title_for_menu(menuname);
				tmp = strchr(title,'\"'); tmp ++;
				tmp2 = strchr(tmp,'\"'); if(tmp2) *tmp2='\0';

				utf8_name = g_locale_to_utf8 (tmp, -1,
							      NULL, NULL,
							      NULL);
				/* fallback to avoid crash */
				if (utf8_name == NULL)
					utf8_name = g_strdup (tmp);

				dtfile->contents = g_strdup_printf 
					("[Desktop Entry]\n"
					 "Encoding=UTF-8\n"
					 /* Note that we include the utf-8
					  * translated name without a language
					  * specifier, but that's OK, it will
					  * work */
					 "Name=%s\n"
					 "Comment=\n"
					 "Icon=%s\n"
					 "Type=Directory\n",
					 utf8_name,
					 get_icon_for_menu(tmp));

				g_free (utf8_name);
				g_free (title);
	} else {
		while (fgets(line,LINESIZE,file) != NULL){
			if (line[0] == '#') continue;
			else if (line[0] == '}') break;
			else if (strstr(line,name)){
				if((tmp = strstr(line,"f.action")) != NULL){
					char *utf8_name = NULL;

					tmp+=8;
					end = strchr(tmp,'\n');
					if (end) *end='\0';
					exec=g_strdup_printf("dtaction %s",tmp);
					icon=get_icon_for_action(tmp);
					if (!icon) icon=g_strdup("NONE");

					utf8_name = g_locale_to_utf8 (name, -1,
								      NULL,
								      NULL,
								      NULL);
					/* fallback to avoid crash */
					if (utf8_name == NULL)
						utf8_name = g_strdup (name);

					dtfile->contents= g_strdup_printf
						("[Desktop Entry]\n"
						 "Encoding=UTF-8\n"
						 /* Note that we include the
						  * utf-8 translated name
						  * without a language
						  * specifier, but that's
						  * OK, it will work */
						 "Name=%s\n"
						 "Comment=\n"
						 "Exec=%s\n"
						 "Icon=%s\n"
						 "Terminal=0\n"
						 "Type=Application\n",
						  utf8_name,
						 exec, icon);
					g_free (utf8_name);
					g_free(icon);
				} else if ((tmp = strstr(line,"f.exec"))!=NULL){
					char *utf8_name = NULL;

					tmp+=6;
					tmp = g_strstrip(tmp);
					/* strip off quotes */
					if (*tmp == '\"') tmp++;
					end = tmp+(strlen(tmp))-1;
					if (*end == '\"') *end = '\0';

					utf8_name = g_locale_to_utf8 (name, -1,
								      NULL,
								      NULL,
								      NULL);
					/* fallback to avoid crash */
					if (utf8_name == NULL)
						utf8_name = g_strdup (name);

					dtfile->contents= g_strdup_printf
						("[Desktop Entry]\n"
						 "Encoding=UTF-8\n"
						 /* Note that we include the
						  * utf-8 translated name
						  * without a language
						  * specifier, but that's
						  * OK, it will work */
						 "Name=%s\n"
						 "Comment=\n"
						 "Exec=%s\n"
						 "Icon=\n"
						 "Terminal=0\n"
						 "Type=Application\n",
						  utf8_name, tmp);
					g_free (utf8_name);
				}
			}
		}
	}

	fclose(file);

	dtfile->current=dtfile->contents;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{

	DtFile *dtfile = (DtFile*)method_handle;

	g_free(dtfile->contents);	

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	int bytes;
	DtFile *dtfile = (DtFile *)method_handle;
	if (dtfile->current == NULL) return GNOME_VFS_ERROR_EOF;

	bytes = snprintf(buffer, num_bytes, "%s",dtfile->current);
	if (bytes == -1 ) dtfile->current+=num_bytes;
	else dtfile->current=NULL;

	*bytes_read=(GnomeVFSFileSize)bytes;
	
	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   GnomeVFSContext *context)
{
	FILE *file;
	gchar *menuname = g_path_get_basename(uri->text);
	if (strcmp(menuname,"/") == 0) menuname = "DtRootMenu";
	
	file = open_and_find_pointer_to_menu(menuname);
	
	*method_handle = (GnomeVFSMethodHandle *)file;

	return file ? GNOME_VFS_OK : GNOME_VFS_ERROR_NOT_FOUND;
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
	fclose((FILE *)method_handle);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	gchar line[LINESIZE];
	gchar *tmp, *tmp2;
	FILE *file = (FILE *)method_handle;
	/*static int sep=0;*/
	
	file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;

        while (fgets(line,LINESIZE,file) != NULL){
			if (line[0] == '#') continue;
			else if (line[0] == '}') return GNOME_VFS_ERROR_EOF;
			else if (strstr(line,"f.action") && (strstr(line,"ExitSession")
					|| strstr(line,"LockDisplay") 
					|| strstr(line,"AddItemToMenu")
					|| strstr(line,"CustomizeWorkspaceMenu")
					|| strstr(line,"UpdateWorkspaceMenu"))) {
					file_info->name = g_strdup("bogus.desktop");
					file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
 					continue;
			}
			else if (strstr(line,"f.action") || strstr(line,"f.exec")) {
					tmp = strchr(line,'\"'); tmp ++;
					tmp2 = strchr(tmp,'\"'); if(tmp2) *tmp2='\0';
					file_info->name = g_strdup_printf("%s.desktop",tmp);
					file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
					break;
			}
			/*
			else if (strstr(line,"f.separator")) {
				file_info->name = g_strdup_printf("separator%d.desktop",
					sep);
				file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
				sep++; if (sep>10) sep = 0;
				break;
			}
			*/
			else if  (strstr(line,"f.menu")) {
				tmp = strstr(line,"f.menu");
   				tmp+=6;
   				tmp = g_strstrip(tmp);
           		file_info->name = g_strdup(tmp);
				file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
				break;
			}	
        }

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	char *s;

	/* FIXME: This isnt right at all but it satisfies gnome-vfs for now */
	s = gnome_vfs_get_uri_from_local_path(cdemenufiles->data);
	file_uri = gnome_vfs_uri_new(s);
	g_free(s);
	
	result = (* parent_method->get_file_info) (parent_method,
						   file_uri,
						   file_info,
						   options,
						   context); 
	gnome_vfs_uri_unref (file_uri);

	return result;
}

static gboolean
do_is_local (GnomeVFSMethod *method,
             const GnomeVFSURI *uri)
{
	/* FIXME: This isn't always true, some cde menu files are remote */
	return TRUE;
}

/* gnome-vfs bureaucracy */
static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	NULL,				/* create */
	do_close,
	do_read,
	NULL,				/* write */
	NULL,				/* seek */
	NULL,				/* tell */
	NULL,				/* truncate handle */
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	NULL,				/* file info from handle */
	do_is_local,
	NULL,				/* make directory */
	NULL,				/* remove directory */
	NULL,				/* move */
	NULL,				/* unlink */
	NULL,				/* same fs */
	NULL,				/* set file info */
	NULL,				/* truncate */
	NULL,				/* find directory */
	NULL				/* symbolic link */
};

static void
find_all_included_files(gchar *filename)
{
	FILE *file;
	gchar line[LINESIZE];
	gchar *tmp, *expanded;
	file = fopen(filename,"r");
	while (fgets(line,LINESIZE,file) != NULL){
		if (g_ascii_strncasecmp(line, "Include",7) == 0) {
			while (fgets(line,LINESIZE,file) !=NULL){
				if (line[0] == '#') continue;
				if (line[0] == '{') continue;
				else if (line[0] == '}') break;
				tmp = g_strstrip(line);
				if (tmp[0] == '\0') continue;
				expanded = expand_env_vars(tmp);
				if (g_file_test(expanded,G_FILE_TEST_EXISTS)){
               		cdemenufiles = g_slist_append(cdemenufiles, 
						g_strdup(expanded));
					find_all_included_files(expanded);
				}
				free(expanded);
			}
		}
	}
	fclose(file);
}

static void
create_cde_icon_name_cache (void)
{
	/* fork the dttypes command to create the icon name cache */
	int fd, t;
	mode_t old_mask;
	char *icon_name_cache_path;
	
	if ((t = fork ()) < 0) g_error ("Unable to fork.");
	else if (t) wait(NULL);
	else {
		icon_name_cache_path = g_strconcat (g_get_home_dir (), CDE_ICON_NAME_CACHE, NULL);
		old_mask = umask(033);
		fd = open (icon_name_cache_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		dup2 (fd, 1);
		fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); 
		g_free(icon_name_cache_path);
		umask(old_mask);
		execl("/usr/dt/bin/dttypes", "dttypes", "-w", "fld_name", "ICON",
			  "-l", "fld_name", "ICON", NULL);
	}
}

GnomeVFSMethod *
vfs_module_init (const char *method_name, 
		 const char *args)
{
	gchar *files[7];
	int i;
	char *icon_name_cache_path;

	parent_method = gnome_vfs_method_get ("file");

	if (parent_method == NULL) {
		g_error ("Could not find 'file' method for gnome-vfs");
		return NULL;
	}

	/* find the right cde menu file to start with */
	cdemenufiles = NULL;
	files[0] = expand_env_vars("$HOME/.dt/$LC_CTYPE/dtwmrc");
	files[1] = expand_env_vars("$HOME/.dt/dtwmrc");
	files[2] = expand_env_vars("/etc/dt/config/$LC_CTYPE/sys.dtwmrc");
	files[3] = g_strdup("/etc/dt/config/sys.dtwmrc");
	files[4] = expand_env_vars("/usr/dt/config/$LC_CTYPE/sys.dtwmrc");
	files[5] = g_strdup("/usr/dt/config/sys.dtwmrc");
	files[6] = g_strdup("/usr/dt/config/C/sys.dtwmrc");

	for (i=0;i<7;i++){
		if (g_file_test(files[i],G_FILE_TEST_EXISTS)){
			cdemenufiles = g_slist_append(cdemenufiles,
				g_strdup(files[i]));
			break;
		}	
	}
	for (i=0;i<7;i++) g_free(files[i]);

	/* if didnt find any menu start file then we are fecked */
	if (cdemenufiles == NULL) return NULL;

	/* from that file find a list of all included files */
	find_all_included_files(cdemenufiles->data);

	/*create the icon name cache if it does not exist*/
	icon_name_cache_path = g_strconcat (g_get_home_dir (), CDE_ICON_NAME_CACHE, NULL);
	if (!g_file_test(icon_name_cache_path, G_FILE_TEST_EXISTS))
		create_cde_icon_name_cache();

	g_free(icon_name_cache_path);
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	/* free up any stuff in here */

	g_slist_foreach(cdemenufiles, (GFunc)g_free, NULL);
	g_slist_free(cdemenufiles);
}

