/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.c,v $
 * $Author: jik $
 * 
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if !defined(lint) && !defined(SABER)
     static char rcsid_directories_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.c,v 1.12 1989-03-27 12:06:16 jik Exp $";
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <strings.h>
#include "directories.h"
#include "util.h"
#include "mit-copyright.h"

extern char *malloc(), *realloc();
extern char *whoami;

static filerec root_tree;
static filerec cwd_tree;
static char *error_buf;

 /* these are not static because external routines need to be able to */
 /* access them. */
long current_time;


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


initialize_tree()
{
     int status;
     
     root_tree = default_root;
     cwd_tree = default_cwd;

     current_time = time(0);
     error_buf = (char *) malloc(MAXPATHLEN + strlen(whoami) + 5);
     if (! error_buf) {
	  return(1);
     }
     status = get_specs("", &cwd_tree.specs, FOLLOW_LINKS);
     if (! status)
	  status = get_specs("/", &root_tree.specs, FOLLOW_LINKS);
     return(status);
}


filerec *add_path_to_tree(path)
char *path;
{
     filerec *parent, *leaf;
     char next_name[MAXNAMLEN];
     char lpath[MAXPATHLEN], built_path[MAXPATHLEN], *ptr;
     struct stat specs;

     if (get_specs(path, &specs, DONT_FOLLOW_LINKS))
	  return((filerec *) NULL);
     ptr = strcpy(lpath, path); /* we don't want to damage the user's string */
     if (*ptr == '/') {
	  parent = &root_tree;
	  ptr++;
	  strcpy(built_path, "/");
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
     
     strcpy(next_name, firstpart(ptr, ptr));
     while (*ptr) {
	  strcat(built_path, next_name);
	  parent = add_directory_to_parent(parent, next_name, False);
	  if (! parent)
	       return ((filerec *) NULL);
	  strcpy(next_name, firstpart(ptr, ptr));
	  if (get_specs(built_path, &parent->specs, FOLLOW_LINKS))
	       return((filerec *) NULL);
	  strcat(built_path, "/");
     }
     if ((specs.st_mode & S_IFMT) == S_IFDIR)
	  leaf = add_directory_to_parent(parent, next_name, True);
     else
	  leaf = add_file_to_parent(parent, next_name, True);
          
     if (! leaf)
	  return ((filerec *) NULL);
     leaf->specs = specs;

     return(leaf);
}




get_specs(path, specs, follow)
char *path;
struct stat *specs;
int follow;
{
     int status;
     
     if (strlen(path)) if ((path[strlen(path) - 1] == '/') &&
			   (strlen(path) != 1))
	  path[strlen(path) - 1] = '\0';
     if (follow == FOLLOW_LINKS)
	  status = stat(path, specs);
     else 
	  status = lstat(path, specs);

     if (status)
	  return(1);
     else
	  return(0);
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


print_paths_from(leaf)
filerec *leaf;
{
     char buf[MAXPATHLEN];

     printf("%s\n", get_leaf_path(leaf, buf));
     if (leaf->dirs)
	  print_paths_from(leaf->dirs);
     if (leaf->files)
	  print_paths_from(leaf->files);
     if (leaf->next)
	  print_paths_from(leaf->next);
     return(0);
}


print_specified_paths_from(leaf)
filerec *leaf;
{
     char buf[MAXPATHLEN];

     if (leaf->specified)
	  printf("%s\n", get_leaf_path(leaf, buf));
     if (leaf->dirs)
	  print_specified_paths_from(leaf->dirs);
     if (leaf->files)
	  print_specified_paths_from(leaf->files);
     if (leaf->next)
	  print_specified_paths_from(leaf->next);
     return(0);
}
     

filerec *add_file_to_parent(parent, name, specified)
filerec *parent;
char *name;
Boolean specified;
{
     filerec *files, *last = (filerec *) NULL;

     files = parent->files;
     while (files) {
	  if (! strcmp(files->name, name))
	       break;
	  last = files;
	  files = files->next;
     }
     if (files) {
	  files->specified = (files->specified || specified);
	  return(files);
     }
     if (last) {
	  last->next = (filerec *) malloc(sizeof(filerec));
	  if (! last->next)
	       return((filerec *) NULL);
	  *last->next = default_file;
	  last->next->previous = last;
	  last->next->parent = parent;
	  last = last->next;
     }
     else {
	  parent->files = (filerec *) malloc(sizeof(filerec));
	  if (! parent->files)
	       return((filerec *) NULL);
	  *parent->files = default_file;
	  parent->files->parent = parent;
	  parent->files->previous = (filerec *) NULL;
	  last = parent->files;
     }
     strcpy(last->name, name);
     last->specified = specified;
     return(last);
}





filerec *add_directory_to_parent(parent, name, specified)
filerec *parent;
char *name;
Boolean specified;
{
     filerec *directories, *last = (filerec *) NULL;

     directories = parent->dirs;
     while (directories) {
	  if (! strcmp(directories->name, name))
	       break;
	  last = directories;
	  directories = directories->next;
     }
     if (directories) {
	  directories->specified = (directories->specified || specified);
	  return(directories);
     }
     if (last) {
	  last->next = (filerec *) malloc(sizeof(filerec));
	  if (! last->next)
	       return((filerec *) NULL);
	  *last->next = default_directory;
	  last->next->previous = last;
	  last->next->parent = parent;
	  last = last->next;
     }
     else {
	  parent->dirs = (filerec *) malloc(sizeof(filerec));
	  if (! parent->dirs)
	       return((filerec *) NULL);
	  *parent->dirs = default_directory;
	  parent->dirs->parent = parent;
	  parent->dirs->previous = (filerec *) NULL;
	  last = parent->dirs;
     }
     strcpy(last->name, name);
     last->specified = specified;
     return(last);
}





free_leaf(leaf)
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
	       free(leaf);
	  }
     }
     return(0);
}     



filerec *find_child(directory, name)
filerec *directory;
char *name;
{
     filerec *ptr;

     if ((directory->specs.st_mode & S_IFMT) != S_IFDIR)
	  return ((filerec *) NULL);
     ptr = directory->dirs;
     while (ptr)
	  if (strcmp(ptr->name, name))
	       ptr = ptr->next;
	  else
	       break;
     if (ptr)
	  return (ptr);
     ptr = directory->files;
     while (ptr)
	  if (strcmp(ptr->name, name))
	       ptr = ptr->next;
          else
	       break;
     if (ptr)
	  return (ptr);
     return ((filerec *) NULL);
}





change_path(old_path, new_path)
char *old_path, *new_path;
{
     char next_old[MAXNAMLEN], next_new[MAXNAMLEN];
     char rest_old[MAXPATHLEN], rest_new[MAXPATHLEN];

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

     strcpy(next_old, firstpart(old_path, rest_old));
     strcpy(next_new, firstpart(new_path, rest_new));
     while (*next_old && *next_new) {
	  current = find_child(current, next_old);
	  if (current)
	       strcpy(current->name, next_new);
	  else
	       return(1);
	  strcpy(next_old, firstpart(rest_old, rest_old));
	  strcpy(next_new, firstpart(rest_new, rest_new));
     }
     if (! (*next_old || *next_new))
	  return(0);
     else
	  return(1);
}


char *get_leaf_path(leaf, leaf_buf)
filerec *leaf;
char leaf_buf[]; /* RETURN */
{
     char *name_ptr;

     name_ptr = malloc(1);
     if (! name_ptr) {
	  *leaf_buf = '\0';
	  return(leaf_buf);
     }
     *name_ptr = '\0';
     do {
	  name_ptr = realloc(name_ptr, strlen(leaf->name) + 
			     strlen(name_ptr) + 2);
	  if (! name_ptr) {
	       *leaf_buf = '\0';
	       return(leaf_buf);
	  }
	  strcpy(leaf_buf, name_ptr);
	  *name_ptr = '\0';
	  if (leaf->parent) if (leaf->parent->parent)
	       strcat(name_ptr, "/");
	  strcat(name_ptr, leaf->name);
	  strcat(name_ptr, leaf_buf);
	  leaf = leaf->parent;
     } while (leaf);
     strcpy(leaf_buf, name_ptr);
     return(leaf_buf);
}





char **accumulate_names(leaf, strings, num)
filerec *leaf;
char **strings;
int *num;
{
     char newname[MAXPATHLEN];
     
     if (leaf->specified) {
	  *num += 1;
	  strings = (char **) realloc(strings, sizeof(char *) * (*num));
	  if (! strings) {
	       perror(sprintf(error_buf, "%s: accumulate_names", whoami));
	       exit(1);
	  }
	  convert_to_user_name(get_leaf_path(leaf, newname), newname);
	  strings[*num - 1] = malloc(strlen(newname) + 1);
	  if (! strings[*num - 1]) {
	       perror(sprintf(error_buf, "%s: accumulate_names", whoami));
	       exit(1);
	  }
	  strcpy(strings[*num - 1], newname);
     }
     if (leaf->files)
	  strings = accumulate_names(leaf->files, strings, num);
     if (leaf->dirs)
	  strings = accumulate_names(leaf->dirs, strings, num);
     if (leaf->next)
	  strings = accumulate_names(leaf->next, strings, num);

     return(strings);
}
