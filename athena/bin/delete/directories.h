/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.h,v 1.4 1989-01-27 03:13:29 jik Exp $
 * 
 * This file is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

typedef short Boolean;
#define True			(Boolean) 1
#define False			(Boolean) 0

typedef enum {
     FtFile,
     FtDirectory,
     FtUnknown,
} filetype;
	  
typedef struct filrec {
     char name[MAXNAMLEN];
     filetype ftype;
     struct filrec *previous;
     struct filrec *parent;
     struct filrec *dirs;
     struct filrec *files;
     struct filrec *next;
     Boolean specified;
     Boolean freed;
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

filetype find_file_type();

char *get_leaf_path();
