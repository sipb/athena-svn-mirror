/*
 * Things that needed to be added:
 *
 * 1) Check to see if the cwd is /, and if so, use only one root node.
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <strings.h>
#include <errno.h>
#include "directories.h"

extern char *malloc(), *realloc();
extern char *whoami;
extern int errno;

static filerec root_tree;
static filerec cwd_tree;
static char *error_buf;

filerec *add_path_to_tree(), *add_file_to_parent(), *add_directory_to_parent();
char *get_leaf_path(), *lastpart(), *firstpart();
filetype find_file_type();



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
     root_tree = default_root;
     cwd_tree = default_cwd;
     
     error_buf = (char *) malloc(MAXPATHLEN + strlen(whoami) + 5);
     if (! error_buf) {
	  perror(whoami);
	  return(1);
     }
     return(0);
}


filerec *add_path_to_tree(path, ftype, requested)
char *path;
filetype ftype;
Boolean requested;
{
     filerec *parent, *leaf;
     char next_name[MAXNAMLEN];
     char lpath[MAXPATHLEN], *ptr;

     ptr = strcpy(lpath, path); /* we don't want to damage the user's string */
     if (ftype == FtUnknown)
	  ftype = find_file_type(ptr);
     
     if (*ptr == '/') {
	  parent = &root_tree;
	  ptr++;
     }
     else if (! strncmp(ptr, "./", 2)) {
	  parent = &cwd_tree;
	  ptr += 2;
     }
     else
	  parent = &cwd_tree;

     strcpy(next_name, firstpart(ptr, ptr));
     while (*ptr) {
	  parent = add_directory_to_parent(parent, next_name, requested,
					   False);
	  if (! parent) {
	       perror(whoami);
	       return ((filerec *) NULL);
	  }
	  strcpy(next_name, firstpart(ptr, ptr));
     }
     if (ftype == FtFile)
	  leaf = add_file_to_parent(parent, next_name, requested, True);
     else
	  leaf = add_directory_to_parent(parent, next_name, requested,
					 True);

     if (! leaf) {
	  perror(whoami);
	  return ((filerec *) NULL);
     }
     return(leaf);
}




filetype find_file_type(path)
char *path;
{
     struct stat buf;
     
     if ((path[strlen(path) - 1] == '/') && (strlen(path) != 1))
	  path[strlen(path) - 1] = '\0';
     if (lstat(path, &buf))
	  return (FtUnknown);
     else if (buf.st_mode & S_IFDIR)
	  return (FtDirectory);
     else
	  return (FtFile);
}



filerec *next_leaf(leaf)
filerec *leaf;
{
     if (leaf->dirs)
	  return(leaf->dirs);
     else if (leaf->files)
	  return(leaf->files);
     else if (leaf->next)
	  return(leaf->next);
     else
	  return((filerec *) NULL);
}


filerec *next_specified_leaf(leaf)
filerec *leaf;
{
     while (leaf = next_leaf_down(leaf))
	  if (leaf->specified)
	       return(leaf);
     return((filerec *) NULL);
}


filerec *next_directory(leaf)
filerec *leaf;
{
     if (leaf->ftype == FtFile)
	  leaf = leaf->parent;
     if (leaf)
	  return(leaf->next);
     else
	  return((filerec *) NULL);
}


filerec *next_specified_directory(leaf)
filerec *leaf;
{
     if (leaf->ftype == FtFile)
	  leaf = leaf->parent;
     if (! leaf)
	  return ((filerec *) NULL);
     while (leaf->next)
	  if (leaf->next->specified)
	       return(leaf->next);
	  else
	       leaf = leaf->next;
     return ((char *) NULL);
}



filerec *next_in_directory(leaf)
filerec *leaf;
{
     if (leaf->next)
	  return(leaf->next);
     else if ((leaf->ftype == FtDirectory) && leaf->parent)
	  return(leaf->parent->files);
     else
	  return ((filerec *) NULL);
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
     if (leaf->ftype != FtDirectory)
	  return ((filerec *) NULL);
     else if (leaf->dirs)
	  return (leaf->dirs);
     else if (leaf->files)
	  return (leaf->files);
     else
	  return ((filerec *) NULL);
}


filerec *first_specified_in_directory(leaf);
filerec *leaf;
{
     leaf = first_in_directory(leaf);
     
     if (ptr->specified)
	  return(leaf);
     else
	  ptr = next_specified_in_directory(leaf);
     return ((filerec *) NULL);
}


print_paths_from(leaf)
filerec *leaf;
{
     char buf[MAXPATHLEN];
     
     printf("%s\n", get_leaf_path(leaf, buf));
     if (leaf->dirs)
	  print_paths_down_from(leaf->dirs);
     if (leaf->files)
	  print_paths_down_from(leaf->files);
     if (leaf->next)
	  print_paths_down_from(leaf->next);
     return(0);
}


print_specified_paths_from(leaf)
filerec *leaf;
{
     char buf[MAXPATHLEN];
     
     if (leaf->specified)
	  printf("%s\n", get_leaf_path(leaf, buf));
     if (leaf->dirs)
	  print_specified_paths_down_from(leaf->dirs);
     if (leaf->files)
	  print_specified_paths_down_from(leaf->files);
     if (leaf->next)
	  print_specified_paths_down_from(leaf->next);
     return(0);
}
     

filerec *add_file_to_parent(parent, name, requested, specified)
filerec *parent;
char *name;
Boolean requested, specified;
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
	  files->requested = requested;
	  files->specified = (files->specified || specified);
	  return(files);
     }
     if (last) {
	  last->next = (filerec *) malloc(sizeof(filerec));
	  if (! last->next) {
	       perror(whoami);
	       return((filerec *) NULL);
	  }
	  *last->next = default_file;
	  last->next->previous = last;
	  last->next->parent = parent;
	  last = last->next;
     }
     else {
	  parent->files = (filerec *) malloc(sizeof(filerec));
	  if (! parent->files) {
	       perror(whoami);
	       return((filerec *) NULL);
	  }
	  *parent->files = default_file;
	  parent->files->parent = parent;
	  parent->files->previous = (filerec *) NULL;
	  last = parent->files;
     }
     strcpy(last->name, name);
     last->requested = requested;
     last->specified = specified;
     return(last);
}





filerec *add_directory_to_parent(parent, name, requested, specified)
filerec *parent;
char *name;
Boolean requested, specified;
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
	  directories->requested = requested;
	  directories->specified = (directories->specified || specified);
	  return(directories);
     }
     if (last) {
	  last->next = (filerec *) malloc(sizeof(filerec));
	  if (! last->next) {
	       perror(whoami);
	       return((filerec *) NULL);
	  }
	  *last->next = default_directory;
	  last->next->previous = last;
	  last->next->parent = parent;
	  last = last->next;
     }
     else {
	  parent->dirs = (filerec *) malloc(sizeof(filerec));
	  if (! parent->dirs) {
	       perror(whoami);
	       return((filerec *) NULL);
	  }
	  *parent->dirs = default_directory;
	  parent->dirs->parent = parent;
	  parent->dirs->previous = (filerec *) NULL;
	  last = parent->dirs;
     }
     strcpy(last->name, name);
     last->requested = requested;
     last->requested = specified;
     return(last);
}





free_leaf(leaf)
filerec *leaf;
{
     leaf->freed = True;
     if (leaf->previous) {
	  leaf->previous->next = leaf->next;
	  if (leaf->previous->freed)
	       free_leaf(leaf->previous);
     }
     if (leaf->ftype = FtDirectory) {
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
     if (leaf->next)
	  leaf->next->previous = leaf->previous;
     
     if ((leaf->parent) && /* we don't want to call this on a tree root! */
	 (! (leaf->ftype = FtDirectory) && (leaf->files || leaf->dirs)))
#ifdef DEBUG
	  printf("Freeing leaf %s\n", leaf->name);
#endif
	  free(leaf);
     return(0);
}     



filerec *find_child(directory, name)
filerec *directory;
char *name;
{
     filerec *ptr;

     if (directory->ftype != FtDirectory)
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
	  if (current) {
#ifdef DEBUG
	       printf("changing %s to %s\n", current->name, next_new);
#endif	       
	       strcpy(current->name, next_new);
	  }
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
     *name_ptr = '\0';
     do {
	  name_ptr = realloc(name_ptr, strlen(leaf->name) + 
			     strlen(name_ptr) + 2);
	  strcpy(leaf_buf, name_ptr);
	  *name_ptr = '\0';
	  if (leaf->parent)
	       strcat(name_ptr, "/");
	  strcat(name_ptr, leaf->name);
	  strcat(name_ptr, leaf_buf);
	  leaf = leaf->parent;
     } while (leaf);
     strcpy(leaf_buf, name_ptr);
     return(leaf_buf);
}

     

char *lastpart(filename)
char *filename;
{
     char *part;

     part = rindex(filename, '/');

     if (! part)
	  part = filename;
     else if (part == filename)
	  part++;
     else if (part - filename + 1 == strlen(filename)) {
	  part = rindex(--part, '/');
	  if (! part)
	       part = filename;
	  else
	       part++;
     }
     else
	  part++;

     return(part);
}
     

char *firstpart(filename, rest)
char *filename;
char *rest; /* RETURN */
{
     char *part;
     static char buf[MAXPATHLEN];

     strcpy(buf, filename);
     part = index(buf, '/');
     if (! part) {
	  *rest = '\0';
	  return(buf);
     }
     strcpy(rest, part + 1);
     *part = '\0';
     return(buf);
}
