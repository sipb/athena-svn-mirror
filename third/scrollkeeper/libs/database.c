/* copyright (C) 2001 Sun Microsystems, Inc.*/

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <libintl.h>
#include <dirent.h>
#include <scrollkeeper.h>

#define PATHLEN		256


/*
 * Create a directory.  Send errors to appropriate places (STDOUT and log 
 * file) according to command-line flags.
 */
static int sk_mkdir(char *path, mode_t options, char outputprefs)
{
    if (mkdir(path, options) != 0) {
        sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "", 
                  _("Could not create directory %s : %s\n"),path,strerror(errno));
        return 1;
    }
    return 0;
}


/*
 * Create a directory and any parent directories as necessary.  Send errors
 * to appropriate places (STDOUT and log file) according to command-line
 * flags.
 *
 * This function uses strtok, which is pretty lame.  It would be nice to
 * replace it with something which doesn't write over its input variables.
 */
int sk_mkdir_with_parents(char *fullpath, mode_t options, char outputprefs)
{
    char path[1024];
    char slash[]="/";
    char delim[]="/";
    char *token, *pathcopy;
    struct stat buf;

    pathcopy = strdup(fullpath); /* Copy b/c strtok edits the string it operates on */
    path[0] = '\0';              /* Initialize with end of string null character */
    if (pathcopy[0] == slash[0]) sprintf(path, "/"); /* preserve any starting slash */
     
    token = strtok (pathcopy, delim);
    delim[0]=slash[0];
    while(token != NULL) {
        if (strlen(path) == 0 || ((strlen(path) == 1) && (path[0] == slash[0]))) {
                sprintf(path, "%s%s", path, token);
        } else {
                sprintf(path, "%s/%s", path, token);
        }
        if (stat(path, &buf) == -1) {
            if (sk_mkdir(path, options, outputprefs) != 0) {
                return 1;
                }
            }
        delim[0]=slash[0];
        token = strtok (NULL, delim);
        }

    return 0;
}


/* check if the database directory exists, if not create it
   with the locale directories having similar structure in
   $localstatedir as in $datadir/Templates
*/
int create_database_directory(char *scrollkeeper_dir, char *scrollkeeper_data_dir, char outputprefs)
{
    DIR *dir;
    char source_path[PATHLEN], target_path[PATHLEN]; 
    struct dirent *dir_ent;
    struct stat buf;
    int empty;
    char *data_dir, dirname[PATHLEN];
        
    /* check if it's empty */
    
    empty = 1;
    dir = opendir(scrollkeeper_dir);
    if (dir == NULL) {
    	if (sk_mkdir_with_parents(scrollkeeper_dir, S_IRUSR|S_IWUSR|S_IRGRP|
                            S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH, outputprefs) != 0) {
            return 1;
        }
	dir = opendir(scrollkeeper_dir);
    }
    
    
    while((dir_ent = readdir(dir)) != NULL && empty)
    {
        if (dir_ent->d_name[0] == '.')
	    continue;
	    
	empty = 0;
    }
    closedir(dir);
    
    if (!empty)
        return 0;
        
    data_dir = malloc((strlen(scrollkeeper_data_dir)+strlen("/Templates")+1)*
    			sizeof(char));
    check_ptr(data_dir, "scrollkeeper-install");
    sprintf(data_dir, "%s/Templates", scrollkeeper_data_dir);
    
    /* create locale directories and symlinks */
    
    dir = opendir(data_dir);
    
    while((dir_ent = readdir(dir)) != NULL)
    {
        if (dir_ent->d_name[0] == '.')
	    continue;
	    
	snprintf(source_path, PATHLEN, "%s/%s", data_dir, dir_ent->d_name);	
    
        lstat(source_path, &buf);
    
        if (S_ISDIR(buf.st_mode)) /* copy the directory */
	{
	    char source_file[PATHLEN], target_file[PATHLEN];
	
	    snprintf(dirname, PATHLEN, "%s/%s", scrollkeeper_dir, dir_ent->d_name);
	    mkdir(dirname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
	    
	    snprintf(source_file, PATHLEN, "%s/scrollkeeper_cl.xml", source_path);
	    snprintf(target_file, PATHLEN, "%s/%s/scrollkeeper_cl.xml", 
	    		scrollkeeper_dir, dir_ent->d_name);
	    copy_file(source_file, target_file);
	    snprintf(target_file, PATHLEN, "%s/%s/scrollkeeper_extended_cl.xml", 
	    		scrollkeeper_dir, dir_ent->d_name);
	    copy_file(source_file, target_file);
	}
	else /* link the directory */
	{
	    char *target_locale;
	    char aux_path[PATHLEN];

	    realpath(source_path, aux_path);
	    target_locale = strrchr(aux_path, '/');
	    target_locale++;
	    	    
	    snprintf(source_path, PATHLEN, "%s/%s", scrollkeeper_dir, dir_ent->d_name);
	    snprintf(target_path, PATHLEN, "%s", target_locale);
	    	   
	    symlink(target_path, source_path); 
	}
    }
    
    closedir(dir);
    free(data_dir);
    
    /* create TOC and index directory */
    
    snprintf(dirname, PATHLEN, "%s/TOC", scrollkeeper_dir);
    mkdir(dirname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);

    snprintf(dirname, PATHLEN, "%s/index", scrollkeeper_dir);
    mkdir(dirname, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    return 0;
}

