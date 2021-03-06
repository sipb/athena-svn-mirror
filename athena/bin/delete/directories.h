/*
 * $Id: directories.h,v 1.16 1999-01-22 23:08:57 ghudson Exp $
 * 
 * This file is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

#ifndef __DELETE_DIRECTORIES_H__
#define __DELETE_DIRECTORIES_H__

#include "mit-copying.h"
#include <dirent.h>

typedef short Boolean;
#define True			(Boolean) 1
#define False			(Boolean) 0

char *bytes_to_friendly(off_t);

#define specs_to_space(x)	((x).st_size)
#define space_to_friendly(x)	bytes_to_friendly((x))
#define specs_to_friendly(x)	space_to_friendly((x).st_size)

#define FOLLOW_LINKS		1
#define DONT_FOLLOW_LINKS	0

#define DIR_MATCH		1
#define DIR_NO_MATCH		0

typedef struct mystat {
     dev_t st_dev;
     ino_t st_ino;
     unsigned short st_mode;
     off_t st_size;
     time_t st_ctim;
     long st_blocks;
} mystat;

     
typedef struct filrec {
     char name[MAXNAMLEN];
     struct filrec *previous;
     struct filrec *parent;
     struct filrec *dirs;
     struct filrec *files;
     struct filrec *next;
     Boolean specified;
     Boolean freed;
     struct mystat specs;
} filerec;



int add_directory_to_parent();
int add_file_to_parent();
int add_path_to_tree();
int find_child();
int change_path();
filerec *first_in_directory();
filerec *first_specified_in_directory();
filerec *get_cwd_tree();
int initialize_tree(void);
filerec *get_root_tree();
filerec *next_directory();
filerec *next_in_directory();
filerec *next_leaf();
filerec *next_specified_directory();
filerec *next_specified_in_directory();
filerec *next_specified_leaf();

int get_leaf_path(filerec *leaf, char leaf_buf[]);
int accumulate_names();

void free_leaf();

#endif /* ! __DELETE_DIRECTORIES_H__ */
