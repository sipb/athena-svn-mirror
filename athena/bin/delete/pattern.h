/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.h,v $
 * $Author: jik $
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.h,v 1.2 1989-01-27 02:58:39 jik Exp $
 * 
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

char **add_str();
char **find_contents();
char **find_deleted_contents();
char **find_deleted_contents_recurs();
char **find_matches();
char **find_deleted_matches();
char **find_recurses();
char **find_deleted_recurses();

char *parse_pattern();
