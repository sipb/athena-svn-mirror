/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/directories.h,v 1.10 1991-02-22 06:33:34 jik Exp $
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


#define size_to_k(x)		((x) / 1024 + (((x) % 1024) ? 1 : 0))

#define FOLLOW_LINKS		1
#define DONT_FOLLOW_LINKS	0

#define DIR_MATCH		1
#define DIR_NO_MATCH		0

typedef struct mystat {
     dev_t st_dev;
     ino_t st_ino;
     unsigned short st_mode;
     off_t st_size;
     time_t st_ctime;
#ifdef notdef
     /*
      * I've tried, unsuccessfully, to figure out exactly what this
      * field means and how I can use it.  Supposedly, it indicates
      * the number of blocks the file actually occupies, i.e. the size
      * of the file minus any holes in it there may be.  The question,
      * however, is this: what's a "block?"
      *
      * At first, I thought that a block is as big as f_bsize returned
      * by a statfs on the file.  But that doesn't prove to be the
      * case, because my home directory in AFS has f_bsize of 8192,
      * st_size of 8192, and st_blocks of 16 (!!), indicating that a
      * block size of 512 bytes is being used.  Where does that size
      * come from, and why isn't it consistent with the f_bsize
      * retrieved from statfs?
      *
      * Until someone can answer these questions for me enough that
      * I'm willing to trust the value in this field, I can't use it.
      * Besides that, it doesn't even exist in the POSIX stat
      * structure, so I'm not even sure it's worth trying to use it.
      *
      * Here's another dilemma: When I do a statfs on my home
      * directory in AFS, it tells me that the f_bsize is 8192.  If
      * that's the case, then when I create a one-character file in my
      * home directory, my quota usage should go up by 8k.  But it
      * doesn't, it goes up by just 1k.  Which means that the f_bsize
      * I'm getting from statfs has nothing to do with the minimum
      * block size of the filesystem.  So what *does* it have to do
      * with?
      */
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
