/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions for operating-system routines that don't
 * appear to be defined elsewhere, at least in BSD.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/os.h,v $
 *	$Id: os.h,v 1.12 1996-11-15 03:29:56 ghudson Exp $
 *	$Author: ghudson $
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/resource.h>
