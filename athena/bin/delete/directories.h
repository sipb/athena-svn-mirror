/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.h,v $
 * $Author: miki $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.h,v 1.13 1993-05-20 12:55:20 miki Exp $
 * 
 * This file is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

#include "mit-copying.h"

typedef short Boolean;
#define True			(Boolean) 1
#define False			(Boolean) 0


#ifdef USE_BLOCKS
#define specs_to_space(x)	((x).st_blocks)
#define space_to_k(x)		((x) / 2 + (((x) % 2) ? 1 : 0))
#define specs_to_k(x)		space_to_k((x).st_blocks)
#else
#define specs_to_space(x)	((x).st_size)
#define space_to_k(x)		((x) / 1024 + (((x) % 1024) ? 1 : 0))
#define specs_to_k(x)		space_to_k((x).st_size)
#endif

#define FOLLOW_LINKS		1
#define DONT_FOLLOW_LINKS	0

#define DIR_MATCH		1
#define DIR_NO_MATCH		0

typedef struct mystat {
     dev_t st_dev;
     ino_t st_ino;
     unsigned short st_mode;
     off_t st_size;
#ifndef SOLARIS
     time_t st_ctime;
#else
     time_t st_time;
#endif
#ifdef USE_BLOCKS
     long st_blocks;
#endif
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
filerec *first_in_directory();
filerec *first_specified_in_directory();
filerec *get_cwd_tree();
filerec *get_root_tree();
filerec *next_directory();
filerec *next_in_directory();
filerec *next_leaf();
filerec *next_specified_directory();
filerec *next_specified_in_directory();
filerec *next_specified_leaf();

int get_leaf_path();
int accumulate_names();

void free_leaf();
