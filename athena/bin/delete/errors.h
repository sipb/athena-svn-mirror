/*
 * $Id: errors.h,v 1.4 1999-01-22 23:08:58 ghudson Exp $
 * 
 * This file is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */
#include "mit-copying.h"

extern char *whoami;
extern int error_reported;
extern int error_occurred;
extern int report_errors;
extern int error_code;

void error();

#define set_error(cd) {error_code = cd; error_reported = 0; error_occurred = 1;}
#define set_warning(cd) {error_code = cd; error_reported = 0;}
#define set_status(cd) {error_code = cd;}
