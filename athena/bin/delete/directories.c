/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.c,v $
 * $Author: miki $
 * 
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

#if !defined(lint) && !defined(SABER)
     static char rcsid_directories_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.c,v 1.24 1993-04-29 17:39:44 miki Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#ifdef POSIX
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#ifdef SYSV
#include <string.h>
#define index strchr
#define rindex strrchr
#else
#include <strings.h>
#endif /* SYSV */
#include <errno.h>
#include <com_err.h>
#include "delete_errs.h"
#include "util.h"
#include "directories.h"
#include "mit-copying.h"
#include "errors.h"

extern char *realloc();
extern long time();
extern int errno;

static filerec root_tree;
static filerec cwd_tree;

void free_leaf();

 /* These are not static because external routines need to be able to */
 /* access them. 						      */
time_t current_time;


static filerec default_cwd = {
     "",
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     False,
     False,
     {0}
};

static filerec default_root = {
     "/",
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     False,
     False,
     {0}
};

static filerec default_directory = {
     "",
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     False,
     False,
     {0}
};

static filerec default_file = {
     "",
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     (filerec *) NULL,
     False,
     False,
     {0}
};


filerec *get_root_tree()
{
     return(&root_tree);
}



filerec *get_cwd_tree()
{
     return(&cwd_tree);
}


int initialize_tree()
{
     int retval;
     
     root_tree = default_root;
     cwd_tree = default_cwd;

     current_time = time((time_t *)0);
     if (retval = get_specs(".", &cwd_tree.specs, FOLLOW_LINKS)) {
	  error("get_specs on .");
	  return retval;
     }
     if (retval = get_specs("/", &root_tree.specs, FOLLOW_LINKS)) {
	  error("get_specs on /");
	  return retval;
     }
     return 0;
}


int add_path_to_tree(path, leaf)
char *path;
filerec **leaf;
{
     filerec *parent;
     char next_name[MAXNAMLEN];
     char lpath[MAXPATHLEN], built_path[MAXPATHLEN], *ptr;
     struct mystat specs;
     int retval;
     
     if (retval = get_specs(path, &specs, DONT_FOLLOW_LINKS)) {
	  char error_buf[MAXPATHLEN+14];

	  (void) sprintf(error_buf, "get_specs on %s", path);
	  error(error_buf);
	  return retval;
     }
     
     (void) strcpy(lpath, path); /* we don't want to damage the user's */
				 /* string */
     ptr = lpath;
     if (*ptr == '/') {
	  parent = &root_tree;
	  ptr++;
	  (void) strcpy(built_path, "/");
     }
     else if (! strncmp(ptr, "./", 2)) {
	  parent = &cwd_tree;
	  ptr += 2;
	  *built_path = '\0';
     }
     else {
	  parent = &cwd_tree;
	  *built_path = '\0';
     }
     
     (void) strcpy(next_name, firstpart(ptr, ptr));
     while (*ptr) {
	  (void) strcat(built_path, next_name);
	  if (retval = add_directory_to_parent(parent, next_name, False,
					       &parent)) {
	       error("add_directory_to_parent");
	       return retval;
	  }
	  (void) strcpy(next_name, firstpart(ptr, ptr));
	  if (retval = get_specs(built_path, &parent->specs, FOLLOW_LINKS)) {
	       char error_buf[MAXPATHLEN+14];

	       (void) sprintf(error_buf, "get_specs on %s", built_path);
	       error(error_buf);
	       return retval;
	  }
	  (void) strcat(built_path, "/");
     }
     if ((specs.st_mode & S_IFMT) == S_IFDIR) {
	  retval = add_directory_to_parent(parent, next_name, True, leaf);
	  if (retval) {
	       error("add_directory_to_parent");
	       return retval;
	  }
     }
     else {
	  retval = add_file_to_parent(parent, next_name, True, leaf);
	  if (retval) {
	       error("add_file_to_parent");
	       return retval;
	  }
     }          

     (*leaf)->specs = specs;

     return 0;
}



int get_specs(path, specs, follow)
char *path;
struct mystat *specs;
int follow; /* follow symlinks or not? */
{
     int status;
     struct stat realspecs;
     
     if (strlen(path)) if ((path[strlen(path) - 1] == '/') &&
			   (strlen(path) != 1))
	  path[strlen(path) - 1] = '\0';
     if (follow == FOLLOW_LINKS)
	  status = stat(path, &realspecs);
     else 
	  status = lstat(path, &realspecs);

     if (status) {
	  set_error(errno);
	  error(path);
	  return error_code;
     }

     specs->st_dev = realspecs.st_dev;
     specs->st_ino = realspecs.st_ino;
     specs->st_mode = realspecs.st_mode;
     specs->st_size = realspecs.st_size;
#ifdef SOLARIS
     specs->st_time = realspecs.st_ctime;
#else
     specs->st_ctime = realspecs.st_ctime;
#endif
#ifdef USE_BLOCKS
     specs->st_blocks = realspecs.st_blocks;
#endif

     return 0;
}



filerec *next_leaf(leaf)
filerec *leaf;
{
     filerec *new;

     if ((leaf->specs.st_mode & S_IFMT) == S_IFDIR) {
	  new = first_in_directory(leaf);
	  if (new)
	       return(new);
	  new = next_directory(leaf);
	  return(new);
     }
     else {
	  new = next_in_directory(leaf);
	  return(new);
     }
}


filerec *next_specified_leaf(leaf)
filerec *leaf;
{
     while (leaf = next_leaf(leaf))
     if (leaf->specified)
	  return(leaf);
     return((filerec *) NULL);
}


filerec *next_directory(leaf)
filerec *leaf;
{
     filerec *ret;
     if ((leaf->specs.st_mode & S_IFMT) != S_IFDIR)
	  leaf = leaf->parent;
     if (leaf)
	  ret = leaf->next;
     else
	  ret = (filerec *) NULL;
     if (ret) if (ret->freed)
	  ret = next_directory(ret);
     return(ret);
}


filerec *next_specified_directory(leaf)
filerec *leaf;
{
     while (leaf = next_directory(leaf))
	  if (leaf->specified)
	       return(leaf);
     return ((filerec *) NULL);
}



filerec *next_in_directory(leaf)
filerec *leaf;
{
     filerec *ret;

     if (leaf->next)
	  ret = leaf->next;
     else if (((leaf->specs.st_mode & S_IFMT) != S_IFDIR) && leaf->parent)
	  ret = leaf->parent->dirs;
     else
	  ret = (filerec *) NULL;
     if (ret) if (ret->freed)
	  ret = next_in_directory(ret);
     return (ret);
}




filerec *next_specified_in_directory(leaf)
filerec *leaf;
{
     while (leaf = next_in_directory(leaf))
	  if (leaf->specified)
	       return(leaf);
     return ((filerec *) NULL);
}



filerec *first_in_directory(leaf)
filerec *leaf;
{
     filerec *ret;

     if ((leaf->specs.st_mode & S_IFMT) != S_IFDIR)
	  ret = (filerec *) NULL;
     else if (leaf->files)
	  ret = leaf->files;
     else if (leaf->dirs)
	  ret =  leaf->dirs;
     else
	  ret = (filerec *) NULL;
     if (ret) if (ret->freed)
	  ret = next_in_directory(ret);
     return(ret);
}


filerec *first_specified_in_directory(leaf)
filerec *leaf;
{
     leaf = first_in_directory(leaf);
     if (! leaf)
	  return((filerec *) NULL);
     
     if (leaf->specified)
	  return(leaf);
     else
	  leaf = next_specified_in_directory(leaf);
     return (leaf);
}


void print_paths_from(leaf)
filerec *leaf;
{
     /*
      * This is static to prevent multiple copies of it when calling
      * recursively.
      */
     static char buf[MAXPATHLEN];

     printf("%s\n", get_leaf_path(leaf, buf));
     if (leaf->dirs)
	  print_paths_from(leaf->dirs);
     if (leaf->files)
	  print_paths_from(leaf->files);
     if (leaf->next)
	  print_paths_from(leaf->next);
}


void print_specified_paths_from(leaf)
filerec *leaf;
{
     /*
      * This is static to prevent multiple copies of it when calling
      * recursively.
      */
     static char buf[MAXPATHLEN];

     if (leaf->specified)
	  printf("%s\n", get_leaf_path(leaf, buf));
     if (leaf->dirs)
	  print_specified_paths_from(leaf->dirs);
     if (leaf->files)
	  print_specified_paths_from(leaf->files);
     if (leaf->next)
	  print_specified_paths_from(leaf->next);
}
     

int add_file_to_parent(parent, name, specified, last)
filerec *parent, **last;
char *name;
Boolean specified;
{
     filerec *files;

     *last = files = (filerec *) NULL;
     files = parent->files;
     while (files) {
	  if (! strcmp(files->name, name))
	       break;
	  *last = files;
	  files = files->next;
     }
     if (files) {
	  files->specified = (files->specified || specified);
	  *last = files;
	  return 0;
     }
     if (*last) {
	  (*last)->next = (filerec *) Malloc((unsigned) sizeof(filerec));
	  if (! (*last)->next) {
	       set_error(errno);
	       error("Malloc");
	       return error_code;
	  }
	  *(*last)->next = default_file;
	  (*last)->next->previous = *last;
	  (*last)->next->parent = parent;
	  (*last) = (*last)->next;
     }
     else {
	  parent->files = (filerec *) Malloc(sizeof(filerec));
	  if (! parent->files) {
	       set_error(errno);
	       error("Malloc");
	       return error_code;
	  }
	  *parent->files = default_file;
	  parent->files->parent = parent;
	  parent->files->previous = (filerec *) NULL;
	  *last = parent->files;
     }
     (void) strcpy((*last)->name, name);
     (*last)->specified = specified;
     return 0;
}





int add_directory_to_parent(parent, name, specified, last)
filerec *parent, **last;
char *name;
Boolean specified;
{
     filerec *directories;

     *last = (filerec *) NULL;
     directories = parent->dirs;
     while (directories) {
	  if (! strcmp(directories->name, name))
	       break;
	  (*last) = directories;
	  directories = directories->next;
     }
     if (directories) {
	  directories->specified = (directories->specified || specified);
	  *last = directories;
	  return 0;
     }
     if (*last) {
	  (*last)->next = (filerec *) Malloc(sizeof(filerec));
	  if (! (*last)->next) {
	       set_error(errno);
	       error("Malloc");
	       return error_code;
	  }
	  *(*last)->next = default_directory;
	  (*last)->next->previous = *last;
	  (*last)->next->parent = parent;
	  (*last) = (*last)->next;
     }
     else {
	  parent->dirs = (filerec *) Malloc(sizeof(filerec));
	  if (! parent->dirs) {
	       set_error(errno);
	       error("Malloc");
	       return error_code;
	  }
	  *parent->dirs = default_directory;
	  parent->dirs->parent = parent;
	  parent->dirs->previous = (filerec *) NULL;
	  (*last) = parent->dirs;
     }
     (void) strcpy((*last)->name, name);
     (*last)->specified = specified;
     return 0;
}





void free_leaf(leaf)
filerec *leaf;
{
     leaf->freed = True;
     if (! (leaf->dirs || leaf->files)) {
	  if (leaf->previous)
	       leaf->previous->next = leaf->next;
	  if (leaf->next)
	       leaf->next->previous = leaf->previous;
	  if (leaf->parent) {
	       if ((leaf->specs.st_mode & S_IFMT) == S_IFDIR) {
		    if (leaf->parent->dirs == leaf) {
			 leaf->parent->dirs = leaf->next;
			 if (leaf->parent->freed)
			      free_leaf(leaf->parent);
		    }
	       }
	       else {
		    if (leaf->parent->files == leaf) {
			 leaf->parent->files = leaf->next;
			 if (leaf->parent->freed)
			      free_leaf(leaf->parent);
		    }
	       }
	       free((char *) leaf);
	  }
     }
}     



int find_child(directory, name, child)
filerec *directory, **child;
char *name;
{
     filerec *ptr;
     
     *child = (filerec *) NULL;
     if ((directory->specs.st_mode & S_IFMT) != S_IFDIR)
	  return DIR_NOT_DIRECTORY;
     ptr = directory->dirs;
     while (ptr)
	  if (strcmp(ptr->name, name))
	       ptr = ptr->next;
	  else
	       break;
     if (ptr) {
	  *child = ptr;
	  return DIR_MATCH;
     }
     ptr = directory->files;
     while (ptr)
	  if (strcmp(ptr->name, name))
	       ptr = ptr->next;
          else
	       break;
     if (ptr) {
	  *child = ptr;
	  return DIR_MATCH;
     }
     set_status(DIR_NO_MATCH);
     return DIR_NO_MATCH;
}





int change_path(old_path, new_path)
char *old_path, *new_path;
{
     char next_old[MAXNAMLEN], next_new[MAXNAMLEN];
     char rest_old[MAXPATHLEN], rest_new[MAXPATHLEN];
     int retval;
     filerec *current;
     
     if (*old_path == '/') {
	  current = &root_tree;
	  old_path++;
	  new_path++;
     }
     else if (! strncmp(old_path, "./", 2)) {
	  current = &cwd_tree;
	  old_path += 2;
	  new_path += 2;
     }
     else
	  current = &cwd_tree;
     
     (void) strcpy(next_old, firstpart(old_path, rest_old));
     (void) strcpy(next_new, firstpart(new_path, rest_new));
     while (*next_old && *next_new) {
	  retval = find_child(current, next_old, &current);
	  if (retval == DIR_MATCH) {
	       if (current) {
		    (void) strcpy(current->name, next_new);
		    current->specified = False;
	       }
	       else {
		    set_error(INTERNAL_ERROR);
		    error("change_path");
		    return error_code;
	       }
	  }
	  else {
	       error("change_path");
	       return retval;
	  }
	  
	  (void) strcpy(next_old, firstpart(rest_old, rest_old));
	  (void) strcpy(next_new, firstpart(rest_new, rest_new));
     }
     return 0;
}


int get_leaf_path(leaf, leaf_buf)
filerec *leaf;
char leaf_buf[]; /* RETURN */
{
     char *name_ptr;

     name_ptr = Malloc(1);
     if (! name_ptr) {
	  set_error(errno);
	  error("Malloc");
	  *leaf_buf = '\0';
	  return error_code;
     }
     *name_ptr = '\0';
     do {
	  name_ptr = realloc((char *) name_ptr, (unsigned)
			     (strlen(leaf->name) + strlen(name_ptr) + 2));
	  if (! name_ptr) {
	       set_error(errno);
	       *leaf_buf = '\0';
	       error("realloc");
	       return error_code;
	  }
	  (void) strcpy(leaf_buf, name_ptr);
	  *name_ptr = '\0';
	  if (leaf->parent) if (leaf->parent->parent)
	       (void) strcat(name_ptr, "/");
	  (void) strcat(name_ptr, leaf->name);
	  (void) strcat(name_ptr, leaf_buf);
	  leaf = leaf->parent;
     } while (leaf);
     (void) strcpy(leaf_buf, name_ptr);
     return 0;
}





int accumulate_names(leaf, in_strings, num)
filerec *leaf;
char ***in_strings;
int *num;
{
     char **strings;
     int retval;
     
     strings = *in_strings;
     if (leaf->specified) {
	  /*
	   * This array is static so that only one copy of it is allocated,
	   * rather than one copy on the stack for each recursive
	   * invocation of accumulate_names.
	   */
	  static char newname[MAXPATHLEN];

	  *num += 1;
	  strings = (char **) realloc((char *) strings, (unsigned)
				      (sizeof(char *) * (*num)));
	  if (! strings) {
	       set_error(errno);
	       error("realloc");
	       return error_code;
	  }
	  if (retval = get_leaf_path(leaf, newname)) {
	       error("get_leaf_path");
	       return retval;
	  }
	  (void) convert_to_user_name(newname, newname);
	  strings[*num - 1] = Malloc((unsigned) (strlen(newname) + 1));
	  if (! strings[*num - 1]) {
	       set_error(errno);
	       error("Malloc");
	       return error_code;
	  }
	  (void) strcpy(strings[*num - 1], newname);
     }
     if (leaf->files) if (retval = accumulate_names(leaf->files, &strings,
						    num)) {
	  error("accumulate_names");
	  return retval;
     }
     if (leaf->dirs) if (retval = accumulate_names(leaf->dirs, &strings,
						   num)) {
	  error("accumulate_names");
	  return retval;
     }
     if (leaf->next) if (retval = accumulate_names(leaf->next, &strings,
						   num)) {
	  error("accumulate_names");
	  return retval;
     }

     *in_strings = strings;
     return 0;
}
