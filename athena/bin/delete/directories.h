/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.h,v 1.6 1989-03-27 12:06:24 jik Exp $
 * 
 * This file is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"

typedef short Boolean;
#define True			(Boolean) 1
#define False			(Boolean) 0


#define blk_to_k(x)		(x * DEV_BSIZE / 1024)

#define FOLLOW_LINKS		1
#define DONT_FOLLOW_LINKS	0
     
typedef struct filrec {
     char name[MAXNAMLEN];
     struct filrec *previous;
     struct filrec *parent;
     struct filrec *dirs;
     struct filrec *files;
     struct filrec *next;
     Boolean specified;
     Boolean freed;
     struct stat specs;
} filerec;



filerec *add_directory_to_parent();
filerec *add_file_to_parent();
filerec *add_path_to_tree();
filerec *find_child();
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

char *get_leaf_path();
char **accumulate_names();
